/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef FILESINK_H
#define FILESINK_H

#include <stdio.h>

#include <vector>

#include "UnarySink.h"
#include "ByteOrder.h"

/**
 * Sinks to a file.
 */
class FileSink : public UnarySink<float>
{
public:
    FileSink(Plugin<float>* iInput, const char* iObjectName = "FileSink");
    virtual ~FileSink() throw() {}
    void Open(const char* iFile);

private:
    FILE* mFile;
    ByteOrder mByteOrder;
    std::vector<float> mTemp;
    int mMaxSize;
};

#endif /* FILESINK_H */
