/*
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstring>
#include <algorithm>

#include "RtAudioSource.h"

Tracter::RtAudioSource::RtAudioSource(const char* iObjectName)
{
    objectName(iObjectName);
    mFrameRate = config("FrameRate", 8000.0f);
    mFrame.size = config("FrameSize", 1);
    mFrame.period = 1;

    mNChannels = config("NChannels", mFrame.size);
    if ((mNChannels != mFrame.size) && (mFrame.size != 1))
        throw Exception("RtAudioSource: Frame size must be 1 or NChannels");

    float seconds = config("BufferTime", 1.0f);
    SizeType samples = secondsToFrames(seconds);
    minSize(this, samples);

    /* Tell the ComponentBase that we will take care of the pointers */
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
    verbose(3, "Callback: iNFrames = %u\n", iNFrames);

    assert(!mIndefinite);
    if (mSize < (SizeType)iNFrames)
        throw Exception(
            "RtAudioSource::Callback: mSize=%d < iNFrames=%u", mSize, iNFrames
        );

    CachePointer& head = mCluster[0].head;
    CachePointer& tail = mCluster[0].tail;

    SizeType xrun = 0;
    SizeType len0 = mSize - head.offset;
    len0 = std::min((SizeType)iNFrames, len0);
    float* input = (float*)iInputBuffer;
    float* cache = getPointer(head.offset);
    for (SizeType i=0; i<len0; i++)
        if (mFrame.size == mNChannels)
            for (int j=0; j<mNChannels; j++)
                *cache++ = *input++;
        else
        {
            float sum = 0.0f;
            for (int j=0; j<mNChannels; j++)
                sum += *input++;
            cache[i] = sum / mNChannels;
        }
    if ((tail.offset >= head.offset) &&
        (head.index != tail.index) &&
        (tail.offset < head.offset + len0))
        xrun = head.offset + len0 - tail.offset;

    SizeType len1 = iNFrames - len0;
    cache = getPointer(0);
    for (SizeType i=0; i<len1; i++)
        // Code is dupped from above
        if (mFrame.size == mNChannels)
            for (int j=0; j<mNChannels; j++)
                *cache++ = *input++;
        else
        {
            float sum = 0.0f;
            for (int j=0; j<mNChannels; j++)
                sum += *input++;
            cache[i] = sum / mNChannels;
        }
    if (xrun > 0)
        xrun += len1;
    else if ((tail.offset < len1))
        xrun = len1 - tail.offset;

    //printf("s = %d h = %d,%ld t = %d,%ld  len0 = %d len1 = %d xrun = %d\n",
    //       mSize, head.offset, head.index, tail.offset, tail.index,
    //       len0, len1, xrun);

    movePointer(head, iNFrames);
    if (xrun > 0)
        movePointer(tail, xrun);

    return 0;
}

/**
 * Open an RtAudio device.
 */
void Tracter::RtAudioSource::open(
    const char* iDeviceName, TimeType iBeginTime, TimeType iEndTime
)
{
    assert(iDeviceName);

    /* Try to find a device with the given name */
    int device = -1;
    int nDevices = mRtAudio.getDeviceCount();

    verbose(1, "Device requested (length: %d): %s\n",
            strlen(iDeviceName), iDeviceName);
    for (int i=0; i<nDevices; i++)
    {
        RtAudio::DeviceInfo di = mRtAudio.getDeviceInfo(i);
        verbose(1, "Device detected  (length: %d): %s\n",
                strlen(di.name.c_str()), di.name.c_str());
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
    sp.nChannels = mNChannels;
    sp.firstChannel = 0;
    sp.deviceId = device;
    unsigned int bufSize = 100; // This is probably period size
    mRtAudio.openStream(
        0, &sp, RTAUDIO_FLOAT32, mFrameRate, &bufSize, staticCallback, this
    );
    verbose(1, "RtAudio buffer size %u, Tracter buffer size %d\n",
            bufSize, mSize);

    /* Start the stream */
    mRtAudio.startStream();
}

Tracter::SizeType
Tracter::RtAudioSource::fetch(IndexType iIndex, CacheArea& iOutputArea)
{
    verbose(3, "Fetch: requested: %d %d\n",
            iOutputArea.len[0], iOutputArea.len[1]);

    /* The fetch is actually completely asynchronous, so just sleep
     * until the head pointer passes where we need to be */
    timespec req;
    req.tv_sec = 0;
    req.tv_nsec = 100000;
    CachePointer& head = mCluster[0].head;
    while (head.index < iIndex + iOutputArea.length())
    {
        struct timespec rem;
        nanosleep(&req, &rem);
    }

    // Done
    return iOutputArea.length();
}
