/*
   Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <memory>
#include <cstdlib>

// ROOT include(s):
#include <TFile.h>
#include <TError.h>
#include <TString.h>

// EDM include(s):
#include "xAODEgamma/ElectronContainer.h" 
#include "xAODEgamma/PhotonContainer.h" 
#include "xAODEgamma/Electron.h" 
#include "xAODEgamma/Photon.h" 
#include "AsgTools/AnaToolHandle.h"

#include "EgammaAnalysisInterfaces/IAsgPhotonIsEMSelector.h"
#include "EgammaAnalysisInterfaces/IAsgForwardElectronIsEMSelector.h"
#include "EgammaAnalysisInterfaces/IAsgElectronIsEMSelector.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "ElectronPhotonSelectorTools/egammaPIDdefs.h"
//
#include "AsgTools/MessageCheck.h"
#include "AsgTools/MsgStream.h"
// Derivation include
#include "xAODCore/tools/IOStats.h"
#include "xAODCore/tools/ReadStats.h"

// Infrastructure include(s):
#ifdef ROOTCORE
#   include "xAODRootAccess/Init.h"
#   include "xAODRootAccess/TEvent.h"
#   include "xAODRootAccess/TStore.h"
#endif // ROOTCORE

namespace asg{
ANA_MSG_HEADER (msgSelectorCheck)
  ANA_MSG_SOURCE (msgSelectorCheck, "EgammaSelectorCheck") 
}

//main test code
int main( int argc, char* argv[] ) {

  //
  using namespace asg::msgSelectorCheck;
  ANA_CHECK_SET_TYPE (int);
  MSG::Level mylevel=MSG::INFO;
  setMsgLevel(mylevel);
  //

  // The application's name:
  const char* APP_NAME = argv[ 0 ];

  // Check if we received a file name:
  if( argc < 2 ) {
    ANA_MSG_ERROR("No file name received!" );
    ANA_MSG_ERROR( "  Usage: %s [xAOD file name]  %d Num of events to process %d (0 photons , 1 electrons, 2 Forward)");
    return EXIT_FAILURE;
  }

  // Check if we want to process Electron/Photon/Forward Electrons 
  bool isElectron = false;   
  bool isPhoton = false;
  bool isForward =false;
  if( argc < 4 ) {
    ANA_MSG_INFO(APP_NAME<< " By default looking to Electron" );
    isElectron=true;
  } else {  
    int  argv1 = atoi(argv[ 3 ]);
    if(argv1 == 0)   { 
      isPhoton = true;
      ANA_MSG_INFO (APP_NAME<< " We are looking to Photon-ID" );
    }
    else if(argv1 == 1)   {
      isElectron = true;
      ANA_MSG_INFO(APP_NAME<< " We are looking to Electron-ID");
    }
    else if(argv1 == 2)   { 
      isForward = true;
      ANA_MSG_INFO (APP_NAME<< " We are looking to Forward Electron-ID" );
    }
    else{
      ANA_MSG_ERROR ("Usage: %s [xAOD file name]  %d Num of events to process %d (0 photons , 1 electrons, 2 Forward)");
      return EXIT_FAILURE;
    }
  }

  // Initialise the application:
  ANA_CHECK( xAOD::Init( APP_NAME ) );

  // Open the input file:
  const TString fileName = argv[ 1 ];
  Info( APP_NAME, "Opening file: %s", fileName.Data() );
  std::unique_ptr< TFile > ifile( TFile::Open( fileName, "READ" ) );
  ANA_CHECK( ifile.get() );

  // Create a TEvent object (persistent store)
  xAOD::TEvent pers( xAOD::TEvent::kClassAccess );
  // Create a TStore object (transient store)
  xAOD::TStore trans;
  ANA_CHECK( pers.readFrom( ifile.get() ) );
  //
  ANA_MSG_INFO("Number of events in the file: "<< pers.getEntries());

  // Decide how many events to run over:
  Long64_t entries = pers.getEntries();
  if( argc > 2 ) {
    const Long64_t e = atoll( argv[ 2 ] );
    if( e < entries ) {
      entries = e;
    }
  }

  if (isElectron) {
    //Medium cut based Electrons

    asg::AnaToolHandle<IAsgElectronIsEMSelector> electronMediumIsEMSelector ("AsgElectronIsEMSelector/electronMediumIsEMSelector");
    ANA_CHECK(electronMediumIsEMSelector.setProperty("WorkingPoint", "MediumElectron"));
    ANA_CHECK(electronMediumIsEMSelector.setProperty("OutputLevel", mylevel));
    ANA_CHECK(electronMediumIsEMSelector.initialize());

    asg::AnaToolHandle<IAsgElectronLikelihoodTool> electronMediumLHSelector ("AsgElectronLikelihoodTool/electronMediumLHSelector");
    ANA_CHECK(electronMediumLHSelector.setProperty("WorkingPoint", "MediumLHElectron"));
    ANA_CHECK(electronMediumLHSelector.setProperty("OutputLevel", mylevel));
    ANA_CHECK(electronMediumLHSelector.initialize());


    // Loop over the events:
    for( Long64_t entry = 0; entry < entries; ++entry ) {

      // Tell the object which entry to look at:
      pers.getEntry( entry );
      ANA_MSG_INFO("============================");
      ANA_MSG_INFO("Event: " <<entry);

      const xAOD::ElectronContainer* electrons;
      ANA_CHECK(pers.retrieve(electrons, "Electrons"));

      unsigned int counter=0;
      for (const xAOD::Electron* el : *electrons) {
        ANA_MSG_INFO("---------------------------");
        ANA_MSG_INFO("Electron: " << counter);
        ANA_MSG_INFO("Electron LH Medium accept result: " <<electronMediumLHSelector->accept(el));
        ANA_MSG_INFO("Electron Cut based");
        ANA_MSG_INFO("Electron Cut Medium accept result: " <<electronMediumIsEMSelector->accept(el));

        //Bitset manipulation 
        ANA_MSG_INFO("Decision as a bitset: ");
        std::bitset<32> decision = electronMediumIsEMSelector->accept(el).getCutResultBitSet();
        ANA_MSG_INFO("Result bitset: " <<decision);
        std::bitset<32> isEMdecision = electronMediumIsEMSelector->accept(el).getCutResultInvertedBitSet() ;
        ANA_MSG_INFO("isEM Result bitset: " << isEMdecision);
        //
        ANA_MSG_INFO("Masks: " );
        std::bitset<32> MediumMask(egammaPID::ElectronMediumPP);
        ANA_MSG_INFO("Medium mask: " << MediumMask);

        std::bitset<32> HadLeakageOnlyMask( 0x1 << egammaPID::ClusterHadronicLeakage_Electron);
        ANA_MSG_INFO("HadLeakageOnly mask: " << HadLeakageOnlyMask);
        //
        std::bitset<32> MediumWithouHadLeakageMask( egammaPID::ElectronMediumPP ^ (0x1 << egammaPID::ClusterHadronicLeakage_Electron));
        ANA_MSG_INFO("Medium Without Had Leakage  mask: " << MediumWithouHadLeakageMask);
        //

        bool passALLDecisionisem= (isEMdecision&MediumMask)==0;
        std::bitset<32> passALLDecisionisemBitSet(isEMdecision&MediumMask);
        ANA_MSG_INFO("Electron check all cuts via  isem: "<< passALLDecisionisem << " ,bitset " << passALLDecisionisemBitSet);

        bool checkOnlyHadLeakageisem= (isEMdecision&HadLeakageOnlyMask)==0;
        std::bitset<32> checkOnlyHadLeakageisemBitSet(isEMdecision&HadLeakageOnlyMask);
        ANA_MSG_INFO("Electron check Only Had Leakage via isem:: "<< checkOnlyHadLeakageisem << " ,bitset " << checkOnlyHadLeakageisemBitSet);

        bool ignoreHadLeakageisem= (isEMdecision&MediumWithouHadLeakageMask)==0;
        std::bitset<32> ignoreHadLeakageisemBitSet(isEMdecision&MediumWithouHadLeakageMask);
        ANA_MSG_INFO("Electron ignore Had Leakage check all else via isem:: "<< ignoreHadLeakageisem << " ,bitset " << ignoreHadLeakageisemBitSet);

        ++counter;
      }
    } // loop entries
  }// isElectron
  else if(isPhoton) {
    //Tight cut based photon

    asg::AnaToolHandle<IAsgPhotonIsEMSelector> photonTightIsEMSelector ("AsgPhotonIsEMSelector/photonTightIsEMSelector");
    ANA_CHECK(photonTightIsEMSelector.setProperty("WorkingPoint", "TightPhoton"));
    ANA_CHECK(photonTightIsEMSelector.setProperty("OutputLevel", mylevel));
    ANA_CHECK(photonTightIsEMSelector.initialize());
    // Loop over the events:
    for( Long64_t entry = 0; entry < entries; ++entry ) {

      // Tell the object which entry to look at:
      pers.getEntry( entry );
      ANA_MSG_INFO("============================");
      ANA_MSG_INFO("Event: " <<entry);

      const xAOD::PhotonContainer* photons;
      ANA_CHECK(pers.retrieve(photons, "Photons"));
      unsigned int counter=0;
      for (const xAOD::Photon* ph : *photons) {
        ANA_MSG_INFO("---------------------------");
        ANA_MSG_INFO("Photon: " << counter);
        ANA_MSG_INFO("Photon Tight accept result: " <<photonTightIsEMSelector->accept(ph));
        ++counter;       
      }
    }// loop entries
  }//isPhoton
  else if (isForward){
    //Medium cut based Electrons
    asg::AnaToolHandle<IAsgForwardElectronIsEMSelector> electronForwardMedium("AsgForwardElectronIsEMSelector/electronMediumIsEMSelector");
    ANA_CHECK(electronForwardMedium.setProperty("ConfigFile", "ElectronPhotonSelectorTools/offline/mc15_20170711/ForwardElectronIsEMMediumSelectorCutDefs.conf"));
    ANA_CHECK(electronForwardMedium.setProperty("OutputLevel",mylevel));
    ANA_CHECK(electronForwardMedium.initialize());

    //Medium Likelihood electron
    asg::AnaToolHandle<IAsgElectronLikelihoodTool> electronForwardMediumLH ("AsgForwardElectronLikelihoodTool/electronMediumLHSelector");
    ANA_CHECK(electronForwardMediumLH.setProperty("WorkingPoint", "TightLHForwardElectron"));
    ANA_CHECK(electronForwardMediumLH.setProperty("OutputLevel",mylevel));
    ANA_CHECK(electronForwardMediumLH.initialize());

    // Loop over the events:
    for( Long64_t entry = 0; entry < entries; ++entry ) {

      // Tell the object which entry to look at:
      pers.getEntry( entry );
      ANA_MSG_INFO("============================");
      ANA_MSG_INFO("Event: " <<entry);

      const xAOD::ElectronContainer* electrons;
      ANA_CHECK(pers.retrieve(electrons, "ForwardElectrons"));

      unsigned int nVtx(0);
      const xAOD::VertexContainer* vxContainer(0);
      if ( StatusCode::SUCCESS != pers.retrieve( vxContainer, "PrimaryVertices" ) ) {
        ANA_MSG_WARNING ( "Vertex container not found with name: " << "PrimaryVertices" );
      } else {
        for ( unsigned int i=0; i<vxContainer->size(); i++ )
        {
          const xAOD::Vertex* vxcand = vxContainer->at(i);
          if ( vxcand->nTrackParticles() >= 3 ) nVtx++;
        }
      }
      unsigned int counter=0;
      for (const xAOD::Electron* el : *electrons) {

        ANA_MSG_INFO("Electron: " << counter << ", pt " << el->pt() << ", " << el->eta() << ", nPV " << nVtx);
        ANA_MSG_INFO("Electron LH Medium accept result: " <<electronForwardMediumLH->accept(el));
        ANA_MSG_INFO("Electron Cut Medium accept result: " <<electronForwardMedium->accept(el));
        ANA_MSG_INFO("Electron stored Medium: "  << el->passSelection("Medium") );
        ++counter;
      }
    }// loop entries 
  }//isForward
  xAOD::IOStats::instance().stats().printSmartSlimmingBranchList();
  return 0;
}



