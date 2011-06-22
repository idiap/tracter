/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef GATE_H
#define GATE_H

#include <algorithm>

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Gate.
     *
     * Allows frames through from input to output depending on a
     * control input.
     */
    class Gate : public CachedComponent<float>
    {
    public:
        Gate(Component<float>* iInput,
             Component<BoolType>* iControlInput,
             const char* iObjectName = "Gate");

        /**
         * Intercept TimeStamp
         *
         * We need to adjust for the segmenting effect.  Adding the
         * opened time will cause ComponentBase::TimeStamp() to add on
         * an offset.
         */
        TimeType TimeStamp(IndexType iIndex) const
        {
            Verbose(2, "TimeStamp: index %ld  opened %ld\n",
                    iIndex, mOpenedIndex);
            return ComponentBase::TimeStamp(
                iIndex + std::max<IndexType>(mOpenedIndex, (IndexType)0)
            );
        }

    protected:
        bool UnaryFetch(IndexType iIndex, float* oData);
        virtual void Reset(bool iPropagate);

    private:
        Component<float>* mInput;
        Component<BoolType>* mControlInput;

        bool mEnabled;
        bool mSegmenting;
        bool mConcatenate;
        bool mUpstreamEndOfData;

        bool mOpen;
        IndexType mOpenedIndex; ///< Last frame at which gate was opened
        IndexType mClosedIndex; ///< Last frame at which gate was closed
        IndexType mIndexZero;   ///< Zero'th frame from upstream POV
        IndexType mRemoved;     ///< Number of unwanted frames removed

        bool gate(IndexType& iIndex);
        bool readControl(IndexType iIndex);
        bool openGate(IndexType iIndex);
    };
}

#endif /* GATE_H */
