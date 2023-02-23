#ifndef EPOLL_HPP
#define EPOLL_HPP

#include <sys/epoll.h>

struct Epoll;
struct EpollHandler;

typedef void (*EventHandler)(EpollHandler* sender);
typedef void (*EventHandler)(EpollHandler* sender);

Epoll* epollCreate();
void epollRelease(Epoll* epoll);

EpollHandler* epollCreateHandler(int fd);
void epollReleaseHandler(EpollHandler* handler);

void epollHandlerSetOnInput(EpollHandler* epollHandler, EventHandler eventHandler);
void epollHandlerSetOnOutput(EpollHandler* epollHandler, EventHandler eventHandler);
void epollHandlerSetOnDisconnect(EpollHandler* epollHandler, EventHandler eventHandler);

void** epollHandlerData(EpollHandler* handler);

void epollWaitForEvent(Epoll* epoll);

void epollRegisterHandler(Epoll* epoll, EpollHandler* handler);
void epollUnregisterHandler(EpollHandler* handler);
void epollSetHandledEvents(EpollHandler* handler, uint32_t events);

#endif
