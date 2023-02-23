#ifndef HANGMAN_PLAYER_HPP
#define HANGMAN_PLAYER_HPP

class HangmanPlayer;

#include "hangman_server.hpp"
#include "message.hpp"
#include "../client.hpp"
#include <string>

using namespace std;

class HangmanPlayer {
    protected:
    HangmanServer* server;
    u32string name;
    bool isAlive;
    Client* networkClient;

public:
    HangmanPlayer(HangmanServer* server);
    const u32string& getName() const;
    void setName(u32string name);
    bool checkIsAlive();
    void attachNetworkClient(Client* client);

    void onJoin(const HangmanPlayer* player);
    void onLeave(const HangmanPlayer* player);

    void onLose();
    void onWin();

    void onPhraseReveal(const u32string& phrase);
    void onHang(const HangmanPlayer& player, int fails);

    void onScoreChange(const HangmanPlayer& player, int newScore);

    void makeGuess(char32_t guess);

    void parseMessage(char* message, size_t length);
    void parseMessage(const Message& message);

protected:
    void sendToClient(Message& message);
    void log(const string& s) const;
};

#endif