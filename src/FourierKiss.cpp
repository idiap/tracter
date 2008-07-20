/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cassert>

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
struct Tracter::FourierData
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


/*
 * Either allocate space or store the caller supplied space.
 * Tl = Local type, the local storage type
 * Tc = Caller type, the caller's storage type
 */
template<class Tl, class Tc>
Tl* Allocate(int iSize, Tc** ioCData, void** ioLData, bool* iMyData)
{
    Tl* data;
    if (*ioCData)
    {
        // Non null - no need to allocate
        data = (Tl*)*ioCData;
        *ioLData = 0;
        *iMyData = false;
    }
    else
    {
        // Null caller data - need to allocate
        data = (Tl*)KISS_FFT_MALLOC(iSize*sizeof(Tl));
        *ioLData = data;
        *ioCData = (Tc*)data;
        *iMyData = true;
    }
    assert(data);
    return data;
}


/* C to C */
void Tracter::Fourier::Init(int iOrder, complex** ioIData, complex** ioOData)
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

    kiss_fft_cpx* idata =
        Allocate<kiss_fft_cpx, complex>(iOrder, ioIData, &m.IData, &m.MyIData);
    kiss_fft_cpx* odata =
        Allocate<kiss_fft_cpx, complex>(iOrder, ioOData, &m.OData, &m.MyOData);
    m.TmpData = 0;
    m.Config = kiss_fft_alloc(iOrder, 0, 0, 0);
}


/**
 * Real to Complex transform
 */
void Tracter::Fourier::Init(int iOrder, float** ioIData, complex** ioOData)
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

    float* idata =
        Allocate<float, float>(iOrder, ioIData, &m.IData, &m.MyIData);
    kiss_fft_cpx* odata =
        Allocate<kiss_fft_cpx, complex>(iOrder/2+1, ioOData, &m.OData,
                                        &m.MyOData);
    m.TmpData = 0;
    m.Config = kiss_fftr_alloc(iOrder, 0, 0, 0);
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

    FourierKiss::sInstanceCount++;

    assert(mFourierData == 0);
    mFourierData = new FourierData;
    FourierData& m = *mFourierData;

    m.Type = DCT2;
    m.Order = iOrder;

    float* idata =
        Allocate<float, float>(iOrder*2, ioIData, &m.IData, &m.MyIData);
    float* odata =
        Allocate<float, float>(iOrder, ioOData, &m.OData, &m.MyOData);
    m.TmpData =
        (kiss_fft_cpx*)KISS_FFT_MALLOC((iOrder+1)*sizeof(kiss_fft_cpx));
    assert(m.TmpData);
    m.Config = kiss_fftr_alloc(iOrder*2, 0, 0, 0);
}

Tracter::Fourier::~Fourier()
{
    if (!mFourierData)
        return;

    assert(FourierKiss::sInstanceCount > 0);
    FourierData& m = *mFourierData;

    if (m.Config)
        free(m.Config);
    if (m.MyIData && m.IData)
        free(m.IData);
    if (m.MyOData && m.OData)
        free(m.OData);
    if (m.TmpData)
        free(m.TmpData);
    delete mFourierData;

    if (--FourierKiss::sInstanceCount == 0)
        kiss_fft_cleanup();
}

/**
 * Run the actual transform based on the parameters set up in the
 * constructor.
 */
void Tracter::Fourier::Transform()
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
