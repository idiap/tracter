#ifndef SUBTRACT_H
#define SUBTRACT_H

#include "CachedPlugin.h"

/**
 * Subtracts a second input from a first.
 */
class Subtract : public CachedPlugin<float>
{
public:
    Subtract(Plugin<float>* iInput1, Plugin<float>* iInput2,
             const char* iObjectName = "Subtract");

protected:
    PluginObject* GetInput(int iInput);
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    Plugin<float>* mInput1;
    Plugin<float>* mInput2;
};

#endif /* SUBTRACT_H */
