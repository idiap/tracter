/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef ALSASOURCE_H
#define ALSASOURCE_H

#include <alsa/asoundlib.h>
#include "Source.h"
#include "CachedPlugin.h"

namespace Tracter
{
    /**
     * Source plugin that reads data from an ALSA device.
     */
    class ALSASource : public CachedPlugin<short>, public Source
    {
    public:
        ALSASource(const char* iObjectName = "ALSASource");
        ~ALSASource() throw();
        void Open(const char* iDeviceName);
        void asyncCallback();

    protected:
        virtual int Fetch(IndexType iIndex, CacheArea& iOutputArea);

        /** Diverts basic time stamp requests to the Source base class */
        virtual TimeType TimeStamp(IndexType iIndex)
        {
             return Source::TimeStamp(iIndex);
        }

    private:
        snd_output_t* mOutput;
        snd_pcm_t* mHandle;
        snd_pcm_status_t* mStatus;

        static void staticCallback(snd_async_handler_t *iHandler);
        snd_pcm_uframes_t setHardwareParameters();
        void statusDump()
        {
            snd_pcm_status_dump(mStatus, mOutput);
        }
    };
}

#endif /* ALSASOURCE_H */
