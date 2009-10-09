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
# undef VERSION
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

    /*
     * ---------------- Information about transforms ------------
     */
    XFInfo xfInfo;
}


/**
 * Constructor.
 */
Tracter::HTKLib::HTKLib()
{
    mObjectName = "HTKLib";
    mHTKLibSource = 0;

    Verbose(1, "Instantiated\n");
}

/**
 * Initialise
 */
void Tracter::HTKLib::Initialise(const char* iScript)
{
#ifdef HAVE_HTKLIB
    const char* config = GetEnv("Config", "");
    if (*config)
        InitialiseHTK(config, iScript);
#endif
}


/*
 *  -------------------- Initialisation ---------------------
 */
void Tracter::HTKLib::InitialiseHTK(const char* iConfig, const char* iScript)
{
#ifdef HAVE_HTKLIB
    /**
     * Simulate command line options
     *       Option                                       Default
     *
     *       -C cf   Set config file to cf                default
     *       -D      Display configuration variables      off
     *       -S f    Set script file to f                 none
     **/
    int argc = 6;
    char *argv[argc];
    argv[0] = (char*)"Tracter::HTKLib";
    argv[1] = (char*)"-C";
    argv[2] = (char*)iConfig;
    argv[3] = (char*)"-S";
    argv[4] = (char*)iScript;
    argv[5] = (char*)"-D";

    /*
     * Standard Global HTK initialisation
     */
    char *hvite_version = (char*)"!HVER!HVite:   3.4 [CUED 25/04/06]";
    char *hvite_vc_id = (char*)"$Id: HVite.c,v 1.1.1.1 2006/10/11 09:55:02 jal58 Exp $";
    if(InitShell(argc,argv,hvite_version,hvite_vc_id)<SUCCESS)
        HError(2600,(char*)"HTKLib: InitShell failed");
    InitMem();
    InitLabel();
    InitMath();
    InitSigP();
    InitWave();
    InitAudio();
    InitVQ();
    InitModel();
    if(InitParm()<SUCCESS)
        HError(2600,(char*)"HTKLib: InitParm failed");
    InitUtil();
    InitAdapt(&xfInfo);

    if (NumArgs() > 0)
        Verbose(1, "HTK config loaded:\n\t%s\n\t%d segments in %s\n",
                iConfig, NumArgs(), iScript);
#endif
}
