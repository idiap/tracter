#ifndef NOISE_H
#define NOISE_H

#include "UnaryPlugin.h"

/**
 * @brief Stores a noise estimate
 *
 * Reads the first NInit (default 10) frames and uses them to form a
 * noise estimate.  Typically, the input is a Periodogram.
 */
class Noise : public UnaryPlugin<float, float>
{
public:
    Noise(Plugin<float>* iInput, const char* iObjectName = "Noise");

protected:
    bool UnaryFetch(IndexType iIndex, int iOffset);

private:
    bool mValid;
    int mNInit;
};

#endif /* NOISE_H */
