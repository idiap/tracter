/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef CONCATENATE_H
#define CONCATENATE_H

#include "CachedPlugin.h"

/**
 * Constucts a concatenated vector from constituent inputs
 */
class Concatenate : public CachedPlugin<float>
{
public:
    Concatenate(const char* iObjectName = "Concatenate");
    void Add(Plugin<float>* iInput);

protected:
    PluginObject* GetInput(int iInput);
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    std::vector< Plugin<float>* > mInput;
    std::vector<int> mLength;
};

#endif /* CONCATENATE_H */
