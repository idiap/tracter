/*
 * Copyright 2008 by The University of Sheffield
 *
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SPEAKERIDSOCKETSOURCE_H
#define SPEAKERIDSOCKETSOURCE_H

#include "StreamSocketSource.h"

namespace Tracter
{
    class SpeakerIDSocketSource : public StreamSocketSource
    {
    public:
        SpeakerIDSocketSource(
            void* iAuxiliary = 0,
            const char* iObjectName = "SpeakerIDSocketSource"
        );

    protected:
        bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
        int mTimeOffset;
    };
}

#endif /* SPEAKERIDSOCKETSOURCE_H */
