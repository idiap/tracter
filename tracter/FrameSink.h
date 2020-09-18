/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef FRAMESINK_H
#define FRAMESINK_H

#include "Sink.h"

namespace Tracter
{
    /**
     * A FrameSink is a type of sink that just provides a method for
     * reading individual arrays from a graph.  i.e., it is an
     * interface between tracter and non-tracter-aware routines.  The
     * arrays could be feature vectors.
     */
    template <class T>
    class FrameSink : public Sink
    {
    public:

        /** Create a FrameSink with the input min-size set to 1 */
        FrameSink(Component<T>* iInput, const char* iObjectName = "FrameSink")
        {
            objectName(iObjectName);
            mInput = iInput;
            connect(mInput, 21);
            mFrame.size = iInput->frame().size;
            initialise();
            reset();
            verbose(1, "frame rate %f\n", frameRate());
        }

        /** Get the array with the given index. */
        const T* Read(
            int iIndex  ///< Index of the required array
        )
        {
            return mInput->unaryRead(iIndex);
        }

    private:
        Component<T>* mInput;

    };
}

#endif /* FRAMESINK_H */
