/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cstdio>
#include <cassert>

#include "TracterObject.h"
#include "MMap.h"

Tracter::MMap::MMap()
{
    mFD = 0;
    mMap = 0;
}


#ifdef _WIN32

#include <windows.h>

Tracter::MMap::~MMap()
{
    if (mMap)
        UnmapViewOfFile(mMap);
    if (mFD)
        CloseHandle(mFD);
}

/**
 * Map file using Win32 calls
 */
void* Tracter::MMap::Map(const char* iFileName)
{
    assert(iFileName);

    // Close the previous map if there was one
    if (mMap)
        if (UnmapViewOfFile(mMap) != 0)
            throw Exception("MMap: Failed to unmap file");
    if (mFD)
        if (CloseHandle(mFD) != 0)
            throw Exception("MMap: Failed to close file");

    mFD = OpenFileMapping(FILE_MAP_READ, FALSE, iFileName);
    if (mFD == 0)
        throw Exception("MMap: Failed to open file %s", iFileName);

    // Do the actual map.
    mMap = MapViewOfFile(mFD, FILE_MAP_READ, 0, 0, 0);
    if (mMap == 0)
        throw Exception("MMap: Failed to map file %s", iFileName);

    // Get the map size
    MEMORY_BASIC_INFORMATION buf;
    VirtualQuery(mMap, &buf, sizeof(MEMORY_BASIC_INFORMATION));
    mSize = buf.RegionSize;

    if (sVerbose > 1)
        printf("MMap: %s size %d\n", iFileName, (int)mSize);

    return mMap;
}

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>

Tracter::MMap::~MMap()
{
    if (mMap)
        munmap(mMap, mSize);
    if (mFD)
        close(mFD);
}

/**
 * Map file using posix calls
 *
 * This call can fail on NFS or big parallel systems, hence all the
 * checks.  At Idiap I find that the stat works, but sometimes returns
 * zero size.  This in turn caused the map to fail.  So there is a
 * loop.
 */
void* Tracter::MMap::Map(const char* iFileName)
{
    assert(iFileName);

    // Close the previous map if there was one
    if (mMap)
        if (munmap(mMap, mSize) != 0)
            throw Exception("MMap: Failed to unmap file");
    if (mFD)
        if (close(mFD) != 0)
            throw Exception("MMap: Failed to close file");

    mFD = open(iFileName, O_RDONLY);
    if (mFD == -1)
        throw Exception("MMap: Failed to open file %s", iFileName);

    // Get the map size via the file size.  Try to stat the file a few
    // times before failing.
    struct stat buf;
    for (int i=1; i<=5; i++)
    {
        if (fstat(mFD, &buf))
        {
            perror("fstat()");
            throw Exception("MMap: Failed to stat file %s", iFileName);
        }
        if (buf.st_size)
            break;
        if (sVerbose > 1)
            printf("MMap: stat'd zero size file; retry in 1 second %d\n", i);
        sleep(1);
    }
    mSize = buf.st_size;
    if (mSize == 0)
        throw Exception("MMap: Attempt to map zero size file %s", iFileName);

    // Do the actual map.  Hint that the OS can use the same place as before.
    mMap = mmap(mMap, buf.st_size, PROT_READ, MAP_SHARED, mFD, 0);
    if (mMap == MAP_FAILED)
    {
        perror("mmap()");
        throw Exception("MMap: Failed to map file %s", iFileName);
    }

    if (sVerbose > 1)
        printf("MMap: %s size %d\n", iFileName, (int)mSize);

    return mMap;
}

#endif
