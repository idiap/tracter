/*
 * HCopyWrapper.h
 *
 *  Created on: 01-Jul-2008
 *      Author: Vincent Wan
 */

#ifndef HCOPYWRAPPER_H_
#define HCOPYWRAPPER_H_

#include "UnaryPlugin.h"
#include "HShell.h"
#include "HMem.h"
#include "HMath.h"
#include "HSigP.h"
#include "HAudio.h"
#include "HWave.h"
#include "HVQ.h"
#include "HParm.h"
#include "HLabel.h"
#include "HModel.h"
#include "HParm.h"
#include "esignal.h"
#ifdef UNIX
#include <sys/ioctl.h>
#endif

#undef FALSE
#undef TRUE


extern "C"
{
    /******************************************************************
     * Audio interface required by HParm - see cpp file for more info.
     ******************************************************************/
    Ptr  fOpen(Ptr xInfo,char *fn,BufferInfo *info);
    void fClose(Ptr xInfo,Ptr bInfo);
    void fStart(Ptr xInfo,Ptr bInfo);
    void fStop(Ptr xInfo,Ptr bInfo);
    int  fNumSamp(Ptr xInfo,Ptr bInfo);
    int  fGetData(Ptr xInfo,Ptr bInfo,int n,Ptr data);
};

class HCopyWrapper : public UnaryPlugin<float, short>
{
public:
    HCopyWrapper(
        Plugin<short>* iInput, const char* iObjectName = "HCopyWrapper"
    );
    ~HCopyWrapper() throw();
    bool UnaryFetch(IndexType iIndex, int iOffset);
    void setmArraySize( int featureDimension );
    Ptr  fOpen__(char *fn,BufferInfo *info);
    int  fNumSamp__(Ptr bInfo);
    int  fGetData__(Ptr bInfo,int n,Ptr data);

protected:

private:
    long lastSampleCopied;  /* A largest index of samples copied to HTK */
    long lastFrameCopied;   /* A largest index of frames copied into tracter */
    MemHeap iStack;         /* HTK MSTAK memory heap */
    ParmBuf pbuf;           /* HTK buffer where HParm does all its work */
    Observation data;       /* The place where HTK writes the feature vector */
};


#endif /* HCOPYWRAPPER_H_ */
