#include <stdexcept>
#include <string>
#include <list>

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

class Allocator;

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

class Pointer {
public:
    Pointer();
    Pointer(Block*);
    void *get() const;
    void setBase(void*);
    std::size_t getSize() const;
    void setSize(std::size_t);
    Block* getBlock() const;
    void setBlock(Block*);
private:
    Block *block;
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

