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

#include "Component.h"

namespace Tracter
{
    /**
     * This is a type specific implementation of the component object with
     * cache storage.
     */
    template <class T>
    class CachedComponent : public Component<T>
    {
    public:
        virtual ~CachedComponent<T>() throw()
        {
            // Nothing to do
        }

        /**
         * Get a pointer to the cache
         * returns a reference.
         */
        T* GetPointer(int iIndex = 0)
        {
            return &mCache[Component<T>::mFrame.size
                           ? iIndex * Component<T>::mFrame.size
                           : iIndex];
        }

    protected:
        CachedComponent<T>()
        {
            // Nothing to do
        }

        virtual void Resize(int iSize)
        {
            assert(iSize > 0);
            assert(iSize > Component<T>::mSize);
            mCache.resize(Component<T>::mFrame.size
                          ? iSize * Component<T>::mFrame.size
                          : iSize);
            Component<T>::mSize = iSize;
            Verbose(2, "CachedComponent::Resize: %d to %d\n",
                    Component<T>::mSize, iSize);
        }

        virtual void DotHook()
        {
            Component<T>::DotHook();
            DotRecord(2, "cache.size=%d", Component<T>::mSize);
        }

    private:
        std::vector<T> mCache;
    };
}

#endif /* CACHEDPLUGIN_H */
