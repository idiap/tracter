/*
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#ifndef BLAS_LINEAR_INC
#define BLAS_LINEAR_INC

#include "GradientMachine.h"

namespace Torch {

/** BlasLinear layer for #GradientMachine#.
    Formally speaking, $ouputs[i] = \sum_j w_{ij} inputs[i] + b_i$.\\
    $w_{ij}$ and $b_j$ are in #params#, with the following structure:\\
    $w_00... w_0n, b_0, w_10... w_1n, b_1, ...$\\

    If you want, you can add a weight decay which looks like
    $\sum_i,j w_{ij}^2 + sum_i b_i^2$.

    Options:
    \begin{tabular}{lcll}
      "weight decay"  &  real  &  the weight decay & [0]
    \end{tabular}

    @author Ronan Collobert (collober@idiap.ch)
*/
class BlasLinear : public GradientMachine
{
  public:
    real weight_decay;
    real *weights;
    real *bias;
    real *der_weights;
    real *der_bias;
    void reset_();

    //-----

    ///
    BlasLinear(int n_inputs_, int n_outputs_);

    //-----

    virtual void frameForward(int t, real *f_inputs, real *f_outputs);
    virtual void frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_);

    /// Computes new random values for weights.
    virtual void reset();

    virtual ~BlasLinear();
};


}

#endif
