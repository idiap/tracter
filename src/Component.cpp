/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <climits>
#include <cstdarg>
#include "Component.h"

/**
 * Set a CacheArea to represent a particular range at a particular
 * offset.
 */
void Tracter::CacheArea::Set(int iLength, int iOffset, int iSize)
{
    assert(iLength >= 0);
    assert(iOffset >= 0);
    assert(iSize >= 0);
    assert(iOffset < iSize);

    offset = iOffset;
    len[0] = iLength;
    len[1] = 0;
    if (iOffset + iLength > iSize)
    {
        len[1] = iOffset + iLength - iSize;
        len[0] = iLength - len[1];
    }
}

Tracter::ComponentBase::ComponentBase()
{
    mObjectName = 0;
    mSize = 0;
    
    mFrame.period = 1.0f;
    mFrame.size = 1;

    mNOutputs = 0;
    mDownStream = 0;
    mIndefinite = false;
    mMinSize = 0;
    mNInitialised = 0;
    mMinReadAhead = INT_MAX;
    mMaxReadAhead = 0;
    mMinReadBehind = INT_MAX;
    mMaxReadBehind = 0;
    mTotalReadAhead = 0;
    mTotalReadBehind = 0;

    mAsync = false;
    mAuxiliary = 0;
    mEndOfData = -1;
    mDot = -1;
    SetClusterSize(1);
}

void Tracter::ComponentBase::SetClusterSize(int iSize)
{
    assert(iSize > 0);
    mCluster.resize(iSize, ZEROPAIR);
}


/**
 * In fact, this doesn't connect anything as such.  It updates
 * input and output reference counts so that:
 *
 * 1. Inputs can be accessed as mInput[n]
 * 2. Components with multiple outputs can respond to read-aheads by
 * increasing buffer sizes.
 *
 * @returns iInput
 */
Tracter::ComponentBase*
Tracter::ComponentBase::Connect(ComponentBase* iInput, int iSize)
{
    assert(iInput);
    mInput.push_back(iInput);
    iInput->mNOutputs++;
    ReadRange rr(iSize);
    SetReadRange(iInput, rr);
    return iInput;
}

Tracter::ComponentBase*
Tracter::ComponentBase::Connect(ComponentBase* iInput, int iSize, int iReadAhead)
{
    assert(iInput);
    mInput.push_back(iInput);
    iInput->mNOutputs++;
    ReadRange rr(iSize, iReadAhead);
    SetReadRange(iInput, rr);
    return iInput;
}


/**
 * Pass back a minimum size instruction to an input component.  This
 * should be called by the derived class, which doesn't have
 * permission to call the input component directly.
 */
void Tracter::ComponentBase::MinSize(
    ComponentBase* iInput, int iMinSize, int iReadAhead
)
{
    assert(iInput);
    int readBack = 0;
    if (iMinSize > 0)
    {
        // i.e, it's not indefinitely resizing
        readBack = iMinSize - iReadAhead - 1;
    }
    iInput->MinSize(iMinSize, readBack, iReadAhead);
}

/**
 * Allows minsize to be set with specific read ahead and back that
 * don't necessarily add up properly.
 */
void Tracter::ComponentBase::MinSize(
    ComponentBase* iInput, int iMinSize, int iReadBehind, int iReadAhead
)
{
    assert(iInput);
    iInput->MinSize(iMinSize, iReadBehind, iReadAhead);
}


/**
 * Set the minimum size of this cache.  Called by each downstream
 * component.  A negative size means that the cache should grow
 * indefinitely
 */
void Tracter::ComponentBase::MinSize(
    int iMinSize, int iReadBehind, int iReadAhead
)
{
    // Keep track of the maximum read-ahead
    if (mMaxReadAhead < iReadAhead)
        mMaxReadAhead = iReadAhead;
    if (mMinReadAhead > iReadAhead)
        mMinReadAhead = iReadAhead;

    if (mMaxReadBehind < iReadBehind)
        mMaxReadBehind = iReadBehind;
    if (mMinReadBehind > iReadBehind)
        mMinReadBehind = iReadBehind;

    // Only continue if it's not already set to grow indefinitely
    if (mIndefinite)
        return;

    if (iMinSize == ReadRange::INFINITE)
    {
        // It's an indefinitely resizing cache
        mIndefinite = true;
        assert(mObjectName);
        Verbose(1, "cache set to indefinite size\n");
    }
    else
    {
        // A fixed size cache
        assert(iMinSize > 0);
        if (iMinSize > mMinSize)
        {
            mMinSize = iMinSize;
        }
    }
}

