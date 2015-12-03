/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <stdio.h>

#include "Extract.h"
#include "tracter/FPE.h"

using namespace Tracter;

/**
 * Extracter executable.
 */
int main(int argc, char** argv)
{
    /*
     * It's debatable whether it's even worth catching these
     * exceptions.  If not caught, the same error messages appear via
     * the terminate call.  An example of how to do it perhaps?
     */
    try
    {
        /* Instantiate a factory and an extractor */
        ASRFactory factory;
        Extract extract(argc, argv, &factory);

        /* And run it */
        extract.All();
    }
    catch(std::exception& e)
    {
        fprintf(stderr, "Caught exception: %s\n", e.what());
        return 1;
    }
    catch(...)
    {
        fprintf(stderr, "Caught unknown exception\n");
        return 1;
    }

    return 0;
}
