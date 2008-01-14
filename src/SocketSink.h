/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SOCKETSINK_H
#define SOCKETSINK_H

#include "UnarySink.h"

/**
 * Sinks to a socket.
 */
class SocketSink : public UnarySink<float>
{
public:
    SocketSink(Plugin<float>* iInput, const char* iObjectName = "SocketSink");
    ~SocketSink();
    void Pull();

private:
    int mFD;
    unsigned short mPort;
};


#endif /* SOCKETSINK_H */
