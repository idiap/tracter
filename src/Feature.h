/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

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
        Plugin<float>* iStatic,
        Plugin<float>* iDelta1,
        Plugin<float>* iDelta2,
        const char* iObjectName = "Feature"
    );

protected:
    PluginObject* GetInput(int iInput);
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    Plugin<float>* mStatic;
    Plugin<float>* mDelta1;
    Plugin<float>* mDelta2;
    int mNStatic;
    int mNDelta1;
    int mNDelta2;
};

#endif /* FEATURE_H */
