#ifndef _ALLOCATOR_H_
#define _ALLOCATOR_H_
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "base/Mutex.h"

class Allocator
{
public:
    enum {ALIGN = 8};
    enum {MAX_BYTES = 128};
    enum {NFREELISTS = MAX_BYTES / ALIGN};

    union Obj {
        union Obj* next;
        char data[1];
    };

    static void* allocate(uint32_t size);

    static void deallocate(void* p, uint32_t size);

private:
    Allocator() : mStartFree(NULL), mEndFree(NULL), mHeapSize(0)
    {
        mMutex = new Mutex;
        memset(mFreeList, 0, sizeof(mFreeList));
    };

    ~Allocator() {

    };

    static Allocator* getInstance();

    void* alloc(uint32_t size);
    void dealloc(void* p, uint32_t size);

    /* 获取指定字节数在自由链表的下标 */
    uint32_t freelistIndex(uint32_t bytes) {
        return (((bytes) + ALIGN-1) / ALIGN - 1);
    }

    /* 字节对齐 */
    uint32_t roundup(uint32_t bytes) {
        return (((bytes) + ALIGN-1) & ~(ALIGN - 1));
    }

    void *refill(uint32_t bytes);
    char* chunkAlloc(uint32_t size, int& nobjs);

private:
    static Allocator* mAllocator;

    Mutex* mMutex;

    /* 维护缓存块 */
    char* mStartFree;
    char* mEndFree;
    uint32_t mHeapSize;

    Obj* mFreeList[NFREELISTS];

};

#endif //_ALLOCATOR_H_