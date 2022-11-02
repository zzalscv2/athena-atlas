/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoDbTagSvc.h"
#include "RDBMaterialManager.h"
#include "GaudiKernel/ServiceHandle.h"

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "RDBAccessSvc/IRDBRecord.h"

GeoDbTagSvc::GeoDbTagSvc(const std::string& name,ISvcLocator* svc)
  : base_class(name,svc)
  , m_geoConfig(GeoModel::GEO_RUN1)
{
}

StatusCode GeoDbTagSvc::initialize()
{
  ATH_MSG_DEBUG("initialize()");
  return StatusCode::SUCCESS;
}

StatusCode GeoDbTagSvc::finalize()
{
  ATH_MSG_DEBUG("finalize()");
  return StatusCode::SUCCESS;
}

StatusCode GeoDbTagSvc::setupTags()
{
  ATH_MSG_DEBUG("setupTags()");
  
  // Check if the Atlas version has already been set
  if(m_AtlasVersion.empty()) {
    ATH_MSG_FATAL("ATLAS tag not set!");
    return StatusCode::FAILURE;
  }
  
  // Get RDBAccessSvc
  ServiceHandle<IRDBAccessSvc> rdbAccessSvc("RDBAccessSvc", name());
  if(rdbAccessSvc.retrieve().isFailure()) {
    ATH_MSG_FATAL("Failed to retrieve RDBAccessSvc");
    return StatusCode::FAILURE;
  }

  // Get subsystem tags
  m_InDetVersion = (m_InDetVersionOverride.empty() 
		    ? rdbAccessSvc->getChildTag("InnerDetector",m_AtlasVersion,"ATLAS") 
		    : m_InDetVersionOverride);

  m_PixelVersion = (m_PixelVersionOverride.empty()
                    ? rdbAccessSvc->getChildTag("Pixel",m_InDetVersion,"InnerDetector")
		    : m_PixelVersionOverride);

  m_SCT_Version = (m_SCT_VersionOverride.empty()
		   ? rdbAccessSvc->getChildTag("SCT",m_InDetVersion,"InnerDetector")
		   : m_SCT_VersionOverride);

  m_TRT_Version = (m_TRT_VersionOverride.empty()
		   ? rdbAccessSvc->getChildTag("TRT",m_InDetVersion,"InnerDetector")
		   : m_TRT_VersionOverride);

  m_LAr_Version = (m_LAr_VersionOverride.empty()
                   ? rdbAccessSvc->getChildTag("LAr",m_AtlasVersion,"ATLAS")
		   : m_LAr_VersionOverride);

  m_TileVersion = (m_TileVersionOverride.empty()
                   ? rdbAccessSvc->getChildTag("TileCal",m_AtlasVersion,"ATLAS")
		   : m_TileVersionOverride);

  m_MuonVersion = (m_MuonVersionOverride.empty()
                   ? rdbAccessSvc->getChildTag("MuonSpectrometer",m_AtlasVersion,"ATLAS")
		   : m_MuonVersionOverride);

  m_CaloVersion = (m_CaloVersionOverride.empty()
                   ? rdbAccessSvc->getChildTag("Calorimeter",m_AtlasVersion,"ATLAS")
		   : m_CaloVersionOverride);

  m_MagFieldVersion = (m_MagFieldVersionOverride.empty()
		       ? rdbAccessSvc->getChildTag("MagneticField",m_AtlasVersion,"ATLAS")
		       : m_MagFieldVersionOverride);

  m_CavernInfraVersion = (m_CavernInfraVersionOverride.empty()
			  ? rdbAccessSvc->getChildTag("CavernInfra",m_AtlasVersion,"ATLAS")
			  : m_CavernInfraVersionOverride);

  m_ForwardDetectorsVersion = (m_ForwardDetectorsVersionOverride.empty()
			       ? rdbAccessSvc->getChildTag("ForwardDetectors",m_AtlasVersion,"ATLAS")
			       : m_ForwardDetectorsVersionOverride);

  // Retrieve geometry config information (RUN1, RUN2, etc...)
  IRDBRecordset_ptr atlasCommonRec = rdbAccessSvc->getRecordsetPtr("AtlasCommon",m_AtlasVersion,"ATLAS");
  if(atlasCommonRec->size()==0) {
    m_geoConfig = GeoModel::GEO_RUN1;
  }
  else {
    std::string configVal = (*atlasCommonRec)[0]->getString("CONFIG");
    if(configVal=="RUN1")
      m_geoConfig = GeoModel::GEO_RUN1;
    else if(configVal=="RUN2")
      m_geoConfig = GeoModel::GEO_RUN2;
    else if(configVal=="RUN3")
      m_geoConfig = GeoModel::GEO_RUN2;
    else if(configVal=="RUN4")
      m_geoConfig = GeoModel::GEO_RUN4;
    else if(configVal=="TESTBEAM")
      m_geoConfig = GeoModel::GEO_TESTBEAM;
    else {
      ATH_MSG_FATAL("Unexpected value for geometry config read from the database: " << configVal);
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}
