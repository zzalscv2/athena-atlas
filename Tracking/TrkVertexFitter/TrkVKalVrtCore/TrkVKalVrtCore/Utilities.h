/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Author: V.Kostyukhin
 */
#ifndef TRKVKALVRTCORE_UTILITIES_H
#define TRKVKALVRTCORE_UTILITIES_H
#include <array>

#include "TrkVKalVrtCore/TrkVKalVrtCoreBase.h"

namespace Trk {
double cfchi2(double *xyzt, const long int ich, double *part,
              const double *par0, double *wgt, double *rmnd);
double cfchi2(const double *vrtt, const double *part, VKTrack *trk);
double finter(double y0, double y1, double y2, double x0, double x1, double x2);
void tdasatVK(const double *Der, const double *CovI, double *CovF, long int M,
              long int N);
void cfsetdiag(long int n, double *matr, double value) noexcept;
void abcCoef(double g1, double g2, double g3, double &a, double &b, double &c);
void efdCoef(double Ga0, double Gamb, double Gab, double Gw0, double Gwb,
             double alf, double bet, double w, double &d, double &e, double &f);
void ParaMin(double b, double c, double d, double e, double f, double &xmin,
             double &ymin);
void cfTrkCovarCorr(double *cov);

}  // namespace Trk

#endif

