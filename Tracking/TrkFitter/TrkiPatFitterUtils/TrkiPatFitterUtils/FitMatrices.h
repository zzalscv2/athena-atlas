/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////////
// class FitMatrices
//  Storage and manipulation of matrices during track fitting
//  (note the actual matrices are structs (FitMatrix.h) to give faster
//  execution)
//
//  Given matrix of measurement derivatives wrt fit parameters (DerivativeMatrix
//  DM) and vector of differences between measurements and fitted trajectory,
//  solve for:
//   parameter weight matrix = (covariance)-1 = DMtranspose.DM
//   parameter change        = (DMtranspose.DM)-1 * (DMtranspose.differences)
//
//  NOTE:
//  covariances, derivatives etc, use d0, z0, phi, cot(theta), qOverPt as first
//  5 parameters distinguish: full covariance with all parameters:
//			includes misalignments, scattering and energy loss
//               5*5 final covariance with possible external contribution
//               representing 	leading material and field gradient effects
//
//////////////////////////////////////////////////////////////////////////////

#ifndef TRKIPATFITTERUTILS_FITMATRICES_H
#define TRKIPATFITTERUTILS_FITMATRICES_H

#include <cmath>
#include <list>

#include "EventPrimitives/EventPrimitives.h"
#include "GaudiKernel/MsgStream.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkiPatFitterUtils/FitMatrix.h"

namespace Trk {
class FitMeasurement;
class FitParameters;

class FitMatrices {
 public:
  FitMatrices(bool constrainedAlignmentEffects);

  // copy, assignment deleted
  FitMatrices(const FitMatrices&) = delete;
  FitMatrices& operator=(const FitMatrices&) = delete;
  //move defaulted
  FitMatrices(FitMatrices&&) = default;
  FitMatrices& operator=(FitMatrices&&) = default;
  //dtor default
  ~FitMatrices() = default ;

  // debugging aid: check 'smart' pointers
  void checkPointers(MsgStream& log) const;

  // change to chiSquared
  static double chiSquaredChange(void);

  // accessor to DerivativeMatrix (eigen)
  Amg::MatrixX& derivativeMatrix();

  // accessor to final covariance (5*5)
  //   includes leading material effects on construction, but
  //   field gradient effects have to be added by MeasurementProcessor as
  //   'extrapolation aware'
  Amg::MatrixX& finalCovariance();

  // produce full covariance (including scatterer parameters) i.e. invert weight
  // matrix NOTE: this does not contain the external errors which will be
  // included in the finalCovariance
  //       or the leading material contribution
  const Amg::MatrixX* fullCovariance(void);

  // include external errors
  // void			externalMomentumUncertainty (double
  // fractionalErrorSq);

  // number of degrees of freedom
  int numberDoF(void) const;

  // number of driftCircles to fit
  int numberDriftCircles(void) const;

  // chiSquared contribution from perigee measurement
  double perigeeChiSquared(void);

  // debugging aids (n.b. using std::cout)
  void printDerivativeMatrix(void);
  void printWeightMatrix(void);

  // remove leading+trailing zeroes from smart pointers
  void refinePointers(void);

  // release memory (eigen)
  void releaseMemory(void);

  // initialize matrices - set appropriate dimensions for a given set of
  // measurements
  int setDimensions(std::vector<FitMeasurement*>& measurements,
                    FitParameters* parameters);

  // solve matrix equations for parameters change
  bool solveEquations(void);

  // tell fit to use measured perigee parameters
  void usePerigee(const FitMeasurement& measurement);

 private:
   // add perigee measurement
  void addPerigeeMeasurement(void);
  // fix for momentum singularity
  void avoidMomentumSingularity(void);  // using Eigen

  fitMatrix m_fitMatrix;
  int m_columnsDM;
  bool m_constrainedAlignmentEffects;
  Amg::MatrixX m_covariance{};
  Amg::MatrixX m_derivativeMatrix{};
  Amg::MatrixX m_finalCovariance{};
  std::vector<int> m_firstRowForParameter;
  double m_largePhiWeight;
  std::vector<int> m_lastRowForParameter;
  bool m_matrixFromCLHEP;
  std::vector<FitMeasurement*>* m_measurements;  // not owning ptr
  int m_numberDoF;
  int m_numberDriftCircles;
  int m_numberPerigee;
  FitParameters* m_parameters;
  const Amg::VectorX* m_perigee;  // not owning ptr
  Amg::MatrixX m_perigeeDifference;
  const Amg::MatrixX* m_perigeeWeight;  // not owning ptr
  std::vector<double> m_residuals;
  int m_rowsDM;
  bool m_usePerigee;
  Amg::MatrixX m_weight;
  Amg::VectorX m_weightedDifference;
};

//<<<<<< INLINE MEMBER FUNCTIONS                                        >>>>>>

inline Amg::MatrixX& FitMatrices::derivativeMatrix(void) {
  return m_derivativeMatrix;
}

inline Amg::MatrixX& FitMatrices::finalCovariance(void) {
  return m_finalCovariance;
}

inline int FitMatrices::numberDoF(void) const {
  return m_numberDoF;
}

inline int FitMatrices::numberDriftCircles(void) const {
  return m_numberDriftCircles;
}

}  // namespace Trk

#endif  // TRKIPATFITTERUTILS_FITMATRICES_H
