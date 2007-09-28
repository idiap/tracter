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
        // Nothing to do
    }

    /**
     * Get a pointer to the storage
     * returns a reference.
     */
    virtual T* GetPointer(int iIndex = 0) = 0;

protected:

};

#endif /* PLUGIN_H */
