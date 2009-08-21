/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef TOKENISE_H
#define TOKENISE_H

#include <string>
#include "CachedComponent.h"

namespace Tracter
{
    class Tokenise : public CachedComponent<std::string>
    {
    public:
        Tokenise(Component<char>* iInput, const char* iObjectName = "Tokenise");
        virtual ~Tokenise() throw() {}

    protected:
        std::string mQuote;
        std::string mWhite;
        std::string mSpecial;
        std::string mComment;

        bool isWhite(const char& iChar) { return inStr(iChar, mWhite); }
        bool isQuote(const char& iChar) { return inStr(iChar, mQuote); }
        bool isSpecial(const char& iChar) { return inStr(iChar, mSpecial); }
        bool isComment(const char& iChar) { return inStr(iChar, mComment); }

        int mLine;
        int mIndex;

    private:
        Component<char>* mInput;
        bool UnaryFetch(IndexType iIndex, std::string* oData);
        bool inStr(const char& iChar, const std::string& mStr)
        {
            size_t pos = mStr.find(iChar);
            return pos == std::string::npos ? false : true;
        }
    };
}

#endif /* TOKENISE_H */
