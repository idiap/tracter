/*
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <algorithm>

#include "RtAudioSource.h"

Tracter::RtAudioSource::RtAudioSource(const char* iObjectName)
{
    mObjectName = iObjectName;
    mSampleFreq = GetEnv("SampleFreq", 8000.0f);
    mSamplePeriod = 1;

    float seconds = GetEnv("BufferTime", 1.0f);
    int samples = SecondsToSamples(seconds);
    MinSize(this, samples);
    Verbose(1, "buffer set to %d samples\n", samples);

    /* Tell the PluginObject that we will take care of the pointers */
    mAsync = true;
}


/** Pass the callback information to the Callback() method */
int Tracter::RtAudioSource::staticCallback(
    void *iOutputBuffer, void *iInputBuffer, unsigned int iNFrames,
    double iStreamTime, RtAudioStreamStatus iStatus, void *iUserData
)
{
    return ((Tracter::RtAudioSource*)iUserData)->Callback(
        iOutputBuffer, iInputBuffer, iNFrames, iStreamTime, iStatus
    );
}

int Tracter::RtAudioSource::Callback(
    void *iOutputBuffer, void *iInputBuffer, unsigned int iNFrames,
    double iStreamTime, RtAudioStreamStatus iStatus
)
{
    Verbose(3, "Callback: iNFrames = %u\n", iNFrames);

    assert(mSize >= (int)iNFrames);
    assert(!mIndefinite);

    int xrun = 0;
    int len0 = mSize - mHead.offset;
    len0 = std::min((int)iNFrames, len0);
    float* input = (float*)iInputBuffer;
    float* cache = GetPointer(mHead.offset);
    for (int i=0; i<len0; i++)
        cache[i] = input[i];
    if ((mTail.offset >= mHead.offset) &&
        (mHead.index != mTail.index) &&
        (mTail.offset < mHead.offset + len0))
        xrun = mHead.offset + len0 - mTail.offset;

    int len1 = iNFrames - len0;
    cache = GetPointer(0);
    input += len0;
    for (int i=0; i<len1; i++)
        cache[i] = input[i];
    if (xrun > 0)
        xrun += len1;
    else if ((mTail.offset < len1))
        xrun = len1 - mTail.offset;

    //printf("s = %d h = %d,%ld t = %d,%ld  len0 = %d len1 = %d xrun = %d\n",
    //       mSize, mHead.offset, mHead.index, mTail.offset, mTail.index,
    //       len0, len1, xrun);

    MovePointer(mHead, iNFrames);
    if (xrun > 0)
        MovePointer(mTail, xrun);

    return 0;
}

/**
 * Open an RtAudio device.
 */
void Tracter::RtAudioSource::Open(const char* iDeviceName)
{
    assert(iDeviceName);

    /* Try to find a device with the given name */
    int device = -1;
    int nDevices = mRtAudio.getDeviceCount();
    for (int i=0; i<nDevices; i++)
    {
        RtAudio::DeviceInfo di = mRtAudio.getDeviceInfo(i);
        printf("Device: %s\n", di.name.c_str());
        if (di.name == iDeviceName)
        {
            device = i;
            break;
        }
    }
    if (device < 0)
        throw Exception("Device %s not found", iDeviceName);

    /* Given the device, try to open a stream */
    RtAudio::StreamParameters sp;
    sp.nChannels = 1;
    sp.firstChannel = 0;
    sp.deviceId = device;
    unsigned int bufSize = INT_MAX;
    mRtAudio.openStream(
        0, &sp, RTAUDIO_FLOAT32, mSampleFreq, &bufSize, staticCallback, this
    );

    /* Start the stream */
    mRtAudio.startStream();
}

int Tracter::RtAudioSource::Fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    Verbose(3, "Fetch: requested: %d %d\n",
            iOutputArea.len[0], iOutputArea.len[1]);

    /* The fetch is actually completely asynchronous, so just sleep
     * until the head pointer passes where we need to be */
    timespec req;
    req.tv_sec = 0;
    req.tv_nsec = 100000;
    while (mHead.index < iIndex + iOutputArea.Length())
    {
        struct timespec rem;
        nanosleep(&req, &rem);
    }

    // Done
    return iOutputArea.Length();
}
