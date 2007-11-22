/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <stdio.h>

/**
 * Writes a fake audio-like file of short ints with values identical to
 * their index in the file.  Allows checking that the plugins read the right data.
 */
int main(int argc, char** argv)
{
    FILE *fp;
    short i;

    fp = fopen("testfile.dat", "wb");
    if (!fp)
    {
        printf("Can't open testfile.dat\n");
        return 1;
    }
    for (i=0; i<1024; i++)
    {
        fwrite(&i, 2, 1, fp);
    }
    fclose(fp);

    return 0;
}
