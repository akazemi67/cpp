#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/delimited_message_util.h>
#include "MessageTypes.h"
#include "UiNetlibInterfaces.h"
#include "messages.pb.h"

NetworkCom::NetworkCom(int _localPort, std::shared_ptr<UiCallbacks> _uiCallbacks):
        localPort(_localPort), uiCallbacks(_uiCallbacks) {
    isListening = false;
    listenerThread = std::unique_ptr<std::thread>(new std::thread([this](){this->startListening();}));
}

NetworkCom::~NetworkCom(){
    if(isListening)
        close(localSocket);
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

template<typename T>
std::unique_ptr<T> NetworkCom::deserializeMessage(const std::unique_ptr<uint8_t> &buffer,
                                                                       size_t size) {
    google::protobuf::io::ArrayInputStream input_stream(buffer.get(), size);
    std::unique_ptr<T> msg(new T);
    google::protobuf::util::ParseDelimitedFromZeroCopyStream(msg.get(),
                                                             &input_stream, nullptr);
    return msg;
}

MessageHeader NetworkCom::getMessageHeader(std::unique_ptr<uint8_t> headerBuff) {
    auto header = deserializeMessage<messages::MessageHeader>(headerBuff,
                                                    sizeof(messages::MessageHeader));
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
    msgHeader.length = header->length();
    return msgHeader;
}

void NetworkCom::deserializeHandleMessage(int clientSocket, std::unique_ptr<uint8_t> messageBuff,
                                          const MessageHeader &header) {
    ssize_t bufferSize = header.length - sizeof(messages::MessageHeader);
    switch (header.type) {
        case MessageType::AUTH: {
            auto authMsgProto = deserializeMessage<messages::AuthMessage>(messageBuff,
                                                                          bufferSize);
            std::unique_ptr<AuthMessage> authMessage(new AuthMessage(authMsgProto->name()));
            addNewSocket(authMessage->name, clientSocket);
            uiCallbacks->newAuthMessage(std::move(authMessage));
            break;
        }
        case MessageType::TEXT: {
            auto textMsgProto = deserializeMessage<messages::TextMessage>(messageBuff,
                                                                          bufferSize);
            std::unique_ptr<TextMessage> textMessage(new TextMessage(textMsgProto->text()));
            uiCallbacks->newTextMessage(std::move(textMessage));
            break;
        }
        case MessageType::IMAGE: {
            auto imgMsgProto = deserializeMessage<messages::ImageMessage>(messageBuff,
                                                                          bufferSize);
            std::unique_ptr<ImageMessage> imageMessage(new ImageMessage(imgMsgProto->image()));
            uiCallbacks->newImageMessage(std::move(imageMessage));
            break;
        }
    }
}

void NetworkCom::handleConnections(int clientSocket) {
    //TODO: handle connection close
    while(true) {
        size_t headerSize = sizeof(messages::MessageHeader);
        std::unique_ptr<uint8_t> headerBuff(new uint8_t[headerSize]);
        ssize_t bytesReceived = recv(clientSocket, headerBuff.get(), headerSize, 0);
        if(bytesReceived<=0) {
            printf("Error in getting message header!\n");
            return;
        }

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

}

void NetworkCom::connectPeer(const std::string &address, int remotePort) {
    struct hostent* host = gethostbyname(address.c_str());
    sockaddr_in sockAddr{};
    std::memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sockAddr.sin_port = htons(remotePort);

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    int status = connect(clientSocket, (sockaddr*) &sockAddr, sizeof(sockAddr));
    if(status < 0){
        printf("Error connecting to socket!");
        return;
    }

    char msg[100] = "HelloFrom Network!";
    send(clientSocket, &msg, strlen(msg), 0);
    recv(clientSocket, &msg, sizeof(msg), 0);
    printf("REC from SRV: %s\n", msg);
}

void NetworkCom::connectPeer(const Peer& peer) {
    connectPeer(peer.IPv4, peer.port);
}

void NetworkCom::disconnect(const Peer& peer) {

}
