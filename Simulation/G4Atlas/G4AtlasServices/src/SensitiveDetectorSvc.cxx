/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "SensitiveDetectorSvc.h"
#include "G4AtlasInterfaces/ISensitiveDetector.h"

SensitiveDetectorSvc::SensitiveDetectorSvc( const std::string& name, ISvcLocator* pSvcLocator )
  : AthService(name,pSvcLocator)
{
  declareProperty( "SensitiveDetectors" , m_SenDetList , "Tool handle array of all sensitive detector tools" );
}

StatusCode SensitiveDetectorSvc::initialize(){
  // Loop through all the sensitive detectors and retrieve them
  //  This fires initialize() for each of those tools
  for (auto isd : m_SenDetList){
    CHECK(isd.retrieve());
  }
  return StatusCode::SUCCESS;
}

StatusCode SensitiveDetectorSvc::finalize(){
  return StatusCode::SUCCESS;
}

void SensitiveDetectorSvc::EndOfAthenaEvent(){
  // Call gather for all sensitive detectors
  for (auto isd : m_SenDetList){
    isd->Gather();
  }
}

StatusCode SensitiveDetectorSvc::queryInterface(const InterfaceID& riid, void** ppvInterface) 
{
  if ( ISensitiveDetectorSvc::interfaceID().versionMatch(riid) ) {
    *ppvInterface = dynamic_cast<ISensitiveDetectorSvc*>(this);
  } else {
    // Interface is not directly available : try out a base class
    return AthService::queryInterface(riid, ppvInterface);
  }
  addRef();
  return StatusCode::SUCCESS;
}

