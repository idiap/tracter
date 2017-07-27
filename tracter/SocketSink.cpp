/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>  // For perror()
#include <cstring> // For memset()

#ifdef _WIN32
# include <winsock.h>
#else
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <arpa/inet.h>
#endif

#include "SocketSink.h"

/**
 * Constructor.
 */
Tracter::SocketSink::SocketSink(
    Component<float>* iInput,
    const char* iObjectName
)
{
    objectName(iObjectName);
    mInput = iInput;
    connect(mInput);

    mFrame.size = mInput->frame().size;
    initialise();
    reset();

    mPort = config("Port", 30000);
    mHeader = config("Header", 1);

    // Get the file descriptor
    int sockFD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFD < 0)
    {
        perror(objectName());
        throw Exception("%s: socket() failed\n", objectName());
    }

#ifdef _WIN32
    const char yes = 1;
#else
    int yes = 1;
#endif
    if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror(objectName());
        throw Exception("%s: setsockopt() failed\n", objectName());
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(mPort);
    server.sin_addr.s_addr = INADDR_ANY;
    memset(server.sin_zero, '\0', sizeof server.sin_zero);

    if (bind(sockFD, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror(objectName());
        throw Exception("%s: bind() failed for port %hu\n",
                        objectName(), mPort);
    }

    if (listen(sockFD, 1) == -1)
    {
        perror(objectName());
        throw Exception("%s: listen() failed for port %hu\n",
                        objectName(), mPort);
    }

    verbose(1, "waiting for connection\n");
    struct sockaddr_in client;
#ifdef _WIN32
    int clientSize = sizeof(client);
#else
    socklen_t clientSize = sizeof(client);
#endif
    mFD = accept(sockFD, (struct sockaddr *)&client, &clientSize);
    if (mFD == -1)
    {
        perror(objectName());
        throw Exception("%s: accept() failed for port %hu\n",
                        objectName(), mPort);
    }
    verbose(1, "got connection from %s\n", inet_ntoa(client.sin_addr));
#ifdef _WIN32
    closesocket(sockFD);
#else
    close(sockFD);
#endif

    if (mHeader)
    {
        TimeType time = timeStamp() / ONEe6;
        ssize_t nSent = send(mFD, &time, sizeof(TimeType), 0);
        if (nSent == -1)
        {
            perror(objectName());
            throw Exception("%s: send() failed for port %hu\n",
                            objectName(), mPort);
        }
        verbose(1, "Sent time %lld\n", time);
    }
}

Tracter::SocketSink::~SocketSink() throw()
{
    if (mFD)
    {
#ifdef _WIN32
        closesocket(mFD);
#else
        close(mFD);
#endif
        mFD = 0;
    }
}

void Tracter::SocketSink::Pull()
{
    CacheArea ca;
    int index = 0;
    int total = 0;
    int arraySize = mFrame.size == 0 ? 1 : mFrame.size;
    while(mInput->read(ca, index++))
    {
        float* data = mInput->getPointer(ca.offset);
        ssize_t nSend = arraySize*sizeof(float);
#ifdef _WIN32
        ssize_t nSent = send(mFD, (char*)data, nSend, 0);
#else
        ssize_t nSent = send(mFD, data, nSend, 0);
#endif
        verbose(2, "Send %d  sent %d  total %d\n",
                (int)nSend, (int)nSent, total++);
        if (nSent == -1)
        {
            perror(objectName());
            throw Exception("%s: send() failed for port %hu\n",
                            objectName(), mPort);
        }
    }
#ifdef _WIN32
    closesocket(mFD);
#else
    close(mFD);
#endif
    mFD = 0;
}
