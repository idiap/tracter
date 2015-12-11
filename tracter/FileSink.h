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
        virtual ~FileSink() throw() {}
        void Open(const char* iFile);
        
        void setFeatureIndice(int indice) {
            mFeatureIndice = indice;
        }

    private:
        Component<float>* mInput;
        FILE* mFile;
        ByteOrder mByteOrder;
        std::vector<float> mTemp;
        int mMaxSize;
        int mFeatureIndice;
    };
}

#endif /* FILESINK_H */
