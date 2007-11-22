#ifndef CACHEDPLUGIN_H
#define CACHEDPLUGIN_H

#include <assert.h>
#include <stdio.h>

#include <vector>

#include "Plugin.h"

/**
 * This is a type specific implementation of the plugin object with
 * cache storage.
 */
template <class T>
class CachedPlugin : public Plugin<T>
{
public:
    virtual ~CachedPlugin<T>()
    {
        // Nothing to do
    }

    /**
     * Get a pointer to the cache
     * returns a reference.
     */
    T* GetPointer(int iIndex = 0)
    {
        return &mCache[Plugin<T>::mArraySize
                       ? iIndex * Plugin<T>::mArraySize
                       : iIndex];
    }

protected:
    CachedPlugin<T>()
    {
        // Nothing to do
    }

    virtual void Resize(int iSize)
    {
        assert(iSize > 0);
        assert(iSize > Plugin<T>::mSize);
        mCache.resize(Plugin<T>::mArraySize
                      ? iSize * Plugin<T>::mArraySize
                      : iSize);
        //printf("Resize from %d to %d with capacity %u\n",
        //       Plugin<T>::mSize, iSize, mCache.capacity());
    }

private:

    std::vector<T> mCache;
};

#endif /* CACHEDPLUGIN_H */
