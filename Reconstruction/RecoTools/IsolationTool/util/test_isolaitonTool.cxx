/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// Mindlessly copied from CPAnalysisExamples
#ifndef CPANALYSISEXAMPLES_ERRORCHECK_H
#define CPANALYSISEXAMPLES_ERRORCHECK_H

#define CHECK( ARG )                                     \
   do {                                                  \
      const bool result = ARG;                           \
      if( ! result ) {                                   \
         ::Error( APP_NAME, "Failed to execute: \"%s\"", \
                  #ARG );                                \
         return 1;                                       \
      }                                                  \
   } while( false )

#endif // CPANALYSISEXAMPLES_ERRORCHECK_H

#include <iostream>
#include <memory>
#include <cstdlib>

// ROOT include(s):
#include <TFile.h>
#include <TTree.h>
#include <TError.h>
#include <TString.h>
#include <TH1F.h>

// Infrastructure include(s):
#ifdef ROOTCORE
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#endif // ROOTCORE

// EDM include(s):
#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/Muon.h"

/// for testing
#include "IsolationTool/CaloIsolationTool.h"
#include "IsolationTool/TrackIsolationTool.h"
#include "IsolationTool/IsolationHelper.h"
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"

using namespace std;

const float GeV = 1000.;
const float iGeV = 0.001;
const float PI = 3.1416;

int main(int argc, char** argv )
{
  gErrorIgnoreLevel=0;
  const char* APP_NAME = "ISOTEST";
  CHECK( xAOD::Init());

TString fileName = "root://eosatlas//eos/atlas/atlastier0/tzero/prod/valid1/PowhegPythia8_AU2CT10_Zmumu/147807/valid1.147807.PowhegPythia8_AU2CT10_Zmumu.recon.AOD.e2658_s1967_s1964_r5787_v114/valid1.147807.PowhegPythia8_AU2CT10_Zmumu.recon.AOD.e2658_s1967_s1964_r5787_v114._000187.1";
  // TString fileName = "root://eosatlas//eos/atlas/atlastier0/tzero/prod/valid1/PowhegPythia_P2011C_ttbar/117050/valid1.117050.PowhegPythia_P2011C_ttbar.recon.AOD.e2658_s1967_s1964_r5787_v111/valid1.117050.PowhegPythia_P2011C_ttbar.recon.AOD.e2658_s1967_s1964_r5787_v111._000001.4";
  // TString fileName = "root://eosatlas//eos/atlas/atlastier0/tzero/prod/valid1/PowhegPythia8_AU2CT10_Zee/147806/valid1.147806.PowhegPythia8_AU2CT10_Zee.recon.AOD.e2658_s1967_s1964_r5787_v114/valid1.147806.PowhegPythia8_AU2CT10_Zee.recon.AOD.e2658_s1967_s1964_r5787_v114._000001.1";
//   TString fileName = "root://eosatlas//eos/atlas/atlastier0/tzero/prod/valid1/PowhegPythia8_AU2CT10_ggH125_gamgam/160009/valid1.160009.PowhegPythia8_AU2CT10_ggH125_gamgam.recon.AOD.e2658_s1967_s1964_r5787_v114/valid1.160009.PowhegPythia8_AU2CT10_ggH125_gamgam.recon.AOD.e2658_s1967_s1964_r5787_v114._000001.1";
  if(argc>1) fileName = argv[1];

  const TString tag = "t2_";
  Info( APP_NAME, "Opening file: %s", fileName.Data() );
  std::auto_ptr< TFile > ifile( TFile::Open( fileName, "READ" ) );
  ifile.get();

  // Create a TEvent object:
  xAOD::TEvent event( xAOD::TEvent::kClassAccess );
  CHECK( event.readFrom( ifile.get() ));

  /// creating tool
  CP::IsolationHelper* m_isoHelper = new CP::IsolationHelper("isoTester"); 
  m_isoHelper->msg().setLevel( MSG::DEBUG );
  if (!m_isoHelper->setSelectorProperty("MuonWP", "Loose")) {
        Info( APP_NAME, "Could not set muon isolation selector "
        "loose working point.");
    }
//   m_isoHelper->msg().setLevel( MSG::INFO );
  
  xAOD::CaloIsolationTool m_caloIsoTool("CaloIsolationTool");
//   m_caloIsoTool.msg().setLevel( MSG::DEBUG );
//   m_caloIsoTool.msg().setLevel( MSG::INFO );
  InDet::InDetTrackSelectionTool selTool( "TrackSelection" );
  selTool.setProperty("maxZ0SinTheta", 3.);
  selTool.setProperty("minPt", 1000.);
  selTool.setProperty("CutLevel", "Loose");


  xAOD::TrackIsolationTool m_trkIsoTool("TrackIsolationTool");
//   m_trkIsoTool.msg().setLevel( MSG::INFO );
//   m_trkIsoTool.msg().setLevel( MSG::DEBUG );
  m_trkIsoTool.setProperty( "TrackSelectionTool", ToolHandle< InDet::IInDetTrackSelectionTool >(selTool.name()) );

  if(!m_caloIsoTool.initialize().isSuccess()){
    Info( APP_NAME, "m_caloIsoTool.initialize() failed");
  }

  if(!selTool.initialize().isSuccess()){
    Info( APP_NAME, "selTool.initialize() failed");
  }

  if(!m_trkIsoTool.initialize().isSuccess()){
    Info( APP_NAME, "m_trkIsoTool.initialize() failed");
  }
  
  if(!m_isoHelper->initialize().isSuccess()){
    Info( APP_NAME, "m_isoHelper.initialize() failed");
  }

  vector<xAOD::Iso::IsolationType> isoTypes;
  isoTypes.push_back(xAOD::Iso::ptcone40);
  isoTypes.push_back(xAOD::Iso::ptcone30);
  isoTypes.push_back(xAOD::Iso::ptcone20);
  xAOD::TrackCorrection corrlist;
  corrlist.trackbitset.set(static_cast<unsigned int>(xAOD::Iso::coreTrackPtr));

  vector<xAOD::Iso::IsolationType> isoTypesCalo;
  isoTypesCalo.push_back(xAOD::Iso::topoetcone40);
  isoTypesCalo.push_back(xAOD::Iso::topoetcone30);
  isoTypesCalo.push_back(xAOD::Iso::topoetcone20);
  xAOD::CaloCorrection corrlist2;
  corrlist2.calobitset.set(static_cast<unsigned int>(xAOD::Iso::coreCone));
  corrlist2.calobitset.set(static_cast<unsigned int>(xAOD::Iso::pileupCorrection));

  vector<xAOD::Iso::IsolationType> isoTypesPFlow;
  isoTypesPFlow.push_back(xAOD::Iso::neflowisol40);
  isoTypesPFlow.push_back(xAOD::Iso::neflowisol30);
  isoTypesPFlow.push_back(xAOD::Iso::neflowisol20);
  xAOD::CaloCorrection corrlist3;
  corrlist3.calobitset.set(static_cast<unsigned int>(xAOD::Iso::coreCone));
  corrlist3.calobitset.set(static_cast<unsigned int>(xAOD::Iso::pileupCorrection));
  
     vector<xAOD::Iso::IsolationType> types;
     vector<const char*> typeNames;
     types.push_back(xAOD::Iso::IsolationType::ptcone20);
     typeNames.push_back("ptcone20");
     types.push_back(xAOD::Iso::IsolationType::ptcone30);
     typeNames.push_back("ptcone30");
     types.push_back(xAOD::Iso::IsolationType::ptcone40);
     typeNames.push_back("ptcone40");
     types.push_back(xAOD::Iso::IsolationType::ptvarcone20);
     typeNames.push_back("ptvarcone20");
     types.push_back(xAOD::Iso::IsolationType::ptvarcone30);
     typeNames.push_back("ptvarcone30");
     types.push_back(xAOD::Iso::IsolationType::ptvarcone40);
     typeNames.push_back("ptvarcone40");
     types.push_back(xAOD::Iso::IsolationType::topoetcone20);
     typeNames.push_back("topoetcone20");
     types.push_back(xAOD::Iso::IsolationType::topoetcone30);
     typeNames.push_back("topoetcone30");
     types.push_back(xAOD::Iso::IsolationType::topoetcone40);
     typeNames.push_back("topoetcone40");
//         types.push_back(xAOD::Iso::IsolationType::etcone20);
//         typeNames.push_back("etcone20");
//         types.push_back(xAOD::Iso::IsolationType::etcone30);
//         typeNames.push_back("etcone30");
//         types.push_back(xAOD::Iso::IsolationType::etcone40);
//         typeNames.push_back("etcone40");

  Long64_t maxEVT = 10;
  Long64_t entries = event.getEntries();
  if(entries<maxEVT) maxEVT = entries;
  Info( APP_NAME, "%lld events found, %lld events will be processed.", entries, maxEVT);
  const int INTERVAL = maxEVT > 20000? 10000: maxEVT/10;
  for( Long64_t entry = 0; entry < maxEVT; ++entry ) {
     event.getEntry( entry );

     // Print some event information for fun:
     if(entry%INTERVAL==0){
       const xAOD::EventInfo* ei = 0;
       CHECK( event.retrieve( ei, "EventInfo" ) );
       Info( APP_NAME, "%lld events processed, on event %llu of run %u", entry, ei->eventNumber(), ei->runNumber() );
     }

     // get muon container of interest
     const xAOD::MuonContainer* muons = 0;
     CHECK( event.retrieve( muons, "Muons" ) );
     
     // Stores the muons in a vector.
     vector<const xAOD::IParticle*> muonsVec;
     for(auto muon: *muons) {
       muonsVec.push_back((const xAOD::IParticle*) muon);
     }
     
     // get electron container of interest
     const xAOD::ElectronContainer* electrons = 0;
     CHECK( event.retrieve( electrons, "Electrons" ) );
     
     // Stores the electrons in a vector.
     vector<const xAOD::IParticle*> electronsVec;
     for(auto electron: *electrons) {
       electronsVec.push_back((const xAOD::IParticle*) electron);
     }
     
     // get photon container of interest
     const xAOD::PhotonContainer* photons = 0;
     CHECK( event.retrieve( photons, "Photons" ) );
     
     // Stores the electrons in a vector.
     vector<const xAOD::IParticle*> photonsVec;
     for(auto photon: *photons) {
       photonsVec.push_back((const xAOD::IParticle*) photon);
     }

     for(auto muon: *muons){
       Info(APP_NAME, "---------NEW MUON -------");
       xAOD::TrackIsolation result;
       if(m_trkIsoTool.trackIsolation(result, *muon, isoTypes, corrlist)){
         for(unsigned int i=0; i<isoTypes.size(); i++){
           float iso = result.ptcones[i];
           float isoV = result.ptvarcones_10GeVDivPt[i];
           float value0 = -999;
           if(!m_isoHelper->isolation(value0, *muon, isoTypes[i])) Info(APP_NAME, "Muon default track iso not got.");
           Info(APP_NAME, "Muon track isolation %d is %f and %f (=%f?)", i, iso, isoV, value0);
          }
        }
       xAOD::CaloIsolation result2;
       if(m_caloIsoTool.caloTopoClusterIsolation(result2, *muon, isoTypesCalo, corrlist2)){
         for(unsigned int i=0; i<isoTypesCalo.size(); i++){
           float value0 = -999;
           if(!m_isoHelper->isolation(value0, *muon, isoTypesCalo[i], corrlist2.calobitset)) Info(APP_NAME, "Muon coreCone Calo iso not got.");
           Info(APP_NAME, "Muon topo isolation %d is %f (=%f?)", i, result2.etcones[i], value0);
          }
        }
       xAOD::CaloIsolation result3;
       if(m_caloIsoTool.neutralEflowIsolation(result3, *muon, isoTypesPFlow, corrlist3)){
         for(unsigned int i=0; i<isoTypesPFlow.size(); i++){
           float value0 = -999;
           if(!m_isoHelper->isolation(value0, *muon, isoTypesPFlow[i])) Info(APP_NAME, "Muon default pflow iso not got.");
           Info(APP_NAME, "Muon neflow isolation %d is %f (=%f?)", i, result3.etcones[i], value0);
          }
        }
        
        if (m_isoHelper->acceptCorrected(*muon, muonsVec, types)) {
            Info(APP_NAME, "Muon passes Loose working point after correction.");
        } else {
            Info(APP_NAME, "Muon does not pass Loose working point after correction.");
        }
        
        if (m_isoHelper->acceptCorrected(*muon)) {
            Info(APP_NAME, "Muon passes Loose working point before correction.");
        } else {
            Info(APP_NAME, "Muon does not pass Loose working point before correction.");
        }
      
        // Performs overlap removal.
        vector<Float_t> removals;
      
        m_isoHelper->removeOverlap(removals, *muon, types, muonsVec);
      
        for (unsigned int j = 0; j < types.size(); j++) {
          if(j < removals.size()) {
              float value = -999999;
              muon->isolation(value, types.at(j));
              float afterRemoval = value - removals.at(j);
              Info(APP_NAME, "Muon Isolation variable: %s", typeNames.at(j));
              Info(APP_NAME, "Muon Value, removal, corrected value: %f, %f, %f", value, removals.at(j), afterRemoval);
          }
        }
    }
    
    for(auto electron: *electrons){
       Info(APP_NAME, "---------NEW ELECTRON -------");
       xAOD::TrackIsolation result;
       if(m_trkIsoTool.trackIsolation(result, *electron, isoTypes, corrlist)){
         for(unsigned int i=0; i<isoTypes.size(); i++){
           float iso = result.ptcones[i];
           float isoV = result.ptvarcones_10GeVDivPt[i];
           float value0 = -999;
           if(!m_isoHelper->isolation(value0, *electron, isoTypes[i])) Info(APP_NAME, "Electron default track iso not got.");
           Info(APP_NAME, "Electron track isolation %d is %f and %f (=%f?)", i, iso, isoV, value0);
          }
        }
       xAOD::CaloIsolation result2;
       if(m_caloIsoTool.caloTopoClusterIsolation(result2, *electron, isoTypesCalo, corrlist2)){
         for(unsigned int i=0; i<isoTypesCalo.size(); i++){
           float value0 = -999;
           if(!m_isoHelper->isolation(value0, *electron, isoTypesCalo[i], corrlist2.calobitset)) Info(APP_NAME, "Electron coreCone Calo iso not got.");
           Info(APP_NAME, "Electron topo isolation %d is %f (=%f?)", i, result2.etcones[i], value0);
          }
        }
       xAOD::CaloIsolation result3;
      
        // Performs overlap removal.
        vector<Float_t> removals;
      
        m_isoHelper->removeOverlap(removals, *electron, types, muonsVec);
      
        for (unsigned int j = 0; j < types.size(); j++) {
          if(j < removals.size()) {
              float value = -999999;
              electron->isolation(value, types.at(j));
              float afterRemoval = value - removals.at(j);
              Info(APP_NAME, "Electron Isolation variable: %s", typeNames.at(j));
              Info(APP_NAME, "Electron Value, removal, corrected value: %f, %f, %f", value, removals.at(j), afterRemoval);
          }
        }
    }
    
    for(auto photon: *photons){
       Info(APP_NAME, "---------NEW photon -------");
       xAOD::TrackIsolation result;
       if(m_trkIsoTool.trackIsolation(result, *photon, isoTypes, corrlist)){
         for(unsigned int i=0; i<isoTypes.size(); i++){
           float iso = result.ptcones[i];
           float isoV = result.ptvarcones_10GeVDivPt[i];
           float value0 = -999;
           if(!m_isoHelper->isolation(value0, *photon, isoTypes[i])) Info(APP_NAME, "photon default track iso not got.");
           Info(APP_NAME, "Photon track isolation %d is %f and %f (=%f?)", i, iso, isoV, value0);
          }
        }
       xAOD::CaloIsolation result2;
       if(m_caloIsoTool.caloTopoClusterIsolation(result2, *photon, isoTypesCalo, corrlist2)){
         for(unsigned int i=0; i<isoTypesCalo.size(); i++){
           float value0 = -999;
           if(!m_isoHelper->isolation(value0, *photon, isoTypesCalo[i], corrlist2.calobitset)) Info(APP_NAME, "photon coreCone Calo iso not got.");
           Info(APP_NAME, "Photon topo isolation %d is %f (=%f?)", i, result2.etcones[i], value0);
          }
        }
       xAOD::CaloIsolation result3;
      
        // Performs overlap removal.
        vector<Float_t> removals;
      
        m_isoHelper->removeOverlap(removals, *photon, types, muonsVec);
      
        for (unsigned int j = 0; j < types.size(); j++) {
          if(j < removals.size()) {
              float value = -999999;
              photon->isolation(value, types.at(j));
              float afterRemoval = value - removals.at(j);
              Info(APP_NAME, "Photon Isolation variable: %s", typeNames.at(j));
              Info(APP_NAME, "Photon Value, removal, corrected value: %f, %f, %f", value, removals.at(j), afterRemoval);
          }
        }
    }
    
  }
  
  Info(APP_NAME, "Finished successfully!");

  delete m_isoHelper;

  return 0;
}