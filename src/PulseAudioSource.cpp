/*
 * Copyright 2010 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "PulseAudioSource.h"

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
    pa_sample_spec spec = {
        PA_SAMPLE_FLOAT32LE, // Need an endian check here
        mFrameRate,
        mFrame.size
    };
    pa_buffer_attr attr = {
        (uint32_t) -1,
        0, 0, 0,
        (uint32_t) -1
    };

    /* Connect to the device */
    mHandle = pa_simple_new(
        server,
        mObjectName,
        PA_STREAM_RECORD,
        0,
        mObjectName,
        &spec,
        0,
        &attr,
        &error
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
    /* I am assuming that it reads a whole buffer-full here */
    int error;
    int ret = pa_simple_read(mHandle, GetPointer(iOffset), iLength, &error);
    if (ret < 0)
        throw Exception("%s: Failed to read %d samples. %s",
                        mObjectName, iLength, pa_strerror(error));
    return iLength;
}
