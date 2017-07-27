/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <tracter/Source.h>
#include <tracter/SndFileSink.h>
#include <tracter/ASRFactory.h>
#include <tracter/Frame.h>
#include <tracter/Unframe.h>
#include <tracter/Energy.h>
#include <tracter/Modulation.h>
#include <tracter/NoiseVAD.h>
#include <tracter/VADGate.h>

#include <tracter/Mean.h>
#include <tracter/Variance.h>
#include <tracter/Subtract.h>
#include <tracter/Divide.h>
#include <tracter/Select.h>

#include <tracter/Minima.h>
#include <tracter/Comparator.h>
#include <tracter/TimedLatch.h>
#include <tracter/Gate.h>

using namespace Tracter;

class Record : public Tracter::Object
{
public:
    Record()
    {
        objectName("Record");
        ASRFactory fac;
        Component<float>* p = 0;
        VADStateMachine* sm = 0;

        // Choose a VAD
        enum {
            MODULATION,
            NEW_MODULATION
        };
        const StringEnum cVAD[] = {
            {"Modulation",    MODULATION},
            {"NewModulation", NEW_MODULATION},
            {0,               -1}
        };
        int vad = config(cVAD, -1);

        // Source
        p = fac.CreateSource(mSource);

        switch (vad)
        {
            // Modulation VAD
        case MODULATION:
        {
            Component<float>* v = p;
            v = new Frame(v);
            v = new Energy(v);
            Modulation* m = new Modulation(v);
            sm = new NoiseVAD(m, v);
            p = new Frame(p);
            p = new VADGate(p, sm);
            p = new Unframe(p);
            break;
        }

        // New Modulation VAD
        case NEW_MODULATION:
        {
            Component<float>* v = p;
            v = new Frame(v);
            v = new Energy(v);
            Modulation* m = new Modulation(v);
            Component<float>* n = new Minima(v);
            Component<BoolType>* b = new Comparator(m, n);
            b = new TimedLatch(b);
            p = new Frame(p);
            p = new Gate(p, b);
            p = new Unframe(p);
            break;
        }

        default:
            verbose(1, "No VAD, or not compiled in\n");
        }

        // Sink
        mSink = new SndFileSink(p);
    }

    virtual ~Record() throw ()
    {
        // Delete the sink
        delete mSink;
    }

    void All(int argc, char* argv[])
    {
        if (argc != 3)
            throw Exception(
                "argc != 3\n"
                "usage: recorder <insource> <outsink>\n"
                "(and don't forget to set your environment variables!)\n"
            );

        verbose(1, "%s -> %s\n", argv[1], argv[2]);
        mSource->open(argv[1]);
        mSink->open(argv[2]);
    }

private:
    ISource* mSource;
    SndFileSink* mSink;
};

int main(int argc, char* argv[])
{
    Record r;
    r.All(argc, argv);
}
