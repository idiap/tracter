/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SOCKETTEE_H
#define SOCKETTEE_H

#include "CachedComponent.h"

#include <pthread.h>

namespace Tracter
{
    /**
     * Thread class.  Allows creation of threads where the start
     * routine is a method.  In linux, this is just a very thin
     * wrapper around pthreads.
     */
    class Thread
    {
    public:
        Thread();
        virtual ~Thread() throw () {}
        void Start();
        virtual void start() = 0;

    private:
        static void* staticStart(void* iThread);
        pthread_t mThreadId;
    };

    /**
     * Mutex class.  In linux this is just a very thin wrapper around
     * the pthreads mutex.
     */
    class Mutex
    {
    public:
        Mutex();
        virtual ~Mutex() throw ();
        void Lock();
        void Unlock();

    private:
        pthread_mutex_t mMutex;
    };


    /**
     * Socket class.  Maintains a socket.
     */
    class Socket : public Thread
    {
    public:
        Socket();
        virtual ~Socket() throw ();
        void Listen(unsigned short iPort, bool iNonBlock = false);
        int Accept();
        Mutex* AcceptThread(int iNFD, int* iFD);

    private:
        int mFD;
        bool mNonBlock;

        int mAcceptFDSize;
        int mNAcceptFD;
        int* mAcceptFD;
        Mutex mMutex;

        virtual void start();
    };


    /**
     * Socket tee piece.  Passes input to output unchanged, but also
     * passes it into a socket connection.
     */
    class SocketTee : public CachedComponent<float>
    {
    public:
        SocketTee(
            Component<float>* iInput, const char* iObjectName = "SocketTee"
        );
        virtual ~SocketTee() throw();

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        Component<float>* mInput;
        int mFD;
        Socket mSocket;
        Mutex* mAcceptMutex;
    };
}

#endif /* SOCKETTEE_H */
