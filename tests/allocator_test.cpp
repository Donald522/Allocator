#include "Allocator.h"
#include "Pointer.h"

#include <vector>
#include <set>
#include <iostream>
#include <gtest/gtest.h>

using namespace std;
char buf[65536];

TEST(Allocator, AllocInRange) {
    Allocator a(buf, sizeof(buf));

    int size = 500;

    Pointer p = a.alloc(size);
    char *v = reinterpret_cast<char*>(p.get());

    EXPECT_GE(v, buf);
    EXPECT_LE(v + size, buf + sizeof(buf));

    a.free(p);
}

static void writeTo(Pointer &p, size_t size) {
    char *v = reinterpret_cast<char*>(p.get());

    for (int i = 0; i < size; i++) {
        v[i] = i % 31;
    }
}

static bool isDataOk(Pointer &p, size_t size) {
    char *v = reinterpret_cast<char*>(p.get());

    for (int i = 0; i < size; i++) {
        if (v[i] != i % 31) {
            return false;
        }
    }
    return true;
}

static bool isValidMemory(Pointer &p, size_t allocatedSize) {
    char *v = reinterpret_cast<char*>(p.get());
    return (v >= buf && v + allocatedSize <= buf + sizeof(buf));
}

static bool fillUp(Allocator &a, size_t allocSize, vector<Pointer> &out) {
    int Max = (2 * sizeof(buf) / allocSize); // To avoid creating an infinite loop.

    for (int i = 0; i < Max; i++ ) try {
        out.push_back( a.alloc(allocSize) );
        writeTo(out.back(), allocSize);
    } catch (AllocError &) {
        return true;
    }

    return false;
}

TEST(Allocator, AllocReadWrite) {
    Allocator a(buf, sizeof(buf));

    vector<Pointer> ptr;
    size_t size = 300;
    for (int i = 0; i < 20; i++) {
        ptr.push_back( a.alloc(size) );

        EXPECT_TRUE(isValidMemory(ptr.back(), size));
        writeTo(ptr.back(), size);
    }

    for (Pointer &p: ptr) {
        EXPECT_TRUE(isDataOk(p, size)); 
    }
    for (Pointer &p: ptr) {
        a.free(p);
    }
}

TEST(Allocator, AllocNoMem) {
    Allocator a(buf, sizeof(buf));
    size_t size = sizeof(buf) / 5;

    vector<Pointer> ptr;
    try {
        for (int i = 0; i < 6; i++) {
            ptr.push_back( a.alloc(size) );
        }

        EXPECT_TRUE(false);
    } catch (AllocError &e) {
        EXPECT_EQ(e.getType(), AllocErrorType::NoMemory); 
    }

    for (Pointer &p: ptr) {
        a.free(p);
    }
}

TEST(Allocator, AllocReuse) {
    Allocator a(buf, sizeof(buf));

    vector<Pointer> ptrs;
    int size = 135;

    ASSERT_TRUE(fillUp(a, size, ptrs));
    a.free(ptrs[1]);

    EXPECT_EQ(ptrs[1].get(), nullptr);
    ptrs[1] = a.alloc(size);

    EXPECT_NE(ptrs[1].get(), nullptr);
    writeTo(ptrs[1], size);

    for (Pointer &p: ptrs) {
        EXPECT_TRUE(isDataOk(p, size)); 
        a.free(p);
    }
}

TEST(Allocator, DefragMove) {
    Allocator a(buf, sizeof(buf));

    set<void *> initialPtrs;
    vector<Pointer> ptrs;
    int size = 135;

    ASSERT_TRUE(fillUp(a, size, ptrs));
    a.free(ptrs[1]);
    a.free(ptrs[10]);
    a.free(ptrs[15]);

    ptrs.erase(ptrs.begin() + 15);
    ptrs.erase(ptrs.begin() + 10);
    ptrs.erase(ptrs.begin() + 1);

    for (Pointer &p: ptrs) {
        auto r = initialPtrs.insert(p.get());
        // Ensure inserted a new element.
        EXPECT_TRUE(r.second); 
    }

    a.defrag();

    bool moved = false;
    for (Pointer &p: ptrs) {
        EXPECT_TRUE(isDataOk(p, size));
        moved = (moved || initialPtrs.find(p.get()) == initialPtrs.end());
    }

    EXPECT_TRUE(moved);

    for (Pointer &p: ptrs) {
        EXPECT_TRUE(isDataOk(p, size));
        a.free(p);
    }
}

TEST(Allocator, DefragMoveTwice) {
    Allocator a(buf, sizeof(buf));

    vector<Pointer> ptrs;
    int size = 225;

    ASSERT_TRUE(fillUp(a, size, ptrs));

    a.free(ptrs[1]);
    a.free(ptrs[10]);
    ptrs.erase(ptrs.begin() + 10);
    ptrs.erase(ptrs.begin() + 1);

    a.defrag();

    a.free(ptrs[15]);
    ptrs.erase(ptrs.begin() + 15);

    a.defrag();

    ptrs.push_back(a.alloc(size));
    writeTo(ptrs.back(), size);

    for (Pointer &p: ptrs) {
        EXPECT_TRUE(isDataOk(p, size));
        a.free(p);
    }
}


