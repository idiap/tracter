/*
 * Copyright 2007,2008 by IDIAP Research Institute
 *                        http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef COMPONENT_H
#define COMPONENT_H

#include <cassert>
#include <climits>
#include <vector>
#include <algorithm>

#include "tracter/Object.h"

namespace Tracter
{
    typedef char BoolType;        ///< Tracter bool - vector<bool> is special
    typedef long long IndexType;  ///< Type of a frame index (64 bit)
    typedef long SizeType;        ///< Type of a (cache) size

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
        SizeType offset;
        SizeType len[2];

        SizeType length() const
        {
            return len[0] + len[1];
        };

        void set(SizeType iLength, SizeType iOffset, SizeType iSize);
    };

    /** Information about frame size and period */
    struct FrameInfo
    {
        int size;      ///< Size of each cache frame
        float period;  ///< Frame period of this component
    };

    /** Index / Offset pair for internal cache management */
    struct CachePointer
    {
        IndexType index;     ///< Index in logical space
        SizeType offset;     ///< Index in physical space
    };

    /** Head-tail pair of cache pointers */
    struct CachePair
    {
        CachePointer head;   ///< Next position to write to
        CachePointer tail;   ///< Oldest position written to
    };

    static const CachePair ZEROPAIR = {
        {0, 0},
        {0, 0}
    };

    /** Range that a Read() can access */
    class ReadRange
    {
    public:
        static const SizeType INFINITE = LONG_MAX;
        ReadRange(SizeType iSize = 1)
        {
            mSize = iSize;
            mReadAhead = iSize - 1;
        }

        ReadRange(
            SizeType iSize,     ///< Size of the read window
            SizeType iReadAhead ///< How far ahead in time
        )
        {
            mSize = iSize;
            mReadAhead = iReadAhead;
        }
        SizeType size() const { return mSize; }
        SizeType ahead() const { return mReadAhead; }
        SizeType behind() const
        {
            return mSize == INFINITE ? INFINITE : mSize - mReadAhead - 1;
        }
    private:
        SizeType mSize;
        SizeType mReadAhead;
    };

    /* Time type */
    typedef long long TimeType;  ///< 64 bit type like ASIOTimeStamp
    const TimeType ONEe3 = 1000;
    const TimeType ONEe6 = 1000000;
    const TimeType ONEe9 = 1000000000;

    /** Frame rate type */
    struct ExactRateType
    {
        float rate;   ///< Original rate at the source
        float period; ///< Accumulated period
    };

    /**
     * The type independent root of all components.
     *
     * ComponentBase is designed to be inherited by a type specific
     * implementation.
     */
    class ComponentBase : public Tracter::Object
    {
    public:
        ComponentBase(void);
        virtual ~ComponentBase(void) throw () {};

        SizeType Read(CacheArea& oArea, IndexType iIndex, SizeType iLength = 1);
        virtual void reset(bool iPropagate = true);
        void destruct();

        /** Access the Frame structure */
        const FrameInfo& frame() const
        {
            return mFrame;
        };

        /** Call a recursive chain that outputs a dot graph */
        void dot();

        /**
         * Frame rate as a float
         *
         * Common audio sample rates don't have 'nice' reciprocals.
         *
         * Originally, I had this concept of there being a single
         * source rate, and all components running at some integer
         * divisor of that.  The problem is that you could have two
         * sources running at different rates, one being downsampled
         * to the the other's rate, then another component connecting
         * to both.
         *
         * So now the rate is a recursive thing.  It could be just a
         * simple recursion, or more complicated components could faff
         * with it more.
         */
        virtual float frameRate() const
        {
            ExactRateType r = exactFrameRate();
            return r.rate / r.period;
        }

        /** Exact frame rate as a source rate plus divisor */
        virtual ExactRateType exactFrameRate() const
        {
            if (mInput.size() <= 0)
                throw Exception("%s: ExactFrameRate: No inputs", objectName());
            assert(mInput[0]);
            ExactRateType r = mInput[0]->exactFrameRate();
            r.period *= mInput[0]->frame().period;
            return r;
        }

        /** Time stamp for a given frame */
        virtual TimeType timeStamp(IndexType iIndex = 0) const;

        /**
         * Time in seconds
         *
         * Calls timeStamp(), which will return an absolute time.
         */
        double seconds(IndexType iIndex) const
        {
            return (double)timeStamp(iIndex) * 1.0e-9;
        };

        /**
         * Convert time to Frame index
         *
         * This is a relative call; time zero is frame zero.
         */
        IndexType frameIndex(TimeType iTime)
        {
            return (IndexType)(iTime * frameRate() / ONEe9);
        }

    protected:

        /**
         * Set the range that will be read from an input component.
         *
         * This should be called by the derived class, which doesn't
         * have permission to call the input component directly.
         */
        void setReadRange(ComponentBase* iInput, const ReadRange& iReadRange)
        {
            assert(iInput);
            iInput->minSize(
                iReadRange.size(), iReadRange.behind(), iReadRange.ahead()
            );
        }

        void setClusterSize(int iSize);

        void minSize(
            ComponentBase* iObject, SizeType iMinSize, SizeType iReadAhead = 0
        );
        void minSize(
            ComponentBase* iInput, SizeType iMinSize,
            SizeType iReadBehind, SizeType iReadAhead
        );
        void* initialise(
            const ComponentBase* iDownStream = 0,
            SizeType iReadBehind = 0, SizeType iReadAhead = 0
        );
        ComponentBase* connect(ComponentBase* iInput, SizeType iSize = 1);
        ComponentBase* connect(ComponentBase* iInput,
                              SizeType iSize, SizeType iReadAhead);
        void movePointer(CachePointer& iPointer, SizeType iLen);

        virtual void minSize(
            SizeType iMinSize, SizeType iReadBehind, SizeType iReadAhead
        );
        virtual void resize(SizeType iSize) = 0;
        virtual SizeType Fetch(IndexType iIndex, CacheArea& iOutputArea);
        virtual SizeType contiguousFetch(
            IndexType iIndex, SizeType iLength, SizeType iOffset
        );

        /*
         * float mSampleFreq -> mFrame.rate -> frameRate()
         * int mSamplePeriod -> mFrame.period
         * int mArraySize    -> mFrame.size
         */
        FrameInfo mFrame;
        std::vector<ComponentBase*> mInput; ///< Array of input components

        std::vector<CachePair> mCluster; ///< Circular cache maintainance

        SizeType mSize;       ///< Size of the cache counted in frames
        int mNOutputs;        ///< Number of outputs
        bool mIndefinite;     ///< If true, cache grows indefinitely
        bool mAsync;          ///< Flag that the cache is updated asynchronously
        void* mAuxiliary;     ///< Common object for each component chain
        IndexType mEndOfData; ///< Index of the last datum available
        int mNInitialised;    ///< No. of outputs that have called initialise()

        SizeType mMinSize;         ///< Maximum requested minimum size
        SizeType mMaxReadAhead;    ///< Maximum read-ahead of output buffers
        SizeType mMinReadAhead;
        SizeType mMaxReadBehind;
        SizeType mMinReadBehind;
        SizeType mTotalReadAhead;
        SizeType mTotalReadBehind;

        virtual void dotHook() {}
        void dotRecord(int iverbose, const char* iString, ...);

        /** Does exactly what it says on the tin */
        SizeType secondsToFrames(float iSeconds) const
        {
            float samples = iSeconds * frameRate();
            return (SizeType)(samples + 0.5);
        }

        TimeType timeOffset(IndexType iIndex) const;

    private:
        SizeType fetchWrapper(IndexType iIndex, CacheArea& iOutputArea);
        void reset(ComponentBase* iDownStream);
        bool destruct(ComponentBase* iDownStream);

        const ComponentBase* mDownStream;

        int mDot;      ///< Dot index of this component
        struct DotInfo
        {
            int index; ///< Index of this component
            int max;   ///< Maximum upstream index
        };
        DotInfo dot(int iDot);
    };


    /**
     * An interface for the type specific implementation of the component.
     * The implementation could be a cache or memory map.
     */
    template <class T>
    class Component : public ComponentBase
    {
    public:
        /**
         * Get a pointer to the storage
         * returns a reference.
         */
        virtual T* getPointer(SizeType iIndex = 0) = 0;

        /**
         * Unary read
         *
         * If we only want to read one item, the syntax can be rather
         * more simple.
         */
        const T* unaryRead(IndexType iIndex)
        {
            assert(iIndex >= 0);

            // Convert to a full Read(), return null pointer on failure
            CacheArea inputArea;
            SizeType got = Read(inputArea, iIndex);
            if (!got)
                return 0;

            // If successful, return the pointer
            return getPointer(inputArea.offset);
        }

        /**
         * Contiguous read
         *
         * Many components can work well by breaking down a Read()
         * into a pair of contiguous reads.  In this case, we can
         * return a pointer straight away as with the unaryRead().  It
         * must be called twice.
         */
        const T* contiguousRead(IndexType iIndex, SizeType& ioLength)
        {
            assert(iIndex >= 0);

            // Convert to a full Read(), return null pointer on failure
            CacheArea inputArea;
            SizeType got = Read(inputArea, iIndex, ioLength);
            if (!got)
                return 0;

            // If successful, return the pointer and the length
            ioLength = inputArea.len[0];
            return getPointer(inputArea.offset);
        }

    protected:

        /**
         * contiguousFetch() is called by ComponentBase's
         * implementation of Fetch().  In turn, contiguousFetch()
         * calls unaryFetch().  A component should implement one of
         * these, depending on how it wishes to operate.  The latter
         * two are easier in that they are typed and supply pointers
         * directly.
         *
         * @returns the number of frames successfully fetched.  Fewer
         * than asked for indicates end of data (EOD).
         */
        virtual SizeType contiguousFetch(
            IndexType iIndex, SizeType iLength, SizeType iOffset
        )
        {
            assert(iIndex >= 0);
            assert(iLength >= 0);
            assert(iOffset >= 0);

            // Break the contiguous fetch into unary fetches
            for (SizeType i=0; i<iLength; i++)
            {
                T* output = getPointer(iOffset+i);
                if (!unaryFetch(iIndex+i, output))
                    return i;
            }

            return iLength;
        }

        /**
         * unaryFetch()   If a component does not implement Fetch() then it
         * must implement unaryFetch().  A unaryFetch() is only
         * required to return a single datum, but it may need to
         * Read() several input data to do so.
         *
         * @returns true if the fetch was successful, false otherwise,
         * implying end of data (EOD).
         */
        virtual bool unaryFetch(IndexType iIndex, T* oData)
        {
            throw Exception("Component::UnaryFetch called."
                            " %s missing fetch method?", objectName());
            return false;
        }
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
        CacheIterator(Component<T>* iComponent, CacheArea& iCacheArea)
        {
            mComponent = iComponent;
            mCacheArea = iCacheArea;
            mOffset = iCacheArea.offset;
        }

        /**
         * Index operator.  Returns a (reference to) the Nth object.
         */
        T& operator[](SizeType iIndex)
        {
            assert( mOffset >= mCacheArea.offset ||
                    mOffset <  mCacheArea.len[1] );
            return mComponent->getPointer(mOffset)[iIndex];
        }

        /**
         * Dereference operator.  Returns a (reference to) the object.
         */
        T& operator*()
        {
            assert( mOffset >= mCacheArea.offset ||
                    mOffset <  mCacheArea.len[1] );
            return *(mComponent->getPointer(mOffset));
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

        void reset()
        {
            mOffset = mCacheArea.offset;
        }

    private:
        Component<T>* mComponent;
        CacheArea mCacheArea;
        SizeType mOffset;
    };
}

#endif /* COMPONENT_H */
