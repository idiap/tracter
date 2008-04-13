/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "MAPNoise.h"

const float ONE_THIRD = 1.0f/3;

MAPNoise::MAPNoise(float iSNR, float iAlpha)
{
    w = iSNR;
    a = iAlpha;

    // For the quintic
    mLaguerre.SetOrder(5);
    mPoly.resize(6);
}

float MAPNoise::CubicRealRoot(float iA, float iB, float iC, float iD)
{
    /* Variables are for brevity; we're relying somewhat on the optimiser */
    float a2 = iA*iA;
    float a3 = a2*iA;
    float b2 = iB*iB;
    float b3 = b2*iB;

    float q = (3.0f*iA*iC - b2) / (9.0f*a2);
    float r = (9.0f*iA*iB*iC - 27.0f*a2*iD - 2.0f*b3) / (54.0f*a3);

    float st = 0.0f;
    float q3r2 = q*q*q + r*r;

    //printf("q/r = %e/%e = %e, q3r2 = %e\n", q, r, q/r, q3r2);

    if (q3r2 >= 0.0f)
    {
        /* The sqrt is real, so the calculation is all real */
        float rootq3r2 = sqrt(q3r2);

        /* The cube root of -x is minus the cube root of x */
        float rp = r+rootq3r2;
        float rm = r-rootq3r2;
        float s = pow(rp >= 0.0f ? rp : -rp, ONE_THIRD);
        float t = pow(rm >= 0.0f ? rm : -rm, ONE_THIRD);
        //printf("s=%e t=%e\n", s, t);
        st = s+t;
    }
    else
    {
        /* The sqrt is pure imaginary but the final result is real */
        float mag = sqrt(r*r - q3r2);
        float ang = atan2(sqrt(-q3r2), r);
        st = pow(mag, ONE_THIRD) * 2.0f * cos(ang/3.0f);
    }

    float ret = st - iB/(3.0f*iA);
    return ret;
}


float MAPNoise::GammaPrior(float iTotal, float iNoise)
{
    float v = iNoise;
    float v2 = v*v;
    float t = iTotal;

#if 0
    // MAP
    float A = -a/w/v;
    float B = a-2.0f-2.0f*a/w;
    float C = 2.0f*a*v-3.0f*v+t-a/w*v;
    float D = a*v2-v2;
#else
    // MAP log
    float A = -a/w/v;
    float B = a-2.0f*a/w-1.0f;
    float C = 2.0f*a*v+t-a/w*v-v;
    float D = a*v2;
#endif

    float s = CubicRealRoot(A, B, C, D);

    //printf("%f %f %f %f %f\n", A, B, C, D, s);

    return s;
}

float MAPNoise::InverseGammaPrior(float iTotal, float iNoise)
{
    float v = iNoise;
    float v2 = v*v;
    float v3 = v2*v;
    float t = iTotal;

#if 0
    // Basic formulation
    float A = -2.0f-a;
    float B = t+w*v*a-2.0f*a*v-3.0f*v-w*v;
    float C = -v2-2.0f*w*v2+2.0f*w*v2*a-a*v2;
    float D = -w*v3+w*v3*a;
#elif 1
    // Basic with MAP log
    float A = -1.0f-a;
    float B = -2.0f*a*v+t-w*v-v+w*v*a;
    float C = 2.0f*w*v2*a-a*v2-2.0f*w*v2;
    float D = w*v3*a-w*v3;
#else
    // a-1 replaced with a+1
    float A = -2.0f-a;
    float B = t+w*v*a-2.0f*a*v-3.0f*v+w*v;
    float C = -v2+2.0f*w*v2+2.0f*w*v2*a-a*v2;
    float D = w*v3+w*v3*a;
#endif

    float s = CubicRealRoot(A, B, C, D);

    //printf("%f %f %f %f %f\n", A, B, C, D, s);

    return s;
}

float MAPNoise::GammaPrior(float iTotal, float iNoise, float iAvNoise)
{
    float v = iNoise;
    float v2 = v*v;
    float t = iTotal;
    float x = iAvNoise;

    float A = -a/w/x;
    float B = a-2.0f-2.0f*a/w/x*v;
    float C = 2.0f*a*v-3.0f*v+t-a/w/x*v2;
    float D = a*v2-v2;

    float s = CubicRealRoot(A, B, C, D);

    //printf("ABCD: %e %e %e %e %f\n", A, B, C, D, s);

    return s;
}

float MAPNoise::InverseGammaPrior(float iTotal, float iNoise, float iAvNoise)
{
    float v = iNoise;
    float v2 = v*v;
    float t = iTotal;
    float x = iAvNoise;

#if 1
    // Raw solution
    float A = -2.0f-a;
    float B = t+w*x*a-2.0f*a*v-3.0f*v+w*x;
    float C = -v2+2.0f*w*x*v+2.0f*w*x*a*v-a*v2;
    float D = w*x*v2+w*x*a*v2;
#else
    // s/v
    float A = -2.0f - a;
    float B = t/v + w*x*a/v - 2.0f*a - 3.0f + w*x/v;
    float C = -1 + 2.0f*w*x/v + 2.0f*w*x*a/v - a;
    float D = w*x/v + w*x*a/v;
#endif

    float s = CubicRealRoot(A, B, C, D);

    //printf("ABCD: %e %e %e %e %f\n", A, B, C, D, s);

    return s;
}


float MAPNoise::MagGammaPrior(float iTotal, float iNoise)
{
    float v = iNoise;
    float v2 = v*v;
    float t = iTotal;
    float aowv = a / sqrt(w*v);

    mPoly[0] = (a-1)*v2;
    mPoly[1] = -aowv*v2;
    mPoly[2] = 2.0f*(t+(a-2)*v);
    mPoly[3] = -2.0f*v*aowv;
    mPoly[4] = a-3;
    mPoly[5] = -aowv;

    float rootS = mLaguerre.Evaluate(sqrt(t), mPoly);

    //printf("%f %f %f %f %f\n", A, B, C, D, s);

    return rootS * rootS;
}
