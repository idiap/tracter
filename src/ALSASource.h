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
    class ALSASource : public Source< CachedPlugin<short> >
    {
    public:
        ALSASource(const char* iObjectName = "ALSASource");
        ~ALSASource() throw();
        void Open(
            const char* iDeviceName,
            TimeType iBeginTime = -1,
            TimeType iEndTime = -1
        );
        void asyncCallback();

    protected:
        virtual int Fetch(IndexType iIndex, CacheArea& iOutputArea);

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
