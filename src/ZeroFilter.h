#ifndef ZEROFILTER_H
#define ZEROFILTER_H

#include "UnaryPlugin.h"

/**
 * Implements a filter consisting of a single zero.
 */
class ZeroFilter : public UnaryPlugin<float, float>
{
public:
    ZeroFilter(Plugin<float>* iInput, float iZero);
    void MinSize(int iSize);

protected:
    int Process(IndexType iIndex, CacheArea& iOutputArea);

private:
    float mZero;
};

#endif /* ZEROFILTER_H */
