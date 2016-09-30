#include <iostream>
#include <algorithm>
#include "../include/Allocator.h"
#include "../include/Pointer.h"
#include "../include/Block.h"

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

    std::list<Block*>::iterator cur = std::find_if(
            ++(this->allocatedBlocks.begin()),
            this->allocatedBlocks.end(),
            [&](const Block* block) {
                return block->getBase() == p.get();
            });
    if(cur == this->allocatedBlocks.end()) {
        p = this->alloc(N);
        return;
    }
    std::list<Block*>::iterator prev = cur;
    if(cur != (this->allocatedBlocks.begin())) {
        --prev;
    }
    std::list<Block*>::iterator next = cur;
    if(cur != --(this->allocatedBlocks.end())) {
        ++next;
    }
    if((char*)(*next)->getBase() - (char*)(*cur)->getBase() >= N) {
        (*cur)->setSize(N);
        p.setBlock(*cur);
        return;
    }
    if(((char*)(*next)->getBase() - (char*)(*cur)->getBase()) + ((char*)(*cur)->getBase() - (char*)(*prev)->getBase() - (*prev)->getSize()) >= N) {
        for(int i = 0; i < (*cur)->getSize(); ++i) {
            *((char*)(*prev)->getBase() + (*prev)->getSize() + i) = *((char*)(*cur)->getBase() + i);
        }
        (*cur)->setBase((char*)(*prev)->getBase() + (*prev)->getSize());
        (*cur)->setSize(N);
        p.setBlock(*cur);
        return;
    }
    std::size_t totalFreeSize = (*cur)->getSize();
    for(std::list<Block*>::iterator iterator = allocatedBlocks.begin(); iterator != --allocatedBlocks.end(); ++iterator) {
        std::list<Block*>::iterator itNext = iterator;
        itNext++;
        std::size_t freeBlockSize = (char*)(*itNext)->getBase() - ((char*)(*iterator)->getBase() + (*iterator)->getSize());
        totalFreeSize += freeBlockSize;
        if(freeBlockSize >= N) {
            for(int i = 0; i < (*cur)->getSize(); ++i) {
                *((char*)(*iterator)->getBase() + (*iterator)->getSize() + i) = *((char*)(*cur)->getBase() + i);
            }
            (*cur)->setBase((char*)(*iterator)->getBase() + (*iterator)->getSize());
            (*cur)->setSize(N);
            p.setBlock(*cur);
            return;
        }
    }
    if(totalFreeSize >= N) {
        this->defrag();
        auto last = --((this->allocatedBlocks).end());
        auto preLast = --last;
        if((char*)(*last)->getBase() - (char*)(*preLast)->getBase() - (*preLast)->getSize() >= N) {
            for (int i = 0; i < (*cur)->getSize(); ++i) {
                *((char *) (*preLast)->getBase() + (*preLast)->getSize() + i) =
                        *((char *) (*cur)->getBase() + i);
            }
            (*cur)->setBase((char *)(*preLast)->getBase() +
                            (*preLast)->getSize());
            (*cur)->setSize(N);
        } else {
            next = cur;
            ++next;
            while(next != --(this->allocatedBlocks).end()) {
                this->swapPartsOfArray((char*)(*cur)->getBase(), (*cur)->getSize(), (*next)->getSize());
                std::size_t curSize = (*cur)->getSize();
                (*cur)->setSize((*next)->getSize());
                (*next)->setBase((void*)((char*)(*cur)->getBase() + (*cur)->getSize()));
                (*next)->setSize(curSize);

                ++cur;
                ++next;
            }
            (*cur)->setSize(N);
        }
        p.setBlock(*cur);
        return;
    }
}

void Allocator::free(Pointer &pointer) {
    for(std::list<Block*>::iterator it = allocatedBlocks.begin(); it != allocatedBlocks.end(); it++) {
        if((*it)->getBase() == pointer.get()) {
            char *ptr = reinterpret_cast<char*>((*it)->getBase());
            for(std::size_t i = 0; i < (*it)->getSize(); ++i) {
                ptr[i] = 0;
            }
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

void Allocator::swapPartsOfArray(char *pInt, std::size_t size1, std::size_t size2) {
    std::size_t length = (size1 + size2);
    char tmp = 0;
    for(int i = 0; i < length/2; ++i) {
        tmp = *(pInt + i);
        *(pInt + i) = *(pInt + length - 1 - i);
        *(pInt + length - 1 - i) = tmp;
    }
    for(int i = 0; i < size2/2; ++i) {
        tmp = *(pInt + i);
        *(pInt + i) = *(pInt + size2 - 1 - i);
        *(pInt + size2 - 1 - i) = tmp;
    }
    for(int i = 0; i < size1/2; ++i) {
        tmp = *(pInt + size2 + i);
        *(pInt + size2 + i) = *(pInt + size2 + size1 - 1 - i);
        *(pInt + size2 + size1 - 1 - i) = tmp;
    }
}