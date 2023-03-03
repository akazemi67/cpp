#ifndef P2PCHAT_UINETLIBINTERFACES_H
#define P2PCHAT_UINETLIBINTERFACES_H

#include <memory>
#include <thread>
#include "MessageTypes.h"

struct Peer{
    std::string IPv4;
    short port;
    bool operator<(const Peer &p) const{
        return (IPv4 == p.IPv4) ?
               (port < p.port) :
               (IPv4 < p.IPv4);
    }
};

class UiCallbacks{
public:
    virtual void newPingMessage(std::unique_ptr<PingMessage>)=0;
    virtual void newTextMessage(std::unique_ptr<TextMessage>)=0;
    virtual void newImageMessage(std::unique_ptr<ImageMessage>)=0;
    virtual void peerDisconnect(Peer)=0;
};

class NetOps{
public:
    virtual void sendMessage(Peer, Message)=0;
    virtual void connectPeer(Peer)=0;
    virtual void disconnect(Peer)=0;
    virtual ~NetOps() = default;
};

class NetworkCom : NetOps{
private:
    bool isListening;
    int localPort;
    int localSocket;
    std::shared_ptr<UiCallbacks> uiCallbacks;
    std::unique_ptr<std::thread> listenerThread;

    void startListening();
    void handleConnections(int clientSocket);
    void connectPeer(const std::string &address, int remotePort);
public:
    NetworkCom(int _localPort, std::shared_ptr<UiCallbacks> _uiCallbacks);
    ~NetworkCom() override;

    void sendMessage(Peer peer, Message message) override;
    void connectPeer(Peer peer) override;
    void disconnect(Peer peer) override;
};

//TODO: Use factory instead of this method.
std::unique_ptr<NetOps> createNetworking(int port, std::shared_ptr<UiCallbacks> callbacks){
    return std::unique_ptr<NetOps>(reinterpret_cast<NetOps*>(new NetworkCom(port, callbacks)));
}

#endif
