/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <Source.h>
#include <FileSink.h>
#include <ASRFactory.h>
#include <Frame.h>
#include <Energy.h>
#include <Modulation.h>
#include <NoiseVAD.h>
#include <VADGate.h>

#include <Mean.h>
#include <Variance.h>
#include <Subtract.h>
#include <Divide.h>
#include <Select.h>

#include <config.h>

#ifdef HAVE_BSAPI
#ifdef HAVE_TORCH3
# include <BSAPIFrontEnd.h>
# include <MLP.h>
# include <MLPVAD.h>
#endif
#endif

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

        // Source
        p = fac.CreateSource(mSource);

        // Modulation VAD
        if (GetEnv("Modulation", 1))
        {
            Component<float>* v = p;
            v = new Frame(v);
            v = new Energy(v);
            Modulation* m = new Modulation(v);
            sm = new NoiseVAD(m, v);
        }

#ifdef HAVE_BSAPI
#ifdef HAVE_TORCH3
        // MLP VAD
        else if (GetEnv("MLP", 0))
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
        }

        // Modulation MLP VAD
        else if (GetEnv("ModulationMLP", 0))
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
        }
#endif
#endif

        // Gate
        p = new Frame(p);
        if (!GetEnv("NoVAD", 0))
            p = new VADGate(p, sm);

        // Sink
        mSink = new FileSink(p);
    }

    virtual ~Record() throw ()
    {
        // Delete the sink
        delete mSink;
    }

    void All(int argc, char* argv[])
    {
        if (argc != 3)
            throw Exception("argc != 3");

        Verbose(0, "%s -> %s\n", argv[1], argv[2]);
        mSource->Open(argv[1]);
        mSink->Open(argv[2]);
    }

private:
    ISource* mSource;
    FileSink* mSink;
};

int main(int argc, char* argv[])
{
    Record r;
    r.All(argc, argv);
}
