/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "MLP.h"
#include "DiskXFile.h"

Tracter::MLP::MLP(Plugin<float>* iInput, const char* iObjectName)
    : UnaryPlugin<float, float>(iInput)
{
    mObjectName = iObjectName;
    mInputs = iInput->GetArraySize();
    //mArraySize = iInput->GetArraySize();
    assert(mInputs > 0);

    mTheta = GetEnv("Theta", 9);
    assert(mTheta > 0);

    const char *fname = GetEnv("Weights", "");
    assert(strlen(fname) > 0);

    Torch::DiskXFile xfile(fname,"rb");
    mMLP.loadXFile(&xfile);
    //mMLP.build();
    if (mMLP.machine_infos[mMLP.n_layers-1][0]->desc == Torch::LOGSOFTMAX){
        Torch::SoftMax *sm = new Torch::SoftMax(mMLP.n_outputs);
        mMLP.removeFCL();
        mMLP.addFCL(sm);
    }
    mMLP.build();
    mMLP.setPartialBackprop();

    mWindow = mTheta*2 + 1;
    mFeature.resize(mMLP.n_inputs);
    if (mMLP.n_inputs != mWindow*mInputs)
        throw Exception("MLP: Dimension mismatch: %d != %d",
                        mMLP.n_inputs, mWindow*mInputs);
    PluginObject::MinSize(mInput, mWindow, mTheta);

    mArraySize = mMLP.n_outputs;
    assert(mArraySize > 0);
}

/*
 * This is the calculation for one frame.  Pretty trivial, but the
 * edge effects make the code quite long.
 */
bool Tracter::MLP::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    CacheArea inputArea;

    // Decide how many frames to request depending on whether need to
    // take into account the edge-effect at the beginning.
    int wanted;
    int readIndex;
    if (iIndex < mTheta)
    {
        wanted = mWindow - (mTheta - iIndex);
        readIndex = 0;
    }else{
        wanted = mWindow;
        readIndex = iIndex - mTheta;
    }
    int lenGot = mInput->Read(inputArea, readIndex, wanted);
    if (lenGot < mTheta+1)
        return false;

    // To handle the edges, duplicate frames at the edges
    int feature = 0;
    int offset = inputArea.offset;
    for (int i=0; i<mWindow-wanted; i++)
    {
        float* p = mInput->GetPointer(offset);
        for (int j=0; j<mInputs ; j++, p++){
            mFeature[feature++] = *p;
        }
    }

    // Copy pointers as if no edge effect
    for (int i=0; i<lenGot; i++)
    {
        if (i == inputArea.len[0])
            offset = 0;
        float* p = mInput->GetPointer(offset++);
        for (int j=0; j<mInputs ; j++, p++){
          mFeature[feature++] = *p;
        }
    }

    // If we got frames back, but fewer than required, then we need to
    // consider the edge-effect at the end.
    int lastFeature = feature-mInputs;
    for (int i=0; i<wanted-lenGot; i++)
    {
        float* p =  &(mFeature[lastFeature]);
        for (int j=0; j<mInputs ; j++, p++){
            mFeature[feature++] = *p;
        }
    }
    assert(feature == mWindow*mInputs);

    // The actual calculation
    float* cache = GetPointer(iOffset);

    //printf("runnng MLP on data frame %i\n",iIndex);

    mMLP.frameForward((int)iIndex,&(mFeature[0]),cache);

    return true;
}
