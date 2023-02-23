#include "hangman_player.hpp"

#include "../unicode.hpp"

#include <iostream>
using namespace std;

/**
 * Creates a new hangman player
 * @param server The game server
 */
HangmanPlayer::HangmanPlayer(HangmanServer* server) {
    this->server = server;
    this->name = U"Unnamed player";
    this->isAlive = true;
    this->networkClient = nullptr;
}

/**
 * Returns the player name
 */
const u32string& HangmanPlayer::getName() const {
    return this->name;
}

/**
 * Sets the player name
 * @param name The player name
 */
void HangmanPlayer::setName(u32string name) {
    this->name = name;
}

/**
 * Checks if the player is alive
 */
bool HangmanPlayer::checkIsAlive() {
    return this->isAlive;
}

/**
 * Attaches the network client to the player
 * @param client The client to attach
 */
void HangmanPlayer::attachNetworkClient(Client* client) {
    // If the network client has just disconnected, leave the game
    if(client == nullptr && this->networkClient != nullptr){
        this->server->leavePlayer(this);
    }

    this->networkClient = client;
}

/**
 * Handles the join event
 * @param player The player that has joined
 */
void HangmanPlayer::onJoin(const HangmanPlayer* player) {
    log(utf32ToUtf8(player->getName()) + " has joined the game.");
    Message response(MDIR_NOTIFY | MTYPE_JOIN, U"{\"name\": \"" + player->getName() + U"\"}");
    this->sendToClient(response);
}

/**
 * Handles the leave event
 * @param player The player that has left
 */
void HangmanPlayer::onLeave(const HangmanPlayer* player) {
    log(utf32ToUtf8(player->getName()) + " has left the game.");
    Message response(MDIR_NOTIFY | MTYPE_LEAVE, U"{\"name\": \"" + player->getName() + U"\"}");
    this->sendToClient(response);
}

/**
 * Fired when this player has lost
 */
void HangmanPlayer::onLose() {
    this->isAlive = false;
    log("Lost.");
    Message response(MDIR_NOTIFY | MTYPE_LOSE, U"{}");
    this->sendToClient(response);
}

/**
 * Fired when this player has won
 */
void HangmanPlayer::onWin() {
    this->isAlive = false;
    log("Won.");
    Message response(MDIR_NOTIFY | MTYPE_WIN, U"{}");
    this->sendToClient(response);
}

/**
 * Fired when the phrase has been revealed
 * @param phrase The new phrase
 */
void HangmanPlayer::onPhraseReveal(const u32string& phrase) {
    log("Phrase revealed: " + utf32ToUtf8(phrase));
    Message response(MDIR_NOTIFY | MTYPE_GUESS, U"{\"phrase\": \"" + phrase + U"\"}");
    this->sendToClient(response);
}

/**
 * Fired when the player is hung
 * @param player The player who's hung
 * @param fails The number of body parts hung
 */
void HangmanPlayer::onHang(const HangmanPlayer& player, int fails) {
    log(utf32ToUtf8(player.getName()) + " has been hanged (" + to_string(fails) + " fails).");
    Message response(MDIR_NOTIFY | MTYPE_HANG, U"{\"player\": \"" + player.getName() + U"\", \"fails\": " + intToUtf32(fails) + U"}");
    this->sendToClient(response);
}

/**
 * Fired when the player score changes
 * @param player The player whose score changed
 * @param newScore The new player score
 */
void HangmanPlayer::onScoreChange(const HangmanPlayer& player, int newScore) {
    log(utf32ToUtf8(player.getName()) + " has " + to_string(newScore) + " points.");
    Message response(MDIR_NOTIFY | MTYPE_SCORE, U"{\"player\": \"" + player.getName() + U"\", \"score\": " + intToUtf32(newScore) + U"}");
    this->sendToClient(response);
}

/**
 * Tries to make a guess
 * @param guess The guess
 */
void HangmanPlayer::makeGuess(char32_t guess) {
    // Dead players cannot make guesses
    if (!this->isAlive) return;

    if (this->server != nullptr) {
        this->server->makeGuess(this, guess);
    }
}

/**
 * Parses the incoming message
 * @param data The received bytes
 * @param length Number of bytes received
 */
void HangmanPlayer::parseMessage(char* data, size_t length){
    uint8_t messageType = data[0];
    string messageBody(data + 1,  length - 1);
    u32string messageBodyUtf32 = utf8ToUtf32(messageBody);

    Message message(messageType, messageBodyUtf32);
    this->parseMessage(message);
}

/**
 * Parses the incoming message
 * @param message The message
 */
void HangmanPlayer::parseMessage(const Message& message) {
    // Only requests are valid to come in to the player
    // Other messages are sent by the player
    log("Message arrived of type: " + to_string((int)message.type));
    if((message.type & MDIR_MASK) != MDIR_REQUEST){
        log("Stumbled upon a message that's not a request.");
        return;
    }

    switch(message.type & MTYPE_MASK) {
        case MTYPE_JOIN: {
            // Client asked to join the game
            log("Trying to join...");
            this->setName(message.content);
            bool success = this->server->joinPlayer(this);
            if(!success){
                this->setName(U"");
            }
            u32string content = success ? U"{\"success\":1}" : U"{\"success\":0}";
            Message response(MDIR_RESPONSE | MTYPE_JOIN, content);
            this->sendToClient(response);
            break;
        }
        case MTYPE_LEAVE: {
            // Client has left
            log("Leaving the game...");
            this->server->leavePlayer(this);
            Message response(MDIR_RESPONSE | MTYPE_JOIN, U"{}");
            this->sendToClient(response);
            break;
        }
        case MTYPE_GUESS: {
            // Client made a guess or asks for the phrase
            if(message.content.length() == 0){
                this->onPhraseReveal(this->server->getCurrentPhrase());
            }else{
                log("Guessing...");
                this->makeGuess(message.content[0]);
            }
            break;
        }
        case MTYPE_SCORE: {
            // Client requested the scoreboard
            log("Requesting the scoreboard...");
            const map<u32string, int>* scores = this->server->getScores();
            const map<u32string, int>* fails = this->server->getFails();

            u32string responseContent = U"[";
            bool isFirst = true;
            for (auto& score : *scores) {
                if (!isFirst) {
                    responseContent += U", ";
                }
                responseContent += U"{\"name\": \"" + score.first + U"\","
                        + U"\"score\": " + intToUtf32(score.second) + U","
                        + U"\"fails\": " + intToUtf32(fails->at(score.first)) + U"}";
                isFirst = false;
            }
            responseContent += U"]";

            Message response(MDIR_RESPONSE | MTYPE_SCORE, responseContent);
            this->sendToClient(response);
            break;
        }
        default: {
            log("Unknown message type.");
        }
    }
}

/**
 * Sends the message to the client
 * @param message The message to send
 */
void HangmanPlayer::sendToClient(Message& message){
    log("Sending response.");
    if(this->networkClient == nullptr) return;

    string messageBodyUtf8 = utf32ToUtf8(message.content);
    string messageContent = string(1, (char)message.type) + messageBodyUtf8;

    clientWrite(this->networkClient, messageContent.c_str(), messageContent.length());
}

void HangmanPlayer::log(const string& s) const{
    cout << "\x1b[1;32m[PLAYER: " << utf32ToUtf8(this->getName()) << "]\x1b[0m " << s << endl;
}
