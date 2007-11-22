#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "MMap.h"

MMap::MMap()
{
    mFD = 0;
    mMap = 0;
}

MMap::~MMap()
{
    if (mMap)
        munmap(mMap, mSize);
    if (mFD)
        close(mFD);
}

size_t MMap::GetSize()
{
    return mSize;
}

void* MMap::Map(const char* iFileName)
{
    assert(iFileName);

    // Close the previous map if there was one
    if (mMap)
       if (munmap(mMap, mSize) != 0)
        {
            printf("Failed to unmap file\n");
            exit(EXIT_FAILURE);
        }
    if (mFD)
        if (close(mFD) != 0)
        {
            printf("Failed to close file\n");
            exit(EXIT_FAILURE);
        }

    mFD = open(iFileName, O_RDONLY);
    if (mFD == -1)
    {
        printf("MMap: Failed to open file %s\n", iFileName);
        exit(1);
    }

    // Get the map size via the file size
    struct stat buf;
    fstat(mFD, &buf);
    mSize = buf.st_size;

    // Do the actual map.  Hint that the OS can use the same place as before.
    mMap = mmap(mMap, buf.st_size, PROT_READ, MAP_SHARED, mFD, 0);
    if (mMap == MAP_FAILED)
    {
        printf("MMap: Failed to map file %s\n", iFileName);
        exit(1);
    }

    return mMap;
}
