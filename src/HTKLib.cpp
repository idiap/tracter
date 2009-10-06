/*
 * Copyright 2009 by Idiap Research Institute, http://www.idiap.ch
 *
 * Copyright 2008 by The University of Sheffield
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "config.h"
#include "HTKLib.h"

namespace Tracter
{
#ifdef HAVE_HTKLIB
# include "HShell.h"
# include "HMem.h"
# include "HMath.h"
# include "HAudio.h"
# include "HWave.h"
# include "HParm.h"
# include "HLabel.h"
# include "HModel.h"
# include "HSigP.h"
# include "HVQ.h"
# include "HUtil.h"
# include "HTrain.h"
# include "HAdapt.h"
#endif

    /* This is the singleton HTKLib instantiation */
    HTKLib sHTKLib;
}


Tracter::HTKLib::HTKLib()
{
    mObjectName = "HTKLib";
    mHTKLibSource = 0;

    Verbose(1, "Instantiated\n");
}
