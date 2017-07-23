/*
 * Copyright 2007 by IDIAP Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <algorithm>

#include <time.h>
#include <sys/time.h>

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
        throw Exception("ALSA error at %s line %d: %s:\n", \
                        __FILE__, __LINE__, snd_strerror(alsaErr)); \
    }

/**
 * ALSA Source
 *
 * The variable FrameSize is the ALSA frame size and the tracter frame
 * size.  It is effectively the number of channels.
 */
Tracter::ALSASource::ALSASource(const char* iObjectName)
{
    objectName(iObjectName);
    mFrameRate = config("FrameRate", 8000.0f);
    mFrame.size = config("FrameSize", 1);
    mFrame.period = 1;
    mHandle = 0;

    float seconds = config("BufferTime", 1.0f);
    int samples = SecondsToFrames(seconds);
    MinSize(this, samples);
    Verbose(1, "buffer set to %d samples\n", samples);

    /* Tell the ComponentBase that we will take care of the pointers */
    mAsync = true;

    /* Allocate space and output to stdout */
    ALSACheck( snd_pcm_status_malloc(&mStatus) );
    ALSACheck( snd_output_stdio_attach(&mOutput, stdout, 0) );

    /* Limit the time for which we can connect */
    float maxTime = config("MaxTime", 0.0f);
    mMaxIndex = SecondsToFrames(maxTime);
}

Tracter::ALSASource::~ALSASource() throw()
{
    snd_pcm_close(mHandle);
    snd_pcm_status_free(mStatus);
}

/** Pass the callback information to the asyncCallback() method */
void Tracter::ALSASource::staticCallback(snd_async_handler_t *iHandler)
{
    void* object = snd_async_handler_get_callback_private(iHandler);
    ((Tracter::ALSASource*)object)->asyncCallback();
}

void Tracter::ALSASource::asyncCallback()
{
    assert(mHandle);
    snd_pcm_sframes_t avail = snd_pcm_avail_update(mHandle);
    Verbose(3, "asyncCallback: avail = %ld\n", avail);
    if (avail < 0)
        throw Exception("Aaagh, avail < 0 %ld", avail);

    assert(mSize >= avail);
    assert(!mIndefinite);

    CachePointer& head = mCluster[0].head;
    CachePointer& tail = mCluster[0].tail;

    SizeType xrun = 0;
    SizeType len0 = mSize - head.offset;
    len0 = std::min((SizeType)avail, len0);
    if (len0 > 0)
        ALSACheck( snd_pcm_readi(mHandle, GetPointer(head.offset), len0) );
    if ((tail.offset >= head.offset) &&
        (head.index != tail.index) &&
        (tail.offset < head.offset + len0))
        xrun = head.offset + len0 - tail.offset;

    SizeType len1 = avail - len0;
    if (len1 > 0)
        ALSACheck( snd_pcm_readi(mHandle, GetPointer(0), len1) );
    if (xrun > 0)
        xrun += len1;
    else if ((tail.offset < len1))
        xrun = len1 - tail.offset;

    //printf("s = %d h = %d,%ld t = %d,%ld  len0 = %d len1 = %d xrun = %d\n",
    //       mSize, head.offset, head.index, tail.offset, tail.index,
    //       len0, len1, xrun);

    MovePointer(head, avail);
    if (xrun > 0)
        MovePointer(tail, xrun);
}

/**
 * Open an ALSA device.
 *
 * This currently uses the call snd_async_add_pcm_handler(), and seems
 * to work OK.  However, there is talk that this is not a 'good'
 * solution:
 * http://mailman.alsa-project.org/pipermail/alsa-devel/2008-May/008030.html
 * says that poll() would be better.  One day maybe...
 */
