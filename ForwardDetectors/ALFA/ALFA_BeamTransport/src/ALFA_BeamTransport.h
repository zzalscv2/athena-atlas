/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// BeamTransport.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef ALFA_BEAMTRACKBEAMTRANSPORT_H
#define ALFA_BEAMTRACKBEAMTRANSPORT_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

#include "GeneratorObjects/McEventCollection.h"
#include "AtlasHepMC/GenEvent.h"
#include "TH1.h"

// FPTracker
#include "FPTracker/Particle.h"
#include "FPTracker/Point.h"

#include "ALFA_BeamTrack.h"
#include "ALFA_FPConfig.h"

#include <iostream>
#include <string>


class AtlasDetectorID;
class Identifier;
class StoreGateSvc;

/** @class ALFA_BeamTransport

      This is the ALFA Beam Transport package.
      It is an interface which uses FPTracker for beamtransportation.
      Getting from the HepMC eventrecord the particle data at the IP
      it calculates the particle porition at the first RP.


      @author  Daniel Pelikan <daniel.pelikan@cern.ch>
  */

class ALFA_BeamTransport : public ::AthAlgorithm
{
public:
  /** Standard Athena-Algorithm Constructor */
  ALFA_BeamTransport(const std::string& name, ISvcLocator* pSvcLocator);
  /** Default Destructor */
  ~ALFA_BeamTransport();

  /** standard Athena-Algorithm method */
  StatusCode initialize();
  /** standard Athena-Algorithm method */
  StatusCode execute();
  /** standard Athena-Algorithm method */
  StatusCode finalize();

  /**convert unit MeV to GeV for energy and momenta*/
  void MeVToGeV(HepMC::GenEvent& evt);
  /**convert GeV to MeV for HepMC event record*/
  void GeVToMeV(HepMC::GenEvent& evt);
  /**Selects particles for beam transported
  Sets event status code of outgoing particles from generator to status != 1*/
  int SelectParticles(HepMC::GenEvent* evt);
  /**Function which calls BeamTrack class to calcualte Position at RPs*/
  int DoBeamTracking(int evt_number);
  /**This function is event selection, tracking and HepMC save ine one function
   */
  int TransportSelectedParticle(HepMC::GenEvent& evt, int evt_number);

private:
  // some storegate variables

  SG::ReadHandleKey<McEventCollection> m_MCKey{
    this,
    "McEventCollectionName",
    "GEN_EVENT",
    "MC Event Collection name"
  };

  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{ this,
                                                     "EvtInfo",
                                                     "EventInfo",
                                                     "EventInfo name" };

  // Set FPConfiguration variables

  FPConfig m_FPConfig;

  // added
  FPTracker::Particle m_Particle1;
  FPTracker::Particle m_Particle2;
  ALFA_BeamTrack m_BeamTracker;
  // Position at RP
  FPTracker::Point m_PosRP1;
  FPTracker::Point m_PosRP3;
  // Momentum at RP
  FPTracker::Point m_MomRP1;
  FPTracker::Point m_MomRP3;

  double m_EnergyRP1 = 0.0;
  double m_EnergyRP3 = 0.0;

  std::string m_FPOutputBeam1;
  std::string m_FPOutputBeam2;

  // The two counters have to be set zero at the beginning at each execute run
  // since for every new call of the execute() funktion they have to be zero
  // counter for particles marked as outgoing in event record
  int m_pcount = 0;
  // counter for particles marked as incomming in event record
  int m_pint = 0;

  double m_EtaCut;
  double m_XiCut;

  // debug stuff
  std::ofstream m_FileBeam1;
  std::ofstream m_FileBeam2;

  bool m_WriteDebugOutput;
};

#endif
