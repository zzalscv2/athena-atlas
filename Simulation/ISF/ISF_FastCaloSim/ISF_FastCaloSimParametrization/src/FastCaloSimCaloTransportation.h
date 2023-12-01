/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FastCaloSimCaloTransportation_H
#define FastCaloSimCaloTransportation_H

#include "CxxUtils/CachedPointer.h"
// ISF includes
#include "ISF_FastCaloSimParametrization/IFastCaloSimCaloTransportation.h"
#include "ISF_FastCaloSimEvent/TFCSExtrapolationState.h"
// ATLAS tracking tools includes
#include "TrkExInterfaces/ITimedExtrapolator.h"
#include "TrkEventPrimitives/PdgToParticleHypothesis.h"

class ITimedExtrapolator;
class TFCSTruthState;
class G4FieldTrack;

class FastCaloSimCaloTransportation: public extends<AthAlgTool, IFastCaloSimCaloTransportation>
{

public:
  
  FastCaloSimCaloTransportation(const std::string& t, const std::string& n, const IInterface* p);
  ~FastCaloSimCaloTransportation() = default;

  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;

  virtual std::vector<G4FieldTrack> transport(const TFCSTruthState* truth, bool forceNeutral = false) const override final;

protected:
  
  // Converts a vector of Trk::HitInfo elements to a vector of G4FieldTrack
  std::vector<G4FieldTrack> convertToFieldTrack(const std::vector<Trk::HitInfo>& vec) const;
  // Main extrapolation tool for transporting particles in magnetic field through calorimter system using ATLAS tracking tool
  PublicToolHandle<Trk::ITimedExtrapolator> m_extrapolator{this, "Extrapolator", "TimedExtrapolator"};
  // Calo entrance and particle hypothesis for transportation
  CxxUtils::CachedPointer<const Trk::TrackingVolume> m_caloEntrance;
  StringProperty m_caloEntranceName{this, "CaloEntrance", ""};
  Trk::PdgToParticleHypothesis m_pdgToParticleHypothesis;
};

#endif // FastCaloSimCaloTransportation_H