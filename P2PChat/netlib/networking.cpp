#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/delimited_message_util.h>

#include <memory>
#include "MessageTypes.h"
#include "UiNetlibInterfaces.h"
#include "messages.pb.h"
#include "logging.h"

#define MSG_HEADER_PADD 1000000000
#define MSG_HEADER_LEN  8

NetworkCom::NetworkCom(int _localPort, UiCallbacks *_uiCallbacks):
        localPort(_localPort), uiCallbacks(_uiCallbacks) {
    isListeningLocally = false;
    listenerThread = std::make_unique<std::thread>([this](){this->startListening();});
}

NetworkCom::~NetworkCom(){
}

void NetworkCom::addNewSocket(const std::string &peerName, int clientSocket) {
    std::unique_lock<std::shared_mutex> lock(openSocketsMutex);
    openSockets[peerName] = clientSocket;
    openSocketsToName[clientSocket] = peerName;
}

void NetworkCom::removeSocket(const std::string &peerName) {
    std::unique_lock<std::shared_mutex> lock(openSocketsMutex);
    auto it = openSockets.find(peerName);
    int clientSocket =-1;
    if(it != openSockets.end()) {
        clientSocket = it->second;
        openSockets.erase(it);
    }
    auto iter = openSocketsToName.find(clientSocket);
    if(iter!=openSocketsToName.end()){
        openSocketsToName.erase(iter);
    }
}

int NetworkCom::getClientSocket(const std::string &peerName) {
    std::shared_lock<std::shared_mutex> lock(openSocketsMutex);
    auto it = openSockets.find(peerName);
    if(it != openSockets.end())
        return it->second;
    return -1;
}

std::string NetworkCom::getSocketName(int clientSocket) {
    std::shared_lock<std::shared_mutex> lock(openSocketsMutex);
    auto it = openSocketsToName.find(clientSocket);
    if(it != openSocketsToName.end())
        return it->second;
    return "[INVALID]";
}

void NetworkCom::stopListening() {
    std::unique_lock<std::shared_mutex> lock(openSocketsMutex);
    if(!isListeningLocally) {
        getLogger()->info("Application is not listening on any ports. Exiting.");
        return;
    }

    getLogger()->info("Disconnecting from all peers and shutting down networking.");
    for (auto &[clientSocket, name]: openSocketsToName) {
        close(clientSocket);
    }
    close(localSocket);
    isListeningLocally = false;
    getLogger()->info("Networking stopped!");
}

