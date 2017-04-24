/*
 * Copyright 2011 by Idiap Research Institute, http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 *
 * Author(s):
 *   Phil Garner, June 2011
 */

#ifndef UNFRAME_H
#define UNFRAME_H

#include "CachedComponent.h"

namespace Tracter
{
    class Unframe : public CachedComponent<float>
    {
    public:
        Unframe(Component<float>* iInput,
                const char* iObjectName = "Unframe");

    protected:
        SizeType ContiguousFetch(IndexType iIndex,
                            SizeType iLength, SizeType iOffset);

    private:        Component<float>* mInput;
    };
}

#endif /* UNFRAME_H */
