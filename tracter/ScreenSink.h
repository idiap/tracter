/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SCREENSINK_H
#define SCREENSINK_H

#include <cstdio>
#include <vector>

#include "Component.h"
#include "Sink.h"
#include "ByteOrder.h"

namespace Tracter
{
    /**
     * Sinks to screen.
     */
    class ScreenSink : public Sink
    {
    public:
        ScreenSink(
            Component<float>* iInput, const char* iObjectName = "ScreenSink"
        );
        virtual ~ScreenSink() throw() {}
        void Open(const char* iFile=0);

    private:
        Component<float>* mInput;
        int mMaxSize;
    };
}

#endif /* SCREENSINK_H */
