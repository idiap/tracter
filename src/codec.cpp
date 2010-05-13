/*
 * Copyright 2010 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <SndFileSource.h>
#include <SndFileSink.h>

using namespace Tracter;

class Codec : public Tracter::Object
{
public:
    Codec()
    {
        mObjectName = "Codec";
        //Component<float>* p = 0;

        // Source
        SndFileSource *s = new SndFileSource();
        mSink = new SndFileSink(s);
        mSource = s;
    }

    virtual ~Codec() throw ()
    {
        // Delete the sink
        delete mSink;
    }

    void All(int argc, char* argv[])
    {
        if (argc != 3)
            throw Exception(
                "argc != 3\n"
                "usage: codec <infile> <outfile>\n"
                "(and don't forget to set your environment variables!)\n"
            );

        Verbose(0, "%s -> %s\n", argv[1], argv[2]);
        mSource->Open(argv[1]);
        mSink->Open(argv[2]);
    }

private:
    ISource* mSource;
    SndFileSink* mSink;
};

int main(int argc, char* argv[])
{
    Codec r;
    r.All(argc, argv);
}
