/*
 * Copyright 2010 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef PULSEAUDIOSOURCE_H
#define PULSEAUDIOSOURCE_H

#include <pulse/simple.h>

#include "Source.h"
#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Source component that reads data from a PulseAudio device.
     */
    class PulseAudioSource : public Source< CachedComponent<float> >
    {
    public:
        PulseAudioSource(const char* iObjectName = "PulseAudioSource");
        ~PulseAudioSource();
        void open(
            const char* iDeviceName,
            TimeType iBeginTime = -1,
            TimeType iEndTime = -1
        );

    protected:
        virtual SizeType contiguousFetch(
            IndexType iIndex, SizeType iLength, SizeType iOffset
        );

    private:
        pa_simple* mHandle;
        IndexType mMaxIndex;
    };
}

#endif /* PULSEAUDIOSOURCE_H */
