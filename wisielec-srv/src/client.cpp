#include "client.hpp"

#include "buffer.hpp"
#include "game/hangman_player.hpp"
#include "game/hangman_server.hpp"

#include <cstdio>
#include <sys/socket.h>
#include <string>
#include <unistd.h>

using namespace std;

void clientOnInput(EpollHandler* sender);
void clientOnOutput(EpollHandler* sender);
void clientOnDisconnect(EpollHandler* sender);
void clientProcessInputData(Client* client);
void clientFinishWrite(Client* client);
void clientLog(Client* client, const string& text);

/**
 * States of client reading the data
 */
enum ClientReadStatus {
    READING_LENGTH,
    READING_CONTENT
};

/**
 * Structure representing the client
 */
struct Client {
    int sockFd;                     // Socket descriptor for the connection
    Buffer* readBuffer;             // Buffer used while reading data from remote
    ClientReadStatus readStatus;    // What is the current read state
    Buffer* writeBuffer;            // Buffer containing the data to send to remote
    EpollHandler* epollHandler;     // A handler that describes reactions to epoll events
    Server* server;                 // Server that the client is connected to
    HangmanPlayer* player;          // Game player associated with the connection
};

/**
 * Creates a new client on the socket
 * @param sockFd Socket descriptor to use
 * @param epoll An epoll instance serving as the event handler
 * @param server Server that accepted the connection
 * @return New client
 */
Client* clientCreate(int sockFd, Epoll* epoll, Server* server){
    auto client = new Client();
    client->sockFd = sockFd;
    client->readBuffer = bufferCreate(sizeof(uint32_t));
    client->readStatus = READING_LENGTH;
    client->writeBuffer = nullptr;
    client->epollHandler = epollCreateHandler(sockFd);
    epollHandlerSetOnInput(client->epollHandler, clientOnInput);
    epollHandlerSetOnOutput(client->epollHandler, clientOnOutput);
    epollHandlerSetOnDisconnect(client->epollHandler, clientOnDisconnect);

    client->server = server;
    client->player = new HangmanPlayer(HangmanServer::getInstance());
    client->player->attachNetworkClient(client);

    // Store a pointer to server in the epollHandler
    *(Client**)(epollHandlerData(client->epollHandler)) = client;
    epollRegisterHandler(epoll, client->epollHandler);
    epollSetHandledEvents(client->epollHandler, EPOLLIN);

    clientLog(client, "Created");

    return client;
}

/**
 * Handles the input event
 * @param sender The epoll handler that's related to this event
 */
void clientOnInput(EpollHandler* sender){
    // Read the client
    Client* client = *(Client**)epollHandlerData(sender);

    char* buffer = bufferGetData(client->readBuffer);
    size_t remaining = bufferGetRemaining(client->readBuffer);
    ssize_t readBytes = read(client->sockFd, buffer, remaining);

    if(readBytes == -1){
        // Error
        clientLog(client, "An error happened during read.");
        return;
    }
    else if(readBytes == 0){
        // EOF
        clientClose(client);
        return;
    }
    bufferMovePointer(client->readBuffer, readBytes);

    string msg = "Read " + to_string(readBytes) + "/" + to_string(bufferGetLength(client->readBuffer)) + " bytes";
    clientLog(client, msg);

    if(bufferGetRemaining(client->readBuffer) == 0){
        // Finished reading size or message content
        clientProcessInputData(client);
    }
}

/**
 * Handles the output event
 * @param sender The epoll handler that's related to this event
 */
void clientOnOutput(EpollHandler* sender) {
    // Read the client
    Client* client = *(Client**)epollHandlerData(sender);
    if(client->writeBuffer == nullptr){
        clientFinishWrite(client);
        return;
    }

    char* buffer = bufferGetData(client->writeBuffer);
    size_t len = bufferGetRemaining(client->writeBuffer);
    ssize_t writtenBytes = write(client->sockFd, buffer, len);

    if(writtenBytes == -1){
        // Error
        clientFinishWrite(client);
        clientLog(client, "An error happened during write.");
        return;
    }
    else if(writtenBytes == 0){
        // EOF
        clientClose(client);
        return;
    }
    bufferMovePointer(client->writeBuffer, writtenBytes);
    string msg = "Written " + to_string(writtenBytes) + "/" + to_string(bufferGetLength(client->writeBuffer)) + " bytes";
    clientLog(client, msg);

    if(bufferGetRemaining(client->writeBuffer) == 0){
        clientFinishWrite(client);
    }
}

