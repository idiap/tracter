/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef PLUGINOBJECT_H
#define PLUGINOBJECT_H

#include <climits>
#include <algorithm>

#include "TracterObject.h"

namespace Tracter
{
    /**
     * A contiguous area of interest of a circular cache.
     *
     * Represents an area of a cache buffer.  Typically this is a
     * sub-array, but also deals with the case where the sub-array wraps
     * around the end of the circular buffer.  i.e., it is contiguous in
     * data space, but not necessarily in memory.
     */
    class CacheArea
    {
    public:
        int offset;
        int len[2];

        int Length() const
        {
            return len[0] + len[1];
        };

        void Set(int iLength, int iOffset, int iSize);
    };

    typedef long IndexType;
    typedef long long TimeType;  ///< 64 bit type like ASIOTimeStamp

    /** Index / Offset pair for internal cache management */
    struct CachePointer
    {
        IndexType index;
        int offset;
    };

    /** Storage of minimum / maximum */
    class MinMax
    {
    public:
        /** Storage */
        int min;
        int max;

        /** Constructor */
        MinMax()
        {
            min = INT_MAX;
            max = INT_MIN;
        }

        /** Add new datum */
        void Update(int iVal)
        {
            min = std::min(min, iVal);
            max = std::max(min, iVal);
        }
    };

    /**
     * The type independent root of all plugins.
     *
     * PluginObject is designed to be inherited by a type specific
     * implementation.
     */
    class PluginObject : public Tracter::Object
    {
        friend class Source;

    public:
        PluginObject(void);
        virtual ~PluginObject(void) throw () {};

        int Read(CacheArea& oArea, IndexType iIndex, int iLength = 1);
        virtual void Reset(bool iPropagate = true);
        void Delete();

        virtual TimeType TimeStamp(IndexType iIndex = 0);

        /** Time in seconds */
        double Seconds(IndexType iIndex)
        {
            return (double)TimeStamp(iIndex) * 1.0e-9;
        };

        /** Returns the array size of the cache. */
        int GetArraySize()
        {
            return mArraySize;
        }

        /** Get the sample frequency of this plugin */
        float GetSampleFreq()
        {
            return mSampleFreq;
        }

    protected:
        void MinSize(PluginObject* iObject, int iMinSize, int iReadAhead = 0);
        void MinSize(
            PluginObject* iInput, int iMinSize, int iReadBack, int iReadAhead
        );
        void* Initialise(
            const PluginObject* iDownStream = 0,
            int iReadBack = 0, int iReadAhead = 0
        );
        PluginObject* Connect(PluginObject* iInput);
        void MovePointer(CachePointer& iPointer, int iLen);

        virtual void MinSize(int iMinSize, int iReadBack, int iReadAhead);
        virtual void Resize(int iSize) = 0;
        virtual int Fetch(IndexType iIndex, CacheArea& iOutputArea);
        virtual bool UnaryFetch(IndexType iIndex, int iOffset);
        virtual PluginObject* GetInput(int iInput) { return 0; }

        TimeType TimeOffset(IndexType iIndex);

        int mSize;          ///< Size of the cache counted in frames
        int mArraySize;     ///< Size of each cache frame
        int mNInputs;       ///< Number of inputs
        int mNOutputs;      ///< Number of outputs
        bool mIndefinite;   ///< If true, cache grows indefinitely
        CachePointer mHead; ///< Next position to write to
        CachePointer mTail; ///< Oldest position written to
        int mMinSize;       ///< Maximum requested minimum size
        bool mAsync;        ///< Flag that the cache is updated asynchronously

        int mNInitialised;
        int mMaxReadAhead;     ///< Maximum read-ahead of output buffers
        int mMinReadAhead;
        int mMaxReadBack;
        int mMinReadBack;
        int mTotalReadAhead;
        int mTotalReadBack;

        MinMax mGlobalReadAhead;
        MinMax mGlobalReadBack;

        float mSampleFreq;  ///< The source sample frequency in Hertz
        int mSamplePeriod;  ///< Integer sample period of this plugin

        int SecondsToSamples(float iSeconds)
        {
            float samples =  iSeconds * mSampleFreq / mSamplePeriod;
            return (int)(samples + 0.5);
        }

        void* mAuxiliary; ///< Common object for each component chain

    private:
        void Reset(PluginObject* iDownStream);
        bool Delete(PluginObject* iDownStream);
        const PluginObject* mDownStream;
    };
}
#endif /* PLUGINOBJECT_H */
