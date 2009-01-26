/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cassert>
#include <climits>
#include "PluginObject.h"

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

Tracter::PluginObject::PluginObject()
{
    mObjectName = 0;
    mSize = 0;
    mArraySize = 0;
    mNInputs = 0;
    mNOutputs = 0;
    mHead.index = 0;
    mHead.offset = 0;
    mTail.index = 0;
    mTail.offset = 0;
    mDownStream = 0;
    mIndefinite = false;
    mMinSize = 0;
    mNInitialised = 0;
    mMinReadAhead = INT_MAX;
    mMaxReadAhead = 0;
    mMinReadBack = INT_MAX;
    mMaxReadBack = 0;
    mTotalReadAhead = 0;
    mTotalReadBack = 0;

    mSampleFreq = 0.0f;
    mSamplePeriod = 0;
    mAsync = false;
    mAuxiliary = 0;
    mEndOfData = -1;
}

/**
 * In fact, this doesn't connect anything as such.  It updates
 * input and output reference counts so that:
 *
 * 1. Inputs can be requested by GetInput()
 * 2. Plugins with multiple outputs can respond to read-aheads by
 * increasing buffer sizes.
 *
 * It also duplicates the sample frequency and period of the first
 * input plugin connected.
 *
 * @returns iInput
 */
Tracter::PluginObject* Tracter::PluginObject::Connect(PluginObject* iInput)
{
    assert(iInput);
    mNInputs++;
    iInput->mNOutputs++;
    if (mSamplePeriod == 0)
    {
        mSampleFreq = iInput->mSampleFreq;
        mSamplePeriod = iInput->mSamplePeriod;
        assert(mSamplePeriod);
    }
    return iInput;
}


/**
 * Pass back a minimum size instruction to an input plugin.  This
 * should be called by the derived class, which doesn't have
 * permission to call the input plugin directly.
 */