void NetworkCom::startListening() {
    getLogger()->info("Creating local socket on: {}",localPort);

    localSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (localSocket < 0) {
        getLogger()->error("socket creation on {} failed. errno: {}", localPort, errno);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr{};
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(localPort);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(localSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        getLogger()->error("Failed to bind for socker on {}. errno: {}", localPort, errno);
        exit(EXIT_FAILURE);
    }

    if (listen(localSocket, SOMAXCONN) < 0) {
        getLogger()->error("Failed to listen on {}. errno: {}", localPort, errno);
        exit(EXIT_FAILURE);
    }

    getLogger()->info("Waiting for connections on: {}", localPort);
    uiCallbacks->bindSucceeded();
    isListeningLocally = true;

    while(true) {
        struct sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sockfd = accept(localSocket, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_sockfd < 0) {
            getLogger()->error("accept() failed on {}. errno: {}", localPort, errno);
            break;
        }
        getLogger()->info("Received a new connection. Waiting for AUTH message.");
        new std::thread([this, client_sockfd](){ this->handleConnections(client_sockfd);});
    }
}

std::tuple< std::unique_ptr<std::vector<uint8_t>>, int> NetworkCom::serializeMessage(const Message &message) {
    std::unique_ptr<messages::MessageHeader> headerProto(new messages::MessageHeader);
    std::unique_ptr<std::vector<uint8_t>> bodyBuffer(new std::vector<uint8_t>());

    int bodySize = 0;
    switch (message.header.type) {
        case MessageType::AUTH: {
            headerProto->set_type(messages::MessageType::AUTH);
            auto authMsg = dynamic_cast<const AuthMessage*>(&message);
            std::unique_ptr<messages::AuthMessage> authMsgProto(new messages::AuthMessage());
            authMsgProto->set_name(authMsg->name);

            bodySize = authMsgProto->ByteSizeLong();
            bodyBuffer->reserve(bodySize);
            authMsgProto->SerializeToArray(bodyBuffer->data(), bodySize);
            break;
        }
        case MessageType::TEXT: {
            headerProto->set_type(messages::MessageType::TEXT);
            auto textMsg = dynamic_cast<const TextMessage*>(&message);
            std::unique_ptr<messages::TextMessage> textMsgProto(new messages::TextMessage());
            textMsgProto->set_text(textMsg->text);

            bodySize = textMsgProto->ByteSizeLong();
            bodyBuffer->reserve(bodySize);
            textMsgProto->SerializeToArray(bodyBuffer->data(), bodySize);
            break;
        }
        case MessageType::IMAGE: {
            headerProto->set_type(messages::MessageType::IMAGE);
            auto imgMsg = dynamic_cast<const ImageMessage*>(&message);
            std::unique_ptr<messages::ImageMessage> imgMsgProto(new messages::ImageMessage());
            *imgMsgProto->mutable_image() = {imgMsg->image->begin(), imgMsg->image->end()};

            bodySize = imgMsgProto->ByteSizeLong();
            bodyBuffer->reserve(bodySize);
            imgMsgProto->SerializeToArray(bodyBuffer->data(), bodySize);
            break;
        }
        default:
            //TODO: exception
            return {nullptr, 0};
    }

    int headerSize = MSG_HEADER_LEN;
    int bufferLen = headerSize + bodySize;
    headerProto->set_length(MSG_HEADER_PADD + bufferLen );

    std::unique_ptr<std::vector<uint8_t>> messageBuffer(new std::vector<uint8_t>(bufferLen));
    headerProto->SerializeToArray(messageBuffer->data(), headerSize);
    getLogger()->info("Serializing message with body size: {}", bodySize);

    std::copy(bodyBuffer->data(), bodyBuffer->data()+bodySize, messageBuffer->data()+headerSize);
    return {std::move(messageBuffer), bufferLen};
}

MessageHeader NetworkCom::getMessageHeader(std::unique_ptr<uint8_t> headerBuff) {
    std::unique_ptr<messages::MessageHeader> header(new messages::MessageHeader());
    header->ParseFromArray(headerBuff.get(), MSG_HEADER_LEN);
    MessageHeader msgHeader{};
    switch (header->type()) {
        case messages::MessageType::AUTH:
            msgHeader.type = MessageType::AUTH;
            break;
        case messages::MessageType::TEXT:
            msgHeader.type = MessageType::TEXT;
            break;
        case messages::MessageType::IMAGE:
            msgHeader.type = MessageType::IMAGE;
            break;
    }
    msgHeader.length = header->length() - MSG_HEADER_PADD;
    return msgHeader;
}

void NetworkCom::deserializeHandleMessage(int clientSocket,
                                          std::unique_ptr<uint8_t> messageBuff,
                                          const MessageHeader &header) {
    ssize_t bufferSize = header.length - MSG_HEADER_LEN;
    std::string socketName = getSocketName(clientSocket);
    //TODO: do it in a more general way!
    switch (header.type) {
        case MessageType::AUTH: {
            std::unique_ptr<messages::AuthMessage> authMsgProto(new messages::AuthMessage());
            authMsgProto->ParseFromArray(messageBuff.get(), bufferSize);
            std::unique_ptr<AuthMessage> authMessage(new AuthMessage(authMsgProto->name()));

            socketName = authMessage->name;
            addNewSocket(socketName, clientSocket);
            getLogger()->info("Received AUTH message from {}", socketName);
            uiCallbacks->newAuthMessage(socketName, std::move(authMessage));
            break;
        }
        case MessageType::TEXT: {
            std::unique_ptr<messages::TextMessage> textMsgProto(new messages::TextMessage());
            textMsgProto->ParseFromArray(messageBuff.get(), bufferSize);
            std::unique_ptr<TextMessage> textMessage(new TextMessage(textMsgProto->text()));

            getLogger()->info("Received TEXT message from {}", socketName);
            uiCallbacks->newTextMessage(socketName, std::move(textMessage));
            break;
        }
        case MessageType::IMAGE: {
            std::unique_ptr<messages::ImageMessage> imgMsgProto(new messages::ImageMessage());
            imgMsgProto->ParseFromArray(messageBuff.get(), bufferSize);
            std::unique_ptr<ImageMessage> imageMessage(new ImageMessage(imgMsgProto->image()));

            getLogger()->info("Received IMAGE message from {}", socketName);
            uiCallbacks->newImageMessage(socketName, std::move(imageMessage));
            break;
        }
    }
}

void NetworkCom::handleConnections(int clientSocket) {
    bool waitAndReceive = true;
    while(waitAndReceive) {
        size_t headerSize = MSG_HEADER_LEN;
        std::unique_ptr<uint8_t> headerBuff(new uint8_t[headerSize]);
        ssize_t bytesReceived = recv(clientSocket, headerBuff.get(), headerSize, 0);
        if(bytesReceived == 0)
            break;
        if(bytesReceived <= 0) {
            getLogger()->error("Error in receiving message header. errno: {}", errno);
            break;
        }

        MessageHeader msgHeader = getMessageHeader(std::move(headerBuff));
        size_t msgBodySize = msgHeader.length - headerSize;
        getLogger()->info("Received a message with body size: {}", msgBodySize);
        std::unique_ptr<uint8_t> messageBuffer(new uint8_t[msgBodySize]);
        size_t totalBytesReceived = 0;

        while (totalBytesReceived < msgBodySize) {
            bytesReceived = recv(clientSocket, messageBuffer.get() + totalBytesReceived,
                                 msgBodySize - totalBytesReceived, 0);
            if (bytesReceived <= 0) {
                getLogger()->error("Error getting message body! errno: {}", errno);
                waitAndReceive = false;
                break;
            }
            totalBytesReceived += bytesReceived;
        }

        deserializeHandleMessage(clientSocket, std::move(messageBuffer), msgHeader);
    }

    auto peerName = getSocketName(clientSocket);
    getLogger()->warn("Network connection to {} ended.", peerName);
    removeSocket(peerName);
    uiCallbacks->peerDisconnected(peerName);
}

void NetworkCom::sendMessage(const std::string & peerName, const Message& message) {
    int clientSocket = getClientSocket(peerName);
    if(clientSocket<0){
        getLogger()->warn("Invalid peer with name: {}", peerName);
        return;
    }

    auto [messageBytes, len] = serializeMessage(message);
    getLogger()->info("Sending {} bytes of data to peer: {}", len, peerName);
    send(clientSocket, messageBytes->data(), len, 0);
}

bool NetworkCom::connectPeer(const Peer& peer) {
    struct hostent* host = gethostbyname(peer.IPv4.c_str());
    sockaddr_in sockAddr{};
    std::memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sockAddr.sin_port = htons(peer.port);

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    int status = connect(clientSocket, (sockaddr*) &sockAddr, sizeof(sockAddr));
    if(status < 0){
        getLogger()->warn("Cannot connect to {} on port {}", peer.IPv4, peer.port);
        return false;
    }

    getLogger()->info("Successfully connected to peer {} on {}:{}", peer.name, peer.IPv4, peer.port);
    addNewSocket(peer.name, clientSocket);

    //Waiting for messages from connected peer
    new std::thread([this, clientSocket](){ this->handleConnections(clientSocket);});
    return true;
}

std::unique_ptr<NetOps> createNetworking(int port, UiCallbacks *callbacks) {
    return std::unique_ptr<NetOps>(reinterpret_cast<NetOps*>(new NetworkCom(port, callbacks)));
}
