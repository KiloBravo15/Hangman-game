#ifndef CLIENT_HPP
#define CLIENT_HPP

struct Client;

#include "epoll.hpp"
#include "server.hpp"

Client* clientCreate(int sockFd, Epoll* epoll, Server* server);
void clientClose(Client* client);

void clientWrite(Client* client, const char* data, size_t length);

#endif
