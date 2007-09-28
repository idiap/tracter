#ifndef DELTA_H
#define DELTA_H

#include <vector>

#include "CachedPlugin.h"

/**
 * Plugin to calculate delta features
 */
class Delta : public CachedPlugin<float>
{
public:
    Delta(Plugin<float>* iInput, int iArraySize, int iTheta);

protected:
    bool ProcessFrame(IndexType iIndex, int iOffset);
    virtual void MinSize(int iSize);
    PluginObject* GetInput(int iInput);

private:
    Plugin<float>* mInput;
    int mTheta;
    int mWindow;
    std::vector<float*> mFeature;
    std::vector<float> mWeight;
};

#endif /* DELTA_H */
