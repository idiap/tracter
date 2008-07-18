/*
 * HCopyWrapper.h
 *
 *  Created on: 01-Jul-2008
 *      Author: Vincent Wan
 */

#ifndef HCOPYWRAPPER_H_
#define HCOPYWRAPPER_H_

/*
 * Basically, HCopy reads type short.  However, tracter is geared
 * around float.  This causes the plugin to convert from float to
 * short.
 */
#define HCOPY_FLOAT
#ifdef HCOPY_FLOAT
typedef float hcopy_t;
#else
typedef short hcopy_t;
#endif

#include "UnaryPlugin.h"

/* As few headers as possible to allow the class to compile */
#include "HShell.h"
#include "HMem.h"
#include "HWave.h"
#include "HAudio.h"
#include "HParm.h"

class HCopyWrapper
    : public UnaryPlugin<float, hcopy_t>
{
public:
    HCopyWrapper(
        Plugin<hcopy_t>* iInput, const char* iObjectName = "HCopyWrapper"
    );
    ~HCopyWrapper() throw();
    bool UnaryFetch(IndexType iIndex, int iOffset);
    void setmArraySize(int featureDimension);
    Ptr fOpen__(char *fn, BufferInfo *info);
    int fNumSamp__(Ptr bInfo);
    int fGetData__(Ptr bInfo, int n, Ptr data);

protected:

private:
    long lastSampleCopied;  ///< A largest index of samples copied to HTK
    long lastFrameCopied;   ///< A largest index of frames copied into tracter
    MemHeap iStack;         ///< HTK MSTAK memory heap
    ParmBuf pbuf;           ///< HTK buffer where HParm does all its work
    Observation data;       ///< The place where HTK writes the feature vector
};


#endif /* HCOPYWRAPPER_H_ */
