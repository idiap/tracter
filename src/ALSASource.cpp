/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "ALSASource.h"

/*
 * Error handler
 * Most ALSA functions return something that this can handle.
 */
static int alsaErr;
#define ALSACheck(f) \
    alsaErr = f; \
    if (alsaErr < 0) \
    { \
        printf("ALSA error at %s line %d: %s:\n", \
               __FILE__, __LINE__, snd_strerror(alsaErr)); \
        exit(1); \
    }

ALSASource::ALSASource(const char* iDevice)
{
    assert(iDevice);

    /* Allocate space and initialise a handle */
    ALSACheck( snd_pcm_status_malloc(&mStatus) );
    ALSACheck( snd_output_stdio_attach(&mOutput, stdout, 0) );
    ALSACheck( snd_pcm_open(&mHandle, iDevice, SND_PCM_STREAM_CAPTURE, 0) );
    assert(snd_pcm_state(mHandle) == SND_PCM_STATE_OPEN);

    setHardwareParameters();
}

ALSASource::~ALSASource()
{
    snd_pcm_status_free(mStatus);
}

snd_pcm_uframes_t ALSASource::setHardwareParameters()
{
    /* Default values for parameters */
    int dir;
    unsigned int sampleRate = 11025;
    snd_pcm_uframes_t bufferSize;
    snd_pcm_uframes_t periodSize = 256;

    /* Allocate a structure and populate with possible values */
    snd_pcm_hw_params_t* hwparams;
    ALSACheck( snd_pcm_hw_params_malloc(&hwparams) );
    ALSACheck( snd_pcm_hw_params_any(mHandle, hwparams) );

    /* Refine the values */
    ALSACheck( snd_pcm_hw_params_set_access(mHandle, hwparams,
                                            SND_PCM_ACCESS_RW_INTERLEAVED) );
    ALSACheck( snd_pcm_hw_params_set_format(mHandle, hwparams,
                                            SND_PCM_FORMAT_S16_LE) );
    ALSACheck( snd_pcm_hw_params_set_channels(mHandle, hwparams, 1) );
    ALSACheck( snd_pcm_hw_params_set_rate_near(mHandle, hwparams,
                                               &sampleRate, 0) );
    ALSACheck( snd_pcm_hw_params_get_buffer_size_max(hwparams, &bufferSize) );
    ALSACheck( snd_pcm_hw_params_set_buffer_size(mHandle, hwparams,
                                                 bufferSize) );
    ALSACheck( snd_pcm_hw_params_set_period_size_near(mHandle, hwparams,
                                                      &periodSize, &dir) );

    /* Write the parameters to the card and free the space */
    ALSACheck( snd_pcm_hw_params(mHandle, hwparams) );
    snd_pcm_hw_params_free(hwparams);

    /* The above should bring the card state up to PREPARED */
    assert(snd_pcm_state(mHandle) == SND_PCM_STATE_PREPARED);
    return bufferSize;
}

void ALSASource::Start()
{
    /* Start the PCM */
    ALSACheck( snd_pcm_start(mHandle) );
    assert(snd_pcm_state(mHandle) == SND_PCM_STATE_RUNNING);

    /* Cycle the MMAP calls once to get the Source location */
    //const snd_pcm_channel_area_t* area;
    //snd_pcm_uframes_t offset, frames;
    //ALSACheck( snd_pcm_mmap_begin(mHandle, &area, &offset, &frames) );
    //ALSACheck( snd_pcm_mmap_commit(mHandle, offset, 0) );
    //mData = (short*)area[0].addr;
    //assert(area[0].step == 16);

    // info
    //printf("Begin; step = %u, offset = %lu, frames = %lu\n", area[0].step, offset, frames);
    printf("State: %s\n", snd_pcm_state_name(snd_pcm_state(mHandle)));
    statusDump();
}

//
// This is really crap for at least two reasons:
// 1. It's request driven, so dangerous if the ALSA buffer is too small
// 2. The skipping actually reads rather than ignores
//
#ifndef Min
# define Min(a, b) ((a<b) ? (a) : (b))
#endif
int ALSASource::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    printf("ALSASource::Fetch\n");
    snd_pcm_sframes_t avail;

    // However unlikely, it's possible that the read is ahead of the
    // current sample.  In this case, we need to skip samples.
    int skip = iIndex - mHead.index;
    printf("Skipping %d samples:", skip);
    while (skip > 0)
    {
        static short skipBuffer[1024];
        ALSACheck( snd_pcm_wait(mHandle, -1) );
        avail = snd_pcm_avail_update(mHandle);
        int read = Min(skip, avail);
        read = Min(read, 1024);
        ALSACheck( snd_pcm_readi(mHandle, skipBuffer, read) );
        skip -= read;
        assert(skip >= 0);
        printf(" %d", read);
    }
    printf("\n");

    // After possible skipping, read the number of samples required
    while ((avail = snd_pcm_avail_update(mHandle)) < iOutputArea.Length())
        ALSACheck( snd_pcm_wait(mHandle, -1) );

    printf("Avail: %ld  requested: %d %d\n",
           avail, iOutputArea.len[0], iOutputArea.len[1]);

    ALSACheck(snd_pcm_readi(mHandle, GetPointer(iOutputArea.offset),
                            iOutputArea.len[0]) );
    if (iOutputArea.len[1])
        ALSACheck( snd_pcm_readi(mHandle, GetPointer(), iOutputArea.len[1]) );

    // Done
    return iOutputArea.Length();
}
