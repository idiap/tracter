/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "Tokenise.h"

/* Rather arbitrary, but should stop too many cache fetches */
const int BLOCK_SIZE = 16;

Tracter::Tokenise::Tokenise(Component<char>* iInput, const char* iObjectName)
{
    mObjectName = iObjectName;
    mInput = iInput;
    Connect(mInput, BLOCK_SIZE);

    mWhite = " \n\t";
    mQuote = "\"\'";
    mSpecial = "+-={}[]";
    mComment = "#;";

    mIndex = 0;
    mLine = 0;
}

bool Tracter::Tokenise::UnaryFetch(IndexType iIndex, std::string* oData)
{
    assert(iIndex >= 0);

    bool inToken = false;
    bool inComment = false;
    char quote = 0;
    std::string& token = *oData;
    token.clear();

    // Read the input
    CacheArea inputArea;
    while (mInput->Read(inputArea, mIndex, BLOCK_SIZE))
    {
        CacheIterator<char> mIterator(mInput, inputArea);
        for (int i=0; i<inputArea.Length(); ++i, ++mIterator, ++mIndex)
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
        throw Exception("Unexpected EOF at line %d\n", mLine);

    // Deals with lack of a final newline
    if (inToken)
        return true;

    return false;
}
