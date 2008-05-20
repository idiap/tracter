/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "FileSource.h"
#include "Tokenise.h"
#include "ArraySink.h"

int main()
{
    FileSource<char>* f = new FileSource<char>();
    Tokenise* t = new Tokenise(f);
    ArraySink<std::string> sink = ArraySink<std::string>(t);

    char* file = "testfile.txt";
    printf("Opening file %s\n", file);
    f->Open(file);

    int index = 0;
    std::string* p;
    while (sink.GetArray(p, index++))
    {
        printf("Got: %s\n", p->c_str());
    }
}
