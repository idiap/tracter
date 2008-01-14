/*
 * Copyright 2007,2008 by IDIAP Research Institute
 *                        http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef PIXMAP_H
#define PIXMAP_H

#include "UnaryPlugin.h"

/**
 * Writes a Pixmap (image) representing the input
 */
class Pixmap : public UnaryPlugin<float, float>
{
public:
    Pixmap(Plugin<float>* iInput, const char* iObjectName = "Pixmap");

protected:
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    void write();

    int mLoIndex;
    int mHiIndex;
    float mMin;
    float mMax;

    bool mLog;
};


#endif /* PIXMAP_H */
