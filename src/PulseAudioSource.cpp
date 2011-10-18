/*
 * Copyright 2010 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "PulseAudioSource.h"

#include <cstring>
#include <pulse/error.h>

/**
 * PulseAudio Source
 *
 * The variable FrameSize is effectively the number of channels.
 */
Tracter::PulseAudioSource::PulseAudioSource(const char* iObjectName)
{
    mObjectName = iObjectName;
    mFrameRate = GetEnv("FrameRate", 8000.0f);
    mFrame.size = GetEnv("FrameSize", 1);
    mFrame.period = 1;
    mHandle = 0;

    /* Limit the time for which we can connect */
    float maxTime = GetEnv("MaxTime", 0.0f);
    mMaxIndex = SecondsToFrames(maxTime);
}

Tracter::PulseAudioSource::~PulseAudioSource() throw()
{
    if (mHandle)
        pa_simple_free(mHandle);
    mHandle = 0;
}

/**
 * Open a PulseAudio device.
 */
void Tracter::PulseAudioSource::Open(
    const char* iDeviceName, TimeType iBeginTime, TimeType iEndTime
)
{
    /* A bit harsh maybe; should really reconnect */
    if (mHandle)
        throw Tracter::Exception("PulseAudioSource::Open: handle exists");


    /* Helper variables for the device */
    int error;
    const char* server = (*iDeviceName == 0) ? 0 : iDeviceName;
    if (server && (strcmp(server, "default") == 0))
        server = 0;

    /* Sample spec */
    pa_sample_spec spec;
    spec.format = PA_SAMPLE_FLOAT32NE;
    spec.rate = mFrameRate;
    spec.channels = mFrame.size;

    /* Buffer attributes */
    // pa_buffer_attr attr = {
    //     (uint32_t) -1,
    //     0, 0, 0,
    //     (uint32_t) -1
    // };

    /* Connect to the device */
    mHandle = pa_simple_new(
        server,           // Server name
        mObjectName,      // Application name
        PA_STREAM_RECORD, // Stream direction
        0,                // Device
        mObjectName,      // Stream name
        &spec,            // Sample format
        0,                // Channel map
        0,                // Buffering attributes
        &error            // Error code
    );

    /* Die if it failed */
    if (!mHandle)
        throw Exception("%s: Failed to connect to device %s. %s",
                        mObjectName, iDeviceName, pa_strerror(error));
}

/**
 * Contiguous Fetch.
 * The simple API is blocking, which is perfect for tracter.  Fab.
 */
int Tracter::PulseAudioSource::ContiguousFetch(
    IndexType iIndex, int iLength, int iOffset
)
{
    if (mMaxIndex)
        if (iIndex > mMaxIndex)
            return 0;

    /* I'm assuming that it reads a whole buffer-full here */
    int error;
    int ret = pa_simple_read(
        mHandle,
        GetPointer(iOffset),
        iLength * sizeof(float),
        &error
    );
    if (ret < 0)
        throw Exception("%s: Failed to read %d samples. %s",
                        mObjectName, iLength, pa_strerror(error));

    if (mMaxIndex)
        if (iIndex + iLength > mMaxIndex)
            return mMaxIndex - iIndex;
    return iLength;
}
