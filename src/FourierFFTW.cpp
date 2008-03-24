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

    fftwf_complex* idata = 0;
    if (*ioIData)
    {
        idata = (fftwf_complex*)*ioIData;
        m.IData = 0;
    }
    else
    {
        idata = (fftwf_complex*)fftwf_malloc(iOrder * sizeof(fftwf_complex));
        m.IData = idata;
        *ioIData = (complex*)idata;
    }

    fftwf_complex* odata = 0;
    if (*ioOData)
    {
        odata = (fftwf_complex*)*ioOData;
        m.OData = 0;
    }
    else
    {
        odata = (fftwf_complex*)fftwf_malloc(iOrder * sizeof(fftwf_complex));
        m.OData = odata;
        *ioOData = (complex*)odata;
    }

    assert(idata);
    assert(odata);
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

    float* idata = 0;
    if (*ioIData)
    {
        idata = *ioIData;
        m.IData = 0;
    }
    else
    {
        idata = (float*)fftwf_malloc(iOrder * sizeof(float));
        m.IData = idata;
        *ioIData = idata;
    }

    fftwf_complex* odata = 0;
    if (*ioOData)
    {
        odata = (fftwf_complex*)*ioOData;
        m.OData = 0;
    }
    else
    {
        odata =
            (fftwf_complex*)fftwf_malloc((iOrder/2+1) * sizeof(fftwf_complex));
        m.OData = odata;
        *ioOData = (complex*)odata;
    }

    assert(idata);
    assert(odata);
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

    float* idata = 0;
    if (*ioIData)
    {
        idata = *ioIData;
        m.IData = 0;
    }
    else
    {
        idata = (float*)fftwf_malloc(iOrder * sizeof(float));
        m.IData = idata;
        *ioIData = idata;
    }

    float* odata = 0;
    if (*ioOData)
    {
        odata = *ioOData;
        m.OData = 0;
    }
    else
    {
        odata = (float*)fftwf_malloc(iOrder * sizeof(float));
        m.OData = odata;
        *ioOData = odata;
    }

    assert(idata);
    assert(odata);
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
    {
        fftwf_free(m.IData);
        m.IData = 0;
    }

    if (m.OData)
    {
        fftwf_free(m.OData);
        m.OData = 0;
    }

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
