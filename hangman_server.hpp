#ifndef HANGMAN_SERVER_HPP
#define HANGMAN_SERVER_HPP

class HangmanServer;

#include "hangman_player.hpp"
#include <list> //create and manage lists
#include <map> //create and manage associative containers where values are associated with unique keys
#include <set> //create and manage sets, which are containers that store unique elements in sorted order
#include <string> //reate and manage strings, which are sequences of characters
#include <ctime> //provides functions for working with time and dates

using namespace std;

class HangmanServer {
    protected:
    static HangmanServer* instance; //static pointer to a HangmanServer object that will be used to implement the Singleton design pattern. This pattern ensures that there is only one instance of the HangmanServer class throughout the lifetime of the program
    time_t newRoundTime; //time of the next round [in seconds]

    list<HangmanPlayer*> players; //list of pointers to HangmanPlayer objects that represent the players in the game
    set<u32string> playerNames; //set of u32string objects that stores the unique names of the players.
    map<u32string, int> scores; //map that associates player names with their scores
    map<u32string, int> fails; //map that associates player names with the number of failed guesses

    u32string currentWord; //u32string objects that store the current word
    u32string currentWordObscured; //u32string objects that store obscured version of the current word

    HangmanServer(); //HangmanServer() constructor is declared but not defined, which means it is only accessible from within the class and its derived classes
    
    public:
    static HangmanServer* getInstance(); //static function that returns a pointer to the HangmanServer instance

    bool joinPlayer(HangmanPlayer* player); //adds a new player to the game
    void leavePlayer(HangmanPlayer* player); //removes a player from the game

    void makeGuess(HangmanPlayer* player, char32_t guess); //allows a player to make a guess

    const map<u32string, int>* getScores(); //return pointers to maps that associate player names with their scores
    const map<u32string, int>* getFails(); //return pointers to maps that associate player names with their failed guesses

    u32string getCurrentPhrase(); //returns the current obscured phrase

    void startNewRoundIfNeeded(); //starts a new round if needed

    protected:
    u32string generatePhrase(); //generates a new random phrase for the game
    void obscurePhrase(); //obscures the current phrase

    int countAlivePlayers() const; //returns the number of alive players
    void log(const string& s) const; //utility function used for logging
};

 #endif //marks the end of the HangmanServer class. The #endif preprocessor directive is used to terminate the conditional compilation started by the #ifndef directive at the beginning of the header file