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
    virtual void bindSucceeded()=0;
    virtual void newAuthMessage(std::string &peerName, std::unique_ptr<AuthMessage> authMsg)=0;
    virtual void newTextMessage(std::string &peerName, std::unique_ptr<TextMessage> txtMsg)=0;
    virtual void newImageMessage(std::string &peerName, std::unique_ptr<ImageMessage> imgMsg)=0;
    virtual void peerDisconnect(const std::string &peerName)=0;
    virtual ~UiCallbacks()=default;
};

class NetOps {
public:
    virtual void sendMessage(const std::string &, const Message &)=0;
    virtual bool connectPeer(const Peer&)=0;
    virtual void disconnect(const Peer&)=0;
    virtual ~NetOps() = default;
};

class NetworkCom : NetOps{
private:
    bool isListeningLocally;
    int localPort;
    int localSocket;
    UiCallbacks *uiCallbacks;
    std::unique_ptr<std::thread> listenerThread;
    std::map<std::string, int> openSockets; //unique_name -> socket_fd
    std::map<int, std::string> openSocketsToName;
    std::shared_mutex openSocketsMutex;

    void addNewSocket(const std::string& peerName, int clientSocket);
    void removeSocket(const std::string& peerName);
    int getClientSocket(const std::string& peerName);
    std::string getSocketName(int clientSocket);

    //template<typename T> std::unique_ptr<T> deserializeMessage(const std::unique_ptr<uint8_t>& buffer, size_t size);
    MessageHeader getMessageHeader(std::unique_ptr<uint8_t> headerBuff);
    void deserializeHandleMessage(int clientSocket, std::unique_ptr<uint8_t> messageBuff, const MessageHeader &);
    std::tuple<std::unique_ptr<std::vector<uint8_t>>, int> serializeMessage(const Message& message);

    void startListening();
    void handleConnections(int clientSocket);
public:
    NetworkCom(int _localPort, UiCallbacks *_uiCallbacks);
    ~NetworkCom() override;

    void sendMessage(const std::string &peer, const Message& message) override;
    bool connectPeer(const Peer& peer) override;
    void disconnect(const Peer& peer) override;
};

//TODO: Use factory instead of this method.
std::unique_ptr<NetOps> createNetworking(int port, UiCallbacks *callbacks);

#endif
