/*
 * Copyright 2010 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <tracter/SndFileSource.h>
#include <tracter/SndFileSink.h>

#include <tracter/Frame.h>
#include <tracter/FourierTransform.h>
#include <tracter/OverlapAdd.h>

using namespace Tracter;

/**
 * Originally, perhaps only, a testbed for overlap add.  The idea here
 * is that the class copies audio to audio allowing the various
 * conversions that the components allow.
 */
class Codec : public Tracter::Object
{
public:
    Codec()
    {
        objectName("Codec");

        // Source
        SndFileSource *s = new SndFileSource();
        Component<float>* p = s;
#if 1
        Component<complex>* c = 0;
        p = new Frame(p);
        c = new FourierTransformR2C(p, "DFT");
        p = new FourierTransformC2R(c, "IDFT");
        p = new OverlapAdd(p);
#endif
        mSink = new SndFileSink(p);
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

        verbose(0, "%s -> %s\n", argv[1], argv[2]);
        mSource->open(argv[1]);
        mSink->open(argv[2]);
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
