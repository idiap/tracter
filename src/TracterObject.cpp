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

#include "TracterObject.h"

bool Tracter::sInitialised = false;
bool Tracter::sShowConfig = false;
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

    // else...
    sShowConfig = GetEnv("shConfig", 0);
    if (Tracter::sShowConfig)
        GetEnv("shConfig", 0); // Do it again to get the output :-)
    sVerbose = GetEnv("Verbose", 0);
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
    const char* iSuffix, const char* iDefault
)
{
    assert(mObjectName);
    char env[256];
    snprintf(env, 256, "%s_%s", mObjectName, iSuffix);
    const char* ret = getenv(env);
    if (Tracter::sShowConfig)
    {
        snprintf(env, 256, "export %s_%s=%s", mObjectName, iSuffix,
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
    if (Tracter::sShowConfig)
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
    if (Tracter::sShowConfig)
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
    if (Tracter::sShowConfig)
        snprintf(def, 256, "%s", iDefault);
    if (const char* env = getEnv(iSuffix, def))
        return env;
    return iDefault;
}

#include <cstdarg>

/**
 * Verbose output.  Prints output to stdout depending on the verbosity
 * level.  Written using cstdarg such that printf like parameter lists
 * can be passed.
 */
void Tracter::Object::Verbose(int iLevel, const char* iString, ...)
{
    if (iLevel > sVerbose)
        return;

    printf("%s: ", mObjectName);
    va_list ap;
    va_start(ap, iString);
    vprintf(iString, ap);
    va_end(ap);
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
