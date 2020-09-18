/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MINIMA_H
#define MINIMA_H

#include <vector>
#include <cstdio>

#include "CachedComponent.h"

namespace Tracter
{
    /**
     *  A moveable window onto a data buffer.
     *
     *  @author Mike Flynn, Idiap Research Institute
     */
    class SlidingWindow
    {

    protected:

        IndexType windowOffset;     ///< current position of the window.
        const int   windowSize;     ///< size of the window.
        IndexType dataSize;         ///< size of the data buffer.

        IndexType mMinIndex;
        CacheArea mCacheArea;
        Component<float>* mInput;
        int mDataIndex;

    public:

        /**
         *  Construct a SlidingWindow on a data buffer.
         *  @param data     the data buffer.
         *  @param dataSize     the size of the data, in elements.
         *  @param windowSize   the size of the window.
         *  @param windowOffset the initial offset of the window.
         */
        SlidingWindow(int windowSize, int windowOffset);

        void    setWindowOffset(int windowOffset);

        /**
         *  Re-initialise the window to be completely before the left end
         *  of the buffer.
         */
        void    reset();

        /**
         *  Move the window one element to the right.
         */
        inline void shift() {
            windowOffset++;
        }

        void setData(
            Component<float>* iInput, int iDataIndex,
            IndexType iMinIndex, CacheArea iCacheArea
        );

        void setDataSize(IndexType iDataSize) {
            dataSize = iDataSize;
        };

        /**
         *  Retrieve an element from the window.
         *  @param index    the index into the window.
         */
        float    get(IndexType index);
        //{
        //    return data[index+windowOffset];
        //}

        /**
         *  Write a textual representation of this object to a FILE stream.
         *  @param out  the FILE stream for writing.
         */
        void    dump(FILE *out);
    };


    class MinimaWindow : public SlidingWindow {

    protected:

        IndexType* mins;    ///< array of data indices of the minumum window values.
        const int   minsSize;       ///< number of elements in the minimums array.
        int minCount;       ///< number of valid elements in the minimums array.
        float maxValue;       ///< value of the largest of the minimums.
        int maxIndex;       ///< index of the largest of the valid minimums.
        double minSum;         ///< sum of the valid minimums.

        void determineMaximum(); ///< determine which of the minimums is the largest.
        void scanMinimum();      ///< determine the minimum unheld element in the window.

    public:

        /**
         *  Construct a MinimaWindow on a data buffer.
         *  @param data     data buffer.
         *  @param dataSize     size of the data, in elements.
         *  @param window       size of the window.
         *  @param nMins        number of minimums to keep for the window.
         */
        MinimaWindow(int window, int nMins);
        ~MinimaWindow();

        /**
         *  Re-initialise the window to be completely before the left end
         *  of the buffer.
         */
        void reset();

        /**
         *  Move the window one element to the right.
         */
        inline void shift()
        {       // was 21%   !!!!!
            SlidingWindow::shift();
            shrinkLeft(); // should be (data, windowOffset)
            growRight();  // should be (data, index, windowSize)
        }

        void shrinkLeft();       ///< collapse the left end of the window right one element.
        void growRight();        ///< extend the right end of the window right one element.

        /**
         *  Calculate the arithmetic mean of the minimums in the window.
         */
        inline float getMean() {
            assert(minCount);
            return minSum/minCount;
        }

        /**
         *  Calculate the arithmetic means of the whole data buffer.
         *  @param means an array, as long as the data buffer, to hold the
         *  means; if NULL, or not supplied, an appropriate array will be
         *  allocated and returned.
         */
        //float    *getMeans(float *means = NULL);

        /**
         *  Write a textual representation of this object to a FILE
         *  stream.  @param out the FILE stream for writing.
         */
        void    dump(FILE *out);
    };


    class Minima : public CachedComponent<float>
    {
    public:
        Minima(Component<float>* iInput, const char* iObjectName = "Minima");
        virtual ~Minima();

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);
        void reset(bool iPropagate);

    private:
        Component<float>* mInput;
        int mNWindow;
        int mNAhead;
        float mCorrection;
        IndexType mLastIndex;
        std::vector< MinimaWindow* > mWindow;
        bool uFetch(IndexType iIndex, float* oData);
    };
}

#endif /* MINIMA_H */
