/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_MATRIX_H
#define TRKVKALVRTCORE_MATRIX_H

namespace Trk {
double cfSmallEigenvalue(double *cov, long int n);
int cfInv5(double *cov, double *wgt);
int cfdinv(double *cov, double *wgt, long int NI);
void dsinv(long int n, double *a, long int DIM, long int *ifail) noexcept;
int vkMSolve(double *a, double *b, long int n, double *ainv = nullptr);
void vkSVDCmp(double **a, int m, int n, double w[], double **v);
void vkGetEigVal(const double ci[], double d[], int n);
void vkGetEigVect(const double ci[], double d[], double vect[], int n);

}  // namespace Trk

#endif
