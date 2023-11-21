/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDC_DetTool.h"
#include "ZDC_DetFactory.h" 
#include "ZDC_DetManager.h" 
#include "GeoModelUtilities/GeoModelExperiment.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/getMessageSvc.h"

ZDC_DetTool::ZDC_DetTool(const std::string& type, const std::string& name, const IInterface* parent)
  : GeoModelTool(type, name, parent)
{

  if (msgLevel(MSG::DEBUG))
    msg(MSG::DEBUG) << "INSIDE CONSTRUCTOR OF DETTOOL"                            << endmsg
		    << "INSIDE CONSTRUCTOR OF DETTOOL string& type "      << type << endmsg
		    << "INSIDE CONSTRUCTOR OF DETTOOL std::string& name " << name << endmsg;
}

ZDC_DetTool::~ZDC_DetTool()
{
  // This will need to be modified once we register the Toy DetectorNode in the Transient Detector Store
  
  if (nullptr != m_detector) {

    delete m_detector;
    m_detector = nullptr;
  }
}

StatusCode ZDC_DetTool::create()
{ 
  if (msgLevel(MSG::DEBUG)) msg(MSG::DEBUG) << " Building ZDC geometry " << endmsg;
  
  // Locate the top level experiment node  
  GeoModelExperiment* theExpt = nullptr;
  
  if (StatusCode::SUCCESS != detStore()->retrieve(theExpt, "ATLAS")) {

    if (msgLevel(MSG::ERROR)) msg(MSG::ERROR) << " Could not find GeoModelExperiment ATLAS " << endmsg; 
    return (StatusCode::FAILURE); 
  } 

  
  ZDC_DetFactory theZDCFactory(detStore().operator->());

  IGeoDbTagSvc *geoDbTag;
  StatusCode sc = service ("GeoDbTagSvc",geoDbTag);
  if (sc.isFailure()) {
    msg(MSG::FATAL) << "Could not locate GeoDbTagSvc" << endmsg;
    return StatusCode::FAILURE;
  }

  GeoModel::GeoConfig geoConfig = geoDbTag->geoConfig();
  
  //Set the geometry configuration
  if(geoConfig==GeoModel::GEO_RUN2){ 
    msg(MSG::INFO) << "Initializing ZDC geometry for PbPb2015" << endmsg;
    theZDCFactory.initializePbPb2015();
  }else if(geoConfig==GeoModel::GEO_RUN3){ 
    msg(MSG::INFO) << "Initializing ZDC geometry for PbPb2023" << endmsg;
    theZDCFactory.initializePbPb2023();
  }else if(geoConfig==GeoModel::GEO_RUN4){
    msg(MSG::ERROR) << "No ZDC geometry defined for RUN4" << endmsg;
  }

  if (nullptr == m_detector) { // Create the ZDCDetectorNode instance
    
    try { 
      // This strange way of casting is to avoid an utterly brain damaged compiler warning.
      GeoPhysVol* world = &*theExpt->getPhysVol();
      theZDCFactory.create(world);  
    } 
    catch (const std::bad_alloc&) {
      
      if (msgLevel(MSG::FATAL)) msg(MSG::FATAL) << "Could not create new ZDC DetectorNode!" << endmsg;
      return StatusCode::FAILURE; 
    }
    
    // Register the ZDC DetectorNode instance with the Transient Detector Store
    theExpt->addManager(theZDCFactory.getDetectorManager());
    if(detStore()->record(theZDCFactory.getDetectorManager(),theZDCFactory.getDetectorManager()->getName())==StatusCode::SUCCESS){
      return StatusCode::SUCCESS;}
    else{
      msg(MSG::FATAL) << "Could not register ZDC detector manager" << endmsg;}

  }
  
  return StatusCode::FAILURE;
}
