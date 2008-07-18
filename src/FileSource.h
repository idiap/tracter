/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef FILESOURCE_H
#define FILESOURCE_H

#include <stdlib.h>
#include <assert.h>
#include "Plugin.h"
#include "Source.h"
#include "MMap.h"

/**
 * File source template
 * Reads raw files as file maps.
 */
template <class T>
class FileSource : public Plugin<T>, public Tracter::Source
{
public:

    FileSource(const char* iObjectName = "FileSource")
    {
        Plugin<T>::mObjectName = iObjectName;
        Plugin<T>::mSampleFreq = Plugin<T>::GetEnv("SampleFreq", 8000.0f);
        Plugin<T>::mSamplePeriod = 1;
    }
    virtual ~FileSource() throw() {}

    virtual void Open(const char* iFileName)
    {
        // The file map *is* the cache
        assert(iFileName);
        mCache = (T*)mMap.Map(iFileName);
        Plugin<T>::mSize = mMap.GetSize() / sizeof(T);
        Plugin<T>::mTail.index = 0;
        Plugin<T>::mTail.offset = 0;
        Plugin<T>::mHead.index = Plugin<T>::mSize;
        Plugin<T>::mHead.offset = 0;
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
    T* mCache;

    void Resize(int iSize)
    {
        return;
    }

    virtual int Fetch(IndexType iIndex, CacheArea& iOutputArea)
    {
        // If this gets called by the base, it probably means we're out of data
        assert(iIndex >= 0);
        return 0;
    }
};

#endif /* FILESOURCE_H */
