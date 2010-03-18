/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "SocketTee.h"

#include <cstdio>  // For perror()
#include <cstring> // For memset()

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include "config.h"

Tracter::Thread::Thread()
{
    mThreadId = 0;
}

void Tracter::Thread::Start()
{
    // Create a new thread with default attributes
    if (pthread_create(&mThreadId, 0, staticStart, this))
        throw Exception("Unable to create thread");
}

void* Tracter::Thread::staticStart(void* iThread)
{
    // Incoming iThread is 'this' pointer
    ((Thread*)iThread)->start();
    return 0;
}

Tracter::Mutex::Mutex()
{
    pthread_mutex_init(&mMutex, 0);
}

Tracter::Mutex::~Mutex() throw ()
{
    pthread_mutex_destroy(&mMutex);
}

void Tracter::Mutex::Lock()
{
    pthread_mutex_lock(&mMutex);
}

void Tracter::Mutex::Unlock()
{
    pthread_mutex_unlock(&mMutex);
}


/**
 * Constructor.  Obtains a socket descriptor to use with subsequent
 * operations.  The socket can be set as non-blocking, in which case
 * subsequent calls to Accept() will not block.
 */
Tracter::Socket::Socket()
{
    // Get the file descriptor
    mFD = socket(AF_INET, SOCK_STREAM, 0);
    if (mFD < 0)
    {
        perror("Socket");
        throw Exception("Socket: socket() failed\n");
    }
}

Tracter::Socket::~Socket() throw ()
{
    if (mFD)
        close(mFD);
    mFD = 0;
}

/**
 * Binds a socket to a local address and instructs it to listen for
 * connections.
 */
void Tracter::Socket::Listen(unsigned short iPort, bool iNonBlock)
{
    // Set (non-)blocking behaviour
    mNonBlock = iNonBlock;
    if (mNonBlock)
        if (fcntl(mFD, F_SETFL, O_NONBLOCK) == -1)
        {
            perror("Socket");
            throw Exception("Failed to set O_NONBLOCK");
        }

    // Allow binding
    int yes = 1;
    if (setsockopt(mFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("Socket");
        throw Exception("Socket: setsockopt(SO_REUSEADDR) failed\n");
    }

    // Bind
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(iPort);
    server.sin_addr.s_addr = INADDR_ANY;
    memset(server.sin_zero, '\0', sizeof(server.sin_zero));
    if (bind(mFD, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("Socket");
        throw Exception("Socket: bind() failed for port %hu\n", iPort);
    }

    // listen
    if (listen(mFD, 1) == -1)
    {
        perror("Socket");
        throw Exception("Socket: listen() failed for port %hu\n", iPort);
    }
}

/**
 * Accept new connection.  If the socket is blocking then this call
 * will block until a new connection is accepted, then return a file
 * descriptor.  If it is non-blocking, it returns either a descriptor
 * (iff there was a connection waiting in the queue) or -1.
 */
int Tracter::Socket::Accept()
{
    struct sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    int fd = accept(mFD, (struct sockaddr *)&client, &clientSize);
    if ((fd == -1) && (!mNonBlock || (errno != EAGAIN)))
    {
        perror("Socket");
        throw Exception("Socket: accept() failed\n");
    }

    return fd;
}

/**
 * Start an accept thread.  Accept() runs blocked in a distinct
 * thread, unblocking when an incoming connection arrives.  When a
 * connection is accepted, the file descriptor is stored in the
 * supplied array.  The array should only be accessed while the
 * returned mutex is locked.
 */
Tracter::Mutex* Tracter::Socket::AcceptThread(int iNFD, int* iFD)
{
    assert(iFD);
    assert(iNFD > 0);
    assert(!mNonBlock); // Must be blocking
    mAcceptFD = iFD;
    mAcceptFDSize = iNFD;
    mNAcceptFD = 0;
    Start();
    return &mMutex;
}

void Tracter::Socket::start()
{
    struct sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    while (mNAcceptFD < mAcceptFDSize)
    {
        int fd = accept(mFD, (struct sockaddr *)&client, &clientSize);
        if (fd == -1)
        {
            perror("Socket");
            throw Exception("Socket: accept() failed\n");
        }
        mMutex.Lock();
        mAcceptFD[mNAcceptFD++] = fd;
        mMutex.Unlock();
    }
}

/**
 * Constructor.
 */
Tracter::SocketTee::SocketTee(Component<float>* iInput, const char* iObjectName)
{
    mObjectName = iObjectName;
    mFrame.size = iInput->Frame().size;
    assert(mFrame.size >= 0);
    mInput = iInput;
    Connect(mInput, 1);

    unsigned short port = GetEnv("Port", 30000);
    mFD = 0;
    mSocket.Listen(port, false);
    mAcceptMutex = mSocket.AcceptThread(1, &mFD);
}

bool Tracter::SocketTee::UnaryFetch(IndexType iIndex, float* oData)
{
    assert(iIndex >= 0);

    CacheArea inputArea;
    if (!mInput->Read(inputArea, iIndex))
        return false;

    // Copy input to output
    float* input  = mInput->GetPointer(inputArea.offset);
    for (int i=0; i<mFrame.size; i++)
        oData[i] = input[i];

    // If there is a connection, output to the socket too
    mAcceptMutex->Lock();
    if (mFD)
    {
        /*
         * If the other end breaks the connection, we'd normally get a
         * SIG_PIPE signal (= bomb out).  This one returns EPIPE.
         */
#ifdef HAVE_LINUX
          ssize_t s = send(mFD, input, sizeof(float) * mFrame.size, MSG_NOSIGNAL);
#endif

#ifdef HAVE_DARWIN
          ssize_t s = send(mFD, input, sizeof(float) * mFrame.size, SO_NOSIGPIPE);
#endif

        if (s == -1)
            switch (errno)
            {
            case EPIPE:
                // Broken connection
                close(mFD);
                mFD = 0;
                break;
            default:
                perror(mObjectName);
                throw Exception("%s: Failed to write()", mObjectName);
            }
    }
    mAcceptMutex->Unlock();

    return true;
}

Tracter::SocketTee::~SocketTee() throw()
{
    if (mFD)
        close(mFD);
    mFD = 0;
}
