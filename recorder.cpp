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

#ifdef HAVE_BSAPI
#ifdef HAVE_TORCH3
# include <tracter/ViterbiVAD.h>
# include <tracter/ViterbiVADGate.h>
# include <tracter/BSAPIFrontEnd.h>
# include <tracter/MLP.h>
# include <tracter/MLPVAD.h>
#endif
#endif

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
        mObjectName = "Record";
        ASRFactory fac;
        Component<float>* p = 0;
        VADStateMachine* sm = 0;

        // Choose a VAD
        enum {
            MODULATION,
            NEW_MODULATION,
            MLP,
            MODULATION_MLP,
            VITERBI_MLP
        };
        const StringEnum cVAD[] = {
            {"Modulation",    MODULATION},
            {"NewModulation", NEW_MODULATION},
            {"MLP",           MLP},
            {"ModulationMLP", MODULATION_MLP},
            {"ViterbiMLP",    VITERBI_MLP},
            {0,               -1}
        };
        int vad = GetEnv(cVAD, -1);

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

#ifdef HAVE_BSAPI
#ifdef HAVE_TORCH3
        // MLP VAD
        case MLP:
        {
            Component<float>* v = p;
            v = new Frame(v, "MLPFrame");
            v = new BSAPIFrontEnd(v, "PLPFrontEnd");
            Mean* mlpm = new Mean(v);
            v = new Subtract(v, mlpm);
            Variance* mlpv = new Variance(v);
            v = new Divide(v, mlpv);
            v = new MLP(v);
            sm = new MLPVAD(v);
            p = new Frame(p);
            p = new VADGate(p, sm);
            p = new Unframe(p);
            break;
        }

        // Modulation MLP VAD
        case MODULATION_MLP:
        {
            Component<float>* v = p;
            v = new Frame(v, "MLPFrame");
            v = new BSAPIFrontEnd(v, "PLPFrontEnd");
            Mean* mlpm = new Mean(v);
            v = new Subtract(v, mlpm);
            Variance* mlpv = new Variance(v);
            v = new Divide(v, mlpv);
            v = new MLP(v);
            v = new Select(v);
            v = new Modulation(v);
            sm = new MLPVAD(v);
            p = new Frame(p);
            p = new VADGate(p, sm);
            p = new Unframe(p);
            break;
        }

        // Viterbi MLP VAD
        case VITERBI_MLP:
        {
            Component<float>* v = p;
            v = new Frame(v, "MLPFrame");
            v = new BSAPIFrontEnd(v, "PLPFrontEnd");
            Mean* mlpm = new Mean(v);
            v = new Subtract(v, mlpm);
            Variance* mlpv = new Variance(v);
            v = new Divide(v, mlpv);
            v = new MLP(v);
            v = new Select(v,"SilSelect");
            ViterbiVAD* vit = new ViterbiVAD(v);
            p = new Frame(p);
            p = new ViterbiVADGate(p, vit);
            p = new Unframe(p);
            break;
        }

#endif
#endif
        default:
            Verbose(1, "No VAD, or not compiled in\n");
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

        Verbose(1, "%s -> %s\n", argv[1], argv[2]);
        mSource->Open(argv[1]);
        mSink->Open(argv[2]);
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