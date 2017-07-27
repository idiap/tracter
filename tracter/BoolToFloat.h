/*
 * Copyright 2015 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef BOOLTOFLOAT_H
#define BOOLTOFLOAT_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Cache component that converts boolean input into
     * float data.
     *
     * This behavior is useful when concatenating float
     * features with boolean values.
     */
    class BoolToFloat : public CachedComponent<float>
    {
    public:
    	BoolToFloat(Component<BoolType>* iInput, 
                    const char* iObjectName = "BoolToFloat");
        
        virtual ~BoolToFloat() throw() {};

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        Component<BoolType>* mInput;
    };
}

#endif /* BOOLTOFLOAT_H */
