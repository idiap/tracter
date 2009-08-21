/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cassert>
#include <cstdio>
#include <errno.h>

#ifdef _WIN32
# include <direct.h>
#else
# include <sys/stat.h>
# include <sys/types.h>
#endif

#include "TracterObject.h"
#include "FilePath.h"

/**
 * Breaks the given filename into path, base and extension.  To
 * specify a directory rather than a file, add a trailing '/'.  This
 * call can be repeated as often as necessary for many filenames as
 * the internal state is cleared each time.  Repeating the call in
 * this way is more efficient than creating a new instance for each
 * filename as storage is re-used without being re-allocated.
 */
void Tracter::FilePath::SetName(const char* iFilePath)
{
    assert(iFilePath);

    // Reset
    mPath.clear();
    mBase.clear();
    mExtension.clear();
    int ext = 0;
    int base = 0;
    int len = 0;

    // Scan for separators
    for (len=0; iFilePath[len] != '\0'; len++)
    {
        switch(iFilePath[len])
        {
        case '/':
            base = len + 1;
            break;
        case '.':
            ext = len + 1;
            break;
        }
    }
    if (base >= ext)
        ext = 0;

    // Copy to local storage
    if (base > 0)
        mPath.assign(iFilePath, base-1);
    mBase.assign(iFilePath+base, ext ? ext-base-1 : len-base);
    if (ext > 0)
        mExtension.assign(iFilePath+ext, len-ext);
}

/**
 * Creates the path section of the stored filename meaning that a
 * subsequent open of the full filename using standard library
 * routines should work.  Similar to 'mkdir -p'.
 */
void Tracter::FilePath::MakePath()
{
    // Return if the path isn't defined
    if (mPath.size() == 0)
        return;

    // Copy to temp storage so we can mess with it
    mTmp.resize(mPath.size()+1);
    mPath.copy(&mTmp[0], mTmp.size());
    mTmp[mPath.size()] = '\0';

    // Replace separators with nulls
    // Ignore the first character
    for (unsigned int i=1; i<mTmp.size(); i++)
    {
        if ((mTmp[i] == '/') || (mTmp[i] == '\0'))
        {
            mTmp[i] = '\0';
            if (
#ifdef _WIN32
                mkdir(&mTmp[0]) &&
#else
                mkdir(&mTmp[0], 0777) &&
#endif
                (errno != EEXIST)
            )
            {
                perror("mkdir");
                Dump();
                throw Exception("FilePath: MakePath failed for \"%s\"",
                                &mTmp[0]);
            }
            mTmp[i] = '/';
        }
    }
}

/**
 * Dump the internal state.  Basically for debugging.
 */
void Tracter::FilePath::Dump()
{
    printf(" Path: %s\n", mPath.c_str());
    printf(" Base: %s\n", mBase.c_str());
    printf(" Extension: %s\n", mExtension.c_str());
}
