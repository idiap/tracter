/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef HTKSOURCE_H
#define HTKSOURCE_H

#include "CachedComponent.h"
#include "ByteOrder.h"
#include "Source.h"
#include "MMap.h"

namespace Tracter
{
    /**
     * Component to deal with HTK feature files
     */
    class HTKSource : public Source< CachedComponent<float> >
    {
    public:
        HTKSource(const char* iObjectName = "HTKSource");
        void open(
            const char* iFileName,
            TimeType iBeginTime = -1,
            TimeType iEndTime = -1
        );

    private:
        ByteOrder mByteOrder;
        MMap mMap;
        float* mMapData;
        IndexType mNSamples;
        virtual SizeType fetch(IndexType iIndex, CacheArea& iOutputArea);

        IndexType mBeginFrame;
        IndexType mEndFrame;
    };
}

#endif /* HTKSOURCE_H */