void Tracter::ALSASource::Open(
    const char* iDeviceName, TimeType iBeginTime, TimeType iEndTime
)
{
    assert(iDeviceName);

    if (mHandle)
        throw Tracter::Exception("ALSASource::Open: handle exists");

    /* Get a PCM handle and set it up */
    ALSACheck(
        snd_pcm_open(&mHandle, iDeviceName, SND_PCM_STREAM_CAPTURE, 0)
    );
    assert(snd_pcm_state(mHandle) == SND_PCM_STATE_OPEN);
    setHardwareParameters();

    /* Add the callback information */
    snd_async_handler_t* ahandler;
    ALSACheck(
        snd_async_add_pcm_handler(&ahandler, mHandle, staticCallback, this)
    );

    /* Start the PCM */
    ALSACheck( snd_pcm_start(mHandle) );
    assert(snd_pcm_state(mHandle) == SND_PCM_STATE_RUNNING);

    /* Set time */
    struct timeval tv;
    if (gettimeofday(&tv, 0))
        throw Exception("gettimeofday failed");
    mTime  = (TimeType)tv.tv_sec * ONEe9;
    mTime += (TimeType)tv.tv_usec * ONEe3;
    Verbose(1, "Time is %lld\n", mTime);

    if (sVerbose > 1)
        snd_pcm_dump(mHandle, mOutput);

    if (sVerbose > 0)
        statusDump();
}

snd_pcm_uframes_t Tracter::ALSASource::setHardwareParameters()
{
    /* Default values for parameters.  ALSA likes unsigned types, but
     * tracter avoids them. */
    int dir;
    unsigned int sampleRate = (unsigned int)mFrameRate;
    unsigned int nChannels = (unsigned int)mFrame.size;

    snd_pcm_uframes_t bufferSize;
    snd_pcm_uframes_t periodSize = 160;

    /* Allocate a structure and populate with possible values */
    snd_pcm_hw_params_t* hwparams;
    ALSACheck( snd_pcm_hw_params_malloc(&hwparams) );
    ALSACheck( snd_pcm_hw_params_any(mHandle, hwparams) );

    /* Refine the values */
    ALSACheck( snd_pcm_hw_params_set_access(mHandle, hwparams,
                                            SND_PCM_ACCESS_RW_INTERLEAVED) );
    ALSACheck( snd_pcm_hw_params_set_format(mHandle, hwparams,
                                            SND_PCM_FORMAT_S16_LE) );
    ALSACheck( snd_pcm_hw_params_set_channels(mHandle, hwparams, nChannels) );
    ALSACheck( snd_pcm_hw_params_set_rate_near(mHandle, hwparams,
                                               &sampleRate, 0) );
    ALSACheck( snd_pcm_hw_params_get_buffer_size_max(hwparams, &bufferSize) );
    ALSACheck( snd_pcm_hw_params_set_buffer_size(mHandle, hwparams,
                                                 bufferSize) );
    ALSACheck( snd_pcm_hw_params_set_period_size_near(mHandle, hwparams,
                                                      &periodSize, &dir) );
    if ((int)periodSize*2 > mSize)
    {
        Resize(periodSize*2);
    }

    if (sVerbose > 1)
        snd_pcm_hw_params_dump(hwparams, mOutput);

    /* Write the parameters to the card and free the space */
    ALSACheck( snd_pcm_hw_params(mHandle, hwparams) );
    snd_pcm_hw_params_free(hwparams);

    /* The above should bring the card state up to PREPARED */
    assert(snd_pcm_state(mHandle) == SND_PCM_STATE_PREPARED);

    return bufferSize;
}

Tracter::SizeType
Tracter::ALSASource::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    Verbose(3, "Fetch: requested: %d %d\n",
            iOutputArea.len[0], iOutputArea.len[1]);
    if (mMaxIndex)
        if (iIndex > mMaxIndex)
            return 0;

    /* The fetch is actually completely asynchronous, so just sleep
     * until the head pointer passes where we need to be */
    timespec req;
    req.tv_sec = 0;
    req.tv_nsec = 100000;
    CachePointer& head = mCluster[0].head;
    while (head.index < iIndex + iOutputArea.Length())
    {
        assert(snd_pcm_state(mHandle) == SND_PCM_STATE_RUNNING);
        struct timespec rem;
        nanosleep(&req, &rem);
    }

    // Done
    if (mMaxIndex)
        if (iIndex + iOutputArea.Length() > mMaxIndex)
            return mMaxIndex - iIndex;
    return iOutputArea.Length();
}
