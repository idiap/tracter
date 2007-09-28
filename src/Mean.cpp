#include "Mean.h"

Mean::Mean(Plugin<float>* iInput, int iArraySize)
    : UnaryPlugin<float, float>(iInput, iArraySize)
{
    assert(iArraySize > 0);

    // Set the input buffer to store everything
    PluginObject::MinSize(mInput, -1);
    mMean.resize(mArraySize);
    for (int i=0; i<mArraySize; i++)
        mMean[i] = 0.0;
    mValid = false;
}

void Mean::Reset(bool iPropagate)
{
    // Zero the mean
    for (int i=0; i<mArraySize; i++)
        mMean[i] = 0.0;
    mValid = false;

    // Call the base class
    UnaryPlugin<float, float>::Reset(iPropagate);
}

bool Mean::ProcessFrame(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    if (!mValid)
    {
        // Calculate mean over whole input range
        int frame = 0;
        while(mInput->Read(inputArea, frame++))
        {
            assert(inputArea.Length() == 1);
            float* p = mInput->GetPointer(inputArea.offset);
            for (int i=0; i<mArraySize; i++)
                mMean[i] += p[i];
        }
        if (frame - 1 > 0)
            for (int i=0; i<mArraySize; i++)
                mMean[i] /= frame - 1;
        mValid = true;

        printf("Mean got %d frames\n", frame-1);
        for (int i=0; i<4; i++)
            printf(" %f", mMean[i]);
    }

    // Copy to output, which is a bit of a waste if the output is only
    // size 1 and there's only one mean.  Maybe there's an
    // optimisation possible.
    float* output = GetPointer(iOffset);
    for (int i=0; i<mArraySize; i++)
        output[i] = mMean[i];

    return true;
}
