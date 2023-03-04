#ifndef P2PCHAT_PEERSINFO_H
#define P2PCHAT_PEERSINFO_H

#include <string>

struct Peer{
    std::string IPv4;
    short port{};
    std::string name;

    Peer() = default;
    Peer(const std::string &name, const std::string &iPv4, short port) : name(name), IPv4(iPv4), port(port) {}

    bool operator<(const Peer &p) const{
        return (IPv4 == p.IPv4) ?
               (port < p.port) :
               (IPv4 < p.IPv4);
    }
};

#endif
