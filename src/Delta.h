#ifndef DELTA_H
#define DELTA_H

#include <vector>

#include "UnaryPlugin.h"

/**
 * Plugin to calculate delta features
 */
class Delta : public UnaryPlugin<float, float>
{
public:
    Delta(Plugin<float>* iInput, const char* iObjectName = "Delta");

protected:
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    int mTheta;
    int mWindow;
    std::vector<float*> mFeature;
    std::vector<float> mWeight;
};

#endif /* DELTA_H */
