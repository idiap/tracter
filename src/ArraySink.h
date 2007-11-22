#ifndef ARRAYSINK_H
#define ARRAYSINK_H

#include "UnarySink.h"

/**
 * An ArraySink is a type of sink that just provides a method for
 * reading individual arrays from a graph.  i.e., it is an interface
 * between tracter and non-tracter-aware routines.  The arrays could
 * be feature vectors.
 */
class ArraySink : public UnarySink<float>
{
public:

    /** Create an ArraySink with the input min-size set to 1 */
    ArraySink(Plugin<float>* iInput, const char* iObjectName = "ArraySink")
        : UnarySink<float>(iInput)
    {
        mObjectName = iObjectName;
        mArraySize = iInput->GetArraySize();
        MinSize(iInput, 1);        
    }

    /** Get the array with the given index.  Returns true if successful. */
    bool GetArray(
        float*& ioData, ///< Pointer to the returned data
        int iIndex      ///< Index of the required array
    )
    {
        CacheArea ca;
        int got = mInput->Read(ca, iIndex);
        if (got == 0)
        {
            ioData = 0;
            return false;
        }

        ioData = mInput->GetPointer(ca.offset);
        return true;
    }
};

#endif /* ARRAYSINK_H */
