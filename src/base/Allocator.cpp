#include <stdlib.h>
#include <iostream>

#include "base/Allocator.h"

Allocator* Allocator::mAllocator = NULL;

void* Allocator::allocate(uint32_t size)
{
    return getInstance()->alloc(size);
}

void Allocator::deallocate(void* p, uint32_t size)
{
    getInstance()->dealloc(p, size);
}

Allocator* Allocator::getInstance()
{
    if(!mAllocator)
        mAllocator = new Allocator();

    return mAllocator;
}

void* Allocator::alloc(uint32_t size)
{
    Obj* result;
    uint32_t index;

    MutexLockGuard mutexLockGuard(mMutex);

    /* 如果分配内存大于 MAX_BYTES，那么就直接通过 malloc 分配 */
    if(size > MAX_BYTES)
        return malloc(size);
    
    index = freelistIndex(size);
    result = mFreeList[index];

    /* 如果没有找到则重新分配内存 */
    if(!result)
    {
        void* r = refill(roundup(size));
        return r;
    }

    /* 找到了就从链表中删除内存块 */
    mFreeList[index] = result->next;

    return result;
}

void Allocator::dealloc(void* p, uint32_t size)
{
    Obj* obj = (Obj*)p;
    uint32_t index;

    MutexLockGuard mutexLockGuard(mMutex);

    /* 如果释放内存大于 MAX_BYTES，那么就直接通过 free 释放 */
    if(size > MAX_BYTES)
        free(p);

    index = freelistIndex(size); //获取该大小在freelist的下标

    /* 将内存块添加进链表中 */
    obj->next = mFreeList[index];
    mFreeList[index] = obj;
}

/* 重新分配内存 */
void* Allocator::refill(uint32_t bytes)
{
    int nobjs = 20;
    char* chunk = chunkAlloc(bytes, nobjs); //分配内存
    Obj* result;
    Obj* currentObj;
    Obj* nextObj;
    int i;
    uint32_t index;

    /* 如果只有一个节点，那么直接放回，不需要处理剩余内存 */
    if(1 == nobjs)
        return chunk;

    result = (Obj*)chunk;
    index = freelistIndex(bytes);
    mFreeList[index] = nextObj = (Obj*)(chunk + bytes);

    /* 将剩余内存连成链表 */
    for(i = 1; ; ++i)
    {
        currentObj = nextObj;
        nextObj = (Obj*)((char*)nextObj + bytes);
    
        if(nobjs-1 == i) //最后一个节点
        {
            currentObj->next = 0;
            break;
        }
        else
        {
            currentObj->next = nextObj;
        }
    }

    return result;
}

char* Allocator::chunkAlloc(uint32_t size, int& nobjs)
{
    char* result;
    uint32_t totalBytes = size * nobjs; //总共需求的内存
    uint32_t bytesLeft = mEndFree - mStartFree; //缓存块中剩余的内存大小

    if(bytesLeft > totalBytes) //如果缓存块的内存满足需求，则直接从缓存块中获取内存
    {
        result = mStartFree;
        mStartFree += totalBytes;
        return result;
    }
    else if(bytesLeft > size) //如果缓存块剩余大小大于一个节点的大小，则尽可能返回多个节点
    {
        nobjs = bytesLeft / size;
        totalBytes = size * nobjs;
        result = mStartFree;
        mStartFree += totalBytes;
        return result;
    }
    else
    {
        uint32_t bytesToGet = 2 * totalBytes + roundup(mHeapSize >> 4); //至少两倍增长
        
        if(bytesLeft > 0) //如果缓存块还剩余内存，那么它肯定可以插入到某个节点中
        {
            uint32_t index = freelistIndex(bytesLeft);
            ((Obj*)(mStartFree))->next = mFreeList[index];
            mFreeList[index] = (Obj*)mStartFree;
        }

        /* 重新申请内存 */
        mStartFree = (char*)malloc(bytesToGet);

        mHeapSize += bytesToGet;
        mEndFree = mStartFree + bytesToGet;

        /* 递归调用chunkAlloc，重新分配 */
        return chunkAlloc(size, nobjs);
    }
}