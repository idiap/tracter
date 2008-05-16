/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef UNARYSINK_H
#define UNARYSINK_H

#include "Sink.h"
#include "Plugin.h"

/**
 * A unary sink is a sink with just one input.
 */
template <class T>
class UnarySink : public Sink
{
public:

    /** A UnarySink destructor calls a recursive delete of the whole graph */
    virtual ~UnarySink()
    {
        Delete();
    }

    /** Initialises a single input */
    UnarySink<T>(Plugin<T>* iInput)
    {
        mObjectName = "UnarySink";
        Connect(iInput);
        mInput = iInput;
    }

protected:

    /** Return a pointer to the one and only input */
    PluginObject* GetInput(int iInput)
    {
        assert(iInput == 0);
        assert(mNInputs == 1);
        return mInput;
    }

    Plugin<T>* mInput;

};

#endif /* UNARYSINK_H */
