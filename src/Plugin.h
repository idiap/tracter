/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef PLUGIN_H
#define PLUGIN_H

#include "PluginObject.h"

/**
 * This is an interface for the type specific implementation of the
 * plugin.  The implementation could be a cache or memory map.
 */
template <class T>
class Plugin : public PluginObject
{
public:
    virtual ~Plugin<T>()
    {
        // A kind of post-hoc check that the next plugin called Connect()
        assert(mNOutputs > 0);
    }

    /**
     * Get a pointer to the storage
     * returns a reference.
     */
    virtual T* GetPointer(int iIndex = 0) = 0;

protected:

};

#endif /* PLUGIN_H */
