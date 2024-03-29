/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "LArDetectorFactoryH62003.h"
#include "LArDetectorConstructionH62003.h"

#include "LArReadoutGeometry/FCALDetectorManager.h"
#include "LArReadoutGeometry/FCALModule.h"
#include "LArReadoutGeometry/FCAL_ChannelMap.h"

#include "GeoModelKernel/GeoPhysVol.h"  
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoShapeUnion.h"

#include "CLHEP/Geometry/Transform3D.h" 

#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "GeoModelUtilities/StoredPhysVol.h"

#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Bootstrap.h"


LArGeo::LArDetectorFactoryH62003::LArDetectorFactoryH62003()
  : m_detectorManager(nullptr),
    m_fcalVisLimit(-1),
    m_axisVisState(-1)
{}


LArGeo::LArDetectorFactoryH62003::~LArDetectorFactoryH62003()
= default;


// Place the cryostats into a container physical volume.
void LArGeo::LArDetectorFactoryH62003::create( GeoPhysVol* a_container )
{
  
  ISvcLocator *svcLocator = Gaudi::svcLocator();
  IMessageSvc * msgSvc;
  if (svcLocator->service("MessageSvc", msgSvc, true )==StatusCode::FAILURE) {
    throw std::runtime_error("Error in LAr::DetectorFactor, cannot access MessageSvc");
  }
  
  MsgStream log(msgSvc, "LAr::DetectorFactory"); 
  
  log  << "++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
  log << "+                                                   +" << std::endl;
  log << "+         HELLO from LAr::DetectorFactory           +" << std::endl;
  log << "+                                                   +" << std::endl;
  log << "+++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
  

  
  StoreGateSvc *detStore;
  if (svcLocator->service("DetectorStore", detStore, false )==StatusCode::FAILURE) {
    throw std::runtime_error("Error in LArDetectorFactoryH62003, cannot access DetectorStore");
  }

  // Get access to the material manager:
  
  LArDetectorConstructionH62003 BeamLineDets;
  BeamLineDets.SetFCALVisLimit(m_fcalVisLimit);
  BeamLineDets.SetAxisVisState(m_axisVisState);
    
  GeoVPhysVol* Envelope = nullptr;

  Envelope = BeamLineDets.GetEnvelope();
    
  a_container->add(new GeoNameTag("LAr"));
  a_container->add(Envelope);



  FCALDetectorManager *fcalDetectorManager = new FCALDetectorManager();  

  std::cout <<"In the LArDetectorFactory.........." << std::endl;

  // Get the full physical volumes that were created and stored by the construction:
  {
    StoredPhysVol *fcal1,*fcal2, *fcal3;
    if (StatusCode::SUCCESS!=detStore->retrieve(fcal1,"FCAL1_NEG")) {
      log << MSG::WARNING << "No volume created for FCAL 1 Neg" << endmsg;
    }
    else {
      FCALModule *detDescr = new FCALModule(fcal1->getPhysVol(),FCALModule::FCAL1,FCALModule::NEG);
      fcalDetectorManager->addModule(detDescr);
    }
    if (StatusCode::SUCCESS!=detStore->retrieve(fcal2,"FCAL2_NEG")) {
      log << MSG::WARNING << "No volume created for FCAL 2 Neg" << endmsg;
    }
    else {
      FCALModule *detDescr = new FCALModule(fcal2->getPhysVol(),FCALModule::FCAL2,FCALModule::NEG);
      fcalDetectorManager->addModule(detDescr);
    }
    if (StatusCode::SUCCESS!=detStore->retrieve(fcal3,"FCAL3_NEG")) {
      log << MSG::WARNING << "No volume created for FCAL 3 Neg" << endmsg;
    }
    else {
      FCALModule *detDescr = new FCALModule(fcal3->getPhysVol(),FCALModule::FCAL3,FCALModule::NEG);
      fcalDetectorManager->addModule(detDescr);
    }
  }

  detStore->record(fcalDetectorManager,fcalDetectorManager->getName()).ignore();
  m_detectorManager = new LArDetectorManager(nullptr,nullptr,nullptr,fcalDetectorManager);
  m_detectorManager->addTreeTop(Envelope);

  
}


const LArDetectorManager * LArGeo::LArDetectorFactoryH62003::getDetectorManager() const
{
  return m_detectorManager;
}

