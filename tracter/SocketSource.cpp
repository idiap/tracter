/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>  // For perror()
#include <cstring>

#ifdef _WIN32
# include <winsock.h>
#else
# include <netdb.h>
#endif

#ifndef _WIN32
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <arpa/inet.h>
#endif

#include "SocketSource.h"


Tracter::socketSource::socketSource(const char* iObjectName)
{
    objectName(iObjectName);
    mPort = config("Port", 30000);
    mBufferSize = config("BufferSize", 0);
    mFD = 0;
}

Tracter::socketSource::~socketSource() throw()
{
#ifdef _WIN32
    closesocket(mFD);
#else
    close(mFD);
#endif
    mFD = 0;
}


/**
 * Connects to a socket.
 */
void Tracter::socketSource::open(const char* iHostName)
{
    assert(iHostName);

    if (mFD)
    {
#ifdef _WIN32
        closesocket(mFD);
#else
        close(mFD);
#endif
        mFD = 0;
    }

    // For the host address (man suggests this call is obsolete)
    struct hostent* hostEnt = gethostbyname(iHostName);
    if (!hostEnt)
    {
        // Would be better to parse h_errno here
        throw Exception("%s: gethostbyname() failed for %s\n",
                        objectName(), iHostName);
    }

    // For the file descriptor
    mFD = socket(PF_INET, SOCK_STREAM, 0);
    if (mFD < 0)
    {
        perror(objectName());
        throw Exception("%s: socket() failed for %s\n",
                        objectName(), iHostName);
    }

    // Socket options
    int optVal = 0;
    socklen_t optLen = sizeof(int);

    // Activate keepalive
    optVal = 1;
    optLen = sizeof(optVal);
    if(setsockopt(mFD, SOL_SOCKET, SO_KEEPALIVE, &optVal, optLen) < 0)
    {
        perror(objectName());
        close(mFD);
        throw Exception("%s: setsockopt() failed for %s\n",
                        objectName(), iHostName);
    }

    // Check the status
    if(getsockopt(mFD, SOL_SOCKET, SO_KEEPALIVE, &optVal, &optLen) < 0)
    {
        perror(objectName());
        close(mFD);
        throw Exception("%s: getsockopt() failed for %s\n",
                        objectName(), iHostName);
    }
    verbose(1, "SO_KEEPALIVE is %s\n", (optVal ? "ON" : "OFF"));

    if (getsockopt(mFD, SOL_SOCKET, SO_RCVBUF, &optVal, &optLen))
        throw Exception("getsockopt failed");
    verbose(1, "recvbuf is size %d\n", optVal);

    if (mBufferSize > 0)
    {
        if (setsockopt(mFD, SOL_SOCKET, SO_RCVBUF, &mBufferSize, optLen))
            throw Exception("setsockopt failed");
        if (getsockopt(mFD, SOL_SOCKET, SO_RCVBUF, &optVal, &optLen))
            throw Exception("getsockopt failed");
        verbose(1, "resize: requested %d granted %d\n", mBufferSize, optVal);
    }

    // Connect using the host address, port and file descriptor
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(mPort);
    server.sin_addr = *((struct in_addr *)hostEnt->h_addr);
    memset(server.sin_zero, '\0', sizeof server.sin_zero);

    if (connect(mFD, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror(objectName());
        throw Exception("%s: connect() failed for %s:%hu\n",
                        objectName(), iHostName, mPort);
    }
}


/**
 * Receive data from socket.  Calls recv() until either iNBytes have
 * been received, or recv() returns 0.
 *
 * @returns the number of bytes actually received.
 */
int Tracter::socketSource::Receive(int iNBytes, char* iBuffer)
{
    assert(iNBytes >= 0);
    assert(iBuffer);
    ssize_t nGet = iNBytes;
    ssize_t nGot = 0;
    while (nGot < nGet)
    {
        ssize_t n = recv(mFD, iBuffer+nGot, nGet-nGot, 0);
        nGot += n;
        if (n == -1)
        {
            perror(objectName());
            throw Exception("%s: recv failed for %d bytes",
                            objectName(), iNBytes);
        }
        if (n == 0)
        {
            verbose(1, "Read 0\n");
            return (int)nGot;
        }
    }
    return (int)nGot;
}

/**
 * Send data to a socket
 */
void Tracter::socketSource::Send(int iNBytes, char* iBuffer)
{
    send(mFD, iBuffer, iNBytes, 0);
}
