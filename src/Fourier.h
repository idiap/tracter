/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef FOURIER_H
#define FOURIER_H

#include <complex>

namespace Tracter
{
    typedef std::complex<float> complex;

    /**
     * Implementation specific data
     */
    struct FourierData;

    /*
     * Interface to Fourier transform objects
     *
     * The general idea is that you choose at compile time which one you
     * want to use.  IPP, FFTW, Kiss etc.
     *
     * This is not an interface in the pure virtual sense.  The
     * implementation is chosen at compile time, not run time, so this is
     * the class that actually gets instantiated.  To get implementation
     * specific behaviour, the implementation specific FourierData is
     * aggregated with a pre-declaration, then declared and allocated in
     * the implementation's constructors.
     */
    class Fourier
    {
    public:
        /** Default constructor */
        Fourier()
        {
            mFourierData = 0;
        }

        /** Destructor */
        virtual ~Fourier();

        /** Constructor, including initialisation of complex to complex
         * transform */
        Fourier(int iOrder, complex** ioIData, complex** ioOData)
        {
            mFourierData = 0;
            Init(iOrder, ioIData, ioOData);
        }

        /** Initialise a complex to complex transform */
        void Init(int iOrder, complex** ioIData, complex** ioOData);

        /** Constructor, including initialisation of real to complex
         * transform */
        Fourier(int iOrder, float** ioIData, complex** ioOData)
        {
            mFourierData = 0;
            Init(iOrder, ioIData, ioOData);
        }

        /** Initialise a real to complex transform */
        void Init(int iOrder, float** ioIData, complex** ioOData);

        /** Constructor, including initialisation of real to real transform */
        Fourier(int iOrder, float** ioIData, float** ioOData)
        {
            mFourierData = 0;
            Init(iOrder, ioIData, ioOData);
        }

        /** Initialise real to real transform (DCT2) */
        void Init(int iOrder, float** ioIData, float** ioOData);

        /** Run the actual transform */
        void Transform();

    private:
        /** Implementation specific data */
        FourierData* mFourierData;
    };
}

#endif /* FOURIER_H */
