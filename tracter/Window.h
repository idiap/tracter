/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <vector>

#include "tracter/Object.h"

namespace Tracter
{
    /**
     * A window
     *
     * Defaults to Hamming, but can represent a variety of different
     * window shapes.  The shapes are taken from
     * http://en.wikipedia.org/wiki/Window_function
     */
    class Window : public Object
    {
    public:
        enum Shape
        {
            RECTANGULAR,
            HANN,
            HAMMING,
            GAUSS
        };
        Window(const char* iObjectName = "Window", int iSize = 0);
        virtual ~Window() throw () {}
        void resize(int iSize, bool iDivideN = false);
        float* Apply(const float* iData, float* oData) const;
        const float operator[](int iIndex) { return mWeight[iIndex]; }
        const float at(int iIndex) { return mWeight.at(iIndex); }

        /**
         * Scale the window by a constant value.  Particularly
         * (perhaps only) useful to divide through by N in an inverse
         * transform.
         */
        void Scale(float iScale) {
            for (unsigned int i=0; i<mWeight.size(); i++)
                mWeight[i] *= iScale;
        }

    private:
        std::vector<float> mWeight;
        Shape mShape;
    };
}

#endif /* WINDOW_H */
