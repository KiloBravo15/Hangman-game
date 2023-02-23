#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <cstdint>

#define MTYPE_MASK 0x3f
#define MTYPE_JOIN 0x01     // Joining the game
#define MTYPE_LEAVE 0x02    // Leaving the game
#define MTYPE_GUESS 0x11    // Guessing a letter
#define MTYPE_SCORE 0x12    // Changing score
#define MTYPE_HANG 0x13     // Hanging a player
#define MTYPE_WIN 0x21      // Player has won
#define MTYPE_LOSE 0x22     // Player has lost

#define MDIR_MASK 0xc0
#define MDIR_REQUEST 0x00   // Requesting something
#define MDIR_RESPONSE 0x40  // Responding to a request
#define MDIR_NOTIFY 0x80    // Notify everyone

using namespace std;

class Message {
    public:
    uint8_t type;
    u32string content;

    Message(uint8_t type, u32string content);
};

#endif
