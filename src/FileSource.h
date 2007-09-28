#ifndef FILESOURCE_H
#define FILESOURCE_H

#include <stdlib.h>
#include <assert.h>
#include "Plugin.h"
#include "MMap.h"

/**
 * File source template
 * Reads raw files as file maps.
 */
template <class T>
class FileSource : public Plugin<T>
{
public:
    ~FileSource<T>()
    {
    }

    virtual void Map(const char* iFileName)
    {
        // The file map *is* the cache
        assert(iFileName);
        mCache = (T*)mMap.Map(iFileName);
        Plugin<T>::mSize = mMap.GetSize() / sizeof(T);
        Plugin<T>::mTail.index = 0;
        Plugin<T>::mTail.offset = 0;
        Plugin<T>::mHead.index = Plugin<T>::mSize;
        Plugin<T>::mHead.offset = 0;
        printf("FileSource: Mapped size %u\n", mMap.GetSize());
    }

    T* GetPointer(int iIndex = 0)
    {
        return &mCache[iIndex];
    }

    virtual void Reset(bool iPropagate)
    {
        // Don't call the base class, don't reset the pointers
        return;
    }

private:
    MMap mMap;
    short* mCache;

    void Resize(int iSize)
    {
        return;
    }

    virtual int Process(IndexType iIndex, CacheArea& iOutputArea)
    {
        // If this gets called by the base, it probably means we're out of data
        assert(iIndex >= 0);
        printf("File out of data\n");
        return 0;
    }
};

#endif /* FILESOURCE_H */
