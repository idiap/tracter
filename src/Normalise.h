#ifndef NORMALISE_H
#define NORMALISE_H

#include "UnaryPlugin.h"

/**
 * Normalises an audio input of type short to be a float.
 * Should scale it to between -1 and +1 too, but doesn't yet.
 */
class Normalise : public UnaryPlugin<float, short>
{
public:
    Normalise(Plugin<short>* iInput);
    void MinSize(int iSize);

protected:
    int Process(IndexType iIndex, CacheArea& iOutputArea);
};

#endif /* NORMALISE_H */
