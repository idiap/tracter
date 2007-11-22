#ifndef HTKSOURCE_H
#define HTKSOURCE_H

#include "CachedPlugin.h"
#include "ByteOrder.h"
#include "Source.h"
#include "MMap.h"

/**
 * Plugin to deal with HTK feature files
 */
class HTKSource : public CachedPlugin<float>, public Source
{
public:
    HTKSource(const char* iObjectName = "HTKSource");
    void Open(const char* iFileName);

private:
    ByteOrder mByteOrder;
    MMap mMap;
    float* mMapData;
    IndexType mNSamples;
    virtual int Fetch(IndexType iIndex, CacheArea& iOutputArea);
};

#endif /* HTKSOURCE_H */
