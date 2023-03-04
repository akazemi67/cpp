#ifndef P2PCHAT_PEERSINFO_H
#define P2PCHAT_PEERSINFO_H

#include <string>

struct Peer{
    std::string IPv4;
    short port;
    std::string name;
    bool operator<(const Peer &p) const{
        return (IPv4 == p.IPv4) ?
               (port < p.port) :
               (IPv4 < p.IPv4);
    }
};

#endif
