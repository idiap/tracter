/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SCREENSINK_H
#define SCREENSINK_H

#include <stdio.h>

#include <vector>

#include "UnarySink.h"
#include "ByteOrder.h"

/**
 * Sinks to screen.
 */
class ScreenSink : public UnarySink<float>
{
public:
    ScreenSink(Plugin<float>* iInput, const char* iObjectName = "ScreenSink");
    virtual ~ScreenSink() throw() {}
    void Open();

private:
    int mMaxSize;
};

#endif /* SCREENSINK_H */
