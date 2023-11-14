///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// PhysValExample.h 
// Header file for class PhysValExample
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef PHYSVALMONITORING_PHYSVALEXAMPLE_H
#define PHYSVALMONITORING_PHYSVALEXAMPLE_H 1

// STL includes
#include <string>

// FrameWork includes
#include "GaudiKernel/ServiceHandle.h"

// Local includes
#include "AthenaMonitoring/ManagedMonitorToolBase.h"
#include "KinematicsPlots.h"
#include "TrkAndVtxPlots.h"
#include "METPlots.h"
#include "BTagPlots.h"
#include "EventInfoPlots.h"

// Root includes
#include "TH1.h"

// Forward declaration

namespace PhysVal {

class PhysValExample
  : public ManagedMonitorToolBase
{ 
  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  PhysValExample( const std::string& type,
		  const std::string& name, 
		  const IInterface* parent );

  /// Destructor: 
  virtual ~PhysValExample(); 

  // Athena algtool's Hooks
  virtual StatusCode initialize();
  virtual StatusCode bookHistograms();
  virtual StatusCode fillHistograms();
  virtual StatusCode procHistograms();


  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 


  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Default constructor: 
  PhysValExample();

  // Containers
  std::string m_jetName;
  std::string m_elecName; 
  std::string m_photonName; 
  std::string m_muonName; 
  std::string m_tauName; 
  std::string m_metName; 
  std::string m_trackName; 
  std::string m_vertexName; 

  // Hists
  PhysVal::KinematicsPlots m_jetPlots;
  PhysVal::KinematicsPlots m_elecPlots;
  PhysVal::KinematicsPlots m_photonPlots;
  PhysVal::KinematicsPlots m_muonPlots;
  PhysVal::KinematicsPlots m_tauPlots;
  PhysVal::TrkAndVtxPlots m_trkvtxPlots;
  PhysVal::EventInfoPlots m_evtPlots;
  PhysVal::METPlots m_metPlots;
  PhysVal::BTagPlots m_btagPlots;

  StatusCode book(PlotBase& plots);

  //Flags to determine whether to fill example jet,met,btag plots
  bool m_doExJet;
  bool m_doExBtag;
  bool m_doExMET;


}; 

}

#endif //> !PHYSVALMONITORING_PHYSVALEXAMPLE_H
