/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "CachedComponent.h"
#include "FileSource.h"
#include "Normalise.h"
#include "HTKSource.h"
#include "Delta.h"
#include "Concatenate.h"
#include "Periodogram.h"
#include "ZeroFilter.h"
#include "MelFilter.h"
#include "Cepstrum.h"
#include "Mean.h"
#include "FrameSink.h"
#include "LNASource.h"
#include "ByteOrder.h"
#include "ComplexSample.h"
#include "ComplexPeriodogram.h"
#include "FilePath.h"
#include "Frame.h"

#include "config.h"

#ifdef HAVE_ALSA
# include "ALSASource.h"
#endif

#ifdef _WIN32
# include <windows.h>
# define setenv(a, b, c) SetEnvironmentVariable(a, b)
#endif

using namespace Tracter;

class SinkSucker : public Sink
{
public:

    SinkSucker(Component<float>* iInput, const char* iObjectName = "SinkSucker")
    {
        mObjectName = iObjectName;
        mInput = iInput;
        Connect(mInput, 10);
        mFrame.size = iInput->Frame().size;
        Initialise();
        Reset();
    }

    void Pull(int iIndex, int len)
    {
        CacheArea br;
        int got = mInput->Read(br, iIndex, len);
        if (got != len)
            printf("Asked for %u, got %u\n", len, got);
        printf("Suck: len %u br %u %u %u\n",
               len, br.len[0], br.len[1], br.offset);
        int offset = br.offset;
        for (int i=0; i<br.Length(); i++)
        {
            float* f = mInput->GetPointer(offset);
            if (i == br.len[0])
            {
                offset = 0;
                printf("---\n");
            }
            printf("%2u", iIndex+i);
            for (int j=0; j<5; j++)
                printf(" %e", f[j]); // * 1.1327);
            printf("...\n");
            printf("  ");

            // 39 is actually a bit stupid, but...
            for (int j=39; j<44; j++)
                printf(" %e", f[j]);
            printf("...\n");
            offset++;
        }
    }

private:
    Component<float>* mInput;
};


int main(int argc, char** argv)
{
    printf("Feature creature\n");

    setenv("FileSource_FrameRate", "2000", 1);
    setenv("Frame_Size", "64", 1);
    setenv("Frame_Period", "32", 1);
    setenv("Cepstrum_NCepstra", "8", 1);
    setenv("Cepstrum_C0", "0", 1);
    setenv("MelFilter_MaxHertz", "1000", 1);
    setenv("MelFilter_NBins", "10", 1);
    setenv("MelFilter_LoHertz", "0", 1);
    setenv("MelFilter_HiHertz", "1000", 1);

#if 1
    FileSource<short>* a = new FileSource<short>;
    a->Open("testfile.dat");
#endif

#if 0
    ALSASource* a = new ALSASource;
#endif

    Normalise* n = new Normalise(a);
    ZeroFilter* zf = new ZeroFilter(n);
    Frame* fr = new Frame(zf);
    Periodogram* p = new Periodogram(fr);
    MelFilter* mf = new MelFilter(p);
    Cepstrum* c = new Cepstrum(mf);
    SinkSucker s(c);
    s.Reset(true);

    //a.Start();
    s.Pull(0, 4);
    s.Pull(28, 5);

    setenv("DeltaDelta_Theta", "3", 1);

    printf("HTKSource...\n");
    HTKSource* h = new HTKSource();
    Delta* d1 = new Delta(h);
    Delta* d2 = new Delta(d1, "DeltaDelta");
    Concatenate* f = new Concatenate();
    f->Add(h);
    f->Add(d1);
    f->Add(d2);
    //Mean* m = new Mean(h);
    SinkSucker as(f);

    as.Reset();
    h->Open("data/4k0a0101.plp");
    as.Pull(0, 10);

    as.Reset();
    h->Open("data/4k0a0102.plp");
    as.Pull(0, 10);

    printf("FrameSink...\n");
    FileSource<short>* hh = new FileSource<short>();
    Normalise* nn = new Normalise(hh);
    FrameSink<float> fs(nn);
    hh->Open("testfile.dat");
    fs.Reset();
    int index = 0;
    while(const float* frame = fs.Read(index++))
    {
        printf("%.3f\n", frame[0] * 32768);
        if (index >= 9)
            break;
    }

    printf("LNA...\n");
    LNASource* lna = new LNASource();
    FrameSink<float> ls(lna);
    lna->Open("data/NU-1004.zipcode.lna");
    ls.Reset();
    index=0;
    while(const float* frame = ls.Read(index++))
    {
        printf("%f\n", frame[4]);
        if (index >= 9)
            break;
    }

    printf("ComplexSample...\n");
    FileSource<short>* cs = new FileSource<short>();
    Normalise* cn = new Normalise(cs);
    ComplexSample* ccs = new ComplexSample(cn);
    //ComplexPeriodogram* cp = new ComplexPeriodogram(ccs);
    FrameSink<complex> csink(ccs);
    cs->Open("testfile.dat");
    csink.Reset();
    index = 0;
    while(const complex* cframe = csink.Read(index++))
    {
        printf("%f %f\n", cframe[0].real() * 32768, cframe[0].imag() * 32768);
        if (index >= 9)
            break;
    }

    FilePath path;
#define FILETEST(p) printf("Test:" p "\n"); path.SetName(p); path.Dump()
    FILETEST("hello");
    FILETEST("hello.ext");
    FILETEST("path/hello");
    FILETEST("/path/hello.ext");
    FILETEST("./hello");
    FILETEST("./hello.ext1.ext2");
    FILETEST("./hello.ext1.ext2/file/");
    FILETEST("./very/long/path/name/file-name.ext");
    //path.MakePath();

    printf("Done\n");
    return 0;
}
