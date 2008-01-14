/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef ARRAYSINK_H
#define ARRAYSINK_H

#include "UnarySink.h"

/**
 * An ArraySink is a type of sink that just provides a method for
 * reading individual arrays from a graph.  i.e., it is an interface
 * between tracter and non-tracter-aware routines.  The arrays could
 * be feature vectors.
 */
template <class T>
class ArraySink : public UnarySink<T>
{
public:

    /** Create an ArraySink with the input min-size set to 1 */
    ArraySink(Plugin<T>* iInput, const char* iObjectName = "ArraySink")
        : UnarySink<T>(iInput)
    {
        UnarySink<T>::mObjectName = iObjectName;
        UnarySink<T>::mArraySize = iInput->GetArraySize();
        MinSize(iInput, 1);
    }

    /** Get the array with the given index.  Returns true if successful. */
    bool GetArray(
        T*& ioData, ///< Pointer to the returned data
        int iIndex  ///< Index of the required array
    )
    {
        CacheArea ca;
        int got = UnarySink<T>::mInput->Read(ca, iIndex);
        if (got == 0)
        {
            ioData = 0;
            return false;
        }

        ioData = UnarySink<T>::mInput->GetPointer(ca.offset);
        return true;
    }
};

#endif /* ARRAYSINK_H */
