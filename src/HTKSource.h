/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef HTKSOURCE_H
#define HTKSOURCE_H

#include "CachedPlugin.h"
#include "ByteOrder.h"
#include "Source.h"
#include "MMap.h"

namespace Tracter
{
    /**
     * Plugin to deal with HTK feature files
     */
    class HTKSource : public Source< CachedPlugin<float> >
    {
    public:
        HTKSource(const char* iObjectName = "HTKSource");
        virtual ~HTKSource() throw() {}
        void Open(const char* iFileName);

    private:
        ByteOrder mByteOrder;
        MMap mMap;
        float* mMapData;
        IndexType mNSamples;
        virtual int Fetch(IndexType iIndex, CacheArea& iOutputArea);
    };
}

#endif /* HTKSOURCE_H */
