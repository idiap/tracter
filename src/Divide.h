/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef DIVIDE_H
#define DIVIDE_H

#include "CachedPlugin.h"

namespace Tracter
{
    /**
     * Divides a first input by a second.
     */
    class Divide : public CachedPlugin<float>
    {
    public:
        Divide(Plugin<float>* iInput1, Plugin<float>* iInput2,
               const char* iObjectName = "Divide");

    protected:
        PluginObject* GetInput(int iInput);
        bool UnaryFetch(IndexType iIndex, int iOffset);

    private:
        Plugin<float>* mInput1;
        Plugin<float>* mInput2;
    };
}

#endif /* DIVIDE_H */
