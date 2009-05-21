/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef WINDOW_H
#define WINDOW_H

#include <vector>

#include "TracterObject.h"

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
        void Resize(int iSize, bool iDivideN = false);
        float* Apply(const float* iData, float* oData) const;
        const float operator[](int iIndex) { return mWeight[iIndex]; }

    private:
        std::vector<float> mWeight;
        Shape mShape;
    };
}

#endif /* WINDOW_H */
