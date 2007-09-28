#include <stdio.h>
#include "CachedPlugin.h"
#include "FileSource.h"
#include "Normalise.h"
#include "HTKFile.h"
#include "Delta.h"
#include "Feature.h"
#include "ALSASource.h"
#include "Periodogram.h"
#include "ZeroFilter.h"
#include "MelFilter.h"
#include "Cepstrum.h"
#include "Mean.h"

class Sucker : public PluginObject
{
public:
    Sucker(Plugin<float>* iInput)
    {
        assert(iInput);
        mInput = iInput;
        mNInputs++;
        MinSize(mInput, 5);
    }
    ~Sucker()
    {
        // Recursive delete
        Delete();
    }
    PluginObject* GetInput(int iInput)
    {
        assert(iInput == 0);
        return mInput;
    }
    int Process(IndexType iIndex, CacheArea& iOutputArea)
    {
        assert(0);
    }
    void Suck(int iIndex, int len)
    {
        printf("Sucking %d\n", len);
        CacheArea br;
        int got = mInput->Read(br, iIndex, len);
        if (got != len)
            printf("Asked for %u, got %u\n", len, got);
        printf("Suck: len %u br %u %u %u\n", len, br.len[0], br.len[1], br.offset);
        float* f = mInput->GetPointer();
        int offset = br.offset;
        for (int i=0; i<br.Length(); i++)
        {
            if (i == br.len[0])
            {
                offset = 0;
                printf(" |");
            }
            printf(" %u:(%1.2f)", i+iIndex, f[offset++]);
        }
        printf("\n");
    }

private:
    Plugin<float>* mInput;
};

class ArraySucker : public PluginObject
{
public:
    ArraySucker(Plugin<float>* iInput, int iArraySize = 1)
    {
        assert(iInput);
        assert(iArraySize);
        mInput = iInput;
        mNInputs++;
        MinSize(mInput, 5);
        mArraySize = iArraySize;
    }

    ~ArraySucker()
    {
        // Recursive delete
        Delete();
    }

    PluginObject* GetInput(int iInput)
    {
        assert(iInput == 0);
        return mInput;
    }
    int Process(IndexType iIndex, CacheArea& iOutputArea)
    {
        assert(iIndex >= 0);
        assert(0);
    }
    void Resize(int iSize)
    {
        assert(iSize >= 0);
        assert(0);
    }
    void Suck(int iIndex, int len)
    {
        CacheArea br;
        int got = mInput->Read(br, iIndex, len);
        if (got != len)
            printf("Asked for %u, got %u\n", len, got);
        printf("Suck: len %u br %u %u %u\n", len, br.len[0], br.len[1], br.offset);
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
            for (int j=13; j<18; j++)
                printf(" %.3f", f[j]);
            printf("...\n");
            offset++;
        }
    }

private:
    Plugin<float>* mInput;
};


int main(int argc, char** argv)
{
    printf("Feature creature\n");

#if 1
    FileSource<short>* a = new FileSource<short>;
    a->Map("testfile.dat");
#endif

#if 0
    ALSASource* a = new ALSASource;
#endif

    Normalise* n = new Normalise(a);
    ZeroFilter* zf = new ZeroFilter(n, 0.97);
    Periodogram* p = new Periodogram(zf, 64, 32);
    MelFilter* mf = new MelFilter(p, 10, 2000, 0, 1000);
    Cepstrum* c = new Cepstrum(mf, 8, true);
    ArraySucker s(c, 9);
    //Sucker s(z);
    s.Reset(true);

    //a.Start();
    s.Suck(0, 4);
    s.Suck(1020, 5);

    printf("HTKFile...\n");
    HTKFile* h = new HTKFile(39);
    //Delta* d1 = new Delta(h, 39, 2);
    //Delta* d2 = new Delta(d1, 39, 2);
    //Feature* f = new Feature(h, 13, d1, 13, d2, 13);
    Mean* m = new Mean(h, 39);
    ArraySucker as(m);

    as.Reset();
    h->Map("htk/4k0a0101.plp");
    as.Suck(0, 5);

    as.Reset();
    h->Map("htk/4k0a0102.plp");
    as.Suck(0, 5);

    return 0;
}
