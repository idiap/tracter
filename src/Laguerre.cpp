/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cmath>
#include <cstdio>
#include <cassert>
#include <cfloat>

#include "Laguerre.h"

void Laguerre::SetOrder(int iOrder)
{
    mOrder = iOrder;
    mD.resize(mOrder);
    mDD.resize(mOrder-1);
}

float Laguerre::Evaluate(float iEstimate, const std::vector<float>& iPoly)
{
    assert(mOrder+1 == (int)iPoly.size());
    return Evaluate(iEstimate, &iPoly[0]);
}

float Laguerre::Evaluate(float iEstimate, const float* iPoly)
{
    // Set first and second differential coefficients
    for (int i=0; i<mOrder; i++)
        mD[i] = iPoly[i+1] * (i+1);
    for (int i=0; i<mOrder-1; i++)
        mDD[i] = mD[i+1] * (i+1);

    float x = iEstimate;
    for (int iter=0; iter<10; iter++)
    {
        float p = iPoly[0];
        for (int i=1; i<mOrder+1; i++)
            p += iPoly[i] * powf(x, i);

        // If p is small, we're there.  This explicitly prevents a
        // division by zero in calculation of G and H
        if (fabs(p) < FLT_MIN)
            break;

        float d = mD[0];
        for (int i=1; i<mOrder; i++)
            d += mD[i] * powf(x, i);
        float dd = mDD[0];
        for (int i=1; i<mOrder-1; i++)
            dd += mDD[i] * powf(x, i);

        float G = d/p;
        float H = G * G - dd/p;
        float nHG2 = H * mOrder - G * G;
        if (nHG2 < 0.0f)
        {
            printf("Laguerre: complex root at iter %d G=%e H=%e\n",
                   iter, G, H);
            return 0.0f;
        }

        // Calculate denomimator with largest absolute value
        float denom = G > 0.0f
            ? G + sqrt(nHG2 * (mOrder-1))
            : G - sqrt(nHG2 * (mOrder-1));
        float a = (float)mOrder / denom;
        x -= a;
        //printf("%d: %f %e %e %e\n", iter, x, a, G, p);

        // If a is too small to make a difference, we're there.  This
        // is less tight than the check on p above.
        if (fabs(a) < FLT_EPSILON)
            break;
    }

    return x;
}

#ifdef MAIN
int main()
{
    std::vector<float> poly;

    // (x-2)(x-3) = 6 - 5x + x^2
    // (x-20)(x-10)(x+1)(x+3) = 600 + 710x + 83x^2 - 26x^3 + x^4
    poly.resize(5);
    poly[0] = 600.0f;
    poly[1] = 710.0f;
    poly[2] = 83.0f;
    poly[3] = -26.0f;
    poly[4] = 1.0f;

    Laguerre l;
    l.SetOrder(poly.size()-1);
    float f = l.Evaluate(-5, poly);
    printf("f: %f\n", f);

    return 0;
}
#endif
