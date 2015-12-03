/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef SOCKETSINK_H
#define SOCKETSINK_H

#include "Component.h"
#include "Sink.h"

namespace Tracter
{
    /**
     * Sinks to a socket.
     */
    class SocketSink : public Sink
    {
    public:
        SocketSink(
            Component<float>* iInput, const char* iObjectName = "SocketSink"
        );
        virtual ~SocketSink() throw();
        void Pull();

    private:
        Component<float>* mInput;
        int mFD;
        unsigned short mPort;
        bool mHeader;
    };
}

#endif /* SOCKETSINK_H */
