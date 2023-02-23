#ifndef SERVER_HPP
#define SERVER_HPP

struct Server;

#include "epoll.hpp"
#include "client.hpp"

Server* serverCreate();
void serverStart(Server* server, short port, Epoll* epoll);
void serverClose(Server* server);

void serverOnClientClose(Server* server, Client* client);

#endif