TEST(Allocator, DefragAvailable) {
    Allocator a(buf, sizeof(buf));

    vector<Pointer> ptrs;
    int size = 135;

    ASSERT_TRUE(fillUp(a, size, ptrs));

    a.free(ptrs[1]);
    a.free(ptrs[10]);
    a.free(ptrs[15]);

    ptrs.erase(ptrs.begin() + 15);
    ptrs.erase(ptrs.begin() + 10);
    ptrs.erase(ptrs.begin() + 1);

    try {
        Pointer p = a.alloc(size * 2);
        a.free(p);

        cerr << "WARNING: Allocator not fragmented initially. Defrag tests are inconclusive." << endl;
    } catch (AllocError &) {} 

    a.defrag();
    Pointer newPtr = a.alloc(size * 2);
    writeTo(newPtr, size * 2);

    for (Pointer &p: ptrs) {
       EXPECT_TRUE(isDataOk(p, size));
       a.free(p); 
    }
}

TEST(Allocator, ReallocFromEmpty) {
    Allocator a(buf, sizeof(buf));
    
    int size = 81;

    Pointer p;
    Pointer p1 = a.alloc(size);

    a.realloc(p, size);
    EXPECT_NE(p.get(), nullptr);

    Pointer p2 = a.alloc(size);

    writeTo(p, size);
    writeTo(p1, size);
    writeTo(p2, size);

    EXPECT_TRUE(isDataOk(p, size));
    EXPECT_TRUE(isDataOk(p1, size));
    EXPECT_TRUE(isDataOk(p2, size));

    a.free(p);
    a.free(p1);
    a.free(p2);
}

TEST(Allocator, ReallocGrowInplace) {
    Allocator a(buf, sizeof(buf));

    int size = 135;
    Pointer p = a.alloc(size);
    writeTo(p, size);
   
    void *ptr = p.get();
    a.realloc(p, size * 2);
 
    EXPECT_EQ(p.get(), ptr);
    EXPECT_TRUE(isDataOk(p, size));
  
    Pointer p2 = a.alloc(size);
    writeTo(p, size * 2);
    writeTo(p2, size);

    EXPECT_TRUE(isDataOk(p, size * 2));
    EXPECT_TRUE(isDataOk(p2, size));

    a.free(p);
    a.free(p2);
}

TEST(Allocator, ReallocShrink) {
    Allocator a(buf, sizeof(buf));

    int size = 135;
    Pointer p = a.alloc(size);
    writeTo(p, size);
    
    void *ptr = p.get();
    a.realloc(p, size / 2);
 
    EXPECT_EQ(p.get(), ptr);
   
    Pointer p2 = a.alloc(size);
    writeTo(p2, size);

    EXPECT_TRUE(isDataOk(p, size / 2));
    EXPECT_TRUE(isDataOk(p2, size));

    a.free(p);
    a.free(p2);
}

TEST(Allocator, ReallocGrow) {
    Allocator a(buf, sizeof(buf));

    int size = 135;
    Pointer p = a.alloc(size);
    Pointer p2 = a.alloc(size);

    writeTo(p, size);
    writeTo(p2, size);

    void *ptr = p.get();
    a.realloc(p, size * 2);
 
    if (p.get() == ptr) {
        cerr << "WARNING: Reallocated chunk was not moved. realloc() grow tests inconclusive." << endl;
    }

    EXPECT_TRUE(isDataOk(p, size));
    writeTo(p, size * 2);

    EXPECT_TRUE(isDataOk(p, size * 2));
    EXPECT_TRUE(isDataOk(p2, size));

    a.free(p);
    a.free(p2);
}

TEST(Allocator, ReallocWithRebase) {
    char buffer[10] = {0};

    Allocator a(buffer, sizeof(buffer));
    Pointer p1 = a.alloc(2);
    writeTo(p1, 2);
    EXPECT_TRUE(isDataOk(p1, 2));

    Pointer p2 = a.alloc(1);
    writeTo(p2, 1);
    EXPECT_TRUE(isDataOk(p2, 1));

    Pointer p3 = a.alloc(4);
    writeTo(p3, 4);
    EXPECT_TRUE(isDataOk(p3, 4));

    a.free(p2);
    EXPECT_EQ(p2.get(), nullptr);

    void *ptr = p1.get();
    a.realloc(p1, 4);
    EXPECT_NE(p1.get(), ptr);
    EXPECT_TRUE(isDataOk(p1, 2));
    EXPECT_TRUE(isDataOk(p3, 4));

    writeTo(p1, 4);
    EXPECT_TRUE(isDataOk(p1, 4));
    EXPECT_TRUE(isDataOk(p3, 4));
}

TEST(Allocator, ReallocWithSwap) {
    char buffer[10] = {0};

    Allocator a(buffer, sizeof(buffer));
    Pointer p1 = a.alloc(4);
    writeTo(p1, 4);
    EXPECT_TRUE(isDataOk(p1, 4));

    Pointer p2 = a.alloc(1);
    writeTo(p2, 1);
    EXPECT_TRUE(isDataOk(p2, 1));

    Pointer p3 = a.alloc(3);
    writeTo(p3, 3);
    EXPECT_TRUE(isDataOk(p3, 3));

    a.free(p2);
    EXPECT_EQ(p2.get(), nullptr);

    void *ptr = p1.get();
    a.realloc(p1, 6);
    EXPECT_NE(p1.get(), ptr);
    EXPECT_TRUE(isDataOk(p1, 4));
    EXPECT_TRUE(isDataOk(p3, 3));

    writeTo(p1, 6);
    EXPECT_TRUE(isDataOk(p1, 6));
    EXPECT_TRUE(isDataOk(p3, 3));

}

