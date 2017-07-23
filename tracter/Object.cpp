/*
 * Copyright 2007 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstdarg>
#include <cstring>

#include "tracter/Object.h"

int Tracter::sVerbose = 0;

void Tracter::Object::verbose(var iVerbose)
{
    sVerbose = iVerbose.cast<int>();
    Verbose(1, "verbosity set to %d\n", sVerbose);
    Verbose(1, "library version %s\n", PACKAGE_VERSION);
}


/**
 * Uses the name of the object as a prefix and iSuffix as a suffix to
 * construct a configuration.
 *
 * @returns The value of the configuration, or 0 if it was not
 * set.
 */
const char* Tracter::Object::getConfig(
    const char* iSuffix, const char* iDefault, bool iEcho
)
{
    assert(objectName());
    const char* ret = Config::config(iSuffix, (const char*)0);
    if (iEcho && (sVerbose > 0))
    {
        if (!ret)
            printf("# ");
        printf("[%s] %s = %s\n", objectName(), iSuffix, ret ? ret : iDefault);
    }
    return ret;
}

/**
 * Get value from configuration.
 * @returns the value, or the value in iDefault if not set.
 */
float Tracter::Object::GetEnv(const char* iSuffix, float iDefault)
{
    char def[256];
    if (sVerbose > 0)
        snprintf(def, 256,
                 (fabs(iDefault) < 1e-2) ? "%.3e" : "%.3f", iDefault);
    if (const char* env = getConfig(iSuffix, def))
        return atof(env);
    return iDefault;
}

/**
 * Get value from configuration.
 * @returns the value, or the value in iDefault if not set.
 */
int Tracter::Object::GetEnv(const char* iSuffix, int iDefault)
{
    char def[256];
    if (sVerbose > 0)
        snprintf(def, 256, "%d", iDefault);
    if (const char* env = getConfig(iSuffix, def))
        return atoi(env);
    return iDefault;
}

/**
 * Get value from configuration.
 * @returns the value, or the value in iDefault if not set.
 */
const char* Tracter::Object::GetEnv(
    const char* iSuffix, const char* iDefault
)
{
    char def[256];
    if (sVerbose > 0)
        snprintf(def, 256, "%s", iDefault);
    if (const char* env = getConfig(iSuffix, def))
        return env;
    return iDefault;
}

/**
 * Get an enumeration from a configuration.
 *
 * The idea here is that all the possible enumerations are requested
 * separately.  That way, the user sees all possible options when they
 * are echoed.
 */
int Tracter::Object::GetEnv(const StringEnum* iStringEnum, int iDefault)
{
    const char* def[] = {"0", "1"};

    /*
     * This is a bit tricky.  We need two passes; the first is to find
     * out whether anything is set, the second echos the setting, be
     * it default or configuration.
     */
    int i = -1;
    int match = 0;
    while (iStringEnum[++i].str)
    {
        // Suppress echo; just count
        if (const char* r = getConfig(iStringEnum[i].str, def[0], false))
            if (strcmp(r, def[0]) != 0)
                match++;
    }
    bool useDefault = (match == 0);

    i = -1;
    int ret = -1;
    while (iStringEnum[++i].str)
    {
        int d = (useDefault && (iStringEnum[i].val == iDefault)) ? 1 : 0;
        if (const char* r = getConfig(iStringEnum[i].str, def[d]))
            if (strcmp(r, def[0]) != 0)
                ret = iStringEnum[i].val;
    }
    if (match > 1)
        throw Exception("%s: enumeration with multiple values", objectName());
    if (useDefault)
        return iDefault;
    return ret;
}


/**
 * Verbose output.  Prints output to stdout depending on the verbosity
 * level.  Written using cstdarg such that printf like parameter lists
 * can be passed.
 *
 * Although the level is somewhat arbitrary, the following seems a
 * reasonable policy:
 *  - 0 Completely Quiet
 *  - 1 Once per invocation (construct, initialise...)
 *  - 2 One per segment (file, utterance...)
 *  - 3 Several times per segment
 *  - 4 Once per frame (but not audio frequency)
 *  - 5 Per frame, unlimited frequency
 */
void Tracter::Object::Verbose(int iLevel, const char* iString, ...) const
{
    if (iLevel > sVerbose)
        return;

    printf("%s: ", objectName());
    va_list ap;
    va_start(ap, iString);
    vprintf(iString, ap);
    va_end(ap);
    fflush(stdout); // Feedback immediately
}

/**
 * Exception constructor.  Allows exception strings up to some fixed
 * length using the cstdarg mechanism.
 */
Tracter::Exception::Exception(const char* iString, ...)
{
    va_list ap;
    va_start(ap, iString);
    int n = vsnprintf(mString, STRING_SIZE, iString, ap);
    va_end(ap);
    if ((n < 0) || (n >= STRING_SIZE))
    {
        // Truncated; replace the last 3 characters with an elipsis
        mString[STRING_SIZE-2] = '.';
        mString[STRING_SIZE-3] = '.';
        mString[STRING_SIZE-4] = '.';
    }
}
