/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SOCKETSOURCE_H
#define SOCKETSOURCE_H

#include <netdb.h>

#include "Source.h"
#include "CachedPlugin.h"

/**
 * Source from a TCP socket.  This allows the source of a tracter
 * graph to be a TCP socket.  It presupposes that some process on
 * another machine exists to supply data to the socket.
 */
class SocketSource : public CachedPlugin<float>, public Source
{
public:
    SocketSource(const char* iObjectName = "SocketSource");
    ~SocketSource();
    void Open(const char* iFileName);

protected:
    int Fetch(IndexType iIndex, CacheArea& iOutputArea);

private:
    int mFD;
    unsigned short mPort;
};

#endif /* SOCKETSOURCE_H */
