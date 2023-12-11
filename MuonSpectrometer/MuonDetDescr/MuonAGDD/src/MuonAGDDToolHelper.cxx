/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonAGDDToolHelper.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IToolSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AGDDControl/AGDDController.h"
#include "AGDDModel/AGDDParameterStore.h"
#include "AGDDControl/AGDD2GeoModelBuilder.h"
#include "AGDDControl/IAGDD2GeoSvc.h"
#include "AGDDKernel/AliasStore.h"
#include "AGDDKernel/AGDDDetector.h"
#include "AGDDKernel/AGDDDetectorStore.h"

#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "StoreGate/DataHandle.h"
#include "StoreGate/StoreGateSvc.h"

#include "MuonAGDDBase/micromegasHandler.h"
#include "MuonAGDDBase/mm_TechHandler.h"
#include "MuonAGDDBase/sTGCHandler.h"
#include "MuonAGDDBase/sTGC_readoutHandler.h"
#include "MuonAGDDBase/sTGC_TechHandler.h"
#include "MuonAGDDBase/mmSpacerHandler.h"
#include "MuonAGDDBase/mmSpacer_TechHandler.h"
#include "MuonAGDDBase/mm_readoutHandler.h"

#include "MuonDetDescrUtils/BuildNSWReadoutGeometry.h"

#include <fstream>

using namespace MuonGM;

MuonAGDDToolHelper::MuonAGDDToolHelper() :
    p_RDBAccessSvc(nullptr),
    p_GeoModelSvc(nullptr),
    m_svcName("AGDDtoGeoSvc") {
	StatusCode result;

	result=Gaudi::svcLocator()->service("GeoModelSvc",p_GeoModelSvc);
  	if (result.isFailure())
    {
	std::cout<<"MuonAGDDToolHelper\tunable to access GeoModelSvc "<<std::endl;
    }
    result=Gaudi::svcLocator()->service("RDBAccessSvc",p_RDBAccessSvc);
    if (result.isFailure())
    {
	std::cout<<"MuonAGDDToolHelper\tunable to access RBDAccessSvc "<<std::endl;
    }
}

std::vector<std::string> MuonAGDDToolHelper::ReadAGDDFlags()
{
   std::vector<std::string> structuresFromFlags;
   std::string agdd2geoVersion = p_RDBAccessSvc->getChildTag("AGDD2GeoSwitches",p_GeoModelSvc->muonVersion(),"MuonSpectrometer");

   if(!agdd2geoVersion.empty())
   {
     std::string TheKEYNAME;
     int TheKEYVALUE;
     IRDBRecordset_ptr pIRDBRecordset = p_RDBAccessSvc->getRecordsetPtr("AGDD2GeoSwitches",p_GeoModelSvc->muonVersion(),"MuonSpectrometer");
     for(unsigned int i=0; i<pIRDBRecordset->size(); i++)
     {
       const IRDBRecord* record = (*pIRDBRecordset)[i];
       TheKEYNAME = record->getString("KEYNAME");
       TheKEYVALUE = record->getInt("KEYVALUE");
       if ( TheKEYVALUE == 1 )
       {
         structuresFromFlags.push_back(TheKEYNAME);
       }
     }
   }
   else
   {
      std::cout<<"MuonAGDDToolHelper\tagdd2geoVersion is empty " <<std::endl;
   }
   return structuresFromFlags;
}


std::string MuonAGDDToolHelper::GetAGDD(const bool dumpIt, const std::string& tableName, const std::string& outFileName)
{

   const IGeoModelSvc * geoModel=p_GeoModelSvc;
   if(!geoModel) return "";

   IRDBAccessSvc * accessSvc=p_RDBAccessSvc;
   if(!accessSvc) return "";

   std::string AtlasVersion = geoModel->atlasVersion();
   std::string MuonVersion  = geoModel->muonVersionOverride();

   std::string detectorKey  = MuonVersion.empty() ? AtlasVersion : MuonVersion;
   std::string detectorNode = MuonVersion.empty() ? "ATLAS" : "MuonSpectrometer";
   if ( MuonVersion == "CUSTOM"){
     detectorKey  = AtlasVersion ;
     detectorNode = "ATLAS"  ;
   }


   IRDBRecordset_ptr recordsetAGDD = accessSvc->getRecordsetPtr(tableName.c_str(),detectorKey,detectorNode);
   if(!recordsetAGDD) return "";

   const IRDBRecord *recordAGDD =  (*recordsetAGDD)[0];
   if(!recordAGDD) return "";
   std::string AgddString = recordAGDD->getString("DATA");

   size_t pos=AgddString.find("AGDD.dtd");
   if (pos!=std::string::npos) AgddString.replace(pos-21,32,"-- Reference to AGDD.dtd automatically removed -->");
   std::ofstream  GeneratedFile;
   if (dumpIt)
   {
	 	std::ofstream GeneratedFile;
	 	GeneratedFile.open(outFileName);
		GeneratedFile<<AgddString;
		GeneratedFile.close();
   }

   return AgddString;

}

bool MuonAGDDToolHelper::BuildMScomponents()
{
  StoreGateSvc* pDetStore=nullptr;
  ISvcLocator* svcLocator = Gaudi::svcLocator();
  if (svcLocator->service("DetectorStore",pDetStore).isFailure()) return false;
  MuonGM::MuonDetectorManager* muonMgr=nullptr;
  if (pDetStore->retrieve(muonMgr).isFailure()) return false;
  bool readoutGeoDone =  BuildNSWReadoutGeometry::BuildReadoutGeometry(muonMgr, nullptr/*, GetMSdetectors*/);
  return readoutGeoDone;
}

void MuonAGDDToolHelper::SetNSWComponents()
{
  IAGDDtoGeoSvc* agddsvc = nullptr;
  if (Gaudi::svcLocator()->service(m_svcName,agddsvc).isFailure()) {
    throw std::runtime_error("MuonAGDDToolHelper::SetNSWComponents() - Could not retrieve "
                             + m_svcName + " from ServiceLocator");
  }

  IAGDDtoGeoSvc::LockedController c = agddsvc->getController();

  agddsvc->addHandler(new micromegasHandler("micromegas", *c));
  agddsvc->addHandler(new mm_TechHandler("mm_Tech", *c));
  agddsvc->addHandler(new sTGCHandler("sTGC", *c));
  agddsvc->addHandler(new sTGC_readoutHandler("sTGC_readout", *c));
  agddsvc->addHandler(new sTGC_TechHandler("sTGC_Tech", *c));
  agddsvc->addHandler(new mmSpacerHandler("mmSpacer", *c));
  agddsvc->addHandler(new mmSpacer_TechHandler("mmSpacer_Tech", *c));
  agddsvc->addHandler(new mm_readoutHandler("mm_readout", *c));
}

void MuonAGDDToolHelper::setAGDDtoGeoSvcName(const std::string& name) {
  m_svcName = name;
}
