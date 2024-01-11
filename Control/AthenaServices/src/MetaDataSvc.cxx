/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/** @file MetaDataSvc.cxx
 *  @brief This file contains the implementation for the MetaDataSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "MetaDataSvc.h"

#include "Gaudi/Interfaces/IOptionsSvc.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IEvtSelector.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IIoComponentMgr.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/System.h"

#include "AthenaBaseComps/AthCnvSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "SGTools/SGVersionedKey.h"
#include "PersistentDataModel/DataHeader.h"
#include "RootAuxDynIO/RootAuxDynIO.h"

#include "OutputStreamSequencerSvc.h"

#include <vector>
#include <sstream>

#include "boost/bind/bind.hpp"
#include "boost/algorithm/string/predicate.hpp"

namespace {
  bool
  leftString(std::string & s, char sc){
    bool truncated{false};
    auto n = s.find(sc);
    if (n!=std::string::npos){
      s.resize(n);
      truncated=true;
    }
    return truncated;
  }

}


//________________________________________________________________________________
MetaDataSvc::MetaDataSvc(const std::string& name, ISvcLocator* pSvcLocator) : ::AthService(name, pSvcLocator),
	m_inputDataStore("StoreGateSvc/InputMetaDataStore", name),
	m_outputDataStore("StoreGateSvc/MetaDataStore", name),
	m_addrCrtr("AthenaPoolCnvSvc", name),
	m_fileMgr("FileMgr", name),
	m_incSvc("IncidentSvc", name),
        m_outSeqSvc("OutputStreamSequencerSvc", name),
	m_storageType(0L),
	m_clearedInputDataStore(true),
	m_clearedOutputDataStore(false),
	m_allowMetaDataStop(false),
        m_outputPrepared(false),
	m_persToClid(),
	m_toolForClid() {
   // declare properties
   declareProperty("MetaDataContainer", m_metaDataCont = "");
   declareProperty("MetaDataTools", m_metaDataTools);
   declareProperty("CnvSvc", m_addrCrtr = ServiceHandle<IAddressCreator>("AthenaPoolCnvSvc", name));
   // persistent class name to transient CLID map
   m_persToClid.insert(std::pair<std::string, CLID>("DataHeader_p5", 222376821));
   m_persToClid.insert(std::pair<std::string, CLID>("EventStreamInfo_p3", 167728019));
   m_persToClid.insert(std::pair<std::string, CLID>("ByteStreamMetadataContainer_p1", 1076128893));
   m_persToClid.insert(std::pair<std::string, CLID>("IOVMetaDataContainer_p1", 1316383046));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::EventFormat_v1", 243004407));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::CutBookkeeperContainer_v1", 1234982351));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::CutBookkeeperAuxContainer_v1", 1147935274));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::TriggerMenuContainer_v1", 1107011239));
   m_persToClid.insert(std::pair<std::string, CLID>("DataVector<xAOD::TriggerMenu_v1>", 1107011239));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::TriggerMenuAuxContainer_v1", 1212409402));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::TriggerMenuJsonContainer_v1", 1221262614));
   m_persToClid.insert(std::pair<std::string, CLID>("DataVector<xAOD::TriggerMenuJson_v1>", 1221262614));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::TriggerMenuJsonAuxContainer_v1", 373045213));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::LumiBlockRangeContainer_v1", 1115934851));
   m_persToClid.insert(std::pair<std::string, CLID>("DataVector<xAOD::LumiBlockRange_v1>", 1115934851));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::LumiBlockRangeAuxContainer_v1", 1251061086));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::FileMetaData_v1", 178309087));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::FileMetaDataAuxInfo_v1", 73252552));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::RingSetConfContainer_v1", 1157997427));
   m_persToClid.insert(std::pair<std::string, CLID>("DataVector<xAOD::RingSetConf_v1>", 1157997427));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::RingSetConfAuxContainer_v1", 1307745126));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::TruthMetaDataContainer_v1", 1188015687));
   m_persToClid.insert(std::pair<std::string, CLID>("DataVector<xAOD::TruthMetaData_v1>", 1188015687));
   m_persToClid.insert(std::pair<std::string, CLID>("xAOD::TruthMetaDataAuxContainer_v1", 1094306618));
   // some classes need to have new/different tools added for metadata propagation
   m_toolForClid.insert(std::pair<CLID, std::string>(167728019, "CopyEventStreamInfo"));
   m_toolForClid.insert(std::pair<CLID, std::string>(243004407, "xAODMaker::EventFormatMetaDataTool"));
   m_toolForClid.insert(std::pair<CLID, std::string>(1234982351, "BookkeeperTool"));
   m_toolForClid.insert(std::pair<CLID, std::string>(1107011239, "xAODMaker::TriggerMenuMetaDataTool"));
   m_toolForClid.insert(std::pair<CLID, std::string>(1221262614, "xAODMaker::TriggerMenuMetaDataTool"));
   m_toolForClid.insert(std::pair<CLID, std::string>(1115934851, "LumiBlockMetaDataTool"));
   m_toolForClid.insert(std::pair<CLID, std::string>(178309087, "xAODMaker::FileMetaDataTool"));
   m_toolForClid.insert(std::pair<CLID, std::string>(1188015687, "xAODMaker::TruthMetaDataTool"));
}
//__________________________________________________________________________
MetaDataSvc::~MetaDataSvc() {
}
//__________________________________________________________________________
StatusCode MetaDataSvc::initialize() {
   ATH_MSG_INFO("Initializing " << name());

   // Retrieve InputMetaDataStore
   ATH_CHECK( m_inputDataStore.retrieve() );
   // Retrieve OutputMetaDataStore
   ATH_CHECK( m_outputDataStore.retrieve() );
   // Retrieve AddressCreator
   ATH_CHECK( m_addrCrtr.retrieve() );

   AthCnvSvc* cnvSvc = dynamic_cast<AthCnvSvc*>(m_addrCrtr.operator->());
   if (cnvSvc) {
      m_storageType = cnvSvc->repSvcType();
   } else {
      ATH_MSG_WARNING("Cannot get ConversionSvc Interface.");
   }
   // Get FileMgr
   ATH_CHECK( m_fileMgr.retrieve() );

   // Set to be listener for end of event
   ATH_CHECK( m_incSvc.retrieve() );

   ATH_CHECK( m_metaDataTools.retrieve() );
   ATH_MSG_INFO("Found " << m_metaDataTools);

   m_incSvc->addListener(this, "FirstInputFile", 80, true);
   m_incSvc->addListener(this, "BeginInputFile", 80, true);
   m_incSvc->addListener(this, "EndInputFile", 10, true);

   // Register this service for 'I/O' events
   ServiceHandle<IIoComponentMgr> iomgr("IoComponentMgr", this->name());
   ATH_CHECK( iomgr.retrieve() );
   ATH_CHECK( iomgr->io_register(this) );

   ServiceHandle<Gaudi::Interfaces::IOptionsSvc> joSvc("JobOptionsSvc", name());
   if (!joSvc.retrieve().isSuccess()) {
      ATH_MSG_WARNING("Cannot get JobOptionsSvc.");
   } else {
      if (joSvc->has("EventSelector.InputCollections")) {
         // Get EventSelector to force in-time initialization and FirstInputFile incident
         ServiceHandle<IEvtSelector> evtsel("EventSelector", this->name());
         if (!evtsel.retrieve().isSuccess()) {
            ATH_MSG_WARNING("Cannot get EventSelector.");
         }
      }
   }
   // retrieve the output sequences service (EventService) if available
   m_outSeqSvc.retrieve().ignore();

   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
StatusCode MetaDataSvc::finalize() {
   // Release IncidentService
   if (!m_incSvc.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release IncidentService.");
   }
   // Release FileMgr
   if (!m_fileMgr.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release FileMgr.");
   }
   // Release AddressCreator
   if (!m_addrCrtr.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release AddressCreator.");
   }
   // Release OutputMetaDataStore
   if (!m_outputDataStore.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release OutputMetaDataStore.");
   }
   // Release InputMetaDataStore
   if (!m_inputDataStore.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release InputMetaDataStore.");
   }
   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
StatusCode MetaDataSvc::stop() {
   ATH_MSG_DEBUG("MetaDataSvc::stop()");

   if( m_outSeqSvc.isValid() and m_outSeqSvc->inUse() ) {
      ATH_MSG_INFO("stop(): OutputSequencer in use, not firing MetaDataStop incident");
   } else {
      // Fire metaDataStopIncident
      Incident metaDataStopIncident(name(), "MetaDataStop");
      m_incSvc->fireIncident(metaDataStopIncident);
   }
   return(StatusCode::SUCCESS);
}
//_______________________________________________________________________
StatusCode MetaDataSvc::queryInterface(const InterfaceID& riid, void** ppvInterface) {
   if (riid == this->interfaceID()) {
      *ppvInterface = this;
   } else if (riid == IMetaDataSvc::interfaceID()) {
     *ppvInterface = dynamic_cast<IMetaDataSvc*>(this);
   } else {
      // Interface is not directly available: try out a base class
      return(::AthService::queryInterface(riid, ppvInterface));
   }
   addRef();
   return(StatusCode::SUCCESS);
}
//________________________________________________________________________________
StatusCode MetaDataSvc::loadAddresses(StoreID::type storeID, IAddressProvider::tadList& tads) {
   if (storeID != StoreID::METADATA_STORE) { // should this (also) run in the INPUT_METADATA_STORE?
      return(StatusCode::SUCCESS);
   }
   // Put Additional MetaData objects into Input MetaData Store using VersionedKey
   std::list<SG::ObjectWithVersion<DataHeader> > allVersions;
   StatusCode sc = m_inputDataStore->retrieveAllVersions(allVersions, name());
   if (!sc.isSuccess()) {
      ATH_MSG_WARNING("Could not retrieve all versions for DataHeader, will not read Metadata");
   } else {
      int verNumber = -1;
      for (SG::ObjectWithVersion<DataHeader>& obj : allVersions) {
         ++verNumber;
         const DataHeader* dataHeader = obj.dataObject.cptr();
         if (dataHeader == nullptr) {
            ATH_MSG_ERROR("Could not get DataHeader, will not read Metadata");
            return(StatusCode::FAILURE);
         }
         for (const DataHeaderElement& dhe : *dataHeader) {
            const CLID clid = dhe.getPrimaryClassID();
            if (clid != ClassID_traits<DataHeader>::ID()) {
               SG::VersionedKey myVersObjKey(dhe.getKey(), verNumber);
               std::string key = dhe.getKey();
               if (verNumber != 0) key = myVersObjKey;
               tads.push_back(dhe.getAddress(key));
            }
         }
      }
   }
   return(StatusCode::SUCCESS);
}
//________________________________________________________________________________
StatusCode MetaDataSvc::newMetadataSource(const Incident& inc)
{
   const FileIncident* fileInc  = dynamic_cast<const FileIncident*>(&inc);
   if (fileInc == nullptr) {
      ATH_MSG_ERROR("Unable to get FileName from EndInputFile incident");
      return StatusCode::FAILURE;
   }
   const std::string guid = fileInc->fileGuid();
   const std::string fileName = fileInc->fileName();
   m_allowMetaDataStop = false;
   if (!boost::starts_with (fileName, "BSF:")) {
      // the input file is _not_ bytestream
      if (!m_clearedInputDataStore) {
         if (!m_inputDataStore->clearStore().isSuccess()) {
            ATH_MSG_WARNING("Unable to clear input MetaData Proxies");
         }
         m_clearedInputDataStore = true;
      }
      if (!initInputMetaDataStore(fileName).isSuccess()) {
         ATH_MSG_ERROR("Unable to initialize InputMetaDataStore");
         return StatusCode::FAILURE;
      }
   }
   StatusCode rc(StatusCode::SUCCESS);
   for (auto it = m_metaDataTools.begin(); it != m_metaDataTools.end(); ++it) {
      ATH_MSG_DEBUG(" calling beginInputFile on " << (*it)->name() << " for GUID \"" << guid << "\"");
      if ( (*it)->beginInputFile(guid).isFailure() ) {
         ATH_MSG_ERROR("Unable to call beginInputFile for " << (*it)->name());
         rc = StatusCode::FAILURE;
      }
   }
   return rc;
}

StatusCode MetaDataSvc::retireMetadataSource(const Incident& inc)
{
   const FileIncident* fileInc  = dynamic_cast<const FileIncident*>(&inc);
   if (fileInc == nullptr) {
      ATH_MSG_ERROR("Unable to get FileName from EndInputFile incident");
      return StatusCode::FAILURE;
   }
   const std::string guid = fileInc->fileGuid();
   ATH_MSG_DEBUG("retireMetadataSource: " << fileInc->fileName());
   for (auto it = m_metaDataTools.begin(); it != m_metaDataTools.end(); ++it) {
      ATH_MSG_DEBUG(" calling endInputFile on " << (*it)->name() << " for GUID \"" << guid << "\"");
      if ( (*it)->endInputFile(guid).isFailure() ) {
         ATH_MSG_ERROR("Unable to call endInputFile for " << (*it)->name());
         return StatusCode::FAILURE;
      }
   }
   m_allowMetaDataStop = true;
   return StatusCode::SUCCESS;
}

StatusCode MetaDataSvc::prepareOutput()
{
   StatusCode rc(StatusCode::SUCCESS);
   // Check if already called
   if (!m_outputPrepared) {
      for (auto it = m_metaDataTools.begin(); it != m_metaDataTools.end(); ++it) {
         ATH_MSG_DEBUG(" calling metaDataStop for " << (*it)->name());
         if ( (*it)->metaDataStop().isFailure() ) {
            ATH_MSG_ERROR("Unable to call metaDataStop for " << (*it)->name());
            rc = StatusCode::FAILURE;
         }
      }
      if (!m_metaDataTools.release().isSuccess()) {
         ATH_MSG_WARNING("Cannot release " << m_metaDataTools);
      }
   }
   m_outputPrepared=true;
   return rc;
}

// like prepareOutput() but for parallel streams
StatusCode MetaDataSvc::prepareOutput(const std::string& outputName)
{
   // default to the serial implementation if no output name given
   if( outputName.empty() ) return prepareOutput();
   ATH_MSG_DEBUG( "prepareOutput('" << outputName << "')" );

   StatusCode rc = StatusCode::SUCCESS;
   for (auto it = m_metaDataTools.begin(); it != m_metaDataTools.end(); ++it) {
      ATH_MSG_DEBUG("  calling metaDataStop for " << (*it)->name());
      // planning to replace the call below with  (*it)->prepareOutput(outputName)
      if ( (*it)->metaDataStop().isFailure() ) {
         ATH_MSG_ERROR("Unable to call metaDataStop for " << (*it)->name());
         rc = StatusCode::FAILURE;
      }
   }
   // MN: not releasing tools here - revisit when clear what happens on new file open
   return rc;
}


StatusCode MetaDataSvc::shmProxy(const std::string& filename)
{
   if (!m_clearedInputDataStore) {
      if (!m_inputDataStore->clearStore(true).isSuccess()) {
         ATH_MSG_ERROR("Unable to clear input MetaData Proxies");
	 return StatusCode::FAILURE;
      }
      m_clearedInputDataStore = true;
   }
   if (!m_clearedOutputDataStore) {
      if (!m_outputDataStore->clearStore(true).isSuccess()) {
         ATH_MSG_ERROR("Unable to clear output MetaData Proxies");
	 return StatusCode::FAILURE;
      }
      m_clearedOutputDataStore = true;
   }
   if (!addProxyToInputMetaDataStore(filename).isSuccess()) {
      ATH_MSG_ERROR("Unable to add proxy to InputMetaDataStore");
      return StatusCode::FAILURE;
   }
   return StatusCode::SUCCESS;
}

//__________________________________________________________________________
void MetaDataSvc::handle(const Incident& inc) {
   const FileIncident* fileInc  = dynamic_cast<const FileIncident*>(&inc);
   if (fileInc == nullptr) {
      ATH_MSG_ERROR("Unable to get FileName from EndInputFile incident");
      return;
   }
   const std::string fileName = fileInc->fileName();
   ATH_MSG_DEBUG("handle() " << inc.type() << " for " << fileName);

   if (inc.type() == "FirstInputFile") {
      // Register open/close callback actions
     using namespace boost::placeholders;
      Io::bfcn_action_t boa = boost::bind(&MetaDataSvc::rootOpenAction, this, _1,_2);
      if (m_fileMgr->regAction(boa, Io::OPEN).isFailure()) {
         ATH_MSG_FATAL("Cannot register ROOT file open action with FileMgr.");
      }
      if (!initInputMetaDataStore(fileName).isSuccess()) {
         ATH_MSG_WARNING("Unable to initialize InputMetaDataStore");
      }
   } else if (inc.type() == "BeginInputFile") {
      if(newMetadataSource(inc).isFailure()) {
         ATH_MSG_ERROR("Could not process new metadata source " << fileName);
      }
   } else if (inc.type() == "EndInputFile") {
      if(retireMetadataSource(inc).isFailure()) {
         ATH_MSG_ERROR("Could not retire metadata source " << fileName);
      }
   }
}

//__________________________________________________________________________
// This method is currently called only from OutputStreamSequencerSvc
StatusCode MetaDataSvc::transitionMetaDataFile(const std::string& outputConn, bool disconnect)
{
   ATH_MSG_DEBUG("transitionMetaDataFile: " << outputConn );

   // this is normally called through EndInputFile inc, simulate it for EvSvc
   FileIncident inc("transitionMetaDataFile", "EndInputFile", "dummyMetaInputFileName", "");
   ATH_CHECK(retireMetadataSource(inc));

   // Reset flag to allow calling prepareOutput again at next transition
   m_outputPrepared = false;

   Incident metaDataStopIncident(name(), "MetaDataStop");
   m_incSvc->fireIncident(metaDataStopIncident);

   if( disconnect ) {
      AthCnvSvc* cnvSvc = dynamic_cast<AthCnvSvc*>(m_addrCrtr.operator->());
      if (cnvSvc) {
         if (!cnvSvc->disconnectOutput(outputConn).isSuccess()) {
            ATH_MSG_WARNING("Cannot get disconnect Output Files");
         }
      }
   }

   return(StatusCode::SUCCESS);
}

//__________________________________________________________________________
StatusCode MetaDataSvc::io_reinit() {
   ATH_MSG_INFO("I/O reinitialization...");
   ATH_MSG_INFO("Dumping InputMetaDataStore: " << m_inputDataStore->dump());
   ATH_MSG_INFO("Dumping OutputMetaDataStore: " << m_outputDataStore->dump());
   for (auto iter = m_metaDataTools.begin(),
 	     last = m_metaDataTools.end(); iter != last; iter++) {
      ATH_MSG_INFO("Attached MetaDataTool: " << (*iter)->name());
   }
   m_outputPrepared = false;
   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
StatusCode MetaDataSvc::rootOpenAction(FILEMGR_CALLBACK_ARGS) {
   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
// check if the metadata object key contains Stream name (added by SharedWriter in MetaDataSvc)
// remove stream part from the key (i.e. modify the parameter) and return it
std::string MetaDataSvc::removeStreamFromKey(std::string& key) {
   size_t pos = key.find(m_streamInKeyMark);
   if( pos==std::string::npos ) return "";
   size_t epos = key.find(']', pos);
   size_t spos = pos + m_streamInKeyMark.size();
   std::string stream = key.substr( spos, epos - spos );
   //cppcheck-suppress uselessCallsSubstr
   key = key.substr(0, pos) + key.substr(epos+1);
   return stream;
}
//__________________________________________________________________________
std::set<std::string> MetaDataSvc::getPerStreamKeysFor(const std::string& key ) const {
   auto iter = m_streamKeys.find( key );
   if( iter == m_streamKeys.end() ) {
      return std::set<std::string>( {key} );
   }
   return iter->second;
}
//__________________________________________________________________________
StatusCode MetaDataSvc::addProxyToInputMetaDataStore(const std::string& tokenStr) {
   std::string fileName = tokenStr.substr(tokenStr.find("[FILE=") + 6);
   leftString(fileName, ']');
   std::string className = tokenStr.substr(tokenStr.find("[PNAME=") + 7);
   leftString(className, ']');
   std::string contName = tokenStr.substr(tokenStr.find("[CONT=") + 6);
   leftString(contName,']');
   std::size_t pos1 = contName.find('(');
   std::string keyName = contName.substr(pos1 + 1, contName.size() - pos1 - 2);
   std::size_t pos2 = keyName.find('/');
   if (pos2 != std::string::npos) keyName = keyName.substr(pos2 + 1);
   std::string numName = tokenStr.substr(tokenStr.find("[NUM=") + 5);
   leftString(numName,']');
   unsigned long num = 0;
   std::istringstream iss(numName);
   iss >> num;
   CLID clid = m_persToClid[className];
   if (clid == 167728019) { // EventStreamInfo, will change tool to combine input metadata, clearing things before...
      bool foundTool = false;
      for (auto iter = m_metaDataTools.begin(), iterEnd = m_metaDataTools.end(); iter != iterEnd; iter++) {
         if ((*iter)->name() == "ToolSvc.CopyEventStreamInfo") foundTool = true;
      }
      if (!foundTool) {
         if (serviceLocator()->existsService("CutFlowSvc")) {
            ServiceHandle<IIncidentListener> cfSvc("CutFlowSvc", this->name()); // Disable CutFlowSvc by stopping its incidents.
            if (cfSvc.retrieve().isSuccess()) {
               ATH_MSG_INFO("Disabling incidents for: " << cfSvc.name());
               m_incSvc->removeListener(cfSvc.get(), IncidentType::BeginInputFile);
               m_incSvc->removeListener(cfSvc.get(), "MetaDataStop");
               cfSvc.release().ignore();
            }
         }
         if (serviceLocator()->existsService("xAODConfigSvc")) {
            ServiceHandle<IIncidentListener> xcSvc("xAODConfigSvc", this->name()); // Disable xAODConfigSvc, fails for merging
            if (xcSvc.retrieve().isSuccess()) {
               ATH_MSG_INFO("Disabling incidents for: " << xcSvc.name());
               m_incSvc->removeListener(xcSvc.get(), IncidentType::BeginInputFile);
               m_incSvc->removeListener(xcSvc.get(), IncidentType::BeginEvent);
               xcSvc.release().ignore();
            }
         }
      }
   }
   const std::string toolName = m_toolForClid[clid];
   if (!toolName.empty()) {
      std::string toolInstName;
      std::size_t pos = toolName.find("::");
      if (pos != std::string::npos) {
         toolInstName = toolName.substr(pos + 2);
      } else {
         toolInstName = toolName;
      }
      bool foundTool = false;
      for (auto iter = m_metaDataTools.begin(), iterEnd = m_metaDataTools.end(); iter != iterEnd; iter++) {
         if ((*iter)->name() == "ToolSvc." + toolInstName) foundTool = true;
      }
      if (!foundTool) {
         if (toolInstName != toolName) {
            toolInstName = toolName + "/" + toolInstName;
         }
         ToolHandle<IMetaDataTool> metadataTool(toolInstName);
         m_metaDataTools.push_back(metadataTool);
         ATH_MSG_DEBUG("Added new MetaDataTool: " << metadataTool->name());
         if (!metadataTool.retrieve().isSuccess()) {
            ATH_MSG_FATAL("Cannot get " << toolInstName);
            return(StatusCode::FAILURE);
         }
      }
   }
   // make stream-unique keys for infile metadata objects
   // AthenaOutputStream will use this to distribute objects to the right stream (and restore the original key)
   if( clid == 178309087 ) {  // FileMetaData
      std::string newName = keyName + m_streamInKeyMark + fileName + "]";
      ATH_MSG_DEBUG("Recording " << keyName << " as " << newName);
      m_streamKeys[keyName].insert(newName);
      keyName = std::move(newName);
   }
   if( clid == 73252552 ) {  // FileMetaDataAuxInfo
      std::string newName = keyName.substr(0, keyName.find(RootAuxDynIO::AUX_POSTFIX)) + m_streamInKeyMark
                          + fileName +  + "]" + RootAuxDynIO::AUX_POSTFIX;
      ATH_MSG_DEBUG("Recording " << keyName << " as " << newName);
      m_streamKeys[keyName].insert(newName);
      keyName = std::move(newName);
   }
   const std::string par[3] = { "SHM" , keyName , className };
   const unsigned long ipar[2] = { num , 0 };
   IOpaqueAddress* opqAddr = nullptr;
   SG::DataProxy* dp = m_inputDataStore->proxy(clid, keyName);
   if (dp != nullptr) {
      ATH_MSG_DEBUG("Resetting duplicate proxy for: " << clid << "#" << keyName << " from file: " << fileName);
      dp->reset();
   }
   if (!m_addrCrtr->createAddress(m_storageType, clid, par, ipar, opqAddr).isSuccess()) {
      ATH_MSG_FATAL("addProxyToInputMetaDataStore: Cannot create address for " << tokenStr);
      return(StatusCode::FAILURE);
   }
   if (m_inputDataStore->recordAddress(keyName, opqAddr).isFailure()) {
      delete opqAddr; opqAddr = nullptr;
      ATH_MSG_FATAL("addProxyToInputMetaDataStore: Cannot create proxy for " << tokenStr);
      return(StatusCode::FAILURE);
   }
   if (m_inputDataStore->accessData(clid, keyName) == nullptr) {
      ATH_MSG_FATAL("addProxyToInputMetaDataStore: Cannot access data for " << tokenStr);
      return(StatusCode::FAILURE);
   }
   if (keyName.find(RootAuxDynIO::AUX_POSTFIX) != std::string::npos
       && m_inputDataStore->symLink(clid, keyName, 187169987).isFailure()) {
      ATH_MSG_WARNING("addProxyToInputMetaDataStore: Cannot symlink to AuxStore for " << tokenStr);
   }
   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
StatusCode MetaDataSvc::initInputMetaDataStore(const std::string& fileName) {
   ATH_MSG_DEBUG("initInputMetaDataStore: file name " << fileName);
   m_clearedInputDataStore = false;
   // Load proxies for InputMetaDataStore
   if (m_metaDataCont.value().empty()) {
      ATH_MSG_DEBUG("MetaDataSvc called without MetaDataContainer set.");
      return(StatusCode::SUCCESS);
   }
   if (boost::starts_with (fileName, "BSF:")) {
      ATH_MSG_DEBUG("MetaDataSvc called for non ROOT file.");
   } else if (fileName.compare(0, 3, "SHM")==0) {
      ATH_MSG_DEBUG("MetaDataSvc called for shared memory.");
   } else {
      const std::string par[2] = { fileName,  m_metaDataCont.value() + "(DataHeader)" };
      const std::string parOld[2] = { fileName,  m_metaDataCont.value() + "DataHeader" };
      for (int verNumber = 0; verNumber < 100; verNumber++) {
         SG::VersionedKey myVersKey(name(), verNumber);
         if (m_inputDataStore->contains<DataHeader>(myVersKey)) {
            ATH_MSG_DEBUG("initInputMetaDataStore: MetaData Store already contains DataHeader, key = " << myVersKey);
         } else {
            const unsigned long ipar[2] = { (unsigned long)verNumber , 0 };
            IOpaqueAddress* opqAddr = nullptr;
            if (!m_addrCrtr->createAddress(m_storageType, ClassID_traits<DataHeader>::ID(), par, ipar, opqAddr).isSuccess()) {
               if (!m_addrCrtr->createAddress(m_storageType, ClassID_traits<DataHeader>::ID(), parOld, ipar, opqAddr).isSuccess()) {
                  break;
               }
            }
            if (m_inputDataStore->recordAddress(myVersKey, opqAddr).isFailure()) {
               delete opqAddr; opqAddr = nullptr;
               ATH_MSG_WARNING("initInputMetaDataStore: Cannot create proxy for DataHeader, key = " << myVersKey);
            }
         }
      }
      std::list<SG::TransientAddress*> tList;
      if (!loadAddresses(StoreID::METADATA_STORE, tList).isSuccess()) {
         ATH_MSG_ERROR("Unable to load MetaData Proxies");
         return StatusCode::FAILURE;
      }
      for (SG::TransientAddress* tad : tList) {
         CLID clid = tad->clID();
          ATH_MSG_VERBOSE("initInputMetaDataStore: add proxy for clid = " << clid << ", key = " << tad->name());
         if (m_inputDataStore->contains(tad->clID(), tad->name())) {
            ATH_MSG_DEBUG("initInputMetaDataStore: MetaData Store already contains clid = " << clid << ", key = " << tad->name());
         } else {
            if (!m_inputDataStore->recordAddress(tad->name(), tad->address())) {
               ATH_MSG_ERROR("initInputMetaDataStore: Cannot create proxy for clid = " << clid << ", key = " << tad->name());
               return StatusCode::FAILURE;
            }
         }

         for (CLID tclid : tad->transientID()) {
           if (tclid != clid) {
             if (m_inputDataStore->symLink (clid, tad->name(), tclid).isFailure()) {
               ATH_MSG_WARNING("Cannot make autosymlink from " <<
                               clid << "/" << tad->name() << " to " << tclid);
             }
           }
         }
         delete tad;
      }
      tList.clear();
   }
   ATH_MSG_DEBUG("Loaded input meta data store proxies");
   return(StatusCode::SUCCESS);
}


const std::string MetaDataSvc::currentRangeID() const
{
   return m_outSeqSvc.isValid()? m_outSeqSvc->currentRangeID() : "";
}


CLID MetaDataSvc::remapMetaContCLID( const CLID& itemID ) const
{
   auto it =  m_handledClasses.find(itemID);
   if (it == m_handledClasses.end()) {
      ATH_MSG_DEBUG("Not translating metadata item ID #" << itemID);
      return itemID;
   }

   std::string itemName;
   CLID contID = 0;
   if (m_classIDSvc->getTypeNameOfID(itemID, itemName).isSuccess()) {
     const std::string contName = "MetaCont<" + itemName + ">";
     ATH_MSG_DEBUG("Transforming " << contName << " to " << itemName
                   << " for output");
     if (m_classIDSvc->getIDOfTypeName(contName, contID).isSuccess())
       return contID;
   }

   return itemID;
}

void MetaDataSvc::recordHook(const std::type_info& typeInfo) {
  const std::string& typeName = System::typeinfoName(typeInfo);
  ATH_MSG_VERBOSE("Handling record event of type " << typeName);

  CLID itemID = 0;
  if (m_classIDSvc->getIDOfTypeInfoName(typeName, itemID).isSuccess()) {
    auto result =  m_handledClasses.insert(itemID);
    if (result.second)
      ATH_MSG_DEBUG("MetaDataSvc will handle " << typeName
                    << " ClassID: " << itemID);
  }
}

void MetaDataSvc::removeHook(const std::type_info& typeInfo) {
  const std::string& typeName = System::typeinfoName(typeInfo);
  ATH_MSG_VERBOSE("Handling removal event of type " << typeName);

  CLID itemID = 0;
  if (m_classIDSvc->getIDOfTypeInfoName(typeName, itemID).isSuccess()) {
    if (0 < m_handledClasses.erase(itemID))
      ATH_MSG_DEBUG("MetaDataSvc will no longer handle " << typeName
                    << " ClassID: " << itemID);
  }
}


void MetaDataSvc::lockTools() const
{
   ATH_MSG_DEBUG("Locking metadata tools");
   for(auto tool : m_metaDataTools ) {
      ILockableTool *lockable = dynamic_cast<ILockableTool*>( tool.get() );
      if( lockable ) lockable->lock_shared();
   }
}


void MetaDataSvc::unlockTools() const
{
   ATH_MSG_DEBUG("Unlocking metadata tools");
   for(auto tool : m_metaDataTools ) {
      ILockableTool *lockable = dynamic_cast<ILockableTool*>( tool.get() );
      if( lockable ) lockable->unlock_shared();
   }
}


