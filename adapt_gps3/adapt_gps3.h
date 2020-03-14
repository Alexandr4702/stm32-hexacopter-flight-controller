/*
 * File: adapt_gps3.h
 *
 * MATLAB Coder version            : 2.7
 * C/C++ source code generated on  : 15-Apr-2019 16:08:52
 */

#ifndef __ADAPT_GPS3_H__
#define __ADAPT_GPS3_H__

/* Include Files */
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "rt_defines.h"
#include "rt_nonfinite.h"
#include "rtwtypes.h"
#include "adapt_gps3_types.h"

/* Function Declarations */
extern void adapt_gps3(const double zom[3], const double za[3], const double
  Vsns[3], boolean_T status_sns, double B_SNS, double *psi, double *theta,
  double *b_gamma);
extern void adapt_gps3_initialize(void);
extern void adapt_gps3_terminate(void);

#endif

/*
 * File trailer for adapt_gps3.h
 *
 * [EOF]
 */
