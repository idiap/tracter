/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memset()
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "SocketSink.h"

SocketSink::SocketSink(
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

    // Get the file descriptor
    int sockFD = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFD < 0)
    {
        printf("%s: socket() failed\n", mObjectName);
        perror(mObjectName);
        exit(EXIT_FAILURE);
    }

    int yes = 1;
    if (setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        printf("%s: setsockopt() failed\n", mObjectName);
        perror(mObjectName);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(mPort);
    server.sin_addr.s_addr = INADDR_ANY;
    memset(server.sin_zero, '\0', sizeof server.sin_zero);

    if (bind(sockFD, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        printf("%s: bind() failed for port %hu\n", mObjectName, mPort);
        perror(mObjectName);
        exit(EXIT_FAILURE);
    }

    if (listen(sockFD, 1) == -1)
    {
        printf("%s: listen() failed for port %hu\n", mObjectName, mPort);
        perror(mObjectName);
        exit(EXIT_FAILURE);
    }

    printf("%s: waiting for connection\n", mObjectName);
    struct sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    mFD = accept(sockFD, (struct sockaddr *)&client, &clientSize);
    if (mFD == -1)
    {
        printf("%s: accept() failed for port %hu\n", mObjectName, mPort);
        perror(mObjectName);
        exit(EXIT_FAILURE);
    }
    printf("%s: got connection from %s\n",
           mObjectName, inet_ntoa(client.sin_addr));
    close(sockFD);
}

SocketSink::~SocketSink() throw()
{
    if (mFD)
    {
        close(mFD);
        mFD = 0;
    }
}

void SocketSink::Pull()
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
        printf("Send %d  sent %d  total %d\n",
               (int)nSend, (int)nSent, total++);
        if (nSent == -1)
        {
            printf("%s: send() failed for port %hu\n", mObjectName, mPort);
            perror(mObjectName);
            exit(EXIT_FAILURE);
        }
    }
    close(mFD);
    mFD = 0;
}
