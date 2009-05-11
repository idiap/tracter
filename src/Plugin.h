/*
 * Copyright 2007,2008 by IDIAP Research Institute
 *                        http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef PLUGIN_H
#define PLUGIN_H

#include <cassert>

#include "PluginObject.h"

namespace Tracter
{
    /**
     * An interface for the type specific implementation of the plugin.
     * The implementation could be a cache or memory map.
     */
    template <class T>
    class Plugin : public PluginObject
    {
    public:
        /**
         * Get a pointer to the storage
         * returns a reference.
         */
        virtual T* GetPointer(int iIndex = 0) = 0;

        /**
         * Unary read
         *
         * If we only want to read one item, the syntax can be rather
         * more simple.
         */
        const T* UnaryRead(int iIndex)
        {
            assert(iIndex >= 0);

            // Convert to a full Read(), return null pointer on failure
            CacheArea inputArea;
            int got = Read(inputArea, iIndex);
            if (!got)
                return 0;

            // If successful, return the pointer
            return GetPointer(inputArea.offset);
        }

        /**
         * Contiguous read
         *
         * Many components can work well by breaking down a Read()
         * into a pair of contiguous reads.  In this case, we can
         * return a pointer straight away as with the UnaryRead().  It
         * must be called twice.
         */
        const T* ContiguousRead(int iIndex, int& ioLength)
        {
            assert(iIndex >= 0);

            // Convert to a full Read(), return null pointer on failure
            CacheArea inputArea;
            int got = Read(inputArea, iIndex, ioLength);
            if (!got)
                return 0;

            // If successful, return the pointer and the length
            ioLength = inputArea.len[0];
            return GetPointer(inputArea.offset);
        }

    protected:

    };


    /**
     * An iterator-like thing for caches.  This has the functionality of
     * an iterator, if not the standard interface.  It handles the
     * wrap-around nature of circular buffers.
     *
     * The implementation is rather ugly; favour the Unary/Contiguous
     * read methods if at all possible.
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
}

#endif /* PLUGIN_H */
