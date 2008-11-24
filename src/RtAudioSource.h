/*
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef RTAUDIOSOURCE_H
#define RTAUDIOSOURCE_H

#include "RtAudio.h"
#include "Source.h"
#include "CachedPlugin.h"

namespace Tracter
{
    /**
     * Source plugin that reads data from an RtAudio stream.
     *
     * Should work for ASIO etc.  One drawback is that under ALSA it
     * uses the hardware directly rather than the pcm or plug.  If
     * that's an issue then use the ALSA source.  Or RtAudio via jack
     * maybe.
     */
    class RtAudioSource : public CachedPlugin<float>, public Source
    {
    public:
        RtAudioSource(const char* iObjectName = "RtAudioSource");
        ~RtAudioSource() throw() {}
        void Open(const char* iDeviceName);

    protected:
        virtual int Fetch(IndexType iIndex, CacheArea& iOutputArea);

    private:
        static int staticCallback(
            void *iOutputBuffer, void *iInputBuffer, unsigned int iNFrames,
            double iStreamTime, RtAudioStreamStatus iStatus, void *iUserData
        );
        int Callback(
            void *iOutputBuffer, void *iInputBuffer, unsigned int iNFrames,
            double iStreamTime, RtAudioStreamStatus iStatus
        );

        RtAudio mRtAudio;
    };
}

#endif /* RTAUDIOSOURCE_H */
