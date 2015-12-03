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

bool Tracter::sInitialised = false;
bool Tracter::sShConfig = false;
bool Tracter::sCshConfig = false;
int Tracter::sVerbose = 0;

/**
 * Constructor.  Initialises static verbosity and config output
 * options, or returns immediately if they are set.
 */
Tracter::Object::Object()
{
    mObjectName = "Tracter";

    if (Tracter::sInitialised)
        return;

    // else... set up the parameter echoing
    sShConfig  = GetEnv("shConfig", 0);
    sCshConfig = GetEnv("cshConfig", 0);

    // Do it again to get the output :-)
    if (sShConfig)
        GetEnv("shConfig", 0);
    if (sCshConfig)
        GetEnv("cshConfig", 0);

    // And the verbosity
    sVerbose = GetEnv("Verbose", 0);
    Verbose(1, "version %s\n", PACKAGE_VERSION);
    sInitialised = true;
}


/**
 * Uses the name of the object as a prefix and iSuffix as a suffix to
 * construct an environment variable.
 *
 * @returns The value of the environment variable, or 0 if it was not
 * set.
 */
const char* Tracter::Object::getEnv(
    const char* iSuffix, const char* iDefault, bool iEcho
)
{
    assert(mObjectName);
    char env[256];
    snprintf(env, 256, "%s_%s", mObjectName, iSuffix);
    const char* ret = getenv(env);
    if (iEcho && (sShConfig || sCshConfig))
    {
        if (sShConfig)
            snprintf(env, 256, "export %s_%s=%s", mObjectName, iSuffix,
                     ret ? ret : iDefault);
        if (sCshConfig)
            snprintf(env, 256, "setenv %s_%s %s", mObjectName, iSuffix,
                     ret ? ret : iDefault);
        printf("%-50s", env);
        if (ret)
            printf("# Environment\n");
        else
            printf("# Default\n");
    }
    return ret;
}

/**
 * Get value from environment variable.
 * @returns the value, or the value in iDefault if not set.
 */
float Tracter::Object::GetEnv(const char* iSuffix, float iDefault)
{
    char def[256];
    if (sShConfig || sCshConfig)
        snprintf(def, 256,
                 (fabs(iDefault) < 1e-2) ? "%.3e" : "%.3f", iDefault);
    if (const char* env = getEnv(iSuffix, def))
        return atof(env);
    return iDefault;
}

/**
 * Get value from environment variable.
 * @returns the value, or the value in iDefault if not set.
 */
int Tracter::Object::GetEnv(const char* iSuffix, int iDefault)
{
    char def[256];
    if (sShConfig || sCshConfig)
        snprintf(def, 256, "%d", iDefault);
    if (const char* env = getEnv(iSuffix, def))
        return atoi(env);
    return iDefault;
}

/**
 * Get value from environment variable.
 * @returns the value, or the value in iDefault if not set.
 */
const char* Tracter::Object::GetEnv(
    const char* iSuffix, const char* iDefault
)
{
    char def[256];
    if (sShConfig || sCshConfig)
        snprintf(def, 256, "%s", iDefault);
    if (const char* env = getEnv(iSuffix, def))
        return env;
    return iDefault;
}

/**
 * Get an enumeration from an environment variable.
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
     * it default or environment.
     */
    int i = -1;
    int match = 0;
    while (iStringEnum[++i].str)
    {
        // Suppress echo; just count
        if (const char* r = getEnv(iStringEnum[i].str, def[0], false))
            if (strcmp(r, def[0]) != 0)
                match++;
    }
    bool useDefault = (match == 0);

    i = -1;
    int ret = -1;
    while (iStringEnum[++i].str)
    {
        int d = (useDefault && (iStringEnum[i].val == iDefault)) ? 1 : 0;
        if (const char* r = getEnv(iStringEnum[i].str, def[d]))
            if (strcmp(r, def[0]) != 0)
                ret = iStringEnum[i].val;
    }
    if (match > 1)
        throw Exception("%s: enumeration with multiple values", mObjectName);
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

    printf("%s: ", mObjectName);
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
