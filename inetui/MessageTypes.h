#ifndef P2PCHAT_MESSAGETYPES_H
#define P2PCHAT_MESSAGETYPES_H

#include <string>
#include <vector>
#include <google/protobuf/repeated_field.h>

enum class MessageType{
    AUTH = 0,
    TEXT = 1,
    IMAGE = 2,
    INVALID = 100
};

struct MessageHeader {
    MessageType type;
    int length{};

    MessageHeader() : length(0), type(MessageType::INVALID) {}
    explicit MessageHeader(MessageType type) : type(type) {}
};

struct Message {
    MessageHeader header;

    Message() = default;
    explicit Message(MessageType type) : header(type) {}
    virtual ~Message() = default;
};

struct AuthMessage : Message {
    std::string name;
    AuthMessage() = default;
    explicit AuthMessage(const std::string &_name) : Message(MessageType::AUTH), name(_name) {}
};

struct TextMessage : Message {
    std::string text;
    TextMessage() = default;
    explicit TextMessage(const std::string &_text) : Message(MessageType::TEXT), text(_text) {}
};

struct ImageMessage : Message {
    std::vector<uint8_t> image;
    ImageMessage() = default;
    explicit ImageMessage(const google::protobuf::RepeatedField<int> &_image) :
                            Message(MessageType::IMAGE), image(_image.begin(), _image.end()) {}
    ImageMessage(const std::vector<uint8_t> &image) : Message(MessageType::IMAGE), image(image) {}
};

#endif