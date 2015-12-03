/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef HTKSINK_H
#define HTKSINK_H

#include <cstdio>
#include <vector>

#include "Component.h"
#include "Sink.h"
#include "ByteOrder.h"

namespace Tracter
{
    /**
     * Sinks to an HTK format parameter file.  The file is always written
     * big-endian.
     */
    class HTKSink : public Sink
    {
    public:
        HTKSink(Component<float>* iInput, const char* iObjectName = "HTKSink");
        virtual ~HTKSink() throw() {}
        void Open(const char* iFile);

    protected:
        void DotHook()
        {
            Sink::DotHook();
            DotRecord(1, "swap=%s", mByteOrder.WrongEndian() ? "yes" : "no");
            DotRecord(1, "parm=0x%x", mParmKind);
        }

    private:
        Component<float>* mInput;
        FILE* mFile;
        ByteOrder mByteOrder;
        std::vector<float> mTemp;

        /* Header */
        int mNSamples;
        int mSampPeriod;
        short mSampSize;
        short mParmKind;

        void WriteHeader(FILE* iFile);
    };
}

#endif /* HTKSINK_H */
