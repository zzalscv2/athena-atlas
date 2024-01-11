/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_ITAUTRUTHMATCHINGTOOL_H
#define TAUANALYSISTOOLS_ITAUTRUTHMATCHINGTOOL_H

/*
  author: Dirk Duschinger
  mail: dirk.duschinger@cern.ch
  documentation in: ../README.rst
*/

// Framework include(s):
#include "AsgTools/IAsgTool.h"

// EDM include(s):
#include "xAODTau/TauJet.h"

// // local include(s)
#include "IBuildTruthTaus.h"

// local include(s)
#include "TauAnalysisTools/Enums.h"

namespace TauAnalysisTools
{

class ITauTruthMatchingTool
  // The order matters, do not switch them !!!
  : public virtual TauAnalysisTools::IBuildTruthTaus
  , public virtual asg::IAsgTool
{

  /// Declare the interface that the class provides
  ASG_TOOL_INTERFACE( TauAnalysisTools::ITauTruthMatchingTool )

public:
  // initialize the tool
  virtual StatusCode initialize() = 0;

  virtual std::unique_ptr<ITruthTausEvent> getEvent() const = 0;

  // get pointer to truth tau, if no truth tau was found a null pointer is returned
  virtual const xAOD::TruthParticle* getTruth(const xAOD::TauJet& xTau) = 0;
  virtual const xAOD::TruthParticle* getTruth(const xAOD::TauJet& xTau,
                                              ITruthTausEvent& truthTausEvent) const = 0;
  virtual std::vector<const xAOD::TruthParticle*> getTruth(const std::vector<const xAOD::TauJet*>& vTaus) = 0;

  // wrapper function to get truth tau visible TLorentzvector
  virtual TLorentzVector getTruthTauP4Vis(const xAOD::TauJet& xTau) = 0;
  virtual TLorentzVector getTruthTauP4Vis(const xAOD::TruthParticle& xTruthTau) const = 0;

  // wrapper function to get truth tau invisible TLorentzvector
  virtual TLorentzVector getTruthTauP4Invis(const xAOD::TauJet& xTau) = 0;
  virtual TLorentzVector getTruthTauP4Invis(const xAOD::TruthParticle& xTruthTau) const = 0;

  // get type of truth match particle (hadronic tau, leptonic tau, electron, muon, jet)
  virtual TauAnalysisTools::TruthMatchedParticleType getTruthParticleType(const xAOD::TauJet& xTau) = 0;

  // wrapper function to count number of decay particles of given pdg id
  virtual int getNTauDecayParticles(const xAOD::TauJet& xTau, int iPdgId, bool bCompareAbsoluteValues = false) = 0;
  virtual int getNTauDecayParticles(const xAOD::TruthParticle& xTruthTau, int iPdgId, bool bCompareAbsoluteValues = false) const = 0;

  // wrapper function to obtain truth version of xAOD::TauJetParameters::DecayMode
  virtual xAOD::TauJetParameters::DecayMode getDecayMode(const xAOD::TauJet& xTau) = 0;
  virtual xAOD::TauJetParameters::DecayMode getDecayMode(const xAOD::TruthParticle& xTruthTau) const = 0;

}; // class ITauTruthMatchingTool

} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_ITAUTRUTHMATCHINGTOOL_H
