#ifndef P2PCHAT_UINETLIBINTERFACES_H
#define P2PCHAT_UINETLIBINTERFACES_H

#include "MessageTypes.h"
#include "PeersInfo.h"

#include <memory>
#include <thread>
#include <shared_mutex>
#include <map>

class UiCallbacks {
public:
    virtual void newAuthMessage(std::unique_ptr<AuthMessage>)=0;
    virtual void newTextMessage(std::unique_ptr<TextMessage>)=0;
    virtual void newImageMessage(std::unique_ptr<ImageMessage>)=0;
    virtual void peerDisconnect(const Peer&)=0;
    virtual ~UiCallbacks()=default;
};

class NetOps {
public:
    virtual void sendMessage(const Peer&, const Message &)=0;
    virtual void connectPeer(const Peer&)=0;
    virtual void disconnect(const Peer&)=0;
    virtual ~NetOps() = default;
};

class NetworkCom : NetOps{
private:
    bool isListening;
    int localPort;
    int localSocket;
    std::shared_ptr<UiCallbacks> uiCallbacks;
    std::unique_ptr<std::thread> listenerThread;
    std::map<std::string, int> openSockets; //unique_name -> socket_fd
    std::shared_mutex openSocketsMutex;

    void addNewSocket(const std::string& peerName, int clientSocket);
    void removeSocket(const std::string& peerName);
    int getClientSocket(const std::string& peerName);

    //template<typename T> std::unique_ptr<T> deserializeMessage(const std::unique_ptr<uint8_t>& buffer, size_t size);
    MessageHeader getMessageHeader(std::unique_ptr<uint8_t> headerBuff);
    void deserializeHandleMessage(int clientSocket, std::unique_ptr<uint8_t> messageBuff, const MessageHeader &);
    std::tuple<std::unique_ptr<std::vector<uint8_t>>, int> serializeMessage(const Message& message);

    void startListening();
    void handleConnections(int clientSocket);
public:
    NetworkCom(int _localPort, std::shared_ptr<UiCallbacks> _uiCallbacks);
    ~NetworkCom() override;

    void sendMessage(const Peer& peer, const Message& message) override;
    void connectPeer(const Peer& peer) override;
    void disconnect(const Peer& peer) override;
};

//TODO: Use factory instead of this method.
std::unique_ptr<NetOps> createNetworking(int port, std::shared_ptr<UiCallbacks> callbacks){
    return std::unique_ptr<NetOps>(reinterpret_cast<NetOps*>(new NetworkCom(port, callbacks)));
}

#endif