/**
 * Initialises read-ahead and read-back by passing back accumulated
 * values.  If a component has more than one output, it sizes the cache
 * to deal with the read-back and read-ahead.  This means that if one
 * branch reads ahead, the data is still around for the other branch
 * to fetch.
 *
 * The algorithm is roughly as follows:
 *
 * There is a local read associated with the immediate downstream
 * component(s) and and a global read associated with further downstream
 * components.  Components with only one output (immediate downstream
 * component) only need concern themselves with the local read.  Those
 * with more than one output need to take into account the global
 * read.
 *
 * Reads are accumulated as they are passed back through components.
 * This is the global read.
 *
 * Local reads are stored in the preceding (upstream) component.  This
 * means that a component with multiple inputs does not need to store
 * distinct reads for each input outside the constructor.  The
 * down-side is that the upstream component cannot distinguish different
 * reads for different immediate downstream components.
 *
 * Components know how many outputs are connected, but not what they are.
 * Each component waits for initialisation from each output until
 * propagating the initialisation to inputs.
 *
 * Aside, this is still a mess.  Some caches are too big.
 */
void* Tracter::ComponentBase::Initialise(
    const ComponentBase* iDownStream, int iReadBehind, int iReadAhead
)
{
    assert((mNOutputs == 0) /* Sink */ ||
           (mNInitialised < mNOutputs) /* Component */);

    // First time: Set the favoured downstream component to the caller
    if (!mDownStream)
        mDownStream = iDownStream;

    // Set to indefinite if necessary
    if (!mIndefinite && (mNOutputs > 1) && (iReadAhead < 0))
    {
        mIndefinite = true;
        Verbose(1, "ComponentBase::Initialise: cache set to indefinite size\n");
    }

    // Accumulate readahead and readback from all outputs
    mTotalReadAhead += iReadAhead;
    mTotalReadBehind += iReadBehind;

    mGlobalReadAhead.Update(iReadAhead);
    mGlobalReadBehind.Update(iReadBehind);

    Verbose(2, "ComponentBase::Initialise:"
            " i [%d:%d] m [%d,%d:%d,%d] tot [%d:%d]\n",
            iReadBehind, iReadAhead,
            mMinReadBehind, mMaxReadBehind, mMinReadAhead, mMaxReadAhead,
            mTotalReadBehind, mTotalReadAhead);
    Verbose(2, " grb: [%d,%d]  gra [%d,%d]\n",
            mGlobalReadBehind.min, mGlobalReadBehind.max,
            mGlobalReadAhead.min, mGlobalReadAhead.max);

    // If the accumulation is complete, then recurse the call
    if ((mNOutputs == 0) || (++mNInitialised == mNOutputs))
    {
        // Resize if necessary
        if (!mIndefinite)
        {
            // Add in the sizes of the previous components
            int readAhead = mTotalReadAhead + mMaxReadAhead;
            int readBack = mTotalReadBehind + mMaxReadBehind;
            int newSize = (mNOutputs > 1)
                ? readBack + 1 + readAhead
                : mMinSize;
            assert(newSize >= mMinSize);
            if (newSize > mSize)
            {
                Resize(newSize);
            }
        }

        // Recurse over *all* inputs
        for (int i=0; i<(int)mInput.size(); i++)
        {
            assert(mInput[i]);
            int readAhead = (mIndefinite || (iReadAhead < 0))
                ? -1
                : (int)(mFrame.period * (mMaxReadAhead+iReadAhead));
            int readBack  = (int)(mFrame.period * (mMaxReadBehind+iReadBehind));
            void* aux = mInput[i]->Initialise(this, readBack, readAhead);
            if (i == 0)
                mAuxiliary = aux;
            else
                if (mAuxiliary != aux)
                    throw Exception("Initialise: Mismatched aux. pointers");

        }
    }

    return mAuxiliary;
}


