//
// Created by anton on 01.10.16.
//

#ifndef ALLOCATOR_POINTER_H
#define ALLOCATOR_POINTER_H

#include <cstddef>

class Allocator;
class Block;

class Pointer {
    friend class Allocator;
public:
    Pointer();
    Pointer(Block*);
    void *get() const;
private:
    void setBase(void*);
    std::size_t getSize() const;
    void setSize(std::size_t);
    Block* getBlock() const;
    void setBlock(Block*);
private:
    Block *block;
};

#endif //ALLOCATOR_POINTER_H
