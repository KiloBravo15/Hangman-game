#include "hangman_server.hpp"

#include "../unicode.hpp"
#include <iostream>
#include <cstdlib>

#define MAX_FAILS 6
#define TIME_MAX (time_t)0x7fffffff

HangmanServer* HangmanServer::instance = nullptr;

/**
 * Creates a new game server
 */
HangmanServer::HangmanServer() {
    srand(time(nullptr));
    this->players = list<HangmanPlayer*>();
    this->currentWord = this->generatePhrase();
    this->newRoundTime = TIME_MAX;
    this->obscurePhrase();
}

/**
 * Returns the game server instance
 */
HangmanServer* HangmanServer::getInstance() {
    if(HangmanServer::instance == nullptr) {
        HangmanServer::instance = new HangmanServer();
    }
    return HangmanServer::instance;
}

/**
 * Joins the player to the game
 * @param player The player to join
 */
bool HangmanServer::joinPlayer(HangmanPlayer* player) {
    // Player cannot have an empty name
    if(player->getName().length() == 0){
        return false;
    }
    // Check if the player's name is already taken
    if(this->playerNames.find(player->getName()) != this->playerNames.end()){
        return false;
    }

    // The joining player is not notified
    for(HangmanPlayer* p : this->players) {
        p->onJoin(player);
    }

    // Remember the player
    this->players.push_back(player);
    this->playerNames.insert(player->getName());
    this->scores[player->getName()] = 0;
    this->fails[player->getName()] = 0;
    this->log("There are " + to_string(this->countAlivePlayers()) + " alive players now.");
    return true;
}

/**
 * Removes the player from the game
 * @param player The player to remove
 */
void HangmanServer::leavePlayer(HangmanPlayer* player) {
    // Forget about the player
    this->players.remove(player);
    this->playerNames.erase(player->getName());
    this->scores.erase(player->getName());
    this->fails.erase(player->getName());
    this->log("There are " + to_string(this->countAlivePlayers()) + " alive players now.");

    // The leaving player is not notified
    for(HangmanPlayer* p : this->players) {
        p->onLeave(player);
    }
}

/**
 * Makes a guess and updates the players' scores
 * @param player The player who's guessing
 * @param guess The letter to guess
 */
void HangmanServer::makeGuess(HangmanPlayer* player, char32_t guess) {
    guess = toupper(guess);

    // Count the guessed letters and remaining ones
    int found = 0;
    int remaining = 0;
    for (size_t i = 0; i < this->currentWord.length(); i++) {
        if (this->currentWord[i] == guess && this->currentWordObscured[i] == U'_') {
            this->currentWordObscured[i] = guess;
            found++;
        }
        if (this->currentWordObscured[i] == U'_') {
            remaining++;
        }
    }

    // If the player hasn't guessed anything, add one fail
    if (found == 0) {
        int failCount = ++this->fails[player->getName()];
        for(HangmanPlayer* p : this->players){
            p->onHang(*player, failCount);
        }

        if (this->fails[player->getName()] >= MAX_FAILS) {
            player->onLose();

            if(this->countAlivePlayers() == 1) {
                // Find the only alive player
                for(auto winner : this->players){
                    if(!winner->checkIsAlive()) continue;
                    winner->onWin();
                    break;
                }
            }

            this->log("There are " + to_string(this->countAlivePlayers()) + " alive players now.");
        }
    } else {
        // Update the player's score
        int score = (this->scores[player->getName()] += found);

        // Send the currently visible phrase to all players
        for(HangmanPlayer* p : this->players) {
            p->onPhraseReveal(this->currentWordObscured);
            p->onScoreChange(*player, score);
        }

        if(remaining == 0) {
            // Set the time when to start the new round (in a few seconds)
            this->newRoundTime = time(nullptr) + 3;
        }
    }
}

/**
 * Checks if the time for this round has elapsed.
 * If so, starts the next round
 */
void HangmanServer::startNewRoundIfNeeded() {
    if(this->newRoundTime > time(nullptr)) return;

    this->newRoundTime = TIME_MAX;
    this->currentWord = this->generatePhrase();
    this->obscurePhrase();

    // Broadcast the new phrase
    for(HangmanPlayer* p : this->players) {
        p->onPhraseReveal(this->currentWordObscured);
    }
}

/**
 * Generates a new phrase
 */
u32string HangmanServer::generatePhrase() {
    u32string phrases[] = {
            U"EEL", U"MOCK", U"HATCH", U"DRIVER", U"HANGMAN",
            U"ELEPHANT", U"PROFESSOR", U"DEPARTMENT", U"CONSEQUENCE",
            U"INTELLIGENCE"
    };
    int choice = rand() % 10; // 10 = phrases[] length
    u32string phrase = phrases[choice];
    log("Chosen phrase: " + utf32ToUtf8(phrase) + " (index: " + to_string(choice) + ")");
    return phrase;
}

/**
 * Replaces the letters with underscores
 */
void HangmanServer::obscurePhrase() {
    this->currentWordObscured = this->currentWord;
    fill(this->currentWordObscured.begin(), this->currentWordObscured.end(), '_');
}

/**
 * Returns the players' scores
 */
const map<u32string, int>* HangmanServer::getScores() {
    return &(this->scores);
}

/**
 * Returns the number of players' fails
 */
const map<u32string, int>* HangmanServer::getFails() {
    return &(this->fails);
}

/**
 * Returns the current phrase as it should be displayed (with underscores)
 */
u32string HangmanServer::getCurrentPhrase() {
    return this->currentWordObscured;
}

/**
 * Counts all players that are still alive
 */
int HangmanServer::countAlivePlayers() const {
    int count = 0;
    for(auto player : this->players){
        if(player->checkIsAlive()) count++;
    }
    return count;
}

void HangmanServer::log(const string& s) const{
    cout << "\x1b[1;34m[GAME SERVER]\x1b[0m " << s << endl;
}
