#include "Normalise.h"

Normalise::Normalise(Plugin<short>* iInput)
    : UnaryPlugin<float, short>(iInput)
{
}

void Normalise::MinSize(int iSize)
{
    // First call the base class to resize this cache
    assert(iSize > 0);
    PluginObject::MinSize(iSize);

    // We expect the input buffer to be at least the size of each request
    printf("Normalise: Resizing input to %u\n", iSize);
    assert(mInput);
    PluginObject::MinSize(mInput, iSize);
}

int Normalise::Process(IndexType iIndex, CacheArea& iOutputArea)
{
    assert(iIndex >= 0);
    CacheArea inputArea;
    int lenGot = mInput->Read(inputArea, iIndex, iOutputArea.Length());
    short* input = mInput->GetPointer();
    float* output = GetPointer();

    int rOffset = inputArea.offset;
    int wOffset = iOutputArea.offset;
    for (int i=0; i<lenGot; i++)
    {
        if (i == inputArea.len[0])
            rOffset = 0;
        if (i == iOutputArea.len[0])
            wOffset = 0;

        //printf("Process: %u->%u:(%.1f)\n", rOffset, wOffset, (float)input[rOffset]);
        output[wOffset++] = (float)input[rOffset++];
    }

    return lenGot;
}
