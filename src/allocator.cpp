#include <iostream>
#include "../include/allocator.h"

Block::Block() {
    this->base = nullptr;
    this->size = 0;
}

Block::Block(void *base, std::size_t size) {
    this->base = base;
    this->size = size;
}

void* Block::getBase() const {
    return this->base;
}

std::size_t Block::getSize() const {
    return this->size;
}

void Block::setBase(void *base) {
    this->base = base;
}

void Block::setSize(std::size_t size) {
    this->size = size;
}

Pointer::Pointer() {
    this->block = nullptr;
}

Pointer::Pointer(Block *block) {
    this->block = block;
}

void* Pointer::get() const {
    return this->block->getBase();
}

void Pointer::setBase(void *base) {
    this->block->setBase(base);
}

std::size_t Pointer::getSize() const {
    return this->block->getSize();
}

void Pointer::setSize(std::size_t size) {
    this->block->setSize(size);
}

Allocator::Allocator(void *base, std::size_t size) {
    this->base = base;
    this->size = size;
    this->allocatedBlocks.clear();
    this->allocatedBlocks.push_back(new Block((char*)base, 0));
    this->allocatedBlocks.push_back(new Block((char*)base + size, 0));
}

Pointer Allocator::alloc(std::size_t requestSize) {
    std::list<Block*>::iterator cur = allocatedBlocks.begin();
    std::list<Block*>::iterator next = allocatedBlocks.begin();
    next++;
    std::size_t maxMatch = this->size;
    std::list<Block*>::iterator insertPlace;
    bool freeBlockFound = false;
    while(next != allocatedBlocks.end()) {
        std::size_t freeSpace = (char*)(*next)->getBase() - ((char*)(*cur)->getBase() + (*cur)->getSize());
        if(freeSpace >= requestSize) {
            freeBlockFound = true;
            if(freeSpace <= maxMatch) {
                maxMatch = freeSpace;
                insertPlace = cur;
            }
        }
        cur++;
        next++;
    }
    if(freeBlockFound) {
        Block *block = new Block((char*)(*insertPlace)->getBase() + (*insertPlace)->getSize(), requestSize);
        insertPlace++;
        this->allocatedBlocks.insert(insertPlace, block);
        return Pointer(block);
    }
    throw AllocError(AllocErrorType::NoMemory, "Bad Alloc");
}

void Allocator::realloc(Pointer &p, std::size_t N) {

}

void Allocator::free(Pointer &pointer) {
    for(std::list<Block*>::iterator it = allocatedBlocks.begin(); it != allocatedBlocks.end(); it++) {
        if((*it)->getBase() == pointer.get()) {
            allocatedBlocks.erase(it);
            break;
        }
    }
    pointer.setBase(nullptr);
    pointer.setSize(0);
}

void Allocator::defrag() {
    std::list<Block*>::iterator cur = allocatedBlocks.begin();
    std::list<Block*>::iterator next = allocatedBlocks.begin();
    next++;
    while(next != --allocatedBlocks.end()) {
        std::size_t freeSpace = (char*)(*next)->getBase() - ((char*)(*cur)->getBase() + (*cur)->getSize());
        if(freeSpace != 0) {
            for(std::size_t i = 0; i < (*next)->getSize(); ++i) {
                *((char*)(*cur)->getBase() + (*cur)->getSize() + i) = *((char*)(*next)->getBase() + i);
            }
            (*next)->setBase((void*)((char*)(*cur)->getBase() + (*cur)->getSize()));
        }
        cur++;
        next++;
    }
}