#ifndef HTKFILE_H
#define HTKFILE_H

#include "CachedPlugin.h"
#include "MMap.h"

/**
 * Plugin to deal with HTK feature files
 */
class HTKFile : public CachedPlugin<float>
{
public:
    HTKFile(int iArraySize);
    void Map(const char* iFileName);

private:
    MMap mMap;
    float* mMapData;
    IndexType mNSamples;
    virtual int Process(IndexType iIndex, CacheArea& iOutputArea);
};

#endif /* HTKFILE_H */
