/*
 * Copyright 2008 by Idiap Research Institute
 *                   http://www.idiap.ch
 *
 * See the file COPYING for the licence associated with this software.
 */

#include "BlasLinear.h"
#include "Random.h"

//extern "C" {
//#include "cblas.h"
//}

namespace Torch {

  
BlasLinear::BlasLinear(int n_inputs_, int n_outputs_) : GradientMachine(n_inputs_, n_outputs_, (n_inputs_+1)*n_outputs_)
{
  addROption("weight decay", &weight_decay, 0, "weight decay");
  weights = params->data[0];
  bias = params->data[0]+n_inputs*n_outputs;
  der_weights = der_params->data[0];
  der_bias = der_params->data[0]+n_inputs*n_outputs;
  reset_();
}

void BlasLinear::reset()
{
  reset_();
}

void BlasLinear::reset_()
{
  // Note: just to be compatible with "Torch II Dev"
  real *weights_ = weights;
  real bound = 1./sqrt((real)n_inputs);

  for(int i = 0; i < n_outputs; i++)
  {
    for(int j = 0; j < n_inputs; j++)
    {
      weights_[j] = Random::boundedUniform(-bound, bound);
    }
    weights_ += n_inputs;
    bias[i] = Random::boundedUniform(-bound, bound);
  }

}

void BlasLinear::frameForward(int t, real *f_inputs, real *f_outputs)
{

  // ATLAS BLAS code - need to have the library installed
  /*
#ifdef USE_DOUBLE
  cblas_dcopy(n_outputs, bias, 1, f_outputs, 1);
  cblas_dgemv(CblasRowMajor, CblasNoTrans, n_outputs, n_inputs, 1., weights, n_inputs, f_inputs, 1, 1., f_outputs, 1);
#else
  cblas_scopy(n_outputs, bias, 1, f_outputs, 1);
  cblas_sgemv(CblasRowMajor, CblasNoTrans, n_outputs, n_inputs, 1., weights, n_inputs, f_inputs, 1, 1., f_outputs, 1);
#endif
  */

  real *weights_ = weights;
  for(int i = 0; i < n_outputs; i++)
  {
    real out = bias[i];

    for(int j = 0; j < n_inputs; j++)
      out += weights_[j] * f_inputs[j];
    weights_ += n_inputs;

    f_outputs[i] = out;
  }
}

void BlasLinear::frameBackward(int t, real *f_inputs, real *beta_, real *f_outputs, real *alpha_)
{
  if(!partial_backprop)
  {
    for(int i = 0; i < n_inputs; i++)
      beta_[i] = 0;
    
    real *weights_ = weights;
    for(int i = 0; i < n_outputs; i++)
    {
      real z = alpha_[i];
      for(int j = 0; j < n_inputs; j++)
        beta_[j] += z * weights_[j];
      weights_ += n_inputs;
    }
  
    // ATLAS BLAS implementation, need to have the library installed
    /*
#ifdef USE_DOUBLE
    cblas_dgemv(CblasRowMajor, CblasTrans, n_outputs, n_inputs, 1., weights, n_inputs, alpha_, 1, 0., beta_, 1);
    #else
    cblas_sgemv(CblasRowMajor, CblasTrans, n_outputs, n_inputs, 1., weights, n_inputs, alpha_, 1, 0., beta_, 1);
#endif
    */
  }

  /*
#ifdef USE_DOUBLE
  cblas_dger(CblasRowMajor, n_outputs, n_inputs, 1., alpha_, 1, f_inputs, 1, der_weights, n_inputs);
  cblas_daxpy(n_outputs, 1., alpha_, 1, der_bias, 1);
#else
  cblas_sger(CblasRowMajor, n_outputs, n_inputs, 1., alpha_, 1, f_inputs, 1, der_weights, n_inputs);
  cblas_saxpy(n_outputs, 1., alpha_, 1, der_bias, 1);
#endif
  */


  real *der_weights_ = der_weights;
  for(int i = 0; i < n_outputs; i++)
  {
    real z = alpha_[i];
    for(int j = 0; j < n_inputs; j++)
      der_weights_[j] += z * f_inputs[j];
    der_weights_ += n_inputs;

    der_bias[i] += z;
  }


  if(weight_decay != 0)
  {
    real *src_ = params->data[0];
    real *dest_ = der_params->data[0];
    // Note: pas de weight decay sur les biais.
    for(int i = 0; i < n_inputs*n_outputs; i++)
      dest_[i] += weight_decay * src_[i];
  }
}

BlasLinear::~BlasLinear()
{
}

}
