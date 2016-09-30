//
// Created by anton on 01.10.16.
//

#ifndef ALLOCATOR_BLOCK_H
#define ALLOCATOR_BLOCK_H

#include <cstddef>

class Block {
public:
    Block();
    Block(void*, std::size_t);
    void *getBase() const;
    std::size_t getSize() const;
    void setBase(void*);
    void setSize(std::size_t);
    Block& operator=(const Block&);
private:
    void *base;
    std::size_t size;
};

#endif //ALLOCATOR_BLOCK_H
