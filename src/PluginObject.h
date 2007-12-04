/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef PLUGINOBJECT_H
#define PLUGINOBJECT_H

namespace Tracter {
    extern bool sShowConfig;
    extern int sVerbose;
}

/**
 * @brief A contiguous area of interest of a circular cache.
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

struct CachePointer
{
    IndexType index;
    int offset;
};

/**
 * @brief The type independent root of all plugins.
 *
 * PluginObject is designed to be inherited by a type specific
 * implementation.
 */
class PluginObject
{
public:
    PluginObject(void);
    virtual ~PluginObject(void) {};
    
    /* Dumps the contents of the class */
    void Dump();
    int Read(CacheArea& oArea, IndexType iIndex, int iLength = 1);
    virtual void Reset(bool iPropagate = true);
    void Delete();

    /** Returns the array size of the cache.  Can be called by a
     * downstream plugin to set its own default size. */
    int GetArraySize()
    {
        return mArraySize;
    }

    void SetArraySize(int iSize)
    {
        assert(iSize >= 0);
        mArraySize = iSize;
    }

protected:
    void MinSize(PluginObject* iObject, int iSize, int iReadAhead = 0);
    void ReadAhead(int iReadAhead = 0);

    float GetEnv(const char* iSuffix, float iDefault);
    int GetEnv(const char* iSuffix, int iDefault);
    const char* GetEnv(const char* iSuffix, const char* iDefault);
    PluginObject* Connect(PluginObject* iInput);

    virtual void MinSize(int iSize, int iReadAhead);
    virtual void Resize(int iSize) = 0;
    virtual int Fetch(IndexType iIndex, CacheArea& iOutputArea);
    virtual bool UnaryFetch(IndexType iIndex, int iOffset);
    virtual PluginObject* GetInput(int iInput) { return 0; }


    const char* mObjectName; ///< Name of this object

    int mSize;          ///< Size of the cache
    int mArraySize;     ///< Size of each cache element
    int mNInputs;       ///< Number of inputs
    int mNOutputs;      ///< Number of outputs
    bool mIndefinite;   ///< If true, cache grows indefinitely
    CachePointer mHead; ///< Next position to write to
    CachePointer mTail; ///< Oldest position written to
    int mReadAhead;     ///< Maximum read-ahead of output buffers

    float mSampleFreq;  ///< The source sample frequency in Hertz
    int mSamplePeriod;  ///< Integer sample period of this plugin

private:
    const char* getEnv(const char* iSuffix, const char* iDefault);
    void Reset(PluginObject* iDownStream);
    bool Delete(PluginObject* iDownStream);
    PluginObject* mDownStream;
};

#endif /* PLUGINOBJECT_H */
