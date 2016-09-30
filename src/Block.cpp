//
// Created by anton on 01.10.16.
//

#include "../include/Block.h"

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

Block& Block::operator=(const Block &other) {
    if(this != &other) {
        this->setBase(other.getBase());
        this->setSize(other.getSize());
    }
}