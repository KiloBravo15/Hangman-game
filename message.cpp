#include "message.hpp"

/**
 * Creates and fills the message object
 * @param type The message type
 * @param content The message content
 */
Message::Message(uint8_t type, u32string content) {
    this->content = content;
    this->type = type;
}