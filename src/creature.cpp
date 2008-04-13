/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <stdio.h>

#include "config.h"

#include "CachedPlugin.h"
#include "FileSource.h"
#include "Normalise.h"
#include "HTKSource.h"
#include "Delta.h"
#include "Concatenate.h"
#include "ALSASource.h"
#include "Periodogram.h"
#include "ZeroFilter.h"
#include "MelFilter.h"
#include "Cepstrum.h"
#include "Mean.h"
#include "UnarySink.h"
#include "ArraySink.h"
#include "LNASource.h"
#include "ByteOrder.h"
#include "ComplexSample.h"
#include "ComplexPeriodogram.h"
#include "FilePath.h"
#include "Resample.h"

#include "TracterObject.h"

class SinkSucker : public UnarySink<float>
{
public:

    SinkSucker(Plugin<float>* iInput, const char* iObjectName = "SinkSucker")
        : UnarySink<float>(iInput)
    {
        mObjectName = iObjectName;
        mArraySize = iInput->GetArraySize();
        MinSize(iInput, 10);
        ReadAhead(0);
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
                printf(" %.3f", f[j]); // * 1.1327);
            printf("...\n");
            printf("  ");

            // 39 is actually a bit stupid, but...
            for (int j=39; j<44; j++)
                printf(" %.3f", f[j]);
            printf("...\n");
            offset++;
        }
    }

};


int main(int argc, char** argv)
{
    printf("Feature creature\n");

    setenv("FileSource_SampleFreq", "2000", 1);
    setenv("Periodogram_FrameSize", "64", 1);
    setenv("Periodogram_FramePeriod", "32", 1);
    setenv("Cepstrum_NCepstra", "8", 1);
    setenv("Cepstrum_C0", "0", 1);
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
    Resample* sr = new Resample(n);
    ZeroFilter* zf = new ZeroFilter(sr);
    Periodogram* p = new Periodogram(zf);
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

    printf("ArraySink...\n");
    FileSource<short>* hh = new FileSource<short>();
    Normalise* nn = new Normalise(hh);
    ArraySink<float> fs(nn);
    hh->Open("testfile.dat");
    fs.Reset();
    float* frame;
    int index = 0;
    while(fs.GetArray(frame, index++) && index < 10)
    {
        printf("%.3f\n", frame[0] * 32768);
    }

    printf("LNA...\n");
    LNASource* lna = new LNASource();
    ArraySink<float> ls(lna);
    lna->Open("data/NU-1004.zipcode.lna");
    ls.Reset();
    index=0;
    while(ls.GetArray(frame, index++) && index < 10)
    {
        printf("%f\n", frame[4]);
    }
#if 1
    printf("ComplexSample...\n");
    FileSource<short>* cs = new FileSource<short>();
    Normalise* cn = new Normalise(cs);
    ComplexSample* ccs = new ComplexSample(cn);
    ComplexPeriodogram* cp = new ComplexPeriodogram(ccs);
    ArraySink<complex> csink(ccs);
    cs->Open("testfile.dat");
    csink.Reset();
    complex* cframe;
    index = 0;
    while(csink.GetArray(cframe, index++) && index < 10)
    {
        printf("%f %f\n", cframe[0].real() * 32768, cframe[0].imag() * 32768);
    }
#endif
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
