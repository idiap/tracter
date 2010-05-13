/*
 * Copyright 2010 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SNDFILESINK_H
#define SNDFILESINK_H

#include <sndfile.h>

#include "Sink.h"

namespace Tracter
{
    /**
     * Sink to a SndFile.
     *
     * Reads data from a graph and writes to a file managed by
     * libsndfile.
     * See http://www.mega-nerd.com/libsndfile/api.html for the formats.
     */
    class SndFileSink : public Sink
    {
    public:
        SndFileSink(
            Component<float>* iInput, const char* iObjectName = "SndFileSink"
        );
        virtual ~SndFileSink() throw() { Delete(); }
        void Open(const char* iFile);

    private:
        Component<float>* mInput;
        SNDFILE* mSndFile;
        int mBlockSize;
        float mFrameRate;
        int mFormat;
    };
}

#endif /* SNDFILESINK_H */
