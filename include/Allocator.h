#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdexcept>
#include <string>
#include <list>

class Pointer;
class Block;

enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError: std::runtime_error {
private:
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, std::string message):
            runtime_error(message),
            type(_type)
    {}

    AllocErrorType getType() const { return type; }
};

class Allocator {
public:
    Allocator(void *base, std::size_t size);

    Pointer alloc(std::size_t N);
    void realloc(Pointer &p, std::size_t N);
    void free(Pointer &p);

    void defrag();
    std::string dump(){}
private:
    Allocator(const Allocator&);
    Allocator& operator=(const Allocator&);

    void swapPartsOfArray(char*, std::size_t, std::size_t);
private:
    void *base;
    std::size_t size;
    std::list<Block*> allocatedBlocks;
};

#endif
