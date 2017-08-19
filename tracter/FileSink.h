/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef FILESINK_H
#define FILESINK_H

#include <cstdio>
#include <vector>

#include "ByteOrder.h"
#include "Sink.h"

namespace Tracter
{
    /**
     * Sinks to a file.
     */
    class FileSink : public Sink
    {
    public:
        FileSink(
            Component<float>* iInput, const char* iObjectName = "FileSink"
        );
        void open(const char* iFile=0);

    private:
        Component<float>* mInput;
        FILE* mFile;
        bool mBinary;
        ByteOrder mByteOrder;
        std::vector<float> mTemp;
        int mMaxSize;
    };
}

#endif /* FILESINK_H */
