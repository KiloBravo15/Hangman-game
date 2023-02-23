#include "buffer.hpp"

/**
 * Structure representing a data buffer
 */
struct Buffer {
    size_t length;
    size_t offset;
    char* data;
    Buffer* nextBuffer;
};

/**
 * Creates a new buffer with the specified capacity. Allocates the space for data
 * @param capacity Capacity of the new buffer
 * @return The buffer
 */
Buffer* bufferCreate(size_t capacity){
    return bufferCreate(capacity, new char[capacity]);
}

/**
 * Creates a new buffer referencing the passed data
 * @param capacity Capacity of the new buffer
 * @param data A pointer to the array with data
 * @return The buffer
 */
Buffer* bufferCreate(size_t capacity, char* data){
    auto buffer = new Buffer();
    buffer->length = capacity;
    buffer->offset = 0;
    buffer->data = data;
    buffer->nextBuffer = nullptr;
    return buffer;
}

/**
 * Releases the buffer and the contained array
 * @param buffer The buffer to release
 */
void bufferRelease(Buffer* buffer){
    delete buffer->data;
    delete buffer;
}

/**
 * Returns the total length of the buffer
 * @param buffer The buffer
 */
size_t bufferGetLength(Buffer* buffer){
    return buffer->length;
}

/**
 * Returns the remaining length of the buffer
 * @param buffer The buffer
 */
size_t bufferGetRemaining(Buffer* buffer){
    return buffer->length - buffer->offset;
}

/**
 * Returns a pointer to the current position inside the buffer
 * @param buffer The buffer
 */
char* bufferGetData(Buffer* buffer){
    return buffer->data + buffer->offset;
}

/**
 * Returns a pointer to the beginning of the data inside the buffer
 * @param buffer
 * @return
 */
char* bufferGetDataOrigin(Buffer* buffer){
    return buffer->data;
}

/**
 * Moves the data pointer inside the buffer
 * @param buffer The buffer
 * @param diff Number of bytes to move the data pointer
 */
void bufferMovePointer(Buffer* buffer, ssize_t diff){
    buffer->offset += diff;
}

/**
 * Attaches a new buffer to the end of the buffer chain
 * @param previous The buffer in an existing chain
 * @param next The buffer to attach
 */
void bufferAttachNext(Buffer* previous, Buffer* next){
    while(previous->nextBuffer != nullptr){
        previous = previous->nextBuffer;
    }
    previous->nextBuffer = next;
}

/**
 * Gets the new buffer in the chain after this one
 * @param buffer The buffer
 */
Buffer* bufferGetNext(Buffer* buffer){
    return buffer->nextBuffer;
}
