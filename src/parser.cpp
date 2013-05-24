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
    FrameSink<Token> sink(t);

    const char* file = "testfile.txt";
    printf("Opening file %s\n", file);
    f->Open(file);

    int index = 0;
    while (const Token* p = sink.Read(index++))
    {
        printf("%ld: %s\n", p->pos, p->str.c_str());
    }
}
