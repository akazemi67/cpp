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

#define MSG_HEADER_PADD 1000000000
#define MSG_HEADER_LEN  8

NetworkCom::NetworkCom(int _localPort, std::shared_ptr<UiCallbacks> _uiCallbacks):
        localPort(_localPort), uiCallbacks(_uiCallbacks) {
    isListening = false;
    listenerThread = std::make_unique<std::thread>([this](){this->startListening();});
}

NetworkCom::~NetworkCom(){
    if(isListening) {
        close(localSocket);
        listenerThread->join();
    }
}

void NetworkCom::addNewSocket(const std::string &peerName, int clientSocket) {
    std::unique_lock<std::shared_mutex> lock(openSocketsMutex);
    openSockets[peerName] = clientSocket;
}

void NetworkCom::removeSocket(const std::string &peerName) {
    std::unique_lock<std::shared_mutex> lock(openSocketsMutex);
    auto it = openSockets.find(peerName);
    if(it != openSockets.end())
        openSockets.erase(it);
}

int NetworkCom::getClientSocket(const std::string &peerName) {
    std::shared_lock<std::shared_mutex> lock(openSocketsMutex);
    auto it = openSockets.find(peerName);
    if(it != openSockets.end())
        return it->second;
    return -1;
}

void NetworkCom::startListening() {
    printf("Creating local socket on: %d\n",localPort);

    localSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (localSocket < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr{};
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(localPort);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(localSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind() failed");
        exit(EXIT_FAILURE);
    }

    if (listen(localSocket, SOMAXCONN) < 0) {
        perror("listen() failed");
        exit(EXIT_FAILURE);
    }

    isListening = true;
    printf("Waiting for connections on: %d\n", localPort);
    while(true) {
        struct sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sockfd = accept(localSocket, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_sockfd < 0) {
            perror("accept() failed");
            continue;
        }
        new std::thread([this, client_sockfd](){ this->handleConnections(client_sockfd);});
    }
}

/*template<typename T>
std::unique_ptr<T> NetworkCom::deserializeMessage(const std::unique_ptr<uint8_t> &buffer,
                                                                       size_t size) {
    google::protobuf::io::ArrayInputStream input_stream(buffer.get(), size);
    std::unique_ptr<T> msg(new T);
    google::protobuf::util::ParseDelimitedFromZeroCopyStream(msg.get(),
                                                             &input_stream, nullptr);
    return msg;
}*/

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
            std::copy(imgMsg->image.begin(), imgMsg->image.end(),
                      imgMsgProto->mutable_image()->mutable_data());

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
    printf("Serialize. headerLen: %d, totalLen: %d\n", headerSize, bufferLen);

    std::copy(bodyBuffer->data(), bodyBuffer->data()+bufferLen, messageBuffer->data()+headerSize);
    return {std::move(messageBuffer), bufferLen};
}

MessageHeader NetworkCom::getMessageHeader(std::unique_ptr<uint8_t> headerBuff) {
    //auto header = deserializeMessage<messages::MessageHeader>(headerBuff,
    //                                               MSG_HEADER_LEN);
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

void NetworkCom::deserializeHandleMessage(int clientSocket, std::unique_ptr<uint8_t> messageBuff,
                                          const MessageHeader &header) {
    ssize_t bufferSize = header.length - MSG_HEADER_LEN;
    //TODO: do it in a more general way!
    switch (header.type) {
        case MessageType::AUTH: {
            //auto authMsgProto = deserializeMessage<messages::AuthMessage>(messageBuff,bufferSize);
            std::unique_ptr<messages::AuthMessage> authMsgProto(new messages::AuthMessage());
            authMsgProto->ParseFromArray(messageBuff.get(), bufferSize);
            std::unique_ptr<AuthMessage> authMessage(new AuthMessage(authMsgProto->name()));
            addNewSocket(authMessage->name, clientSocket);
            uiCallbacks->newAuthMessage(std::move(authMessage));
            break;
        }
        case MessageType::TEXT: {
            //auto textMsgProto = deserializeMessage<messages::TextMessage>(messageBuff,bufferSize);
            std::unique_ptr<messages::TextMessage> textMsgProto(new messages::TextMessage());
            textMsgProto->ParseFromArray(messageBuff.get(), bufferSize);
            std::unique_ptr<TextMessage> textMessage(new TextMessage(textMsgProto->text()));
            uiCallbacks->newTextMessage(std::move(textMessage));
            break;
        }
        case MessageType::IMAGE: {
            //auto imgMsgProto = deserializeMessage<messages::ImageMessage>(messageBuff,bufferSize);
            std::unique_ptr<messages::ImageMessage> imgMsgProto(new messages::ImageMessage());
            imgMsgProto->ParseFromArray(messageBuff.get(), bufferSize);
            std::unique_ptr<ImageMessage> imageMessage(new ImageMessage(imgMsgProto->image()));
            uiCallbacks->newImageMessage(std::move(imageMessage));
            break;
        }
    }
}

void NetworkCom::handleConnections(int clientSocket) {
    //TODO: handle connection close
    while(true) {
        size_t headerSize = MSG_HEADER_LEN;
        std::unique_ptr<uint8_t> headerBuff(new uint8_t[headerSize]);
        ssize_t bytesReceived = recv(clientSocket, headerBuff.get(), headerSize, 0);
        if(bytesReceived<=0) {
            printf("Error in getting message header!\n");
            return;
        }
        printf("Received a message!\n");
        MessageHeader msgHeader = getMessageHeader(std::move(headerBuff));
        size_t msgBodySize = msgHeader.length - headerSize;
        std::unique_ptr<uint8_t> messageBuffer(new uint8_t[msgBodySize]);
        size_t totalBytesReceived = 0;

        while (totalBytesReceived < msgBodySize) {
            bytesReceived = recv(clientSocket, messageBuffer.get() + totalBytesReceived,
                                 msgBodySize - totalBytesReceived, 0);
            if (bytesReceived <= 0) {
                printf("Error getting message body!");
                return;
            }
            totalBytesReceived += bytesReceived;
        }

        deserializeHandleMessage(clientSocket, std::move(messageBuffer), msgHeader);
    }
}

void NetworkCom::sendMessage(const Peer& peer, const Message& message) {
    printf("Getting socket data...\n");
    int clientSocket = getClientSocket(peer.name);
    if(clientSocket<0){
        printf("Invalid peer name!\n");
        return;
    }
    printf("Serializing data...\n");
    auto [messageBytes, len] = serializeMessage(message);
    printf("Sending data...\n");
    send(clientSocket, messageBytes->data(), len, 0);
}

void NetworkCom::connectPeer(const Peer& peer) {
    printf("Connecting peer!\n");
    struct hostent* host = gethostbyname(peer.IPv4.c_str());
    sockaddr_in sockAddr{};
    std::memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sockAddr.sin_port = htons(peer.port);

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    int status = connect(clientSocket, (sockaddr*) &sockAddr, sizeof(sockAddr));
    if(status < 0){
        printf("Error connecting to socket!");
        return;
    }

    printf("Peer connected!\n");
    addNewSocket(peer.name, clientSocket);
}

void NetworkCom::disconnect(const Peer& peer) {

}

