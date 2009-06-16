/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef CACHEDPLUGIN_H
#define CACHEDPLUGIN_H

#include <cassert>
#include <vector>

#include "Plugin.h"

namespace Tracter
{
    /**
     * This is a type specific implementation of the plugin object with
     * cache storage.
     */
    template <class T>
    class CachedPlugin : public Plugin<T>
    {
    public:
        virtual ~CachedPlugin<T>() throw()
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
            Plugin<T>::mSize = iSize;
            Verbose(2, "CachedPlugin::Resize: %d to %d\n",
                    Plugin<T>::mSize, iSize);
        }

        virtual void DotHook()
        {
            Plugin<T>::DotHook();
            DotRecord(2, "cache.size=%d", Plugin<T>::mSize);
        }

    private:
        std::vector<T> mCache;
    };
}

#endif /* CACHEDPLUGIN_H */
