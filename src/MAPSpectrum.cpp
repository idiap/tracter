/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>
#include "MAPSpectrum.h"

PluginObject* MAPSpectrum::GetInput(int iInput)
{
    // Enumerate the inputs
    switch (iInput)
    {
    case 0:
        return mInput1;
    case 1:
        return mInput2;
    case 2:
        return mChannelInput;
    default:
        assert(0);
    }

    // Should never get here
    return 0;
}

MAPSpectrum::MAPSpectrum(
    Plugin<float>* iInput1,
    Plugin<float>* iInput2,
    Plugin<float>* iChannelInput,
    const char* iObjectName
)
{
    mObjectName = iObjectName;
    mArraySize = iInput1->GetArraySize();

    Connect(iInput1);
    Connect(iInput2);
    if (iChannelInput)
        Connect(iChannelInput);

    mInput1 = iInput1;
    mInput2 = iInput2;
    mChannelInput = iChannelInput;

    mEstimator = GetEnv("Estimator", 0);
    mAlpha = GetEnv("Alpha", 1.1f);
    mBeta = GetEnv("Beta", 1.0f);
    mDelta = GetEnv("Delta", 1.0f);
    mSNR = GetEnv("SNR", 1.0f);
    mMAPNoise.SetAlpha(mAlpha);
    mMAPNoise.SetSNR(mSNR);

    mNoise.resize(mArraySize);
    const char* file = GetEnv("File", (char*)0);
    if (file)
    {
        // Load from file
        printf("Loading file %s\n", file);
        FILE* fp = fopen(file, "r");
        assert(fp);
        for (int i=0; i<mArraySize; i++)
        {
            fscanf(fp, "%f", &mNoise[i]);
        }
        fclose(fp);
    }
    else
        // Set to 1 so the effect is nil
        for (int i=0; i<mArraySize; i++)
            mNoise[i] = 1.0f;

    // Set Solver polynomial order etc.
    switch (mEstimator)
    {
    case 12:
        mSolver.SetOrder(4);
        break;

    case 13:
        mSolver.SetOrder(5);
        break;

    case 14:
        mSolver.SetOrder(6);
        break;

    default:
        break;
    }


    for (int i=0; i<mNInputs; i++)
        MinSize(GetInput(i), 1);
}

bool MAPSpectrum::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);
    assert(iOffset >= 0);
    CacheArea inputArea;
    float* cache = GetPointer(iOffset);

    // Start with the second input
    if (mInput2->Read(inputArea, iIndex) == 0)
        return false;
    float *p2 = mInput2->GetPointer(inputArea.offset);

    // Channel input if there
    float *p3 = 0;
    if (mChannelInput)
    {
        if (mChannelInput->Read(inputArea, iIndex) == 0)
            return false;
        p3 = mChannelInput->GetPointer(inputArea.offset);
    }

    // Now the first input
    if (mInput1->Read(inputArea, iIndex) == 0)
        return false;
    float *p1 = mInput1->GetPointer(inputArea.offset);


    // Do the estimation
    // There are lots of possible estimators...
    for (int i=0; i<mArraySize; i++)
        switch (mEstimator)
        {
            float av;

        case 0:
            cache[i] = mMAPNoise.GammaPrior(p1[i], p2[i]);
            break;

        case 1:
            av = Average(p2);
            cache[i] = mMAPNoise.GammaPrior(p1[i], p2[i], av);
            break;

        case 2:
            cache[i] = mMAPNoise.InverseGammaPrior(p1[i], p2[i]);
            break;

        case 3:
            av = Average(p2);
            cache[i] = mMAPNoise.InverseGammaPrior(p1[i], p2[i], av);
            break;

        case 4:
            cache[i] = mMAPNoise.MagGammaPrior(p1[i], p2[i]);
            break;

        case 5:
            cache[i] = InverseGammaFixedMode(p1[i], p2[i], mNoise[i]);
            break;

        case 6:
            cache[i] = InverseGammaFixedModeLog(p1[i], p2[i], mNoise[i]);
            break;

        case 7:
            cache[i] = InverseGammaMode(p1[i], p2[i]);
            break;

        case 8:
            cache[i] = InverseGammaModeLog(p1[i], p2[i]);
            break;

        case 9:
            cache[i] = p1[i] / p2[i] + 1.0f;
            break;

        case 10:
            cache[i] = p1[i] / p3[i];
            break;

        case 11:
            cache[i] = InverseGammaModeLog(p1[i] / p3[i], p2[i] / p3[i]);
            break;

        case 12:
            cache[i] = IncompleteGammaLog(p1[i], p2[i]);
            break;

        case 13:
            cache[i] = IncompleteGammaMag(p1[i], p2[i]);
            break;

        case 14:
            cache[i] = IncompleteGammaMagLog(p1[i], p2[i]);
            break;

        default:
            printf("MAPSpectrum: Unknown estimator\n");
            exit(EXIT_FAILURE);
        }

    return true;
}

