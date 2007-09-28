#ifndef PLUGINOBJECT_H
#define PLUGINOBJECT_H

/**
 * Represents an area of a cache buffer.  Typically this is a
 * sub-array, but also deals with the case where the sub-array wraps
 * around the end of the circular buffer.
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
 * PluginObject
 *
 * This is the type independent part of the plugin.  It is typically
 * inherited by a type specific implementation.
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

    // Returns the step size, if any, of the cache
    int GetArraySize()
    {
        return mArraySize;
    }

protected:
    void MinSize(PluginObject* iObject, int iSize);

    virtual void MinSize(int iSize);
    virtual void Resize(int iSize) = 0;
    virtual int Process(IndexType iIndex, CacheArea& iOutputArea);
    virtual bool ProcessFrame(IndexType iIndex, int iOffset);
    virtual PluginObject* GetInput(int iInput) { return 0; }

    int mSize;
    int mArraySize;
    int mNInputs;
    bool mIndefinite;
    CachePointer mHead; // Next position to write to
    CachePointer mTail; // Oldest position written to

private:
    void Reset(PluginObject* iDownStream);
    bool Delete(PluginObject* iDownStream);
    PluginObject* mDownStream;
};

#endif /* PLUGINOBJECT_H */
