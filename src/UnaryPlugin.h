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
    UnaryPlugin(Plugin<Ti>* iInput, int iArraySize=0)
        : CachedPlugin<T>(iArraySize)
    {
        assert(iInput);
        mInput = iInput;
        this->mNInputs++;
    }

protected:
    PluginObject* GetInput(int iInput)
    {
        assert(iInput == 0);
        return mInput;
    }

    Plugin<Ti>* mInput;
};

#endif /* UNARYPLUGIN_H */
