#include <cassert>
#include <cstdio>
#include <cstdlib>
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
    if (Tracter::sInitialised)
        return;

    mObjectName = "Tracter";

    Tracter::sShowConfig = GetEnv("shConfig", 0);
    if (Tracter::sShowConfig)
        GetEnv("shConfig", 0); // Do it again to get the output :-)
    Tracter::sVerbose = GetEnv("Verbose", 0);
    Tracter::sInitialised = true;
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
        snprintf(def, 256, "%f", iDefault);
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