/**
 * Handles the disconnect event
 * @param sender The epoll handler that's related to this event
 */
void clientOnDisconnect(EpollHandler* sender) {
    // Read the client
    Client* client = *(Client**)epollHandlerData(sender);

    clientLog(client, "Disconnected");
    clientClose(client);
}

/**
 * Closes the client and unregisters it from the epoll mechanism
 * @param client The client to close
 */
void clientClose(Client* client){
    // First, notify others
    if(client->player->checkIsAlive()){
        client->player->attachNetworkClient(nullptr);
    }
    serverOnClientClose(client->server, client);

    // Then release the resources and disappear
    epollUnregisterHandler(client->epollHandler);
    epollReleaseHandler(client->epollHandler);
    shutdown(client->sockFd, SHUT_RDWR);
    close(client->sockFd);
    bufferRelease(client->readBuffer);
    delete client->player;

    clientLog(client, "Closed");
    delete client;
}

/**
 * Processes the whole buffer that was read
 * @param client The client that finished reading data
 */
void clientProcessInputData(Client* client){
    switch(client->readStatus){
        case READING_LENGTH: {
            // The message length was read. Parse it and create a new buffer for the message content.
            char* data = bufferGetDataOrigin(client->readBuffer);
            size_t length = bufferGetLength(client->readBuffer);
            size_t messageLength = 0;
            for(size_t i = 0; i < length; i++){
                messageLength <<= 8;
                messageLength |= data[i];
            }
            bufferRelease(client->readBuffer);
            client->readBuffer = bufferCreate(messageLength);
            client->readStatus = READING_CONTENT;
            break;
        }
        case READING_CONTENT: {
            // The message content has been read. Make use of it.
            clientLog(client, "Completed reading message.");
            client->player->parseMessage(
                    bufferGetDataOrigin(client->readBuffer),
                    bufferGetLength(client->readBuffer));

            bufferRelease(client->readBuffer);
            client->readBuffer = bufferCreate(sizeof(uint32_t));
            client->readStatus = READING_LENGTH;
            break;
        }
    }
}

/**
 * Prepares the write buffer to sending data
 * @param client The client that is to send the data
 * @param data The bytes to send
 * @param length Number of bytes to send
 */
void clientWrite(Client* client, const char* data, size_t length){
    // Copy the data and prepend with a length sequence
    char* rawData = new char[length + sizeof(uint32_t)];
    size_t len = length;
    for(int i = sizeof(uint32_t) - 1; i >= 0; i--) {
        rawData[i] = (char)(len & 0xff);
        len >>= 8;
    }
    for(size_t i = 0; i < length; i++){
        rawData[i + sizeof(uint32_t)] = data[i];
    }

    Buffer* buffer = bufferCreate(length + sizeof(uint32_t), rawData);
    if(client->writeBuffer == nullptr){
        client->writeBuffer = buffer;
    }else{
        bufferAttachNext(client->writeBuffer, buffer);
    }
    epollSetHandledEvents(client->epollHandler, EPOLLIN | EPOLLOUT);
}

/**
 * Finishes the current buffer and turns off listening for outbound events if necessary
 * @param client The client that's to stop sending
 */
void clientFinishWrite(Client* client){
    Buffer* nextBuffer = bufferGetNext(client->writeBuffer);
    bufferRelease(client->writeBuffer);
    client->writeBuffer = nextBuffer;

    if(client->writeBuffer == nullptr){
        epollSetHandledEvents(client->epollHandler, EPOLLIN);
    }
}

void clientLog(Client* client, const string& text){
    printf("\x1b[1;36m[CLIENT: %d]\x1b[0m %s\n", client->sockFd, text.c_str());
}
