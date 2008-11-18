/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cstring> // For memset()

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "SocketSink.h"

/**
 * Constructor.
 */
Tracter::SocketSink::SocketSink(
    Plugin<float>* iInput,
    const char* iObjectName
)
    : UnarySink<float>(iInput)
{
    mObjectName = iObjectName;
    mArraySize = mInput->GetArraySize();
    MinSize(iInput, 1);
    Initialise();
    Reset();

    mPort = GetEnv("Port", 30000);
    mHeader = GetEnv("Header", 1);

    // Get the file descriptor
    int sockFD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFD < 0)
    {
        perror(mObjectName);
        throw Exception("%s: socket() failed\n", mObjectName);
    }

    int yes = 1;
    if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror(mObjectName);
        throw Exception("%s: setsockopt() failed\n", mObjectName);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(mPort);
    server.sin_addr.s_addr = INADDR_ANY;
    memset(server.sin_zero, '\0', sizeof server.sin_zero);

    if (bind(sockFD, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror(mObjectName);
        throw Exception("%s: bind() failed for port %hu\n",
                        mObjectName, mPort);
    }

    if (listen(sockFD, 1) == -1)
    {
        perror(mObjectName);
        throw Exception("%s: listen() failed for port %hu\n",
                        mObjectName, mPort);
    }

    Verbose(1, "waiting for connection\n");
    struct sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    mFD = accept(sockFD, (struct sockaddr *)&client, &clientSize);
    if (mFD == -1)
    {
        perror(mObjectName);
        throw Exception("%s: accept() failed for port %hu\n",
                        mObjectName, mPort);
    }
    Verbose(1, "got connection from %s\n", inet_ntoa(client.sin_addr));
    close(sockFD);

    if (mHeader)
    {
        TimeType time = TimeStamp() / 1e6;
        ssize_t nSent = send(mFD, &time, sizeof(TimeType), 0);
        if (nSent == -1)
        {
            perror(mObjectName);
            throw Exception("%s: send() failed for port %hu\n",
                            mObjectName, mPort);
        }
        Verbose(1, "Sent time %lld\n", time);
    }
}

Tracter::SocketSink::~SocketSink() throw()
{
    if (mFD)
    {
        close(mFD);
        mFD = 0;
    }
}

void Tracter::SocketSink::Pull()
{
    CacheArea ca;
    int index = 0;
    int total = 0;
    int arraySize = mArraySize == 0 ? 1 : mArraySize;
    while(mInput->Read(ca, index++))
    {
        float* data = mInput->GetPointer(ca.offset);
        ssize_t nSend = arraySize*sizeof(float);
        ssize_t nSent = send(mFD, data, arraySize*sizeof(float), 0);
        Verbose(2, "Send %d  sent %d  total %d\n",
                (int)nSend, (int)nSent, total++);
        if (nSent == -1)
        {
            perror(mObjectName);
            throw Exception("%s: send() failed for port %hu\n",
                            mObjectName, mPort);
        }
    }
    close(mFD);
    mFD = 0;
}
