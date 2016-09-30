//
// Created by anton on 01.10.16.
//

#include "../include/Pointer.h"
#include "../include/Block.h"

Pointer::Pointer() {
    this->block = nullptr;
}

Pointer::Pointer(Block *block) {
    this->block = block;
}

void* Pointer::get() const {
    if(this->block == nullptr) {
        return nullptr;
    }
    return this->block->getBase();
}

void Pointer::setBase(void *base) {
    if(this->block != nullptr) {
        this->block->setBase(base);
    }
}

std::size_t Pointer::getSize() const {
    if(this->block == nullptr) {
        return 0;
    }
    return this->block->getSize();
}

void Pointer::setSize(std::size_t size) {
    if(this->block != nullptr) {
        this->block->setSize(size);
    }
}

Block* Pointer::getBlock() const {
    return this->block;
}

void Pointer::setBlock(Block *block) {
    this->setBase(block->getBase());
    this->setSize(block->getSize());
}
