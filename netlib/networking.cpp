#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <map>
#include "MessageTypes.h"
#include "UiNetlibInterfaces.h"
#include "message.pb.h"

NetworkCom::NetworkCom(int _localPort, std::shared_ptr<UiCallbacks> _uiCallbacks):
        localPort(_localPort), uiCallbacks(_uiCallbacks) {
    isListening = false;
    listenerThread = std::unique_ptr<std::thread>(new std::thread([this](){this->startListening();}));
}

NetworkCom::~NetworkCom(){
    if(isListening)
        close(localSocket);
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

void NetworkCom::handleConnections(int clientSocket) {
    char msg[100];
    printf("Received a new connection.");

    recv(clientSocket, &msg, sizeof(msg), 0);
    printf("Recevied %s\n Sending Back!\n", msg);
    send(clientSocket, &msg, strlen(msg), 0);
}

void NetworkCom::sendMessage(Peer peer, Message message) {

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

void NetworkCom::connectPeer(Peer peer) {
    connectPeer(peer.IPv4, peer.port);
}

void NetworkCom::disconnect(Peer peer) {

}
