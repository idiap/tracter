/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>

#include "FileSource.h"
#include "Tokenise.h"
#include "FrameSink.h"

using namespace Tracter;

int main()
{
    FileSource<char>* f = new FileSource<char>();
    Tokenise* t = new Tokenise(f);
    FrameSink<std::string> sink = FrameSink<std::string>(t);

    const char* file = "testfile.txt";
    printf("Opening file %s\n", file);
    f->Open(file);

    int index = 0;
    while (const std::string* p = sink.Read(index++))
    {
        printf("Got: %s\n", p->c_str());
    }
}
