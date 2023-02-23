#ifndef HANGMAN_SERVER_HPP
#define HANGMAN_SERVER_HPP

class HangmanServer;

#include "hangman_player.hpp"
#include <list>
#include <map>
#include <set>
#include <string>
#include <ctime>

using namespace std;

class HangmanServer {
    protected:
    static HangmanServer* instance;
    time_t newRoundTime;

    list<HangmanPlayer*> players;
    set<u32string> playerNames;
    map<u32string, int> scores;
    map<u32string, int> fails;

    u32string currentWord;
    u32string currentWordObscured;

    HangmanServer();
    
    public:
    static HangmanServer* getInstance();

    bool joinPlayer(HangmanPlayer* player);
    void leavePlayer(HangmanPlayer* player);

    void makeGuess(HangmanPlayer* player, char32_t guess);

    const map<u32string, int>* getScores();
    const map<u32string, int>* getFails();

    u32string getCurrentPhrase();

    void startNewRoundIfNeeded();

    protected:
    u32string generatePhrase();
    void obscurePhrase();

    int countAlivePlayers() const;
    void log(const string& s) const;
};

#endif