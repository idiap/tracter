#ifndef FEATURE_H
#define FEATURE_H

#include "CachedPlugin.h"

/**
 * Constucts a feature vector from constituent features
 */
class Feature : public CachedPlugin<float>
{
public:
    Feature(
        Plugin<float>* iStatic, int iNStatic,
        Plugin<float>* iDelta1, int iNDelta1,
        Plugin<float>* iDelta2, int iNDelta2
    );

protected:
    PluginObject* GetInput(int iInput);
    bool ProcessFrame(IndexType iIndex, int iOffset);

private:
    Plugin<float>* mStatic;
    Plugin<float>* mDelta1;
    Plugin<float>* mDelta2;
    int mNStatic;
    int mNDelta1;
    int mNDelta2;
};

#endif /* FEATURE_H */
