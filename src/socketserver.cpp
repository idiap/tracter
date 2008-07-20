/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "FileSource.h"
#include "Normalise.h"
#include "SocketSink.h"

using namespace Tracter;

int main(int argc, char** argv)
{
    printf("Socket client\n");

    Tracter::sShowConfig = true;
    FileSource<short>* fs = new FileSource<short>;
    Normalise* n = new Normalise(fs);
    SocketSink ss(n);
    ss.Reset();

    fs->Open("testfile.dat");
    ss.Pull();

    printf("Done\n");
    return 0;
}
