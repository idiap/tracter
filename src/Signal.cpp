/*
 * Copyright 2009 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <signal.h>
#ifdef HAVE_FPU_CONTROL
# include <fpu_control.h>
#endif

#include "Signal.h"
#include "TracterObject.h"

namespace Tracter
{
    void HandlerFPE(int iSignal);
    void SigActionFPE(int iSignal, siginfo_t* iSigInfo, void* iAddr);
}

/**
 * Single argument signal handler
 * Not used anymore
 */
void Tracter::HandlerFPE(int iSignal)
{
    throw Exception("Caught signal %d\n", iSignal);
}

/**
 * FPE signal handler
 *
 * Uses the 3 argument sigaction form to determine what kind of FPE
 * occured.  Turns it into a Tracter::Exception()
 */
void Tracter::SigActionFPE(int iSignal, siginfo_t* iSigInfo, void* iAddr)
{
    if (iSignal != SIGFPE)
        throw Exception("FPE handler called with signal %d", iSignal);

    const char* code = 0;
    switch (iSigInfo->si_code)
    {
    case FPE_INTDIV:
        code = "integer divide by zero";
        break;

    case FPE_INTOVF:
        code = "integer overflow";
        break;

    case FPE_FLTDIV:
        code = "floating point divide by zero";
        break;

    case FPE_FLTOVF:
        code = "floating point overflow";
        break;

    case FPE_FLTUND:
        code = "floating point underflow";
        break;

    case FPE_FLTRES:
        code = "floating point inexact result";
        break;

    case FPE_FLTINV:
        code = "floating point invalid operation";
        break;

    case FPE_FLTSUB:
        code = "subscript out of range";
        break;

    default:
        code = "unknown exception";
    }

    throw Exception("FPE: %s", code);
}

/**
 * Enable FPE trap
 */
void Tracter::TrapFPE()
{
    /*
     * From /usr/include/fpu_control.h
     * IM: Invalid operation mask
     * DM: Denormalized operand mask
     * ZM: Zero-divide mask
     * OM: Overflow mask
     * UM: Underflow mask
     * PM: Precision (inexact result) mask
     */
#ifdef HAVE_FPU_CONTROL
    fpu_control_t cw;
    _FPU_GETCW(cw);
    cw &= ~(_FPU_MASK_IM | _FPU_MASK_ZM | _FPU_MASK_OM | _FPU_MASK_UM);
    _FPU_SETCW(cw);

    struct sigaction sa;
    sa.sa_sigaction = SigActionFPE;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGFPE, &sa, 0))
        throw Exception("Error setting sigaction\n");
#endif
}
