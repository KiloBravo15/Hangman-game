#include "epoll.hpp"

/**
 * Structure representing an epoll instance
 */
struct Epoll {
    int fd;
};

/**
 * Structure representing a handler of socket events
 */
struct EpollHandler {
    int fd;
    Epoll* epoll;
    void* data;

    EventHandler handleInput;
    EventHandler handleOutput;
    EventHandler handleDisconnect;
};

/**
 * Creates a new epoll instance
 */
Epoll* epollCreate(){
    auto* epoll = new Epoll();
    epoll->fd = epoll_create1(0);
    return epoll;
}

/**
 * Releases the epoll
 */
void epollRelease(Epoll* epoll){
    delete epoll;
}

/**
 * Creates a new epoll handler
 * @param fd Socket descriptor to listen for events on
 * @return A new epoll handler
 */
EpollHandler* epollCreateHandler(int fd){
    EpollHandler* handler = new EpollHandler();
    handler->fd = fd;
    handler->epoll = nullptr;
    handler->handleInput = nullptr;
    handler->handleOutput = nullptr;
    handler->handleDisconnect = nullptr;

    return handler;
}

/**
 * Releases the epoll handler
 * @param handler The handler to release
 */
void epollReleaseHandler(EpollHandler* handler) {
    delete handler;
}

/**
 * Sets a function to invoke on an inbound event
 * @param epollHandler The epoll handler
 * @param eventHandler The function to invoke
 */
void epollHandlerSetOnInput(EpollHandler* epollHandler, EventHandler eventHandler){
    epollHandler->handleInput = eventHandler;
}

/**
 * Sets a function to invoke on an outbound event
 * @param epollHandler The epoll handler
 * @param eventHandler The function to invoke
 */
void epollHandlerSetOnOutput(EpollHandler* epollHandler, EventHandler eventHandler){
    epollHandler->handleOutput = eventHandler;
}

/**
 * Sets a function to invoke on a disconnect event
 * @param epollHandler The epoll handler
 * @param eventHandler The function to invoke
 */
void epollHandlerSetOnDisconnect(EpollHandler* epollHandler, EventHandler eventHandler){
    epollHandler->handleDisconnect = eventHandler;
}

/**
 * Returns a pointer to data associated with the handler. It can point to any desired data
 * @param handler The epoll handler
 * @return Pointer to the data
 */
void** epollHandlerData(EpollHandler* handler){
    return &(handler->data);
}

/**
 * Waits for an event on the epoll
 * @param epoll The epoll to wait on
 */
void epollWaitForEvent(Epoll* epoll){
    epoll_event ee;
    int eventCount = epoll_wait(epoll->fd, &ee, 1, 500);
    if(eventCount <= 0) return;

    EpollHandler* eventSource = (EpollHandler*)ee.data.ptr;
    if((ee.events & EPOLLRDHUP) && eventSource->handleDisconnect != nullptr) {
        eventSource->handleDisconnect(eventSource);
        // The eventSource becomes undefined here. Don't use it.
        // Otherwise, the SIGSEGV will appear.
    }
    else if((ee.events & EPOLLIN) && eventSource->handleInput != nullptr) {
        eventSource->handleInput(eventSource);
    }
    else if((ee.events & EPOLLOUT) && eventSource->handleOutput != nullptr) {
        eventSource->handleOutput(eventSource);
    }
}

/**
 * Registers a handler with the epoll instance. Subscribes for RDHUP event only
 * @param epoll The epoll instance
 * @param handler The epoll handler to register
 */
void epollRegisterHandler(Epoll* epoll, EpollHandler* handler){
    handler->epoll = epoll;
    epoll_event ee { EPOLLRDHUP, {.ptr=handler}};
    epoll_ctl(epoll->fd, EPOLL_CTL_ADD, handler->fd, &ee);
}

/**
 * Unregisters a handler from the epoll instance
 * @param handler The epoll handler to register
 */
void epollUnregisterHandler(EpollHandler* handler){
    epoll_ctl(handler->epoll->fd, EPOLL_CTL_DEL, handler->fd, nullptr);
    handler->epoll = nullptr;
}

/**
 * Subscribes for the specified epoll events
 * @param handler The epoll handler to update the events for
 * @param events The events to listen for
 */
void epollSetHandledEvents(EpollHandler* handler, uint32_t events){
    epoll_event ee { EPOLLRDHUP | events, {.ptr=handler}};
    epoll_ctl(handler->epoll->fd, EPOLL_CTL_MOD, handler->fd, &ee);
}