/**
 * Private reset that is only allowed to propagate back from a single
 * downstream component.  This means that when several components use this
 * one as an input, it only gets reset once.  Calls the public method,
 * which can be hooked.
 */
void Tracter::ComponentBase::Reset(
    ComponentBase* iDownStream ///< this pointer of calling class
)
{
    // Only proceed if the calling component is the favoured one
    if (mDownStream == iDownStream)
        Reset(true);
}

/**
 * Public reset method that actually resets the class.  This should be
 * called whenever necessary to reset the caches to index zero.  Each
 * component is only reset once in a recursive reset - the graph is not
 * expanded to a tree.
 */
void Tracter::ComponentBase::Reset(
    bool iPropagate ///< If true, recursively resets all input components
)
{
    mEndOfData = -1;
    mCluster.assign(mCluster.size(), ZEROPAIR);
    if (iPropagate)
        for (int i=0; i<(int)mInput.size(); i++)
        {
            assert(mInput[i]);
            mInput[i]->Reset(this);
        }
}

/**
 * Recursive delete.  Deletes all input components recursively, except
 * the first one, which is most likely a Sink.
 *
 * @returns true if the caller can delete the object
 */
bool Tracter::ComponentBase::Delete(ComponentBase* iDownStream)
{
    // Initialise must have occured for this to work
    if (!mDownStream)
        assert(0);

    // Return immediately if the caller is not the favoured component
    if (iDownStream != mDownStream)
        return false;

    // Loop backwards!
    // This causes non-favoured components to be deleted first so the method
    // doesn't get called on already deleted objects.
    for (int i=mInput.size()-1; i>=0; i--)
    {
        // Check if this input is a duplicate
        bool dup = false;
        for (size_t d=i+1; d<mInput.size(); d++)
            if (mInput[d] == mInput[i])
            {
                dup = true;
                break;
            }

        if(!dup && mInput[i]->Delete(this))
            delete mInput[i];
    }

    // Tell the caller that it can delete this object
    return true;
}

/**
 * Public interface to recursive delete.  This can only be called by a
 * Sink component (one that has no outputs).  The sink component itself is
 * not actually deleted (it would have to delete itself).  Do not use
 * this call either directly or via a Sink that does so if
 * intermediate components are allocated on the stack.
 */
void Tracter::ComponentBase::Delete()
{
    // The sink component should have no downstream favoured component
    assert(!mDownStream);
    mDownStream = this;
    Delete(this);
}

/**
 * Update a cachepointer.
 * Handles wraparound too.
 */
void Tracter::ComponentBase::MovePointer(CachePointer& iPointer, int iLen)
{
    iPointer.index += iLen;
    iPointer.offset += iLen;
    if (iPointer.offset >= mSize)
        iPointer.offset -= mSize;
}


/**
 * Read data from an input Component.  This is the core of the cached
 * component concept.  If data already exists it just returns the cache
 * location.  Otherwise it calls the Fetch() method to actually
 * calculate new data.
 *
 * @returns the number of data actually available.  It may be less
 * than the number requested.
 */
