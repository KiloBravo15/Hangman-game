#include "epoll.hpp"
#include "server.hpp"
#include "game/hangman_server.hpp"

#include <iostream>
#include <csignal>

using namespace std;

void terminate(int);

Epoll* epoll;
Server* server;

int main(int argc, char** argv){
    if(argc < 2){
        cout << "Missing argument: server port number" << endl;
        return 1;
    }
    auto port = (short)atoi(argv[1]);

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, terminate);

    epoll = epollCreate();
    server = serverCreate();
    serverStart(server, port, epoll);

    HangmanServer* gameServer = HangmanServer::getInstance();

    while(true){
        gameServer->startNewRoundIfNeeded();
        epollWaitForEvent(epoll);
    }
}

void terminate(int){
    cout << endl << "Terminating..." << endl;
    serverClose(server);
    epollRelease(epoll);
    cout << "Terminated." << endl;
    exit(0);
}
