/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef MMAP_H
#define MMAP_H

#ifdef _WIN32
# include <windows.h>
typedef HANDLE FDType;
#else
typedef int FDType;
#endif

namespace Tracter
{
    /**
     * Wraps the usual unix mmap into a class
     */
    class MMap
    {
    public:
        MMap();
        ~MMap();
        void* Map(const char* iFileName);
        long int Size() { return (long int)mSize; }

    private:
        FDType mFD; // File descriptor
        void* mMap; // Mapped memory location
        size_t mSize;
    };
}

#endif /* MMAP_H */
