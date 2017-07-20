/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef EXTRACT_H
#define EXTRACT_H

#include "HTKSink.h"
#include "ASRFactory.h"

namespace Tracter
{
    /**
     * Feature extractor
     *
     * Uses a Factory to construct a feature extractor with a source
     * and sink.  The sink is to HTK format files via HTKSink.
     */
    class Extract : public Object
    {
    public:
        Extract(int iArgc, char** iArgv, ASRFactory* iFactory);
        virtual ~Extract() throw ();
        void All();

    private:
        void File(const char* iFile1, const char* iFile2, bool iLoop=false);
        void List(const char* iFileList);

        var mFile[2];
        var mFileList;
        bool mLoop;

        ISource* mSource;
        HTKSink* mSink;
    };
}

#endif /* EXTRACT_H */
