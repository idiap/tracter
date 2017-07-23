/*
 * Copyright 2007 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef FILESOURCE_H
#define FILESOURCE_H

#include <cassert>

#include "Component.h"
#include "Source.h"
#include "MMap.h"

namespace Tracter
{
    /**
     * File source template
     * Reads raw files as file maps.
     */
    template <class T>
    class FileSource : public Source< Component<T> >
    {
    public:

        FileSource(const char* iObjectName = "FileSource")
        {
            Component<T>::objectName(iObjectName);
            this->mFrameRate = Component<T>::config("FrameRate", 8000.0f);
            Component<T>::mFrame.size = Component<T>::config("FrameSize", 1);
            Component<T>::mFrame.period = 1;
        }
        virtual ~FileSource() throw() {}

        virtual void Open(
            const char* iFileName,
            TimeType iBeginTime = -1,
            TimeType iEndTime = -1
        )
        {
            // The file map *is* the cache
            assert(iFileName);
            mCache = (T*)mMap.Map(iFileName);

            // Convert times to frames
            IndexType begin =
                (iBeginTime >= 0) ? Component<T>::FrameIndex(iBeginTime) : 0;
            IndexType end =
                (iEndTime   >= 0) ? Component<T>::FrameIndex(iEndTime) : -1;
            Component<T>::Verbose(1, "Begin: %ld  end: %ld\n", begin, end);

            // Fix the cache pointers to the given range
            assert(Component<T>::mFrame.size);
            SizeType size =
                mMap.Size() / (Component<T>::mFrame.size * sizeof(T));
            CachePointer& head = Component<T>::mCluster[0].head;
            CachePointer& tail = Component<T>::mCluster[0].tail;

            Component<T>::mSize = size;
            tail.index = 0;
            tail.offset = begin;
            head.index =
                (end >= 0) ? std::min(size, (SizeType)(end-begin+1)) : size;
            head.offset = 0;

        }

        T* GetPointer(SizeType iOffset = 0)
        {
            assert(Component<T>::mFrame.size);
            return &mCache[iOffset * Component<T>::mFrame.size];
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
        void Resize(SizeType iSize)
        {
            return;
        }

        virtual SizeType Fetch(IndexType iIndex, CacheArea& iOutputArea)
        {
            // If this gets called by the base, it probably means
            // we're out of data
            assert(iIndex >= 0);
            return 0;
        }
    };
}

#endif /* FILESOURCE_H */
