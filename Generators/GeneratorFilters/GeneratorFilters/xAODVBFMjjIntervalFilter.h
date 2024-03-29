/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERSXAODVBFMJJINTERVALFILTER_H
#define GENERATORFILTERSXAODVBFMJJINTERVALFILTER_H

#include "AthContainers/ConstDataVector.h"
#include "GeneratorModules/GenFilter.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "xAODJet/JetContainer.h"

#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthParticle.h"

namespace CLHEP {
  class HepRandomEngine;
}

class xAODVBFMjjIntervalFilter : public GenFilter {
public:

  xAODVBFMjjIntervalFilter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode filterInitialize();
  virtual StatusCode filterEvent();

private:

  CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName,
                                          const EventContext& ctx) const;

  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};// Random number generator

   double m_olapPt;
  double m_yMax;                           // Rapidity acceptance
  double m_pTavgMin;                       // Required average dijet pT
  std::string m_TruthJetContainerName;     // Name of the truth jet container

  //long m_total;                            // Total number of events tested
  //long m_passed;                           // Number of events passing all cuts
  //long m_outsideAcceptance;                // Number of events failing rapidity acceptance cuts

  double m_norm;                           // Normalization for weights
  //double m_high;                           // High-side function level
  //bool m_doShape;                          // Attempt to flatten the dY distribution
  double m_prob0;
  double m_prob1;
  double m_prob2low;
  double m_prob2high;
  double m_mjjlow;
  bool m_truncatelowmjj;
  double m_mjjhigh;
  bool m_truncatehighmjj;
  bool m_photonjetoverlap;
  bool m_electronjetoverlap;
  bool m_taujetoverlap;
  double m_alpha;
  bool m_ApplyNjet; 
  unsigned int m_NJetsMin; 
  unsigned int m_NJetsMax; 
  bool m_ApplyWeighting; 
  bool m_applyDphi; 
  double m_dphijj; 


  bool checkOverlap(double, double, const std::vector<const xAOD::TruthParticle*>&);
  bool checkOverlap(double, double, const std::vector<TLorentzVector>&);
  TLorentzVector sumDaughterNeutrinos( const xAOD::TruthParticle* );

public:

  bool ApplyMassDphi(ConstDataVector<xAOD::JetContainer> *jets);
  double getEventWeight(ConstDataVector<xAOD::JetContainer> *jets) const;
};

#endif
