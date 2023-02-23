#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <unistd.h>

struct Buffer;

Buffer* bufferCreate(size_t capacity);
Buffer* bufferCreate(size_t capacity, char* data);
void bufferRelease(Buffer* buffer);

size_t bufferGetLength(Buffer* buffer);
size_t bufferGetRemaining(Buffer* buffer);
char* bufferGetData(Buffer* buffer);
char* bufferGetDataOrigin(Buffer* buffer);
void bufferMovePointer(Buffer* buffer, ssize_t diff);

void bufferAttachNext(Buffer* previous, Buffer* next);
Buffer* bufferGetNext(Buffer* buffer);

#endif
