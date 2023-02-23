#include "server.hpp"

#include <cstdio>
#include <cstdlib>
#include <list>
#include <stdexcept>
#include <string>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

int serverCreateSocket();
void serverBind(int sockFd, short port);
void serverListen(int sockFd);
void serverAccept(EpollHandler* sender);
void serverLog(Server* server, const string& text);

/**
 * Structure representing a server
 */
struct Server {
    int sockFd;                     // Socket file descriptor
    Epoll* epoll;                   // An epoll instance the server is attached to
    EpollHandler* epollHandler;     // A handler that describes reactions to epoll events
    list<Client*>* clients;         // List of clients that are associated with this server
};


/**
 * Creates a new instance of server but doesn't start it
 * @return The server
 */
Server* serverCreate() {
    // Create a new server struct and fill it
    auto server = new Server();
    server->sockFd = serverCreateSocket();
    server->epoll = nullptr;
    server->epollHandler = epollCreateHandler(server->sockFd);
    epollHandlerSetOnInput(server->epollHandler, serverAccept);
    server->clients = new list<Client*>();

    // Store a pointer to server in the epollHandler
    *(Server**)(epollHandlerData(server->epollHandler)) = server;

    serverLog(server, "Created server");
    return server;
}

/**
 * Starts the server
 * @param server The server to start
 * @param port The port on which to listen
 * @param epoll The epoll instance to attach to
 */
void serverStart(Server* server, short port, Epoll* epoll) {
    server->epoll = epoll;
    serverBind(server->sockFd, port);
    serverListen(server->sockFd);
    epollRegisterHandler(epoll, server->epollHandler);
    epollSetHandledEvents(server->epollHandler, EPOLLIN);

    serverLog(server, "Started server on port " + to_string(port));
}

/**
 * Creates a new socket for the server
 * @return Socket descriptor
 */
int serverCreateSocket(){
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == -1){
        perror("Failed to create socket");
        exit(1);
    }

    // Enable REUSE_ADDR option - so that we can use the recently abandoned port
    int one = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    return serverSocket;
}

/**
 * Binds the socket to the port
 * @param sockFd Socket descriptor
 * @param port Port number
 */
void serverBind(int sockFd, short port) {
    sockaddr_in serverAddress {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1){
        throw runtime_error("Failed to bind the socket.");
    }
}

/**
 * Enables listening on the socket
 * @param sockFd Socket descriptor
 */
void serverListen(int sockFd){
    if(listen(sockFd, 16) == -1){
        throw runtime_error("Failed to listen on the socket.");
    }
}

/**
 * Accepts the incoming connection
 * @param sender The handler that received the input event
 */
void serverAccept(EpollHandler* sender){
    // Read the server
    Server* server = *(Server**)epollHandlerData(sender);

    // Accept the connection
    sockaddr_in clientAddress {};
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(server->sockFd, (struct sockaddr*)&clientAddress, &clientAddressLength);

    serverLog(server, "Accepted client on socket " + to_string(clientSocket));

    // Create a new client representing the connection and store it
    Client* c = clientCreate(clientSocket, server->epoll, server);
    server->clients->push_back(c);
}

/**
 * Closes the server and all the clients associated with it
 * @param server The server to close
 */
void serverClose(Server* server) {
    // First, close all the remaining clients
    while(server->clients->begin() != server->clients->end()){
        auto c = server->clients->begin();
        clientClose(*c);
    }

    // Then close the socket
    epollUnregisterHandler(server->epollHandler);
    epollReleaseHandler(server->epollHandler);
    shutdown(server->sockFd, SHUT_RDWR);
    close(server->sockFd);

    serverLog(server, "Closed server");
    delete server;
}

/**
 * A function invoked when the client associated with this server is closed
 * @param server The server
 * @param client The client that's closing
 */
void serverOnClientClose(Server* server, Client* client) {
    // Remove the closed client from the list
    server->clients->remove(client);
}

void serverLog(Server* server, const string& text) {
    printf("\x1b[1;35m[SERVER: %d]\x1b[0m %s\n", server->sockFd, text.c_str());
}
