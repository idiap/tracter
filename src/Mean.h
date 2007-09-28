#ifndef MEAN_H
#define MEAN_H

#include <vector>
#include "UnaryPlugin.h"

/**
 * Calculates the mean (over time) of the input stream, typically for
 * Cepstral Mean Normalisation.
 */
class Mean : public UnaryPlugin<float, float>
{
public:
    Mean(Plugin<float>* iInput, int iArraySize);
    virtual void Reset(bool iPropagate);

protected:
    bool ProcessFrame(IndexType iIndex, int iOffset);

private:
    bool mValid;
    std::vector<float> mMean;
};


#endif /* MEAN_H */