int Tracter::ComponentBase::Read(
    CacheArea& oRange, IndexType iIndex, int iLength
)
{
    Verbose(3, "Read: index %ld  length %d\n", iIndex, iLength);
    assert(iLength >= 0);
    assert(iIndex >= 0);
    assert(mIndefinite || (iLength <= mSize));  // Request > cache size
    int len;

    // Remember:
    //
    // head is the next location to write to (free space)
    CachePointer& head = mCluster[0].head;
    CachePointer& tail = mCluster[0].tail;

    // Case 0: For an indefinitely resizing cache, we just calculate
    // everything between the end of the cache and the end of the
    // requested range.
    if (mIndefinite)
    {
        IndexType finalIndex = iIndex + iLength - 1;
        if (finalIndex >= head.index)
        {
            // Number of new samples required
            int fetch = iIndex + iLength - head.index;
            assert(fetch > 0);
            if (mSize < iIndex + iLength)
                Resize(iIndex + iLength);
            CacheArea area;
            area.Set(fetch, head.offset, mSize);
            len = FetchWrapper(head.index, area);

            // Ugh, why was this if() here?  It causes the component
            // to stop short when the cache is indefinite.
            //if (len > 0)
            //{
                head.index += len;
                head.offset += len;
                len = iLength - fetch + len;
                assert(len >= 0);
            //}
        }
        else
        {
            // As Case 3 below: The requested range is within the
            // current cache range.  Don't need to calculate anything
            len = iLength;
            assert(len >= 0);
        }
        oRange.Set(len, iIndex, mSize);
        return len;
    }

    // Case 1: Cache empty, or there is some discontinuity after the
    // end of the cache.  Start again from the start of the cache
    if ((head.index == tail.index) || (iIndex > head.index))
    {
        oRange.Set(iLength, 0, mSize);
        len = FetchWrapper(iIndex, oRange);
        if (len == 0)
            // Don't mess up the cache if we were off the end
            return 0;
        if (len < iLength)
            oRange.Set(len, 0, mSize);
        if (!mAsync)
        {
            head.index = iIndex + len;
            head.offset = len;
            if (head.offset >= mSize)
                head.offset -= mSize;
            tail.index = iIndex;
            tail.offset = 0;
        }
        return len;
    }

    // iIndex is either contiguous, or inside the cache range
    else if ((iIndex >= tail.index) && (iIndex <= head.index))
    {
        IndexType finalIndex = iIndex + iLength - 1;
        if (finalIndex >= head.index)
        {
            // Case 2: It's an extension of the current range
            int fetch = iIndex + iLength - head.index;
            assert(fetch > 0);
            CacheArea area;
            area.Set(fetch, head.offset, mSize);
            len = FetchWrapper(head.index, area);
            if (!mAsync)
            {
                head.index += len;
                head.offset += len;
                if (head.offset >= mSize)
                    head.offset -= mSize;
                if (head.index - tail.index > mSize)
                {
                    int diff = head.index - tail.index - mSize;
                    tail.index += diff;
                    tail.offset += diff;
                    if (tail.offset >= mSize)
                        tail.offset -= mSize;
                }
            }
            len = iLength - fetch + len;
            assert(len >= 0);
        }
        else
        {
            // Case 3: The requested range is within the current cache
            // range.  Don't need to calculate anything
            len = iLength;
            assert(len >= 0);
        }

        // Whichever of cases 2 and 3, we need to fix up the output
        // range
        int offset = tail.offset + (iIndex - tail.index);
        if (offset >= mSize)
            offset -= mSize;
        oRange.Set(len, offset, mSize);
        return len;
    }

    // Otherwise (case 4) the request was for lost data
    throw Exception("%s: ComponentBase: Backwards cache access, data lost\n"
                    "Head = %ld  Tail = %ld  Request index = %ld\n",
                    mObjectName, head.index, tail.index, iIndex);

    return 0;
}

/**
 * A Fetch() wrapper.  Read() will call this rather blindly.  Here we
 * take care of the EOD flag, preventing unnecessary reads past EOD.
 * This way, components only need to return EOD once.
 *
 * A Read() will never actually pass on a request past EOD to the
 * Fetch() because, in finding out EOD is reached, data up until then
 * should have been read (TODO: unless it's non-contiguous, but we can
 * deal with that with a flag later).  So, if EOD is set, all requests
 * here will be after it.
 */
int Tracter::ComponentBase::FetchWrapper(
    IndexType iIndex, CacheArea& iOutputArea
)
{
    /*
     */
    if ((mEndOfData >= 0) && (iIndex >= mEndOfData))
        return 0;

    int len = Fetch(iIndex, iOutputArea);
    if (len < iOutputArea.Length())
    {
        mEndOfData = iIndex + len;
        Verbose(2, "EOD at index %ld, got %d of %d\n",
                mEndOfData, len, iOutputArea.Length());
    }
    return len;
}

