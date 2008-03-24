#include <stdio.h>
#include "Fourier.h"

int main()
{
    int n = 4;

    /* FFT Real to complex */
    float* fidata = 0;
    complex* fodata = 0;
    Fourier fft(n, &fidata, &fodata);

    for (int i=0; i<n; i++)
        fidata[i] = (float)i+1;

    fft.Transform();

    for (int i=0; i<n/2+1; i++)
    {
        printf(" %d r: %f i: %f\n", i, fodata[i].real(), fodata[i].imag());
    }

    /* DCT Real to real */
    float* cidata = 0;
    float* codata = 0;
    Fourier dct(n, &cidata, &codata);

    for (int i=0; i<n; i++)
        cidata[i] = (float)i+1;

    dct.Transform();

    for (int i=0; i<n; i++)
    {
        printf(" %d: %f\n", i, codata[i]);
    }

    return 0;
}
