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
    Connect(iInput, BLOCK_SIZE);

    mWhite = " \n\t";
    mQuote = "\"\'";
    mSpecial = "+-={}[]";
    mComment = "#;";

    mIndex = 0;
    mLine = 1;
}

bool Tracter::Tokenise::UnaryFetch(IndexType iIndex, Token* oData)
{
    assert(iIndex >= 0);

    bool inToken = false;
    bool inComment = false;
    char quote = 0;
    Token& token = *oData;
    token.str.clear();
    token.pos = mLine;

    // Read the input
    CacheArea inputArea;
    while (mInput->Read(inputArea, mIndex, BLOCK_SIZE))
    {
        CacheIterator<char> mIterator(mInput, inputArea);
        for (SizeType i=0; i<inputArea.Length(); ++i, ++mIterator, ++mIndex)
        {
            char c = *mIterator;

            if (c == '\n')
                token.pos = ++mLine;

            if (quote)
            {
                if (c == quote)
                {
                    mIndex++;
                    return true;
                }
                token.str += c;
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

            if (isSpecial(c))
            {
                if (inToken)
                    return true;
                else
                {
                    token.str += c;
                    mIndex++;
                    return true;
                }
            }

            inToken = true;
            token.str += c;
        }
    }
    // Dropping out here means the end of the file

    // In the middle of something
    if (quote)
        throw Exception("Unexpected EOF at line %ld\n", mLine);

    // Deals with lack of a final newline
    if (inToken)
        return true;

    return false;
}
