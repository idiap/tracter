/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <assert.h>
#include <fftw3.h>
#include "Fourier.h"

namespace FourierFFTW
{
    static int sInstanceCount = 0;
};

struct FourierData
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
void Fourier::Init(int iOrder, complex** ioIData, complex** ioOData)
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
    m.Plan = fftwf_plan_dft_1d(iOrder, idata, odata, FFTW_FORWARD, 0);
}


/*
 * Real to Complex transform
 */
void Fourier::Init(int iOrder, float** ioIData, complex** ioOData)
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
    m.Plan = fftwf_plan_dft_r2c_1d(iOrder, idata, odata, 0);
}

/**
 * Real to Real transform, a.k.a. cosine transform
 * Wired to do DFT2 right now.
 */
void Fourier::Init(int iOrder, float** ioIData, float** ioOData)
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
    m.Plan = fftwf_plan_r2r_1d(iOrder, idata, odata, FFTW_REDFT10, 0);
}

Fourier::~Fourier()
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
void Fourier::Transform()
{
    assert(mFourierData);
    FourierData& m = *mFourierData;
    fftwf_execute(m.Plan);
}
