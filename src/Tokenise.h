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
    /** A token is the thing you'd expect, plus a position, typically
     * a line number */
    struct Token
    {
        std::string str; ///< Token string
        SizeType pos;    ///< Position (typically a line number)
    };

    /**
     * A tokeniser.  In some sense this is to prove that a parser can
     * be written as a data flow module.  This tokeniser takes a file
     * source (actually a char component) and breaks it into tokens.
     * It is designed to be inherited by a concrete tokeniser for a
     * specific file format.
     *
     * Tokens are split on white-space and on "special" characters,
     * all of which are customisable.  Quoted strings and comment
     * lines are also dealt with.  It can easily be configured to
     * parse formats where syntax can be inferred by single characters
     * rather than regular expressions.  i.e., it can probably do XML
     * by defining mSpecial = "<>&", but C is probably difficult
     * because "/", "*" and "\/\*" have wildly different meanings.
     * Windoze ".ini" files are probably easy.
     *
     * Aside, the world has too many XML parsers.  Please use one of
     * them rather than writing one based on this class.
     */
    class Tokenise : public CachedComponent<Token>
    {
    public:
        Tokenise(Component<char>* iInput, const char* iObjectName = "Tokenise");
        virtual ~Tokenise() throw() {}

    protected:
        std::string mQuote;
        std::string mWhite;
        std::string mSpecial;
        std::string mComment;

        bool isWhite(const char& iChar) const { return inStr(iChar, mWhite); }
        bool isQuote(const char& iChar) const { return inStr(iChar, mQuote); }
        bool isSpecial(const char& iChar) const {
            return inStr(iChar, mSpecial);
        }
        bool isComment(const char& iChar) const {
            return inStr(iChar, mComment);
        }

        SizeType mLine;
        IndexType mIndex;

    private:
        Component<char>* mInput;
        bool UnaryFetch(IndexType iIndex, Token* oData);
        bool inStr(const char& iChar, const std::string& mStr) const
        {
            size_t pos = mStr.find(iChar);
            return pos == std::string::npos ? false : true;
        }
    };
}

#endif /* TOKENISE_H */
