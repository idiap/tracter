/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <stdio.h>
#include "MAPNoise.h"

int main()
{
    MAPNoise noise(4, 1.2);
    for(float t=0; t<=20; t+=0.5)
    {
        float s = noise.MagGammaPrior(t, 1.0f);
        printf("%f %f\n", t, s);
    }
}