float MAPSpectrum::Average(float* iArray)
{
    float av = 0.0f;
    for (int i=0; i<mArraySize; i++)
        av += iArray[i];
    av /= mArraySize;
    return av;
}

float MAPSpectrum::InverseGammaFixedMode(
    float iTotal, float iNoise, float iMode
)
{
    float t = iTotal;
    float v = iNoise;
    float v2 = v*v;
    float a = mAlpha;
    float B = mBeta * iMode;

    float s[4];
    s[3] = -a-2.0f;
    s[2] = t+B*a-2.0f*a*v-3.0f*v+B;
    s[1] = -v2+2.0f*B*v+2.0f*B*a*v-a*v2;
    s[0] = B*v2+B*a*v2;

    float sHat = CubicRealRoot(s[3], s[2], s[1], s[0]);
    return sHat;
}

float MAPSpectrum::InverseGammaFixedModeLog(
    float iTotal, float iNoise, float iMode
)
{
    float t = iTotal;
    float v = iNoise;
    float v2 = v*v;
    float a = mAlpha;
    float B = mBeta * iMode;

    float s[4];
    s[3] = -a - 1.0f;
    s[2] = t + B*a - 2.0f*a*v - v + B;
    s[1] = 2.0f*B*v + 2.0f*B*a*v - a*v2;
    s[0] = B*v2 + B*a*v2;

    float sHat = CubicRealRoot(s[3], s[2], s[1], s[0]);
    return sHat;
}

float MAPSpectrum::InverseGammaMode(float iTotal, float iNoise)
{
    float t = iTotal;
    float v = iNoise;
    float v2 = v*v;
    float v3 = v2*v;
    float a = mAlpha;
    float w = mSNR;

    float s[4];
    s[3] = -a - 2.0f;
    s[2] = t + w*v*a - 2.0f*a*v - 3*v + w*v;
    s[1] = -v2 + 2.0f*w*v2 + 2.0f*w*v2*a - a*v2;
    s[0] = w*v3 + w*v3*a;

    float sHat = CubicRealRoot(s[3], s[2], s[1], s[0]);
    return sHat;
}

float MAPSpectrum::InverseGammaModeLog(float iTotal, float iNoise)
{
    float t = iTotal;
    float v = iNoise;
    float v2 = v*v;
    float v3 = v2*v;
    float a = mAlpha;
    float w = mSNR;

    float s[4];
    s[3] = -1.0f - a;
    s[2] = -2.0*a*v - v + t + w*v + w*v*a;
    s[1] = -a*v2 + 2.0f*w*v2 + 2.0f*w*v2*a;
    s[0] = w*v3 + w*v3*a;

    float sHat = CubicRealRoot(s[3], s[2], s[1], s[0]);
    return sHat;
}

float MAPSpectrum::IncompleteGammaLog(float iTotal, float iNoise)
{
    float t = iTotal;
    float v = iNoise;
    float v2 = v*v;
    float a = mAlpha;
    float d = mDelta;
    float B = mBeta;

    float s[5];
    s[4] = 1/B;
    s[3] = a + 1/B*d - 1.0f + 2.0f/B*v;
    s[2] = 2.0f*a*v + t + 1.0f/B*v2 - v + 2.0f/B*v*d;
    s[1] = 1.0f/B*v2*d + v*d + a*v2 + t*d;
    s[0] = v2*d;

    float sHat = mSolver.Evaluate(t, s);
    return sHat;
}

float MAPSpectrum::IncompleteGammaMag(float iTotal, float iNoise)
{
    float t = iTotal;
    float v2 = iNoise;
    float v4 = v2*v2;
    float a = mAlpha;
    float d = mDelta;
    float B = mSNR * iNoise;

    float s[6];
    s[5] = -1.0f/B;
    s[4] = a-3.0f-1.0f/B*d;
    s[3] = -2.0f*d-2.0f/B*v2;
    s[2] = -2.0f/B*v2*d+2.0f*a*v2+2.0f*t-4.0f*v2;
    s[1] = -2.0f*v2*d-1.0f/B*v4+2.0f*t*d;
    s[0] = a*v4-v4-1.0f/B*v4*d;

    float sHat = mSolver.Evaluate(sqrt(t), s);
    return sHat;
}

float MAPSpectrum::IncompleteGammaMagLog(float iTotal, float iNoise)
{
    float t = iTotal;
    float v2 = iNoise;
    float v4 = v2*v2;
    float a = mAlpha;
    float d = mDelta;
    float B = mSNR * iNoise;

    float s[7];
    s[6] = 1.0f;
    s[5] = -1.0/B+d;
    s[4] = 2.0f*v2-1.0f/B*d-3.0f+a;
    s[3] = 2.0f*v2*d-2.0f*d-2.0f/B*v2;
    s[2] = -2.0f/B*v2*d+2.0f*a*v2+2.0f*t-4.0f*v2+v4;
    s[1] = -2.0f*v2*d-1.0f/B*v4+2.0f*t*d+v4*d;
    s[0] = a*v4-v4-1.0f/B*v4*d;

    float sHat = mSolver.Evaluate(t, s);
    return sHat;
}
