#ifndef ALSASOURCE_H
#define ALSASOURCE_H

#include <alsa/asoundlib.h>
#include "CachedPlugin.h"

/**
 * ALSASource
 *
 * Source plugin that reads data from an ALSA device.
 */
class ALSASource : public CachedPlugin<short>
{
public:
    ALSASource(const char* iDevice = "default");
    ~ALSASource();
    void Start();

protected:
    virtual int Process(IndexType iIndex, CacheArea& iOutputArea);

private:
    snd_output_t* mOutput;
    snd_pcm_t* mHandle;
    snd_pcm_status_t* mStatus;
    snd_pcm_uframes_t setHardwareParameters();
    void statusDump()
    {
        snd_pcm_status_dump(mStatus, mOutput);
    }
};

#endif /* ALSASOURCE_H */
