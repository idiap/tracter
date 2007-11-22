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

    /** A sink destructor calls a recursive delete of the whole graph */
    virtual ~UnarySink()
    {
        // Recursive delete
        Delete();
    }

    /** Initialises a single input */
    UnarySink<T>(Plugin<float>* iInput, int iArraySize = 0)
    {
        Connect(iInput);
        mInput = iInput;
        assert(iArraySize >= 0);
        mArraySize = iArraySize;
        //int tmp = iArraySize ? iArraySize : iInput->mArraySize;

        /* This is to ensure the downstream pointers are set */
        Reset();
        ReadAhead(0);
    }

protected:

    /** Return a pointer to the one and only input */
    PluginObject* GetInput(int iInput)
    {
        assert(iInput == 0);
        assert(mNInputs == 1);
        return mInput;
    }

    Plugin<float>* mInput;

};

#endif /* UNARYSINK_H */
