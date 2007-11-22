#ifndef LNASOURCE_H
#define LNASOURCE_H

#include "CachedPlugin.h"
#include "Source.h"
#include "MMap.h"


/**
 * Uses an LNA file as a source
 */
class LNASource : public CachedPlugin<float>, public Source
{
public:
    LNASource(const char* iObjectName = "LNASource");
    void Open(const char* iFileName);

private:
    MMap mMap;
    void* mMapData;
    int mMapSize;
    bool mLNA16;
    virtual bool UnaryFetch(IndexType iIndex, int iOffset);
};

#endif /* LNASOURCE_H */
