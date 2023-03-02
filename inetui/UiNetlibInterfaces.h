#ifndef P2PCHAT_UINETLIBINTERFACES_H
#define P2PCHAT_UINETLIBINTERFACES_H

#include "MessageTypes.h"
#include <memory>

struct Peer{
    std::string IPv4;
    short port;
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
    virtual void connect(Peer)=0;
    virtual void disconnect(Peer)=0;
};

#endif
