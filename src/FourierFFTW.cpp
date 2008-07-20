/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cassert>
#include <fftw3.h>
#include "Fourier.h"

/*
 * From the FFTW FAQ:
 *
 * Question 3.8. FFTW gives different results between runs
 *
 * If you use FFTW_MEASURE or FFTW_PATIENT mode, then the algorithm
 * FFTW employs is not deterministic: it depends on runtime
 * performance measurements. This will cause the results to vary
 * slightly from run to run. However, the differences should be
 * slight, on the order of the floating-point precision, and therefore
 * should have no practical impact on most applications.
 *
 * If you use saved plans (wisdom) or FFTW_ESTIMATE mode, however,
 * then the algorithm is deterministic and the results should be
 * identical between runs.
 *
 * From PNG:
 *
 * FFT_MEASURE is the default.  Floating point precision my arse; it's
 * enough to give 3rd/4th significant digit effects in features.  It's
 * not right for experiments that you want to repeat.
 */

namespace FourierFFTW
{
    static int sInstanceCount = 0;
};

struct Tracter::FourierData
{
    void* IData;
    void* OData;
    fftwf_plan Plan;
};

/*
 * Either allocate space or store the caller supplied space.
 * Tl = Local type, the local storage type
 * Tc = Caller type, the caller's storage type
 */
template<class Tl, class Tc>
Tl* Allocate(int iSize, Tc** ioCData, void** ioLData)
{
    Tl* data;
    if (*ioCData)
    {
        // Non null - no need to allocate
        data = (Tl*)*ioCData;
        *ioLData = 0;
    }
    else
    {
        // Null caller data - need to allocate
        data = (Tl*)fftwf_malloc(iSize * sizeof(Tl));
        *ioLData = data;
        *ioCData = (Tc*)data;
    }
    assert(data);
    return data;
}


/*
 * Complex to Complex transform
 */
void Tracter::Fourier::Init(int iOrder, complex** ioIData, complex** ioOData)
{
    assert(ioIData);
    assert(ioOData);
    assert(iOrder > 0);

    FourierFFTW::sInstanceCount++;

    assert(mFourierData == 0);
    mFourierData = new FourierData;
    FourierData& m = *mFourierData;

    fftwf_complex* idata =
        Allocate<fftwf_complex, complex>(iOrder, ioIData, &m.IData);
    fftwf_complex* odata =
        Allocate<fftwf_complex, complex>(iOrder, ioOData, &m.OData);
    m.Plan = fftwf_plan_dft_1d(iOrder, idata, odata, FFTW_FORWARD,
                               FFTW_ESTIMATE);
}


/*
 * Real to Complex transform
 */
void Tracter::Fourier::Init(int iOrder, float** ioIData, complex** ioOData)
{
    assert(ioIData);
    assert(ioOData);
    assert(iOrder > 0);

    FourierFFTW::sInstanceCount++;

    assert(mFourierData == 0);
    mFourierData = new FourierData;
    FourierData& m = *mFourierData;

    float* idata =
        Allocate<float, float>(iOrder, ioIData, &m.IData);
    fftwf_complex* odata =
        Allocate<fftwf_complex, complex>(iOrder/2+1, ioOData, &m.OData);
    m.Plan = fftwf_plan_dft_r2c_1d(iOrder, idata, odata, FFTW_ESTIMATE);
}

/**
 * Real to Real transform, a.k.a. cosine transform
 * Wired to do DFT2 right now.
 */
void Tracter::Fourier::Init(int iOrder, float** ioIData, float** ioOData)
{
    assert(ioIData);
    assert(ioOData);
    assert(iOrder > 0);

    FourierFFTW::sInstanceCount++;

    assert(mFourierData == 0);
    mFourierData = new FourierData;
    FourierData& m = *mFourierData;

    float* idata =
        Allocate<float, float>(iOrder, ioIData, &m.IData);
    float* odata =
        Allocate<float, float>(iOrder, ioOData, &m.OData);
    m.Plan = fftwf_plan_r2r_1d(iOrder, idata, odata, FFTW_REDFT10,
                               FFTW_ESTIMATE);
}

Tracter::Fourier::~Fourier()
{
    if (!mFourierData)
        return;

    assert(FourierFFTW::sInstanceCount > 0);
    FourierData& m = *mFourierData;

    fftwf_destroy_plan(m.Plan);
    if (m.IData)
        fftwf_free(m.IData);
    if (m.OData)
        fftwf_free(m.OData);
    delete mFourierData;

    if (--FourierFFTW::sInstanceCount == 0)
        fftwf_cleanup();
}

/**
 * Run the actual transform based on the parameters set up in the
 * constructor.
 */
void Tracter::Fourier::Transform()
{
    assert(mFourierData);
    FourierData& m = *mFourierData;
    fftwf_execute(m.Plan);
}
