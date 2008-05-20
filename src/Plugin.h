/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef PLUGIN_H
#define PLUGIN_H

#include <cassert>
#include "PluginObject.h"

/**
 * An interface for the type specific implementation of the plugin.
 * The implementation could be a cache or memory map.
 */
template <class T>
class Plugin : public PluginObject
{
public:
    virtual ~Plugin<T>()
    {
        // A kind of post-hoc check that the next plugin called Connect()
        assert(mNOutputs > 0);
    }

    /**
     * Get a pointer to the storage
     * returns a reference.
     */
    virtual T* GetPointer(int iIndex = 0) = 0;

protected:

};


/**
 * An iterator-like thing for caches.  This has the functionality of
 * an iterator, if not the standard interface.  It handles the
 * wrap-around nature of circular buffers
 */
template <class T>
class CacheIterator
{
public:
    CacheIterator(Plugin<T>* iPlugin, CacheArea& iCacheArea)
    {
        mPlugin = iPlugin;
        mCacheArea = iCacheArea;
        mOffset = iCacheArea.offset;
    }

    /**
     * Index operator.  Returns a (reference to) the Nth object.
     */
    T& operator[](int iIndex)
    {
        assert( mOffset >= mCacheArea.offset ||
                mOffset <  mCacheArea.len[1] );
        return mPlugin->GetPointer(mOffset)[iIndex];
    }

    /**
     * Dereference operator.  Returns a (reference to) the object.
     */
    T& operator*()
    {
        assert( mOffset >= mCacheArea.offset ||
                mOffset <  mCacheArea.len[1] );
        return *(mPlugin->GetPointer(mOffset));
    }

    /**
     * Prefix operator.  Increments the iterator in an efficient way.
     */
    CacheIterator<T>& operator++()
    {
        if (++mOffset >= mCacheArea.offset + mCacheArea.len[0])
            mOffset = 0;
        return *this;
    }

    /**
     * Postfix operator.  Beware; this has to copy the iterator in
     * order to return the old one.  It's quicker to pre-increment if
     * you can.
     */
    CacheIterator<T> operator++(int dummy)
    {
        CacheIterator<T> old = *this; // Copy
        if (++mOffset >= mCacheArea.offset + mCacheArea.len[0])
            mOffset = 0;
        return old;
    }

    void Reset()
    {
        mOffset = mCacheArea.offset;
    }

private:
    Plugin<T>* mPlugin;
    CacheArea mCacheArea;
    int mOffset;
};

#endif /* PLUGIN_H */
