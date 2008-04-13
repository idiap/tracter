/*
 * Copyright 2008 by IDIAP Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef LAGUERRE_H
#define LAGUERRE_H

#include <vector>

/**
 * Solve polynomials via Laguerre's method
 *
 * This is an implementation of the algorithm described at
 * http://en.wikipedia.org/wiki/Laguerre's_method
 *
 * It finds the closest root to a given estimate remarkably quickly,
 * even if the estimate is not all that close.
 */
class Laguerre
{
public:
    void SetOrder(int iOrder);
    float Evaluate(float iEstimate, std::vector<float>& iPoly);
private:
    int mOrder;
    std::vector<float> mD;
    std::vector<float> mDD;
};

#endif /* LAGUERRE_H */
