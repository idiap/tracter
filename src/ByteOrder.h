/*
 * Copyright 2007 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef BYTEORDER_H
#define BYTEORDER_H

#include "TracterObject.h" // for StringEnum

namespace Tracter
{
    /** Indicator of desired or measured byte order */
    enum Endian
    {
        ENDIAN_BIG,
        ENDIAN_LITTLE,
        ENDIAN_NATIVE,
        ENDIAN_UNDEF
    };

    extern const StringEnum cEndian[];

    /**
     * Stores the byte ordering of the machine, along with that of a
     * source and a target.  Given this information, it can determine
     * whether byte swapping is necessary or not.
     */
    class ByteOrder
    {
    public:
        ByteOrder();
        void Swap(void* iData, size_t iDataSize, int iDataCount);

        /** Set the source byte order */
        void SetSource(Endian iEndian)
        {
            mSource = iEndian;
            if (mSource == ENDIAN_NATIVE)
                mSource = mNative;
        }

        /** Set the target byte order */
        void SetTarget(Endian iEndian)
        {
            mTarget = iEndian;
            if (mTarget == ENDIAN_NATIVE)
                mTarget = mNative;
        }

        /** returns true if the source and target byte order are different */
        bool WrongEndian()
        {
            assert(mSource != ENDIAN_NATIVE);
            assert(mTarget != ENDIAN_NATIVE);
            return (mSource != mTarget);
        }

    private:
        Endian mNative;
        Endian mTarget;
        Endian mSource;
    };
}

#endif /* BYTEORDER_H */
