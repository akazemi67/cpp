#ifndef P2PCHAT_MESSAGETYPES_H
#define P2PCHAT_MESSAGETYPES_H

#include <string>
#include <vector>

enum class MessageType{
    PING = 0,
    TEXT = 1,
    IMAGE = 2
};

struct MessageHeader {
    MessageType type;
    int length;
};

struct Message {
    MessageHeader header;
};

struct PingMessage : Message {
    std::string payload;
};

struct TextMessage : Message {
    std::string text;
};

struct ImageMessage : Message {
    std::vector<uint8_t> image;
};

#endif
