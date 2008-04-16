/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Tokenise.h"

/* Rather arbitrary, but should stop too many cache fetches */
const int BLOCK_SIZE = 16;

Tokenise::Tokenise(Plugin<char>* iInput, const char* iObjectName)
    : UnaryPlugin<std::string, char>(iInput)
{
    mObjectName = iObjectName;
    mWhite = " \n\t";
    mQuote = "\"\'";
    mSpecial = "+-={}[]";
    mComment = "#;";
    MinSize(iInput, BLOCK_SIZE);

    mIndex = 0;
    mLine = 0;
}

bool Tokenise::UnaryFetch(IndexType iIndex, int iOffset)
{
    assert(iIndex >= 0);

    bool inToken = false;
    bool inComment = false;
    char quote = 0;
    std::string& token = *GetPointer(iOffset);
    token.clear();

    // Read the input
    CacheArea mInputArea;
    while (mInput->Read(mInputArea, mIndex, BLOCK_SIZE))
    {
        CacheIterator<char> mIterator(mInput, mInputArea);
        for (int i=0; i<mInputArea.Length(); ++i, ++mIterator, ++mIndex)
        {
            char c = *mIterator;

            if (c == '\n')
                mLine++;

            if (quote)
            {
                if (c == quote)
                {
                    mIndex++;
                    return true;
                }
                token += c;
                continue;
            }

            if (inComment)
            {
                if (c == '\n')
                    inComment = false;
                continue;
            }

            if (isComment(c))
            {
                if (inToken)
                {
                    // No mIndex++, keep the quote in the buffer
                    return true;
                }
                inComment = true;
                continue;
            }

            if (isWhite(c))
            {
                if (inToken)
                {
                    mIndex++;
                    return true;
                }
                continue;
            }

            if (isQuote(c))
            {
                if (inToken)
                {
                    // No mIndex++, keep the quote in the buffer
                    return true;
                }
                else
                {
                    inToken = true;
                    quote = c;
                    continue;
                }
            }

            inToken = true;
            token += c;
        }
    }
    // Dropping out here means the end of the file

    // In the middle of something
    if (quote)
    {
        printf("Unexpected EOF at line %d\n", mLine);
        exit(EXIT_FAILURE);
    }

    // Deals with lack of a final newline
    if (inToken)
        return true;

    return false;
}
