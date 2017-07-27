/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef CONCATENATE_H
#define CONCATENATE_H

#include "CachedComponent.h"

namespace Tracter
{
    /**
     * Constucts a concatenated vector from constituent inputs
     */
    class Concatenate : public CachedComponent<float>
    {
    public:
        Concatenate(const char* iObjectName = "Concatenate");
        virtual ~Concatenate() throw() {}
        void Add(Component<float>* iInput);

    protected:
        bool unaryFetch(IndexType iIndex, float* oData);

    private:
        std::vector< Component<float>* > mInput;
        std::vector<int> mLength;
    };
}

#endif /* CONCATENATE_H */
