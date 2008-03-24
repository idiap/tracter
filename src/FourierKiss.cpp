/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <assert.h>
#include "kiss_fft.h"
#include "kiss_fftr.h"
#include "Fourier.h"

namespace FourierKiss
{
    static int sInstanceCount = 0;
};

enum TransformType
{
    REAL_TO_COMPLEX,
    COMPLEX_TO_COMPLEX,
    DCT2
};

/**
 * The class data for FourierKiss
 */
struct FourierData
{
    void* IData;
    void* OData;
    bool MyIData;
    bool MyOData;
    void* Config;
    TransformType Type;
    int Order;
    kiss_fft_cpx* TmpData;
};


/* C to C */
void Fourier::Init(int iOrder, complex** ioIData, complex** ioOData)
{
    assert(ioIData);
    assert(ioOData);
    assert(iOrder > 0);
    assert(sizeof(complex) == sizeof(kiss_fft_cpx));

    FourierKiss::sInstanceCount++;

    assert(mFourierData == 0);
    mFourierData = new FourierData;
    FourierData& m = *mFourierData;

    m.Type = COMPLEX_TO_COMPLEX;

    kiss_fft_cpx* idata = 0;
    if (*ioIData)
    {
        idata = (kiss_fft_cpx*)*ioIData;
        m.IData = 0;
        m.MyIData = false;
    }
    else
    {
        idata = (kiss_fft_cpx*)KISS_FFT_MALLOC(iOrder*sizeof(kiss_fft_cpx));
        m.IData = idata;
        *ioIData = (complex*)idata;
        m.MyIData = true;
    }

    kiss_fft_cpx* odata = 0;
    if (*ioOData)
    {
        odata = (kiss_fft_cpx*)*ioOData;
        m.OData = 0;
        m.MyOData = false;
    }
    else
    {
        odata = (kiss_fft_cpx*)KISS_FFT_MALLOC(iOrder*sizeof(kiss_fft_cpx));
        m.OData = odata;
        *ioOData = (complex*)odata;
        m.MyOData = true;
    }

    m.TmpData = 0;
    assert(idata);
    assert(odata);
    m.Config = kiss_fft_alloc(iOrder, 0, 0, 0);
}


/**
 * Real to Complex transform
 */
void Fourier::Init(int iOrder, float** ioIData, complex** ioOData)
{
    assert(ioIData);
    assert(ioOData);
    assert(iOrder > 0);
    assert(sizeof(complex) == sizeof(kiss_fft_cpx));

    FourierKiss::sInstanceCount++;

    assert(mFourierData == 0);
    mFourierData = new FourierData;
    FourierData& m = *mFourierData;

    m.Type = REAL_TO_COMPLEX;

    float* idata = 0;
    if (*ioIData)
    {
        idata = *ioIData;
        m.IData = 0;
        m.MyIData = false;
    }
    else
    {
        idata = (float*)KISS_FFT_MALLOC(iOrder * sizeof(float));
        m.IData = idata;
        *ioIData = idata;
        m.MyIData = true;
    }

    kiss_fft_cpx* odata = 0;
    if (*ioOData)
    {
        odata = (kiss_fft_cpx*)*ioOData;
        m.OData = 0;
        m.MyOData = false;
    }
    else
    {
        odata =
            (kiss_fft_cpx*)KISS_FFT_MALLOC((iOrder/2+1)*sizeof(kiss_fft_cpx));
        m.OData = odata;
        *ioOData = (complex*)odata;
        m.MyOData = true;
    }

    m.TmpData = 0;
    assert(idata);
    assert(odata);
    m.Config = kiss_fftr_alloc(iOrder, 0, 0, 0);
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

    FourierKiss::sInstanceCount++;

    assert(mFourierData == 0);
    mFourierData = new FourierData;
    FourierData& m = *mFourierData;

    m.Type = DCT2;
    m.Order = iOrder;

    float* idata = 0;
    if (*ioIData)
    {
        /* Disallow this; the caller doesn't know we need double size */
        assert(0);
        idata = *ioIData;
        m.IData = 0;
        m.MyIData = false;
    }
    else
    {
        /* Double size for the duplication */
        idata = (float*)KISS_FFT_MALLOC(iOrder * 2 * sizeof(float));
        m.IData = idata;
        *ioIData = idata;
        m.MyIData = true;
    }

    float* odata = 0;
    if (*ioOData)
    {
        odata = *ioOData;
        m.OData = 0;
        m.MyOData = false;
    }
    else
    {
        odata = (float*)KISS_FFT_MALLOC(iOrder * sizeof(float));
        m.OData = odata;
        *ioOData = odata;
        m.MyOData = true;
    }
    m.TmpData = 
        (kiss_fft_cpx*)KISS_FFT_MALLOC((iOrder+1)*sizeof(kiss_fft_cpx));

    assert(m.TmpData);
    assert(idata);
    assert(odata);
    m.Config = kiss_fftr_alloc(iOrder*2, 0, 0, 0);
}

Fourier::~Fourier()
{
    if (!mFourierData)
        return;

    assert(FourierKiss::sInstanceCount > 0);
    FourierData& m = *mFourierData;

    if (m.Config)
    {
        kiss_fftr_free(m.Config);
        m.Config = 0;
    }

    if (m.MyIData && m.IData)
    {
        free(m.IData);
        m.IData = 0;
        m.MyIData = false;
    }

    if (m.MyOData && m.OData)
    {
        free(m.OData);
        m.OData = 0;
        m.MyOData = 0;
    }

    if (m.TmpData)
    {
        free(m.TmpData);
        m.TmpData = 0;
    }

    delete mFourierData;

    if (--FourierKiss::sInstanceCount == 0)
        kiss_fft_cleanup();
}

/**
 * Run the actual transform based on the parameters set up in the
 * constructor.
 */
void Fourier::Transform()
{
    assert(mFourierData);
    FourierData& m = *mFourierData;

    switch (m.Type)
    {
    case DCT2:
    {
        /* Duplicate */
        float* idata = (float*)m.IData;
        for (int i=0; i<m.Order; i++)
        {
            /* [1 2 3] -> [1 2 3 3 2 1] */
            idata[i+m.Order] = idata[m.Order-1-i];
        }

        /* Transform and rotate */
        kiss_fftr((kiss_fftr_cfg)m.Config, (const float*)m.IData, m.TmpData);
        float* odata = (float*)m.OData;
        for (int i=0; i<m.Order; i++)
        {
            float theta = 2.0f*M_PI*(-0.5f)*i/(2.0f*m.Order);
            odata[i] = ( m.TmpData[i].r * cos(theta) -
                         m.TmpData[i].i * sin(theta) );
        }
        break;
    }
    case REAL_TO_COMPLEX:
        kiss_fftr(
            (kiss_fftr_cfg)m.Config,
            (const float*)m.IData, (kiss_fft_cpx*)m.OData
        );
        break;

    case COMPLEX_TO_COMPLEX:
        kiss_fft(
            (kiss_fft_cfg)m.Config,
            (const kiss_fft_cpx*)m.IData, (kiss_fft_cpx*)m.OData
        );
        break;

    default:
        assert(0);
    }
}
