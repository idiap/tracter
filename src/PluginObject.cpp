#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "PluginObject.h"

/**
 * Set a CacheArea to represent a particular range at a particular
 * offset.
 */
void CacheArea::Set(int iLength, int iOffset, int iSize)
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

PluginObject::PluginObject()
{
    mSize = 0;
    mArraySize = 0;
    mNInputs = 0;
    mHead.index = 0;
    mHead.offset = 0;
    mTail.index = 0;
    mTail.offset = 0;
    mDownStream = 0;
    mIndefinite = false;
}

/**
 * Pass back a minimum size instruction to an input plugin.  This
 * should be called by the derived class, which doesn't have
 * permission to call the input plugin directly.
 */
void PluginObject::MinSize(PluginObject* iObject, int iSize)
{
    assert(iObject);
    iObject->MinSize(iSize);
}

/**
 * Reset that is only allowed to propagate back from a single
 * downstream plugin This means that when several plugins use this one
 * as an input, it only gets reset once.
 */
void PluginObject::Reset(PluginObject* iDownStream)
{
    if (!mDownStream)
        // First time: Set the favoured downstream plugin to the caller
        mDownStream = iDownStream;
    else
        // Only proceed if the calling plugin is the favoured one
        if (mDownStream != iDownStream)
            return;

    Reset(true);
}

/**
 * This method actually resets the class
 */
void PluginObject::Reset(bool iPropagate)
{
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
 * Recursive delete
 */
bool PluginObject::Delete(PluginObject* iDownStream)
{
    // One reset cycle must have occured for this to work
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
 * Public interface to recursive delete
 * The sink plugin is not actually deleted (it would have to delete itself)
 */
void PluginObject::Delete()
{
    // The sink plugin should have no downstream favoured plugin
    assert(!mDownStream);
    mDownStream = this;
    Delete(this);
}

/**
 * Set the minimum size of this cache
 * A negative size means that the cache should grow indefinitely
 */
void PluginObject::MinSize(int iSize)
{
    // Only continue if it's not already set to grow indefinitely
    if (mIndefinite)
        return;

    if (iSize < 0)
    {
        // It's an indefinitely resizing cache
        mIndefinite = true;
        printf("Cache set to indefinite size\n");
    }
    else
    {
        // A fixed size cache
        assert(iSize > 0);
        if (iSize > mSize)
        {
            Resize(iSize);
            mSize = iSize;
        }
    }
}

/**
 * Dump basic data
 */
void PluginObject::Dump()
{
    printf("Tail index(%lu) offset(%u)  Head index(%lu) offset(%u)\n",
           mTail.index, mTail.offset, mHead.index, mHead.offset);
}

/**
 * This is the core of the cached plugin.  If data already exists it
 * just returns the cache location.  Otherwise it calls the processing
 * methods to actually calculate new data.
 */
int PluginObject::Read(CacheArea& oRange, IndexType iIndex, int iLength)
{
    assert(iLength >= 0);
    assert(iIndex >= 0);
    assert(mIndefinite || (iLength <= mSize));  // Request > cache size
    int len;

    // Case 0: For an indefinitely resizing cache, we just calculate
    // everything between the end of the cache and the end of the
    // requested range.
    if (mIndefinite)
    {
        if (iIndex + iLength >= mHead.index)
        {
            // Number of new samples required
            int proc = iIndex + iLength - mHead.index;
            Resize(mSize + proc);
            mSize += proc;
            CacheArea area;
            area.Set(proc, mHead.offset, mSize);
            len = Process(mHead.index, area);
            mHead.index += len;
            mHead.offset += len;
            len = iLength - proc + len;
        }
        else
        {
            // As Case 3 below: The requested range is within the
            // current cache range.  Don't need to calculate anything
            len = iLength;
        }
        oRange.Set(len, iIndex, mSize);
        return len;
    }

    // Case 1: Cache empty, or there is some discontinuity after the
    // end of the cache Start again from the start of the cache
    if ((mHead.index == mTail.index) || (iIndex > mHead.index))
    {
        oRange.Set(iLength, 0, mSize);
        len = Process(iIndex, oRange);
        if (len < iLength)
            oRange.Set(len, 0, mSize);
        mHead.index = iIndex + len;
        mHead.offset = len;
        if (mHead.offset >= mSize)
            mHead.offset -= mSize;
        mTail.index = iIndex;
        mTail.offset = 0;
        return len;
    }

    // iIndex is either contiguous, or inside the cache range
    else if ((iIndex >= mTail.index) && (iIndex <= mHead.index))
    {
        // Case 2: It's an extension of the current range
        if (iIndex + iLength >= mHead.index)
        {
            // Number of new samples required
            int proc = iLength - (mHead.index - iIndex);
            CacheArea area;
            area.Set(proc, mHead.offset, mSize);
            len = Process(mHead.index, area);
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
            len = iLength - proc + len;
        }
        else
        {
            // Case 3: The requested range is within the current cache
            // range Don't need to calculate anything
            len = iLength;
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
    printf("PluginObject: Backwards cache access, data lost\n");
    oRange.Set(0, 0, mSize);

    return 0;
}

/**
 * Default process loop, called when a downstream plugin requests data
 * via Read() that is not cached.
 *
 * This is a convenience method that breaks down a single call for
 * contiguous data into N calls for the N distinct frames.  The
 * implementation has the choice about whether to override this call
 * or implement ProcessFrame().
 */
int PluginObject::Process(IndexType iIndex, CacheArea& iOutputArea)
{
    assert(iIndex >= 0);

    // Split it into N *contiguous* calls.
    int offset = iOutputArea.offset;
    for (int i=0; i<iOutputArea.Length(); i++)
    {
        if (i == iOutputArea.len[0])
            offset = 0;
        if (!ProcessFrame(iIndex++, offset++))
            return i;
    }
    return iOutputArea.Length();
}

/**
 * The implementation has a choice about whether to implement this or
 * just Process().  If it does neither then this default will just
 * bomb out.
 */
bool PluginObject::ProcessFrame(IndexType iIndex, int iOffset)
{
    printf("PluginObject: ProcessFrame called.  This should not happen.\n");
    exit(EXIT_FAILURE);
    return false;
}
