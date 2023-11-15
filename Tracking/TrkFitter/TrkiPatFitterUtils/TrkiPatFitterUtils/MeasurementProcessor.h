/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 process measurements i.e. extrapolate to surface,
                           compute derivatives and residuals
 ***************************************************************************/

#ifndef TRKIPATFITTERUTILS_MEASUREMENTPROCESSOR_H
#define TRKIPATFITTERUTILS_MEASUREMENTPROCESSOR_H

//<<<<<< INCLUDES                                                       >>>>>>

#include <optional>
#include <vector>

#include "EventPrimitives/EventPrimitives.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkExUtils/TrackSurfaceIntersection.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkiPatFitterUtils/ExtrapolationType.h"
//<<<<<< CLASS DECLARATIONS                                             >>>>>>

namespace Trk {
class FitMeasurement;
class FitParameters;
class IIntersector;
class IPropagator;

class MeasurementProcessor {
 public:
  MeasurementProcessor(bool asymmetricCaloEnergy,
                       Amg::MatrixX& derivativeMatrix,
                       const ToolHandle<IIntersector>& intersector,
                       std::vector<FitMeasurement*>& measurements,
                       FitParameters* parameters,
                       const ToolHandle<IIntersector>& rungeKuttaIntersector,
                       const ToolHandle<IPropagator>& stepPropagator,
                       int useStepPropagator);
  ~MeasurementProcessor(void);

  // implicit copy constructor
  // implicit assignment operator

  // compute derivatives and residuals
  bool calculateDerivatives(void);
  bool calculateFittedTrajectory(int iteration);
  void calculateResiduals(void);
  void fieldIntegralUncertainty(MsgStream& log, Amg::MatrixX& covariance);
  void propagationDerivatives(void);

 private:
  void clusterDerivatives(int derivativeFlag, FitMeasurement& measurement);
  void driftDerivatives(int derivativeFlag, FitMeasurement& measurement);
  bool extrapolateToMeasurements(ExtrapolationType type);

  std::vector<FitMeasurement*> m_alignments;
  bool m_asymmetricCaloEnergy;
  FitMeasurement* m_caloEnergyMeasurement;
  double m_cosPhi0;
  double m_cosTheta0;
  double m_delta[ExtrapolationTypes]{};
  double m_derivQOverP0;
  double m_derivQOverP1;
  double m_energyResidual;
  int m_firstScatteringParameter;
  // bool				m_havePhiPseudo;
  const ToolHandle<IIntersector>& m_intersector;
  double m_largeDeltaD0;
  double m_largeDeltaPhi0;
  std::vector<FitMeasurement*>& m_measurements;
  // double				m_minDistanceForAngle;
  bool m_numericDerivatives;
  FitParameters* m_parameters;
  bool m_phiInstability;
  double m_qOverP[ExtrapolationTypes]{};
  double m_qOverPbeforeCalo;
  double m_qOverPafterCalo;
  const ToolHandle<IIntersector>& m_rungeKuttaIntersector;
  const ToolHandle<IPropagator>& m_stepPropagator;
  int m_useStepPropagator;
  std::vector<FitMeasurement*> m_scatterers;
  double m_sinPhi0;
  double m_sinTheta0;
  // double				m_toroidTurn;
  TrackSurfaceIntersection m_vertexIntersect;
  TrackSurfaceIntersection m_intersectStartingValue;
  double m_x0;
  double m_y0;
  double m_z0;
  // bool				m_zInstability;
  Trk::MagneticFieldProperties m_stepField;
};

}  // namespace Trk

#endif  // TRKIPATFITTERUTILS_MEASUREMENTPROCESSOR_H
