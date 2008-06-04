/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef UNARYPLUGIN_H
#define UNARYPLUGIN_H

#include "CachedPlugin.h"

/**
 * Defines input handling code for plugins with only one input.
 */
template <class T, class Ti>
class UnaryPlugin : public CachedPlugin<T>
{
public:
    /**
     * The array size of a unary plugin defaults to either 0 (like 1,
     * but faster) if not supplied, or the array size of the input
     * plugin if negative.
     */
    UnaryPlugin(Plugin<Ti>* iInput)
        : CachedPlugin<T>()
    {
        Connect(iInput);
        mInput = iInput;
    }
    virtual ~UnaryPlugin() throw () {}

protected:
    PluginObject* GetInput(int iInput)
    {
        assert(iInput == 0);
        assert(this->mNInputs == 1);
        return mInput;
    }

    Plugin<Ti>* mInput;
};

#endif /* UNARYPLUGIN_H */
