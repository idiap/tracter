/*
 * Copyright 2008 by The University of Sheffield
 *
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

/*
 * HHTKLibSource.h
 *
 *  Created on: 01-Jul-2008
 *      Author: Vincent Wan
 */

#ifndef HTKLIBSOURCE_H_
#define HTKLIBSOURCE_H_

/* As few headers as possible to allow the class to compile */
#include "HShell.h"
#include "HMem.h"
#include "HWave.h"
#include "HAudio.h"
#include "HParm.h"


#include "CachedPlugin.h"
#include "Source.h"


namespace Tracter
{
    /**
     * Plugin to deal with source files using HTK
     */
    class HTKLibSource : public Source< CachedPlugin<float> >
    {
    public:
        HTKLibSource(const char* iObjectName = "HTKLibSource");
        ~HTKLibSource() throw();
        void Open(const char* iFileName);

    private:
        bool pbufIsOpen;        ///< Remember whether a buffer is open
        MemHeap iStack;         ///< HTK MSTAK memory heap
        ParmBuf pbuf;           ///< HTK buffer where HParm does all its work
        Observation data;       ///< The place where HTK writes the
                                ///  feature vector
        virtual int Fetch(IndexType iIndex, CacheArea& iOutputArea);

    };
}

#endif /* HTKLIBSOURCE_H_ */