/**
 * Fetch() is called when a downstream component requests data via
 * Read(), and the requested data is not cached.
 *
 * If not overridden by a derived class, ComponentBase supplies a
 * Fetch() that that breaks down a single call for contiguous data in
 * data space into two distinct calls to ContiguousFetch() (contiguous
 * in memory).  Each component has the choice about whether to override
 * Fetch() or implement ContiguousFetch() or UnaryFetch().  It is
 * generally easier to implement one of the latter two, but it may be
 * quite inefficient for high frequency samples.
 *
 * @returns the number of data actually available.  It may be less
 * than the number requested, implying end of data (EOD).
 */
int Tracter::ComponentBase::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    assert(iIndex >= 0);

    // Split it into two contiguous calls.
    int len = 0;
    len += ContiguousFetch(iIndex, iOutputArea.len[0], iOutputArea.offset);
    if (len < iOutputArea.len[0])
        return len;
    len += ContiguousFetch(iIndex+len, iOutputArea.len[1], 0);

    return len;
}

/**
 * ContiguousFetch in this class should never be called
 */
int Tracter::ComponentBase::ContiguousFetch(
    IndexType iIndex, int iLength, int iOffset
)
{
    throw Exception("%s: ComponentBase::ContiguousFetch called", mObjectName);
}


/**
 * Get an absolute time stamp for the given index.
 *
 * A time stamp is a 64 bit signed integer counting nanoseconds.  This
 * is based on the type used in ASIO.
 *
 * This call is recursive; it calls itself on the first input until a
 * source returns a time for index 0.  It then calls TimeOffset() to
 * get a time for the given frame and adds them.
 */
Tracter::TimeType Tracter::ComponentBase::TimeStamp(IndexType iIndex) const
{
    if (mInput.size() == 0)
        throw Exception("TimeStamp: No inputs."
                        "  %s probably missing TimeStamp()", mObjectName);
    TimeType time = mInput[0]->TimeStamp();
    if (iIndex)
        time += TimeOffset(iIndex);
    return time;
}

/**
 * Calculate a relative time offset
 *
 * Given a frame index, uses the frame period and frequency to return
 * the time of that index from the point of view of index 0 being time
 * 0.
 */
Tracter::TimeType Tracter::ComponentBase::TimeOffset(IndexType iIndex) const
{
    // There is undoubtedly a right way to do this.  This may not be
    // it.
    TimeType t = (TimeType)((double)iIndex * mFrame.period * 1e9 / FrameRate());
    return t;
}


/**
 * Generate a dot graph
 */
void Tracter::ComponentBase::Dot()
{
    printf("digraph tracter {\n");
    //printf("rankdir=LR;\n");
    Dot(0);
    printf("}\n");
}

/**
 * Generate a dot graph
 *
 * Recursive part of the call that takes an initial node index,
 * returning it's own node index and the maximum index on that branch.
 */
Tracter::ComponentBase::DotInfo
Tracter::ComponentBase::Dot(int iDot)
{
    if (mDot >= 0)
    {
        DotInfo d = {mDot, mDot};
        return d;
    }

    mDot = iDot;
    printf("%d [shape=record, label=\"{%s", mDot, mObjectName);
    if (-sVerbose > 0)
        printf("}|{");
    DotRecord(2, "frame.size=%d", mFrame.size);
    DotRecord(2, "frame.period=%d", mFrame.period);
    DotHook();
    printf("}\"];\n");
    int max = mDot;
    for (int i=0; i<(int)mInput.size(); i++)
    {
        ComponentBase* p = mInput[i];
        DotInfo d = p->Dot(max+1);
        max = std::max(d.max, max);
        printf("  %d -> %d", d.index, mDot);
        if (mInput.size() > 1)
            printf(" [headlabel=\"%d\"]", i);
        printf(";\n");
    }
    DotInfo d = {mDot, max};
    return d;
}

void Tracter::ComponentBase::DotRecord(int iVerbose, const char* iString, ...)
{
    if (iVerbose > -sVerbose)
        return;
    va_list ap;
    va_start(ap, iString);
    vprintf(iString, ap);
    va_end(ap);
    printf("\\l");
}
