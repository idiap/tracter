/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef FILESOURCE_H
#define FILESOURCE_H

#include <cassert>

#include "Plugin.h"
#include "Source.h"
#include "MMap.h"

namespace Tracter
{
    /**
     * File source template
     * Reads raw files as file maps.
     */
    template <class T>
    class FileSource : public Source< Plugin<T> >
    {
    public:

        FileSource(const char* iObjectName = "FileSource")
        {
            Plugin<T>::mObjectName = iObjectName;
            Plugin<T>::mSampleFreq = Plugin<T>::GetEnv("SampleFreq", 8000.0f);
            Plugin<T>::mSamplePeriod = 1;
        }
        virtual ~FileSource() throw() {}

        virtual void Open(
            const char* iFileName,
            TimeType iBeginTime = 0,
            TimeType iEndTime = 0
        )
        {
            // The file map *is* the cache
            assert(iFileName);
            mCache = (T*)mMap.Map(iFileName);

            // Convert times to frames
            IndexType begin = Plugin<T>::FrameIndex(iBeginTime);
            IndexType end = Plugin<T>::FrameIndex(iEndTime);
            Plugin<T>::Verbose(1, "Begin: %ld  end: %ld\n", begin, end);

            // Fix the cache pointers to the given range
            int size = mMap.GetSize() / sizeof(T);
            Plugin<T>::mSize = size;
            Plugin<T>::mTail.index = 0;
            Plugin<T>::mTail.offset = begin;
            Plugin<T>::mHead.index =
                end ? std::min(size, (int)(end-begin+1)) : size;
            Plugin<T>::mHead.offset = 0;
        }

        T* GetPointer(int iOffset = 0)
        {
            return &mCache[iOffset];
        }

        virtual void Reset(bool iPropagate)
        {
            // Don't call the base class, don't reset the pointers
            return;
        }

    private:
        MMap mMap;
        T* mCache;

        /** It makes no sense to Resize a file source */
        void Resize(int iSize)
        {
            return;
        }

        virtual int Fetch(IndexType iIndex, CacheArea& iOutputArea)
        {
            // If this gets called by the base, it probably means
            // we're out of data
            assert(iIndex >= 0);
            return 0;
        }
    };
}

#endif /* FILESOURCE_H */
