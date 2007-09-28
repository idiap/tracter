#include "Feature.h"

PluginObject* Feature::GetInput(int iInput)
{
    // Enumerate the inputs
    switch (iInput)
    {
        case 0:
            return mStatic;
        case 1:
            return mDelta1;
        case 2:
            return mDelta2;
        default:
            assert(0);
    }

    // Should never get here
    return 0;
}

Feature::Feature(
    Plugin<float>* iStatic, int iNStatic,
    Plugin<float>* iDelta1, int iNDelta1,
    Plugin<float>* iDelta2, int iNDelta2
)
    : CachedPlugin<float>(iNStatic + iNDelta1 + iNDelta2)
{
    assert(iStatic);
    assert(iDelta1);
    assert(iDelta2);
    assert(iNStatic);
    assert(iNDelta1);
    assert(iNDelta2);

    mStatic = iStatic;
    mDelta1 = iDelta1;
    mDelta2 = iDelta2;
    mNInputs = 3;

    mNStatic = iNStatic;
    mNDelta1 = iNDelta1;
    mNDelta2 = iNDelta2;

    for (int i=0; i<mNInputs; i++)
        MinSize(GetInput(i), 1);
}

bool Feature::ProcessFrame(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    assert(iOffset >= 0);
    CacheArea inputArea;

    float* cache = GetPointer(iOffset);
    float* p;
    int n;

    // Start with the high order deltas to prevent backwards cache reads
    n = mDelta2->Read(inputArea, iIndex);
    if (n == 0)
        return false;
    p = mDelta2->GetPointer(inputArea.offset);
    for (int i=0; i<mNDelta2; i++)
        cache[mNStatic+mNDelta1+i] = p[i];

    // Now the low order deltas
    n = mDelta1->Read(inputArea, iIndex);
    assert(n);
    p = mDelta1->GetPointer(inputArea.offset);
    for (int i=0; i<mNDelta1; i++)
        cache[mNStatic+i] = p[i];

    // Finally the static features
    n = mStatic->Read(inputArea, iIndex);
    assert(n);
    p = mStatic->GetPointer(inputArea.offset);
    for (int i=0; i<mNStatic; i++)
        cache[i] = p[i];

    return true;
}