void Tracter::PluginObject::MinSize(
    PluginObject* iInput, int iMinSize, int iReadAhead
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
 * Allows minsize to be set with specify read ahead and back that
 * don't necessarily add up properly.
 */
void Tracter::PluginObject::MinSize(
    PluginObject* iInput, int iMinSize, int iReadBack, int iReadAhead
)
{
    assert(iInput);
    iInput->MinSize(iMinSize, iReadBack, iReadAhead);
}


/**
 * Set the minimum size of this cache.  Called by each downstream
 * plugin.  A negative size means that the cache should grow
 * indefinitely
 */
void Tracter::PluginObject::MinSize(
    int iMinSize, int iReadBack, int iReadAhead
)
{
    // Keep track of the maximum read-ahead
    if (mMaxReadAhead < iReadAhead)
        mMaxReadAhead = iReadAhead;
    if (mMinReadAhead > iReadAhead)
        mMinReadAhead = iReadAhead;

    if (mMaxReadBack < iReadBack)
        mMaxReadBack = iReadBack;
    if (mMinReadBack > iReadBack)
        mMinReadBack = iReadBack;

    // Only continue if it's not already set to grow indefinitely
    if (mIndefinite)
        return;

    if (iMinSize < 0)
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
 * values.  If a plugin has more than one output, it sizes the cache
 * to deal with the read-back and read-ahead.  This means that if one
 * branch reads ahead, the data is still around for the other branch
 * to fetch.
 *
 * The algorithm is roughly as follows:
 *
 * There is a local read associated with the immediate downstream
 * plugin(s) and and a global read associated with further downstream
 * plugins.  Plugins with only one output (immediate downstream
 * plugin) only need concern themselves with the local read.  Those
 * with more than one output need to take into account the global
 * read.
 *
 * Reads are accumulated as they are passed back through plugins.
 * This is the global read.
 *
 * Local reads are stored in the preceding (upstream) plugin.  This
 * means that a plugin with multiple inputs does not need to store
 * distinct reads for each input outside the constructor.  The
 * down-side is that the upstream plugin cannot distinguish different
 * reads for different immediate downstream plugins.
 *
 * Plugins know how many outputs are connected, but not what they are.
 * Each plugin waits for initialisation from each output until
 * propagating the initialisation to inputs.
 *
 * Aside, this is still a mess.  Some caches are too big.
 */
void* Tracter::PluginObject::Initialise(
    const PluginObject* iDownStream, int iReadBack, int iReadAhead
)
{
    assert((mNOutputs == 0) /* Sink */ ||
           (mNInitialised < mNOutputs) /* Plugin */);

    // First time: Set the favoured downstream plugin to the caller
    if (!mDownStream)
        mDownStream = iDownStream;

    // Set to indefinite if necessary
    if (!mIndefinite && (mNOutputs > 1) && (iReadAhead < 0))
    {
        mIndefinite = true;
        Verbose(1, "PluginObject::Initialise: cache set to indefinite size\n");
    }

    // Accumulate readahead and readback from all outputs
    mTotalReadAhead += iReadAhead;
    mTotalReadBack += iReadBack;

    mGlobalReadAhead.Update(iReadAhead);
    mGlobalReadBack.Update(iReadBack);

    Verbose(2, "PluginObject::Initialise:"
            " i [%d:%d] m [%d,%d:%d,%d] tot [%d:%d]\n",
            iReadBack, iReadAhead,
            mMinReadBack, mMaxReadBack, mMinReadAhead, mMaxReadAhead,
            mTotalReadBack, mTotalReadAhead);
    Verbose(2, " grb: [%d,%d]  gra [%d,%d]\n",
            mGlobalReadBack.min, mGlobalReadBack.max,
            mGlobalReadAhead.min, mGlobalReadAhead.max);

    // If the accumulation is complete, then recurse the call
    if ((mNOutputs == 0) || (++mNInitialised == mNOutputs))
    {
        // Resize if necessary
        if (!mIndefinite)
        {
            // Add in the sizes of the previous plugins
            int readAhead = mTotalReadAhead + mMaxReadAhead;
            int readBack = mTotalReadBack + mMaxReadBack;
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
        for (int i=0; i<mNInputs; i++)
        {
            PluginObject* input = GetInput(i);
            assert(input);
            int scale = mSamplePeriod / input->mSamplePeriod;
            int readAhead = (mIndefinite || (iReadAhead < 0))
                ? -1
                : (mMaxReadAhead+iReadAhead) * scale;
            int readBack  = (mMaxReadBack+iReadBack) * scale;
            void* aux = input->Initialise(this, readBack, readAhead);
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
 * downstream plugin.  This means that when several plugins use this
 * one as an input, it only gets reset once.  Calls the public method,
 * which can be hooked.
 */
void Tracter::PluginObject::Reset(
    PluginObject* iDownStream ///< this pointer of calling class
)
{
    // Only proceed if the calling plugin is the favoured one
    if (mDownStream == iDownStream)
        Reset(true);
}

/**
 * Public reset method that actually resets the class.  This should be
 * called whenever necessary to reset the caches to index zero.  Each
 * plugin is only reset once in a recursive reset - the graph is not
 * expanded to a tree.
 */
void Tracter::PluginObject::Reset(
    bool iPropagate ///< If true, recursively resets all input plugins
)
{
    mEndOfData = -1;
    mHead.index = 0;
    mHead.offset = 0;
    mTail.index = 0;
    mTail.offset = 0;
    if (iPropagate)
        for (int i=0; i<mNInputs; i++)
        {
            PluginObject* input = GetInput(i);
            assert(input);
            input->Reset(this);
        }
}

/**
 * Recursive delete.  Deletes all input plugins recursively, except
 * the first one, which is most likely a Sink.
 *
 * @returns true if the caller can delete the object
 */
bool Tracter::PluginObject::Delete(PluginObject* iDownStream)
{
    // Initialise must have occured for this to work
    if (!mDownStream)
        assert(0);

    // Return immediately if the caller is not the favoured plugin
    if (iDownStream != mDownStream)
        return false;

    // Loop backwards!
    // This causes non-favoured plugins to be deleted first so the method
    // doesn't get called on already deleted objects.
    for (int i=mNInputs-1; i>=0; i--)
    {
        PluginObject* input = GetInput(i);
        assert(input);
        if(input->Delete(this))
            delete input;
    }

    // Tell the caller that it can delete this object
    return true;
}

/**
 * Public interface to recursive delete.  This can only be called by a
 * Sink plugin (one that has no outputs).  The sink plugin itself is
 * not actually deleted (it would have to delete itself).  Do not use
 * this call either directly or via a Sink that does so if
 * intermediate plugins are allocated on the stack.
 */
void Tracter::PluginObject::Delete()
{
    // The sink plugin should have no downstream favoured plugin
    assert(!mDownStream);
    mDownStream = this;
    Delete(this);
}

/**
 * Update a cachepointer.
 * Handles wraparound too.
 */
void Tracter::PluginObject::MovePointer(CachePointer& iPointer, int iLen)
{
    iPointer.index += iLen;
    iPointer.offset += iLen;
    if (iPointer.offset >= mSize)
        iPointer.offset -= mSize;
}


/**
 * Read data from an input Plugin.  This is the core of the cached
 * plugin concept.  If data already exists it just returns the cache
 * location.  Otherwise it calls the Fetch() method to actually
 * calculate new data.
 *
 * @returns the number of data actually available.  It may be less
 * than the number requested.
 */
int Tracter::PluginObject::Read(
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
    // mHead is the next location to write to (free space)

    // Case 0: For an indefinitely resizing cache, we just calculate
    // everything between the end of the cache and the end of the
    // requested range.
    if (mIndefinite)
    {
        IndexType finalIndex = iIndex + iLength - 1;
        if (finalIndex >= mHead.index)
        {
            // Number of new samples required
            int fetch = iIndex + iLength - mHead.index;
            assert(fetch > 0);
            if (mSize < iIndex + iLength)
                Resize(iIndex + iLength);
            CacheArea area;
            area.Set(fetch, mHead.offset, mSize);
            len = FetchWrapper(mHead.index, area);
            if (len > 0)
            {
                mHead.index += len;
                mHead.offset += len;
                len = iLength - fetch + len;
                assert(len >= 0);
            }
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
    if ((mHead.index == mTail.index) || (iIndex > mHead.index))
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
            mHead.index = iIndex + len;
            mHead.offset = len;
            if (mHead.offset >= mSize)
                mHead.offset -= mSize;
            mTail.index = iIndex;
            mTail.offset = 0;
        }
        return len;
    }

    // iIndex is either contiguous, or inside the cache range
    else if ((iIndex >= mTail.index) && (iIndex <= mHead.index))
    {
        IndexType finalIndex = iIndex + iLength - 1;
        if (finalIndex >= mHead.index)
        {
            // Case 2: It's an extension of the current range
            int fetch = iIndex + iLength - mHead.index;
            assert(fetch > 0);
            CacheArea area;
            area.Set(fetch, mHead.offset, mSize);
            len = FetchWrapper(mHead.index, area);
            if (!mAsync)
            {
                mHead.index += len;
                mHead.offset += len;
                if (mHead.offset >= mSize)
                    mHead.offset -= mSize;
                if (mHead.index - mTail.index > mSize)
                {
                    int diff = mHead.index - mTail.index - mSize;
                    mTail.index += diff;
                    mTail.offset += diff;
                    if (mTail.offset >= mSize)
                        mTail.offset -= mSize;
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
        int offset = mTail.offset + (iIndex - mTail.index);
        if (offset >= mSize)
            offset -= mSize;
        oRange.Set(len, offset, mSize);
        return len;
    }

    // Otherwise (case 4) the request was for lost data
    throw Exception("%s: PluginObject: Backwards cache access, data lost\n"
                    "Head = %ld  Tail = %ld  Request index = %ld\n",
                    mObjectName, mHead.index, mTail.index, iIndex);

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
int Tracter::PluginObject::FetchWrapper(
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
        Verbose(2, "EOD at index %ld\n", mEndOfData);
    }
    return len;
}

/**
 * Fetch() is called when a downstream plugin requests data via
 * Read(), and the requested data is not cached.
 *
 * If not overridden by a derived class, PluginObject supplies a
 * Fetch() that that breaks down a single call for contiguous data
 * into N distinct calls to UnaryFetch().  Each plugin has the choice
 * about whether to override Fetch() or implement UnaryFetch().  It is
 * generally easier to implement UnaryFetch(), but it may be quite
 * inefficient for high frequency samples.
 *
 * @returns the number of data actually available.  It may be less
 * than the number requested.
 */
int Tracter::PluginObject::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    assert(iIndex >= 0);

    // Split it into N *contiguous* calls.
    int offset = iOutputArea.offset;
    for (int i=0; i<iOutputArea.Length(); i++)
    {
        if (i == iOutputArea.len[0])
            offset = 0;
        if (!UnaryFetch(iIndex++, offset++))
            return i;
    }
    return iOutputArea.Length();
}

/**
 * UnaryFetch() is called by PluginObject's implementation of Fetch().
 * If a plugin does not implement Fetch() then it must implement
 * UnaryFetch().  A UnaryFetch() is only required to return a single
 * datum, but it may need to Read() several input data to do so.
 *
 * @returns true if the fetch was successful, false otherwise,
 * implying that the chain is out of data.
 */
bool Tracter::PluginObject::UnaryFetch(IndexType iIndex, int iOffset)
{
    throw Exception("%s: PluginObject: UnaryFetch called."
                    "  This should not happen.", mObjectName);
    return false;
}

/**
 * Get a time stamp for the given index.
 *
 * A time stamp is a 64 bit signed integer counting nanoseconds.  This
 * is based on the type used in ASIO.
 */
Tracter::TimeType Tracter::PluginObject::TimeStamp(IndexType iIndex)
{
    if (mNInputs == 0)
        throw Exception("TimeStamp: No inputs."
                        "  %s probably missing TimeStamp()", mObjectName);
    PluginObject* input = GetInput(0);
    assert(input);
    TimeType time = input->TimeStamp();
    if (iIndex)
        time += TimeOffset(iIndex);
    return time;
}

/**
 * Calculate a time offset
 */
Tracter::TimeType Tracter::PluginObject::TimeOffset(IndexType iIndex)
{
    // There is undoubtedly a right way to do this.  This may not be
    // it.
    TimeType t = (TimeType)((double)iIndex * mSamplePeriod * 1e9 / mSampleFreq);
    return t;
}
