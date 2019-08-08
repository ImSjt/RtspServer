#ifndef _BUFFER_H_
#define _BUFFER_H_
#include <stdlib.h>
#include <algorithm>
#include <stdint.h>
#include <assert.h>



class Buffer
{
public:
    static const int initialSize;

    explicit Buffer() :
        mBufferSize(initialSize),
        mReadIndex(0),
        mWriteIndex(0)
    {
        mBuffer = (char*)malloc(mBufferSize);
    }

    ~Buffer()
    {
        free(mBuffer);
    }

    int readableBytes() const
    { return mWriteIndex - mReadIndex; }

    int writableBytes() const
    { return mBufferSize - mWriteIndex; }

    int prependableBytes() const
    { return mReadIndex; }

    char* peek()
    { return begin() + mReadIndex; }

    const char* peek() const
    { return begin() + mReadIndex; }

    const char* findCRLF() const
    {
        const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findCRLF(const char* start) const
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    const char* findLastCrlf() const
    {    
        const char* crlf = std::find_end(peek(), beginWrite(), kCRLF, kCRLF+2);
        return crlf == beginWrite() ? NULL : crlf;
    }

    void retrieve(int len)
    {
        assert(len <= readableBytes());
        if (len < readableBytes())
        {
            mReadIndex += len;
        }
        else
        {
            retrieveAll();
        }
    }

    void retrieveUntil(const char* end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    void retrieveAll()
    {
        mReadIndex = 0;
        mWriteIndex = 0;
    }

    char* beginWrite()
    { return begin() + mWriteIndex; }

    const char* beginWrite() const
    { return begin() + mWriteIndex; }

    void hasWritten(int len)
    {
        assert(len <= writableBytes());
        mWriteIndex += len;
    }

    void unwrite(int len)
    {
        assert(len <= readableBytes());
        mWriteIndex -= len;
    }

    /* 确保有足够的空间 */
    void ensureWritableBytes(int len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    void makeSpace(int len)
    {
        if (writableBytes() + prependableBytes() < len) //如果剩余空间不足
        {
            /* 扩大空间 */            
            mBufferSize = mWriteIndex+len;
            mBuffer = (char*)realloc(mBuffer, mBufferSize);
        }
        else //剩余空间足够
        {
            /* 移动内容 */
            int readable = readableBytes();
            std::copy(begin()+mReadIndex,
                    begin()+mWriteIndex,
                    begin());
            mReadIndex = 0;
            mWriteIndex = mReadIndex + readable;
            assert(readable == readableBytes());
        }
    }

    void append(const char* data, int len)
    {
        ensureWritableBytes(len); //调整空间
        std::copy(data, data+len, beginWrite()); //拷贝数据
        hasWritten(len); //重新调节写位置
    }

    void append(const void* data, int len)
    {
        append((const char*)(data), len);
    }

    int read(int fd);
    int write(int fd);

private:
    char* begin()
    { return mBuffer; }

    const char* begin() const
    { return mBuffer; }

private:
    char* mBuffer;
    int mBufferSize;
    int mReadIndex;
    int mWriteIndex;

    static const char* kCRLF;
};

#endif //_BUFFER_H_