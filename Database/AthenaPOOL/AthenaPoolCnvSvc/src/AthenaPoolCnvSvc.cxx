/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/** @file AthenaPoolCnvSvc.cxx
 *  @brief This file contains the implementation for the AthenaPoolCnvSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaPoolCnvSvc.h"

#include "GaudiKernel/AttribStringParser.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IIoComponentMgr.h"
#include "GaudiKernel/IOpaqueAddress.h"

#include "AthenaKernel/IAthenaSerializeSvc.h"
#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "AthenaKernel/IMetaDataSvc.h"
#include "PersistentDataModel/Placement.h"
#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/TokenAddress.h"
#include "PersistentDataModel/DataHeader.h"

#include "StorageSvc/DbReflex.h"
#include "FileCatalog/IFileCatalog.h"

#include "AuxDiscoverySvc.h"

#include "boost/algorithm/string.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

//______________________________________________________________________________
// Initialize the service.
StatusCode AthenaPoolCnvSvc::initialize() {
   // Initialize DataModelCompatSvc
   ServiceHandle<IService> dmcsvc("DataModelCompatSvc", this->name());
   ATH_CHECK(dmcsvc.retrieve());
   // Retrieve PoolSvc
   ATH_CHECK(m_poolSvc.retrieve());
   // Retrieve ClassIDSvc
   ATH_CHECK(m_clidSvc.retrieve());
   // Retrieve InputStreamingTool (if configured)
   if (!m_inputStreamingTool.empty()) {
      ATH_CHECK(m_inputStreamingTool.retrieve());
   }
   // Retrieve OutputStreamingTool (if configured)
   if (!m_outputStreamingTool.empty()) {
      ATH_CHECK(m_outputStreamingTool.retrieve());
      if (m_makeStreamingToolClient.value() == -1) {
        // Initialize AthenaRootSharedWriter
        ServiceHandle<IService> arswsvc("AthenaRootSharedWriterSvc", this->name());
        ATH_CHECK(arswsvc.retrieve());
      }
      // Put PoolSvc into share mode to avoid duplicating catalog.
      m_poolSvc->setShareMode(true);
   }
   if (!m_inputStreamingTool.empty() || !m_outputStreamingTool.empty()) {
      // Retrieve AthenaSerializeSvc
      ATH_CHECK(m_serializeSvc.retrieve());
   }
   // Register this service for 'I/O' events
   ServiceHandle<IIoComponentMgr> iomgr("IoComponentMgr", name());
   ATH_CHECK(iomgr.retrieve());
   if (!iomgr->io_register(this).isSuccess()) {
      ATH_MSG_FATAL("Could not register myself with the IoComponentMgr !");
      return(StatusCode::FAILURE);
   }
   // Extracting MaxFileSizes for global default and map by Database name.
   for (std::vector<std::string>::const_iterator iter = m_maxFileSizes.value().begin(),
	   last = m_maxFileSizes.value().end(); iter != last; ++iter) {
      if (auto p = iter->find('='); p != std::string::npos) {
         long long maxFileSize = atoll(iter->data() + (p + 1));
         std::string databaseName = iter->substr(0, iter->find_first_of(" 	="));
         m_databaseMaxFileSize.insert(std::make_pair(databaseName, maxFileSize));
      } else {
         m_domainMaxFileSize = atoll(iter->c_str());
      }
   }
   ATH_MSG_DEBUG("Setting StorageType to " << m_storageTechProp.value());
   #define CHECK_TECH(TECH) \
      if(m_storageTechProp.value() == #TECH) m_dbType = pool::TECH##_StorageType
   CHECK_TECH(ROOTTREE);
   CHECK_TECH(ROOTTREEINDEX);
   CHECK_TECH(ROOTRNTUPLE);
   if( m_dbType == TEST_StorageType ) {
      ATH_MSG_FATAL("Unknown StorageType rquested: " << m_storageTechProp.value());
      return StatusCode::FAILURE;
   } 
   // Extracting INPUT POOL ItechnologySpecificAttributes for Domain, Database and Container.
   extractPoolAttributes(m_inputPoolAttr, &m_inputAttr, &m_inputAttr, &m_inputAttr);
   // Extracting the INPUT POOL ItechnologySpecificAttributes which are to be printed for each event
   extractPoolAttributes(m_inputPoolAttrPerEvent, &m_inputAttrPerEvent, &m_inputAttrPerEvent, &m_inputAttrPerEvent);
   // Setup incident for EndEvent to print out attributes each event
   ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", name());
   long int pri = 1000;
   if (!m_inputPoolAttrPerEvent.value().empty()) {
      // Set to be listener for EndEvent
      incSvc->addListener(this, "EndEvent", pri);
      ATH_MSG_DEBUG("Subscribed to EndEvent for printing out input file attributes.");
   }
   if (!processPoolAttributes(m_inputAttr, "", IPoolSvc::kInputStream, false, true, true).isSuccess()) {
      ATH_MSG_DEBUG("setInputAttribute failed setting POOL domain attributes.");
   }

   if (!m_outputStreamingTool.empty()) {
      incSvc->addListener(this, "StoreCleared", pri);
      ATH_MSG_DEBUG("Subscribed to StoreCleared");
   }
   // Load these dictionaries now, so we don't need to try to do so
   // while multiple threads are running.
   TClass::GetClass ("TLeafI");
   TClass::GetClass ("TLeafL");
   TClass::GetClass ("TLeafD");
   TClass::GetClass ("TLeafF");

   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::io_reinit() {
   ATH_MSG_DEBUG("I/O reinitialization...");
   m_contextAttr.clear();
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::finalize() {
   // Release AthenaSerializeSvc
   if (!m_serializeSvc.empty()) {
      if (!m_serializeSvc.release().isSuccess()) {
         ATH_MSG_WARNING("Cannot release AthenaSerializeSvc.");
      }
   }
   // Release OutputStreamingTool (if configured)
   if (!m_outputStreamingTool.empty()) {
      if (!m_outputStreamingTool.release().isSuccess()) {
         ATH_MSG_WARNING("Cannot release Output AthenaIPCTool.");
      }
   }
   // Release InputStreamingTool (if configured)
   if (!m_inputStreamingTool.empty()) {
      if (!m_inputStreamingTool.release().isSuccess()) {
         ATH_MSG_WARNING("Cannot release Input AthenaIPCTool.");
      }
   }
   // Release ClassIDSvc
   if (!m_clidSvc.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release ClassIDSvc.");
   }
   // Release PoolSvc
   if (!m_poolSvc.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release PoolSvc.");
   }
   // Print Performance Statistics
   // The pattern AthenaPoolCnvSvc.*PerfStats is ignored in AtlasTest/TestTools/share/post.sh
   const std::string msgPrefix{"PerfStats "};
   ATH_MSG_INFO(msgPrefix << std::string(40, '-'));
   ATH_MSG_INFO(msgPrefix << "Timing Measurements for AthenaPoolCnvSvc");
   ATH_MSG_INFO(msgPrefix << std::string(40, '-'));
   for(const auto& [key, value] : m_chronoMap) {
      ATH_MSG_INFO(msgPrefix << "| " << std::left << std::setw(15) << key << " | "
                        << std::right << std::setw(15) << std::fixed << std::setprecision(0) << value << " ms |");
   }
   ATH_MSG_INFO(msgPrefix << std::string(40, '-'));

   m_cnvs.clear();
   m_cnvs.shrink_to_fit();
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::io_finalize() {
   ATH_MSG_DEBUG("I/O finalization...");
   return(StatusCode::SUCCESS);
}
//_______________________________________________________________________
StatusCode AthenaPoolCnvSvc::queryInterface(const InterfaceID& riid, void** ppvInterface) {
   if (IAthenaPoolCnvSvc::interfaceID().versionMatch(riid)) {
      *ppvInterface = dynamic_cast<IAthenaPoolCnvSvc*>(this);
   } else {
      // Interface is not directly available: try out a base class
      return(::AthCnvSvc::queryInterface(riid, ppvInterface));
   }
   addRef();
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::createObj(IOpaqueAddress* pAddress, DataObject*& refpObject) {
   assert(pAddress);
   std::string objName = "ALL";
   if (m_useDetailChronoStat.value()) {
      if (m_clidSvc->getTypeNameOfID(pAddress->clID(), objName).isFailure()) {
         std::ostringstream oss;
         oss << std::dec << pAddress->clID();
         objName = oss.str();
      }
      objName += '#';
      objName += *(pAddress->par() + 1);
   }
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatch("cObj_" + objName, m_chronoMap);
   if (!m_persSvcPerInputType.empty()) { // Use separate PersistencySvc for each input data type
      TokenAddress* tokAddr = dynamic_cast<TokenAddress*>(pAddress);
      if (tokAddr != nullptr && tokAddr->getToken() != nullptr && (boost::starts_with(tokAddr->getToken()->contID(), m_persSvcPerInputType.value() + "(") || boost::starts_with(tokAddr->getToken()->contID(), m_persSvcPerInputType.value() + "_"))) {
         const unsigned int maxContext = m_poolSvc->getInputContextMap().size();
         const unsigned int auxContext = m_poolSvc->getInputContext(tokAddr->getToken()->classID().toString() + tokAddr->getToken()->dbID().toString(), 1);
         char text[32];
         ::sprintf(text, "[CTXT=%08X]", auxContext);
         if (m_poolSvc->getInputContextMap().size() > maxContext) {
            if (m_poolSvc->setAttribute("TREE_CACHE", "0", pool::DbType(pool::ROOTTREE_StorageType).type(), "FID:" + tokAddr->getToken()->dbID().toString(), m_persSvcPerInputType.value(), auxContext).isSuccess()) {
               ATH_MSG_DEBUG("setInputAttribute failed to switch off TTreeCache for id = " << auxContext << ".");
            }
         }
         tokAddr->getToken()->setAuxString(text);
      }
   }
   // Forward to base class createObj
   StatusCode status = ::AthCnvSvc::createObj(pAddress, refpObject);
   return(status);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::createRep(DataObject* pObject, IOpaqueAddress*& refpAddress) {
   assert(pObject);
   std::string objName = "ALL";
   if (m_useDetailChronoStat.value()) {
      if (m_clidSvc->getTypeNameOfID(pObject->clID(), objName).isFailure()) {
         std::ostringstream oss;
         oss << std::dec << pObject->clID();
         objName = oss.str();
      }
      objName += '#';
      objName += pObject->registry()->name();
   }
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatch("cRep_" + objName, m_chronoMap);
   StatusCode status = StatusCode::FAILURE;
   if (pObject->clID() == 1) {
      // No transient object was found use cnv to write default persistent object
      SG::DataProxy* proxy = dynamic_cast<SG::DataProxy*>(pObject->registry());
      if (proxy != nullptr) {
         IConverter* cnv = converter(proxy->clID());
         status = cnv->createRep(pObject, refpAddress);
      }
   } else {
      // Forward to base class createRep
      try {
         status = ::AthCnvSvc::createRep(pObject, refpAddress);
      } catch(std::runtime_error& e) {
         ATH_MSG_FATAL(e.what());
      }
   }
   return(status);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::fillRepRefs(IOpaqueAddress* pAddress, DataObject* pObject) {
   assert(pObject);
   std::string objName = "ALL";
   if (m_useDetailChronoStat.value()) {
      if (m_clidSvc->getTypeNameOfID(pObject->clID(), objName).isFailure()) {
         std::ostringstream oss;
         oss << std::dec << pObject->clID();
         objName = oss.str();
      }
      objName += '#';
      objName += pObject->registry()->name();
   }
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatch("fRep_" + objName, m_chronoMap);
   StatusCode status = StatusCode::FAILURE;
   if (pObject->clID() == 1) {
      // No transient object was found use cnv to write default persistent object
      SG::DataProxy* proxy = dynamic_cast<SG::DataProxy*>(pObject->registry());
      if (proxy != nullptr) {
         IConverter* cnv = converter(proxy->clID());
         status = cnv->fillRepRefs(pAddress, pObject);
      }
   } else {
      // Forward to base class fillRepRefs
      try {
         status = ::AthCnvSvc::fillRepRefs(pAddress, pObject);
      } catch(std::runtime_error& e) {
         ATH_MSG_FATAL(e.what());
      }
   }
   return(status);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::connectOutput(const std::string& outputConnectionSpec,
		const std::string& /*openMode*/) {
   return(connectOutput(outputConnectionSpec));
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::connectOutput(const std::string& outputConnectionSpec) {
// This is called before DataObjects are being converted.
   std::string outputConnection = outputConnectionSpec.substr(0, outputConnectionSpec.find('['));
   // Extract the technology
   int tech = m_dbType.type();
   if (!decodeOutputSpec(outputConnection, tech).isSuccess()) {
      ATH_MSG_ERROR("connectOutput FAILED extract file name and technology.");
      return(StatusCode::FAILURE);
   }
   if (m_makeStreamingToolClient.value() > 0 && !m_outputStreamingTool.empty() && !m_outputStreamingTool[0]->isServer() && !m_outputStreamingTool[0]->isClient()) {
      if (!makeClient(m_makeStreamingToolClient.value()).isSuccess()) {
         ATH_MSG_ERROR("Could not make AthenaPoolCnvSvc a Share Client");
         return(StatusCode::FAILURE);
      }
   }
   if (!m_outputStreamingTool.empty() && m_outputStreamingTool[0]->isClient()
	   && (!m_parallelCompression || outputConnectionSpec.find("[PoolContainerPrefix=" + m_metadataContainerProp.value() + "]") != std::string::npos)) {
      return(StatusCode::SUCCESS);
   }
   if (!m_outputStreamingTool.empty() && !m_outputStreamingTool[0]->isClient()) {
      if (m_parallelCompression && outputConnectionSpec.find("[PoolContainerPrefix=" + m_metadataContainerProp.value() + "]") == std::string::npos) {
         ATH_MSG_DEBUG("connectOutput SKIPPED for metadata-only server: " << outputConnectionSpec);
         return(StatusCode::SUCCESS);
      }
      if (!m_parallelCompression && (m_streamServer == m_outputStreamingTool.size() || !m_outputStreamingTool[m_streamServer < m_outputStreamingTool.size() ? m_streamServer : 0]->isServer())) {
         ATH_MSG_DEBUG("connectOutput SKIPPED for expired server.");
         return(StatusCode::SUCCESS);
      }
      auto it = std::find(m_streamClientFiles.begin(), m_streamClientFiles.end(), outputConnection);
      if (it == m_streamClientFiles.end()) {
         m_streamClientFiles.push_back(outputConnection);
      }
   }

   if (!m_outputStreamingTool.empty() && m_outputStreamingTool[0]->isClient() && m_parallelCompression) {
      outputConnection +=  m_streamPortString.value();
   }
   unsigned int contextId = outputContextId(outputConnection);
   try {
      if (!m_poolSvc->connect(pool::ITransaction::UPDATE, contextId).isSuccess()) {
         ATH_MSG_ERROR("connectOutput FAILED to open an UPDATE transaction.");
         return(StatusCode::FAILURE);
      }
   } catch (std::exception& e) {
      ATH_MSG_ERROR("connectOutput - caught exception: " << e.what());
      return(StatusCode::FAILURE);
   }

   std::unique_lock<std::mutex> lock(m_mutex);
   if (std::find(m_contextAttr.begin(), m_contextAttr.end(), contextId) == m_contextAttr.end()) {
      std::size_t merge = outputConnection.find(m_streamPortString.value()); // Used to remove trailing TMemFile
      int flush = m_numberEventsPerWrite.value();
      m_contextAttr.push_back(contextId);
      // Setting default 'TREE_MAX_SIZE' for ROOT to 1024 GB to avoid file chains.
      std::vector<std::string> maxFileSize;
      maxFileSize.push_back("TREE_MAX_SIZE");
      maxFileSize.push_back("1099511627776L");
      m_domainAttr.emplace_back(std::move(maxFileSize));
      // Extracting OUTPUT POOL ItechnologySpecificAttributes for Domain, Database and Container.
      extractPoolAttributes(m_poolAttr, &m_containerAttr, &m_databaseAttr, &m_domainAttr);
      for (std::vector<std::vector<std::string> >::iterator iter = m_databaseAttr.begin(), last = m_databaseAttr.end();
                      iter != last; ++iter) {
         const std::string& opt = (*iter)[0];
         std::string& data = (*iter)[1];
         const std::string& file = (*iter)[2];
         const std::string& cont = (*iter)[3];
         std::size_t equal = cont.find('='); // Used to remove leading "TTree="
         if (equal == std::string::npos) equal = 0;
         else equal++;
         std::size_t colon = m_containerPrefixProp.value().find(':');
         if (colon == std::string::npos) colon = 0; // Used to remove leading technology
         else colon++;
         const auto& strProp = m_containerPrefixProp.value();
         if (merge != std::string::npos && opt == "TREE_AUTO_FLUSH" && 0 == outputConnection.compare(0, merge, file) && cont.compare(equal, std::string::npos, strProp, colon) == 0 && data != "int" && data != "DbLonglong" && data != "double" && data != "string") {
            flush = atoi(data.c_str());
            if (flush < 0 && m_numberEventsPerWrite.value() > 0) {
               flush = m_numberEventsPerWrite.value();
               std::ostringstream eventAutoFlush;
               eventAutoFlush << flush;
               data = eventAutoFlush.str();
            } else if (flush > 0 && flush < m_numberEventsPerWrite.value()) {
               flush = flush * (int((m_numberEventsPerWrite.value()) / flush - 0.5) + 1);
            }
         }
      }
      if (merge != std::string::npos) {
         ATH_MSG_INFO("connectOutput setting auto write for: " << outputConnection << " to " << flush << " events");
         m_fileFlushSetting[outputConnection.substr(0, merge)] = flush;
      }
   }
   if (!processPoolAttributes(m_domainAttr, outputConnection, contextId).isSuccess()) {
      ATH_MSG_DEBUG("connectOutput failed process POOL domain attributes.");
   }
   if (!processPoolAttributes(m_databaseAttr, outputConnection, contextId).isSuccess()) {
      ATH_MSG_DEBUG("connectOutput failed process POOL database attributes.");
   }
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::commitOutput(const std::string& outputConnectionSpec, bool doCommit) {
// This is called after all DataObjects are converted.
   std::string outputConnection = outputConnectionSpec.substr(0, outputConnectionSpec.find('['));
   if (!m_outputStreamingTool.empty() && m_outputStreamingTool[0]->isClient()
	   && (!m_parallelCompression || outputConnectionSpec.find("[PoolContainerPrefix=" + m_metadataContainerProp.value() + "]") != std::string::npos)) {
      auto it = std::find(m_streamClientFiles.begin(), m_streamClientFiles.end(), outputConnection);
      size_t streamClient = it - m_streamClientFiles.begin();
      if (streamClient == m_streamClientFiles.size()) {
         m_streamClientFiles.push_back(outputConnection);
      }
      if (streamClient >= m_outputStreamingTool.size()) {
         streamClient = 0;
      }
      m_outputStreamingTool[streamClient]->lockObject("wait").ignore();
      if (!this->cleanUp(outputConnection).isSuccess()) {
         ATH_MSG_ERROR("commitOutput FAILED to cleanup converters.");
         return(StatusCode::FAILURE);
      }
      return(StatusCode::SUCCESS);
   }
   if (!m_outputStreamingTool.empty() && !m_outputStreamingTool[0]->isClient() && m_metadataContainerProp.value().empty()
                  && m_streamServer == m_outputStreamingTool.size()) {
      ATH_MSG_DEBUG("commitOutput SKIPPED for expired server.");
      return(StatusCode::SUCCESS);
   }
   if (!m_outputStreamingTool.empty() && !m_outputStreamingTool[0]->isClient() && !m_outputStreamingTool[m_streamServer < m_outputStreamingTool.size() ? m_streamServer : 0]->isServer()) {
      ATH_MSG_DEBUG("commitOutput SKIPPED for uninitialized server: " << m_streamServer << ".");
      return(StatusCode::SUCCESS);
   }
   if (!m_outputStreamingTool.empty() && !m_outputStreamingTool[0]->isClient() && m_streamServer == m_outputStreamingTool.size()) {
      auto it = std::find(m_streamClientFiles.begin(), m_streamClientFiles.end(), outputConnection);
      if (it == m_streamClientFiles.end()) {
         ATH_MSG_DEBUG("commitOutput SKIPPED for unconnected file: " << outputConnection << ".");
         return(StatusCode::SUCCESS);
      }
   }
   std::map<void*, RootType> commitCache;
   std::string fileName;
   if (!m_outputStreamingTool.empty() && !m_outputStreamingTool[0]->isClient() && m_streamServer < m_outputStreamingTool.size()
		   && m_outputStreamingTool[m_streamServer]->isServer()) {
      auto& streamingTool = m_outputStreamingTool[m_streamServer];
      // Clear object to get Placements for all objects in a Stream
      const char* placementStr = nullptr;
      int num = -1;
      StatusCode sc = streamingTool->clearObject(&placementStr, num);
      if (sc.isSuccess() && placementStr != nullptr && strlen(placementStr) > 6 && num > 0) {
         fileName = strstr(placementStr, "[FILE=");
         fileName = fileName.substr(6, fileName.find(']') - 6);
         if (!this->connectOutput(fileName).isSuccess()) {
            ATH_MSG_ERROR("Failed to connectOutput for " << fileName);
            return abortSharedWrClients(num);
         }
         IConverter* DHcnv = converter(ClassID_traits<DataHeader>::ID());
         bool dataHeaderSeen = false;
         std::string dataHeaderID;
         while (num > 0) {
            std::string objName = "ALL";
            if (m_useDetailChronoStat.value()) {
               std::string objName(placementStr); //FIXME, better descriptor
            }
            // StopWatch listens from here until the end of this current scope
            {
               PMonUtils::BasicStopWatch stopWatch("cRep_" + objName, m_chronoMap);
               std::string tokenStr = placementStr;
               std::string contName = strstr(placementStr, "[CONT=");
               tokenStr.erase(tokenStr.find("[CONT=")); //throws if [CONT= not found
               tokenStr.append(contName, contName.find(']') + 1);
               contName = contName.substr(6, contName.find(']') - 6);
               std::string className = strstr(placementStr, "[PNAME=");
               className = className.substr(7, className.find(']') - 7);
               RootType classDesc = RootType::ByNameNoQuiet(className);
               void* obj = nullptr;
               std::ostringstream oss2;
               oss2 << std::dec << num;
               std::string::size_type len = m_metadataContainerProp.value().size();
               bool foundContainer = false;
               std::size_t pPos = contName.find('(');
               if (contName.compare(0, pPos, m_metadataContainerProp.value()) == 0) {
                  foundContainer = true;
               }
               else {
                  for (const auto& item: m_metadataContainersAug.value()) {
                     if (contName.compare(0, pPos, item) == 0){
                        foundContainer = true;
                        len = item.size();
                        break;
                     }
                  }
               }
               if (len > 0 && foundContainer && contName[len] == '(' ) {
                  ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", name());
                  // For Metadata, before moving to next client, fire file incidents
                  if (m_metadataClient != num) {
                     if (m_metadataClient != 0) {
                        std::ostringstream oss1;
                        oss1 << std::dec << m_metadataClient;
                        std::string memName = "SHM[NUM=" + oss1.str() + "]";
                        FileIncident beginInputIncident(name(), "BeginInputFile", memName);
                        incSvc->fireIncident(beginInputIncident);
                        FileIncident endInputIncident(name(), "EndInputFile", memName);
                        incSvc->fireIncident(endInputIncident);
                     }
                     m_metadataClient = num;
                  }
                  // Retrieve MetaDataSvc
                  ServiceHandle<IMetaDataSvc> metadataSvc("MetaDataSvc", name());
                  ATH_CHECK(metadataSvc.retrieve());
                  sc = metadataSvc->shmProxy(std::string(placementStr) + "[NUM=" + oss2.str() + "]");
                  if (sc.isRecoverable()) {
                     ATH_MSG_WARNING("MetaDataSvc::shmProxy() no proxy added.");
                  } else if (sc.isFailure()) {
                     ATH_MSG_FATAL("MetaDataSvc::shmProxy() failed!");
                     return abortSharedWrClients(num);
                  }
               } else {
                  Token readToken;
                  readToken.setOid(Token::OID_t(num, 0));
                  readToken.setAuxString("[PNAME=" + className + "]");
                  this->setObjPtr(obj, &readToken); // Pull/read Object out of shared memory
                  if (len == 0 || contName.compare(0, len, m_metadataContainerProp.value()) != 0) {
                     // Write object
                     Placement placement;
                     placement.fromString(placementStr); placementStr = nullptr;
                     std::unique_ptr<Token> token(registerForWrite(&placement, obj, classDesc));
                     if (token == nullptr) {
                        ATH_MSG_ERROR("Failed to write Data for: " << className);
                        return abortSharedWrClients(num);
                     }
                     tokenStr = token->toString();
                     if (className == "DataHeader_p6") {
                        // Found DataHeader
                        GenericAddress address(POOL_StorageType, ClassID_traits<DataHeader>::ID(),
                                               tokenStr, placement.auxString());
                        // call DH converter to add the ref to DHForm (stored earlier) and to itself
                        if (!DHcnv->updateRep(&address, static_cast<DataObject*>(obj)).isSuccess()) {
                           ATH_MSG_ERROR("Failed updateRep for obj = " << tokenStr);
                           return abortSharedWrClients(num);
                        }
                        dataHeaderSeen = true;
                        // This dataHeaderID is used in DataHeaderCnv to index the DataHeaderForm cache.
                        // It must be unique per worker per stream so that we have a correct DataHeader(Form) association.
                        // This is achieved by building it as "CONTID/WORKERID/DBID".
                        // CONTID, e.g., POOLContainer(DataHeader), allows us to distinguish data and metadata headers,
                        // WORKERID allows us to distinguish AthenaMP workers,
                        // and DBID allows us to distinguish streams.
                        dataHeaderID = token->contID();
                        dataHeaderID += '/';
                        dataHeaderID += oss2.str();
                        dataHeaderID += '/';
                        dataHeaderID += token->dbID().toString();
                     } else if (dataHeaderSeen) {
                        dataHeaderSeen = false;
                        // next object after DataHeader - may be a DataHeaderForm
                        // in any case we need to call the DH converter to update the DHForm Ref
                        if (className == "DataHeaderForm_p6") {
                           // Tell DataHeaderCnv that it should use a new DHForm
                           GenericAddress address(POOL_StorageType, ClassID_traits<DataHeader>::ID(),
                                                  tokenStr, dataHeaderID);
                           if (!DHcnv->updateRepRefs(&address, static_cast<DataObject*>(obj)).isSuccess()) {
                              ATH_MSG_ERROR("Failed updateRepRefs for obj = " << tokenStr);
                              return abortSharedWrClients(num);
                           }
                        } else {
                           // Tell DataHeaderCnv that it should use the old DHForm
                           GenericAddress address(0, 0, "", dataHeaderID);
                           if (!DHcnv->updateRepRefs(&address, nullptr).isSuccess()) {
                              ATH_MSG_ERROR("Failed updateRepRefs for DataHeader");
                              return abortSharedWrClients(num);
                           }
                        }
                     }
                     if (className != "Token" && className != "DataHeaderForm_p6" && !classDesc.IsFundamental()) {
                        commitCache.insert(std::pair<void*, RootType>(obj, classDesc));
                     }
                  }
               }
               // Send Token back to Client
               sc = streamingTool->lockObject(tokenStr.c_str(), num);
               while (sc.isRecoverable()) {
                  sc = streamingTool->lockObject(tokenStr.c_str(), num);
               }
               if (!sc.isSuccess()) {
                  ATH_MSG_ERROR("Failed to lock Data for " << tokenStr);
                  return abortSharedWrClients(-1);
               }
            }
            sc = streamingTool->clearObject(&placementStr, num);
            while (sc.isRecoverable()) {
               sc = streamingTool->clearObject(&placementStr, num);
            }
            if (sc.isFailure()) {
               // no more clients, break the loop and exit
               num = -1;
            }
         }
         if (dataHeaderSeen) {
            // DataHeader was the last object, need to tell the converter there is no DHForm coming
            GenericAddress address(0, 0, "", dataHeaderID);
            if (!DHcnv->updateRepRefs(&address, nullptr).isSuccess()) {
               ATH_MSG_ERROR("Failed updateRepRefs for DataHeader");
               return abortSharedWrClients(-1);
            }
         }
         placementStr = nullptr;
      } else if (sc.isSuccess() && placementStr != nullptr && strncmp(placementStr, "stop", 4) == 0) {
         return(StatusCode::RECOVERABLE);
      } else if (sc.isRecoverable() || num == -1) {
         return(StatusCode::RECOVERABLE);
      }
      if (sc.isFailure() || fileName.empty()) {
         ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", name());
         std::ostringstream oss1;
         oss1 << std::dec << m_metadataClient;
         std::string memName = "SHM[NUM=" + oss1.str() + "]";
         FileIncident beginInputIncident(name(), "BeginInputFile", memName);
         incSvc->fireIncident(beginInputIncident);
         FileIncident endInputIncident(name(), "EndInputFile", memName);
         incSvc->fireIncident(endInputIncident);
         if (sc.isFailure()) {
            ATH_MSG_INFO("All SharedWriter clients stopped - exiting");
         } else {
            ATH_MSG_INFO("Failed to get Data for client: " << num);
         }
         return(StatusCode::FAILURE);
      }
   }
   if (m_parallelCompression && !fileName.empty()) {
      ATH_MSG_DEBUG("commitOutput SKIPPED for metadata-only server: " << outputConnectionSpec);
      return(StatusCode::SUCCESS);
   }
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatch("commitOutput", m_chronoMap);
   std::unique_lock<std::mutex> lock(m_mutex);
   if (outputConnection.empty()) {
      outputConnection = fileName;
   }
   // Extract the technology
   int tech = m_dbType.type();
   if (!decodeOutputSpec(outputConnection, tech).isSuccess()) {
      ATH_MSG_ERROR("connectOutput FAILED extract file name and technology.");
      return(StatusCode::FAILURE);
   }
   const std::string oldOutputConnection = outputConnection;
   if (!m_outputStreamingTool.empty() && m_outputStreamingTool[0]->isClient() && m_parallelCompression) {
      m_fileCommitCounter[outputConnection] = m_fileCommitCounter[outputConnection] + 1;
      if (m_fileFlushSetting[outputConnection] > 0 && m_fileCommitCounter[outputConnection] % m_fileFlushSetting[outputConnection] == 0) {
         doCommit = true;
         ATH_MSG_DEBUG("commitOutput sending data.");
      }
      outputConnection += m_streamPortString.value();
   }
   unsigned int contextId = outputContextId(outputConnection);
   if (!processPoolAttributes(m_domainAttr, outputConnection, contextId).isSuccess()) {
      ATH_MSG_DEBUG("commitOutput failed process POOL domain attributes.");
   }
   if (!processPoolAttributes(m_databaseAttr, outputConnection, contextId).isSuccess()) {
      ATH_MSG_DEBUG("commitOutput failed process POOL database attributes.");
   }
   if (!processPoolAttributes(m_containerAttr, outputConnection, contextId).isSuccess()) {
      ATH_MSG_DEBUG("commitOutput failed process POOL container attributes.");
   }
   // lock.unlock();  //MN: first need to make commitCache slot-specific
   try {
      if (doCommit) {
         if (!m_poolSvc->commit(contextId).isSuccess()) {
            ATH_MSG_ERROR("commitOutput FAILED to commit OutputStream.");
            return(StatusCode::FAILURE);
         }
      } else {
         if (!m_poolSvc->commitAndHold(contextId).isSuccess()) {
            ATH_MSG_ERROR("commitOutput FAILED to commitAndHold OutputStream.");
            return(StatusCode::FAILURE);
         }
      }
   } catch (std::exception& e) {
      ATH_MSG_ERROR("commitOutput - caught exception: " << e.what());
      return(StatusCode::FAILURE);
   }
   if (!this->cleanUp(oldOutputConnection).isSuccess()) {
      ATH_MSG_ERROR("commitOutput FAILED to cleanup converters.");
      return(StatusCode::FAILURE);
   }
   for (std::map<void*, RootType>::iterator iter = commitCache.begin(), last = commitCache.end(); iter != last; ++iter) {
      iter->second.Destruct(iter->first);
   }
   // Check FileSize
   long long int currentFileSize = m_poolSvc->getFileSize(outputConnection, m_dbType.type(), contextId);
   if (m_databaseMaxFileSize.find(outputConnection) != m_databaseMaxFileSize.end()) {
      if (currentFileSize > m_databaseMaxFileSize[outputConnection]) {
         ATH_MSG_WARNING("FileSize > " << m_databaseMaxFileSize[outputConnection] << " for " << outputConnection);
         return(StatusCode::RECOVERABLE);
      }
   } else if (currentFileSize > m_domainMaxFileSize) {
      ATH_MSG_WARNING("FileSize > " << m_domainMaxFileSize <<  " for " << outputConnection);
      return(StatusCode::RECOVERABLE);
   }
   return(StatusCode::SUCCESS);
}

//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::disconnectOutput(const std::string& outputConnectionSpec) {
   std::string outputConnection = outputConnectionSpec.substr(0, outputConnectionSpec.find('['));
   if (!m_outputStreamingTool.empty() && m_outputStreamingTool[0]->isClient()
	   && (!m_parallelCompression || outputConnectionSpec.find("[PoolContainerPrefix=" + m_metadataContainerProp.value() + "]") != std::string::npos)) {
      return(StatusCode::SUCCESS);
   }
   if (!m_outputStreamingTool.empty() && !m_outputStreamingTool[0]->isClient()) {
      if (m_metadataContainerProp.value().empty() && m_streamServer == m_outputStreamingTool.size()) {
         ATH_MSG_DEBUG("disconnectOutput SKIPPED for expired server.");
         return(StatusCode::SUCCESS);
      } else if (!m_metadataContainerProp.value().empty() && m_streamServer < m_outputStreamingTool.size()) {
         m_streamServer = m_outputStreamingTool.size();
         ATH_MSG_DEBUG("disconnectOutput SKIPPED to expire server.");
         return(StatusCode::SUCCESS);
      } else {
         m_streamServer = m_outputStreamingTool.size();
      }
      ATH_MSG_DEBUG("disconnectOutput not SKIPPED for server: " << m_streamServer);
   }
   if (!m_outputStreamingTool.empty() && m_outputStreamingTool[0]->isClient() && m_parallelCompression) {
      outputConnection += m_streamPortString.value();
   }
   unsigned int contextId = outputContextId(outputConnection);
   StatusCode sc = m_poolSvc->disconnect(contextId);
   return sc;
}

//______________________________________________________________________________
unsigned int AthenaPoolCnvSvc::outputContextId(const std::string& outputConnection) {
   return m_persSvcPerOutput?
      m_poolSvc->getOutputContext(outputConnection) : (unsigned int)IPoolSvc::kOutputStream;
}

//______________________________________________________________________________
IPoolSvc* AthenaPoolCnvSvc::getPoolSvc() {
   return(&*m_poolSvc);
}
//______________________________________________________________________________
Token* AthenaPoolCnvSvc::registerForWrite(Placement* placement, const void* obj, const RootType& classDesc) {
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatch("cRepR_ALL", m_chronoMap);
   if (m_makeStreamingToolClient.value() > 0 && !m_outputStreamingTool.empty() && !m_outputStreamingTool[0]->isServer() && !m_outputStreamingTool[0]->isClient()) {
      if (!makeClient(m_makeStreamingToolClient.value()).isSuccess()) {
         ATH_MSG_ERROR("Could not make AthenaPoolCnvSvc a Share Client");
         return(nullptr);
      }
   }
   Token* token = nullptr;
   if (!m_outputStreamingTool.empty() && m_outputStreamingTool[0]->isClient()
	   && (!m_parallelCompression || placement->containerName().compare(0, m_metadataContainerProp.value().size(), m_metadataContainerProp.value()) == 0)) {
      const std::string &fileName = placement->fileName();
      auto it = std::find(m_streamClientFiles.begin(), m_streamClientFiles.end(), fileName);
      size_t streamClient = it - m_streamClientFiles.begin();
      if (streamClient == m_streamClientFiles.size()) {
         m_streamClientFiles.push_back(fileName);
      }
      if (streamClient >= m_outputStreamingTool.size()) {
         streamClient = 0;
      }
      // Lock object
      std::string placementStr = placement->toString();
      placementStr += "[PNAME=";
      placementStr += classDesc.Name();
      placementStr += ']';
      ATH_MSG_VERBOSE("Requesting write object for: " << placementStr);
      StatusCode sc = m_outputStreamingTool[streamClient]->lockObject(placementStr.c_str());
      while (sc.isRecoverable()) {
         //usleep(100);
         sc = m_outputStreamingTool[streamClient]->lockObject(placementStr.c_str());
      }
      if (!sc.isSuccess()) {
         ATH_MSG_ERROR("Failed to lock Data for " << placementStr);
         return(nullptr);
      }
      // Serialize object via ROOT
      const void* buffer = nullptr;
      std::size_t nbytes = 0;
      bool own = true;
      if (classDesc.Name() == "Token") {
         nbytes = strlen(static_cast<const char*>(obj)) + 1;
         buffer = obj;
         own = false;
      } else if (classDesc.IsFundamental()) {
         nbytes = classDesc.SizeOf();
         buffer = obj;
         own = false;
      } else {
         buffer = m_serializeSvc->serialize(obj, classDesc, nbytes);
      }
      // Share object
      sc = m_outputStreamingTool[streamClient]->putObject(buffer, nbytes);
      while (sc.isRecoverable()) {
         //usleep(100);
         sc = m_outputStreamingTool[streamClient]->putObject(buffer, nbytes);
      }
      if (own) { delete [] static_cast<const char*>(buffer); }
      buffer = nullptr;
      if (!sc.isSuccess()) {
         ATH_MSG_ERROR("Could not share object for: " << placementStr);
         m_outputStreamingTool[streamClient]->putObject(nullptr, 0).ignore();
         return(nullptr);
      }
      AuxDiscoverySvc auxDiscover;
      if (!auxDiscover.sendStore(m_serializeSvc.get(), m_outputStreamingTool[streamClient].get(), obj, pool::DbReflex::guid(classDesc), placement->containerName()).isSuccess()) {
         ATH_MSG_ERROR("Could not share dynamic aux store for: " << placementStr);
         m_outputStreamingTool[streamClient]->putObject(nullptr, 0).ignore();
         return(nullptr);
      }
      if (!m_outputStreamingTool[streamClient]->putObject(nullptr, 0).isSuccess()) {
         ATH_MSG_ERROR("Failed to put Data for " << placementStr);
         return(nullptr);
      }
      // Get Token back from Server
      const char* tokenStr = nullptr;
      int num = -1;
      sc = m_outputStreamingTool[streamClient]->clearObject(&tokenStr, num);
      while (sc.isRecoverable()) {
         //usleep(100);
         sc = m_outputStreamingTool[streamClient]->clearObject(&tokenStr, num);
      }
      if (!sc.isSuccess()) {
         ATH_MSG_ERROR("Failed to get Token");
         return(nullptr);
      }
      if (!strcmp(tokenStr, "ABORT")) {
         ATH_MSG_ERROR("Writer requested ABORT");
         // tell the server we are leaving
         m_outputStreamingTool[streamClient]->stop().ignore();
         return nullptr;
      }
      Token* tempToken = new Token();
      tempToken->fromString(tokenStr); tokenStr = nullptr;
      tempToken->setClassID(pool::DbReflex::guid(classDesc));
      token = tempToken; tempToken = nullptr;
// Client Write Request
   } else {
      if (!m_outputStreamingTool.empty() && !m_outputStreamingTool[0]->isClient() && m_metadataContainerProp.value().empty()
		      && (m_streamServer == m_outputStreamingTool.size() || !m_outputStreamingTool[m_streamServer < m_outputStreamingTool.size() ? m_streamServer : 0]->isServer())) {
         ATH_MSG_DEBUG("registerForWrite SKIPPED for expired server, Placement = " << placement->toString());
         Token* tempToken = new Token();
         tempToken->setClassID(pool::DbReflex::guid(classDesc));
         token = tempToken; tempToken = nullptr;
      } else if (!m_outputStreamingTool.empty() && !m_outputStreamingTool[0]->isClient() && m_streamServer != m_outputStreamingTool.size() && !m_outputStreamingTool[m_streamServer < m_outputStreamingTool.size() ? m_streamServer : 0]->isServer()) {
         ATH_MSG_DEBUG("registerForWrite SKIPPED for uninitialized server, Placement = " << placement->toString());
         Token* tempToken = new Token();
         tempToken->setClassID(pool::DbReflex::guid(classDesc));
         token = tempToken; tempToken = nullptr;
      } else if (!m_outputStreamingTool.empty() && !m_outputStreamingTool[0]->isClient() && m_streamServer == m_outputStreamingTool.size()) {
         const std::string &fileName = placement->fileName();
         auto it = std::find(m_streamClientFiles.begin(), m_streamClientFiles.end(), fileName);
         if (it == m_streamClientFiles.end()) {
            ATH_MSG_DEBUG("registerForWrite SKIPPED for unconnected file: " << fileName << ".");
            Token* tempToken = new Token();
            tempToken->setClassID(pool::DbReflex::guid(classDesc));
            token = tempToken; tempToken = nullptr;
         } else {
            ATH_MSG_VERBOSE("Requested write object for: " << placement->toString());
            token = m_poolSvc->registerForWrite(placement, obj, classDesc);
         }
      } else {
         if (!m_outputStreamingTool.empty() && m_outputStreamingTool[0]->isClient() && m_parallelCompression) {
            placement->setFileName(placement->fileName() + m_streamPortString.value());
         }
         if (m_persSvcPerOutput) { // Use separate PersistencySvc for each output stream/file
            char text[32];
            ::sprintf(text, "[CTXT=%08X]", m_poolSvc->getOutputContext(placement->fileName()));
            placement->setAuxString(text);
         }
         token = m_poolSvc->registerForWrite(placement, obj, classDesc);
      }
   }
   return(token);
}
//______________________________________________________________________________
void AthenaPoolCnvSvc::setObjPtr(void*& obj, const Token* token) {
   ATH_MSG_VERBOSE("Requesting object for: " << token->toString());
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatchOuter("cObjR_ALL", m_chronoMap);
   if (m_makeStreamingToolClient.value() > 0 && !m_inputStreamingTool.empty() && !m_inputStreamingTool->isServer() && !m_inputStreamingTool->isClient()) {
      if (!makeClient(-m_makeStreamingToolClient.value()).isSuccess()) {
         ATH_MSG_ERROR("Could not make AthenaPoolCnvSvc a Share Client");
      }
   }
   if (!m_outputStreamingTool.empty() && m_streamServer < m_outputStreamingTool.size()
		   && m_outputStreamingTool[m_streamServer]->isServer()) {
      if (token->dbID() == Guid::null()) {
         int num = token->oid().first;
         // Get object from SHM
         void* buffer = nullptr;
         std::size_t nbytes = 0;
         StatusCode sc = m_outputStreamingTool[m_streamServer]->getObject(&buffer, nbytes, num);
         while (sc.isRecoverable()) {
            //usleep(100);
            sc = m_outputStreamingTool[m_streamServer]->getObject(&buffer, nbytes, num);
         }
         if (!sc.isSuccess()) {
            ATH_MSG_ERROR("Failed to get Data for " << token->toString());
            obj = nullptr;
         } else {
            if (token->classID() != Guid::null()) {
               // Deserialize object
               RootType cltype(pool::DbReflex::forGuid(token->classID()));
               obj = m_serializeSvc->deserialize(buffer, nbytes, cltype); buffer = nullptr;
            } else {
               // Deserialize object
               std::string className = token->auxString();
               className = className.substr(className.find("[PNAME="));
               className = className.substr(7, className.find(']') - 7);
               RootType cltype(RootType::ByNameNoQuiet(className));
               obj = m_serializeSvc->deserialize(buffer, nbytes, cltype); buffer = nullptr;
            }
            AuxDiscoverySvc auxDiscover;
            if (!auxDiscover.receiveStore(m_serializeSvc.get(), m_outputStreamingTool[m_streamServer].get(), obj, num).isSuccess()) {
               ATH_MSG_ERROR("Failed to get Dynamic Aux Store for " << token->toString());
               obj = nullptr;
            }
         }
      }
   }
   if (!m_inputStreamingTool.empty() && m_inputStreamingTool->isClient() && (m_streamingTechnology.value() < 0 || token->technology() == m_streamingTechnology.value())) {
      ATH_MSG_VERBOSE("Requesting remote object for: " << token->toString());
      if (!m_inputStreamingTool->lockObject(token->toString().c_str()).isSuccess()) {
         ATH_MSG_ERROR("Failed to lock Data for " << token->toString());
         obj = nullptr;
      } else {
         void* buffer = nullptr;
         std::size_t nbytes = 0;
         StatusCode sc = StatusCode::FAILURE;
         // StopWatch listens from here until the end of this current scope
         {
            PMonUtils::BasicStopWatch stopWatchInner("gObj_ALL", m_chronoMap);
            sc = m_inputStreamingTool->getObject(&buffer, nbytes);
            while (sc.isRecoverable()) {
               // sleep
               sc = m_inputStreamingTool->getObject(&buffer, nbytes);
            }
         }
         if (!sc.isSuccess()) {
            ATH_MSG_ERROR("Failed to get Data for " << token->toString());
            obj = nullptr;
         } else {
            obj = m_serializeSvc->deserialize(buffer, nbytes, token->classID()); buffer = nullptr;
            AuxDiscoverySvc auxDiscover;
            if (!auxDiscover.receiveStore(m_serializeSvc.get(), m_inputStreamingTool.get(), obj).isSuccess()) {
               ATH_MSG_ERROR("Failed to get Dynamic Aux Store for " << token->toString());
               obj = nullptr;
            }
         }
      }
   } else if (token->dbID() != Guid::null()) {
      ATH_MSG_VERBOSE("Requesting object for: " << token->toString());
      m_poolSvc->setObjPtr(obj, token);
   }
}
//______________________________________________________________________________
bool AthenaPoolCnvSvc::useDetailChronoStat() const {
   return(m_useDetailChronoStat.value());
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::createAddress(long svcType,
		const CLID& clid,
		const std::string* par,
		const unsigned long* ip,
		IOpaqueAddress*& refpAddress) {
   if (svcType != POOL_StorageType) {
      ATH_MSG_ERROR("createAddress: svcType != POOL_StorageType " << svcType << " " << POOL_StorageType);
      return(StatusCode::FAILURE);
   }
   if (m_makeStreamingToolClient.value() > 0 && !m_inputStreamingTool.empty() && !m_inputStreamingTool->isServer() && !m_inputStreamingTool->isClient()) {
      if (!makeClient(-m_makeStreamingToolClient.value()).isSuccess()) {
         ATH_MSG_ERROR("Could not make AthenaPoolCnvSvc a Share Client");
         return(StatusCode::FAILURE);
      }
   }
   Token* token = nullptr;
   if (par[0].compare(0, 3, "SHM") == 0) {
      token = new Token();
      token->setOid(Token::OID_t(ip[0], ip[1]));
      token->setAuxString("[PNAME=" + par[2] + "]");
      RootType classDesc = RootType::ByNameNoQuiet(par[2]);
      token->setClassID(pool::DbReflex::guid(classDesc));
   } else if (!m_inputStreamingTool.empty() && m_inputStreamingTool->isClient()) {
      Token addressToken;
      addressToken.setDb(par[0].substr(4));
      addressToken.setCont(par[1]);
      addressToken.setOid(Token::OID_t(ip[0], ip[1]));
      if (!m_inputStreamingTool->lockObject(addressToken.toString().c_str()).isSuccess()) {
         ATH_MSG_WARNING("Failed to lock Address Token: " << addressToken.toString());
         return(StatusCode::FAILURE);
      }
      void* buffer = nullptr;
      std::size_t nbytes = 0;
      StatusCode sc = m_inputStreamingTool->getObject(&buffer, nbytes);
      while (sc.isRecoverable()) {
         // sleep
         sc = m_inputStreamingTool->getObject(&buffer, nbytes);
      }
      if (!sc.isSuccess()) {
         ATH_MSG_WARNING("Failed to get Address Token: " << addressToken.toString());
         return(StatusCode::FAILURE);
      }
      token = new Token();
      token->fromString(static_cast<const char*>(buffer)); buffer = nullptr;
      if (token->classID() == Guid::null()) {
         delete token; token = nullptr;
      }
      m_inputStreamingTool->getObject(&buffer, nbytes).ignore();
   } else {
      token = m_poolSvc->getToken(par[0], par[1], ip[0]);
   }
   if (token == nullptr) {
      return(StatusCode::RECOVERABLE);
   }
   refpAddress = new TokenAddress(POOL_StorageType, clid, "", par[1], IPoolSvc::kInputStream, token);
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::createAddress(long svcType,
		const CLID& clid,
		const std::string& refAddress,
		IOpaqueAddress*& refpAddress) {
   if (svcType != POOL_StorageType) {
      ATH_MSG_ERROR("createAddress: svcType != POOL_StorageType " << svcType << " " << POOL_StorageType);
      return(StatusCode::FAILURE);
   }
   refpAddress = new GenericAddress(POOL_StorageType, clid, refAddress);
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::convertAddress(const IOpaqueAddress* pAddress,
		std::string& refAddress) {
   assert(pAddress);
   const TokenAddress* tokAddr = dynamic_cast<const TokenAddress*>(pAddress);
   if (tokAddr != nullptr && tokAddr->getToken() != nullptr) {
      refAddress = tokAddr->getToken()->toString();
   } else {
      refAddress = *pAddress->par();
   }
   return(StatusCode::SUCCESS);
}
//__________________________________________________________________________
StatusCode
AthenaPoolCnvSvc::decodeOutputSpec(std::string& fileSpec, int& outputTech) const
{
  if (boost::starts_with (fileSpec, "oracle") || boost::starts_with (fileSpec, "mysql")) {
      outputTech = pool::POOL_RDBMS_StorageType.type();
   } else if (boost::starts_with (fileSpec, "ROOTKEY:")) {
      outputTech = pool::ROOTKEY_StorageType.type();
      fileSpec.erase(0, 8);
   } else if (boost::starts_with (fileSpec, "ROOTTREE:")) {
      outputTech = pool::ROOTTREE_StorageType.type();
      fileSpec.erase(0, 9);
   } else if (boost::starts_with (fileSpec, "ROOTTREEINDEX:")) {
      outputTech = pool::ROOTTREEINDEX_StorageType.type();
      fileSpec.erase(0, 14);
   } else if (outputTech == 0) {
      outputTech = m_dbType.type();
   }
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::registerCleanUp(IAthenaPoolCleanUp* cnv) {
   m_cnvs.push_back(cnv);
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::cleanUp(const std::string& connection) {
   bool retError = false;
   std::size_t cpos = connection.find(':');
   std::size_t bpos = connection.find('[');
   if (cpos == std::string::npos) {
      cpos = 0;
   } else {
      cpos++;
   }
   if (bpos != std::string::npos) bpos = bpos - cpos;
   const std::string conn = connection.substr(cpos, bpos);
   ATH_MSG_VERBOSE("Cleanup for Connection='"<< conn <<"'");
   for (auto converter : m_cnvs) {
      if (!converter->cleanUp(conn).isSuccess()) {
         ATH_MSG_WARNING("AthenaPoolConverter cleanUp failed.");
         retError = true;
      }
   }
   return(retError ? StatusCode::FAILURE : StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::setInputAttributes(const std::string& fileName) {
   // Set attributes for input file
   m_lastInputFileName = fileName; // Save file name for printing attributes per event
   if (!processPoolAttributes(m_inputAttr, m_lastInputFileName, IPoolSvc::kInputStream, false, true, false).isSuccess()) {
      ATH_MSG_DEBUG("setInputAttribute failed setting POOL database/container attributes.");
   }
   if (!processPoolAttributes(m_inputAttr, m_lastInputFileName, IPoolSvc::kInputStream, true, false).isSuccess()) {
      ATH_MSG_DEBUG("setInputAttribute failed getting POOL database/container attributes.");
   }
   if (!m_persSvcPerInputType.empty()) {
      // Loop over all extra event input contexts and switch off TTreeCache
      const auto& extraInputContextMap = m_poolSvc->getInputContextMap();
      for (const auto& [label, id]: extraInputContextMap) {
         if (m_poolSvc->setAttribute("TREE_CACHE", "0", pool::DbType(pool::ROOTTREE_StorageType).type(), m_lastInputFileName, m_persSvcPerInputType.value(), id).isSuccess()) {
            ATH_MSG_DEBUG("setInputAttribute failed to switch off TTreeCache for = " << label << ".");
         }
      }
   }
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::makeServer(int num) {
   if (num < 0) {
      num = -num;
      m_streamServer = int(num / 1024);
      num = num % 1024;
      if (!m_outputStreamingTool.empty() && m_streamServer < m_outputStreamingTool.size()
		      && !m_outputStreamingTool[m_streamServer]->isServer()) {
         ATH_MSG_DEBUG("makeServer: " << m_outputStreamingTool << " = " << num);
         ATH_MSG_DEBUG("makeServer: Calling shared memory tool with port suffix " << m_streamPortString);
         const std::string streamPortSuffix = m_streamPortString.value();
         if (m_outputStreamingTool[m_streamServer]->makeServer(num, streamPortSuffix).isFailure()) {
            ATH_MSG_ERROR("makeServer: " << m_outputStreamingTool << " failed");
            return(StatusCode::FAILURE);
         }
         m_streamClientFiles.clear();
         // Disable PersistencySvc per output file mode, for SharedWriter Server
         m_persSvcPerOutput.setValue(false);
         return(StatusCode::SUCCESS);
      }
      return(StatusCode::RECOVERABLE);
   }
   if (m_inputStreamingTool.empty()) {
      return(StatusCode::RECOVERABLE);
   }
   ATH_MSG_DEBUG("makeServer: " << m_inputStreamingTool << " = " << num);
   return(m_inputStreamingTool->makeServer(num, ""));
}
//________________________________________________________________________________
StatusCode AthenaPoolCnvSvc::makeClient(int num) {
   if (!m_outputStreamingTool.empty()/* && !m_outputStreamingTool[0]->isClient() && num > 0*/) {
      ATH_MSG_DEBUG("makeClient: " << m_outputStreamingTool << " = " << num);
      for (std::size_t streamClient = 0; streamClient < m_outputStreamingTool.size(); streamClient++) {
         std::string streamPortSuffix;
         if (m_outputStreamingTool[streamClient]->makeClient(num, streamPortSuffix).isFailure()) {
            ATH_MSG_ERROR("makeClient: " << m_outputStreamingTool << ", " << streamClient << " failed");
            return(StatusCode::FAILURE);
         } else if (streamClient == 0 && m_streamPortString.value().find("localhost:0") != std::string::npos) {
            // We don't seem to use a dedicated port per stream so doing this for the first client is probably OK
            ATH_MSG_DEBUG("makeClient: Setting conversion service port suffix to " << streamPortSuffix);
            m_streamPortString.setValue(streamPortSuffix);
         }
      }
   }
   if (m_inputStreamingTool.empty()) {
      return(StatusCode::SUCCESS);
   }
   ATH_MSG_DEBUG("makeClient: " << m_inputStreamingTool << " = " << num);
   std::string dummyStr;
   return(m_inputStreamingTool->makeClient(num, dummyStr));
}
//________________________________________________________________________________
StatusCode AthenaPoolCnvSvc::readData() {
   if (m_inputStreamingTool.empty()) {
      return(StatusCode::FAILURE);
   }
   const char* tokenStr = nullptr;
   int num = -1;
   StatusCode sc = m_inputStreamingTool->clearObject(&tokenStr, num);
   if (sc.isSuccess() && tokenStr != nullptr && strlen(tokenStr) > 0 && num > 0) {
      ATH_MSG_DEBUG("readData: " << tokenStr << ", for client: " << num);
   } else {
      return(sc);
   }
   // Read object instance via POOL/ROOT
   void* instance = nullptr;
   Token token;
   token.fromString(tokenStr); tokenStr = nullptr;
   if (token.classID() != Guid::null()) {
      std::string objName = "ALL";
      if (m_useDetailChronoStat.value()) {
         objName = token.classID().toString();
      }
      // StopWatch listens from here until the end of this current scope
      PMonUtils::BasicStopWatch stopWatchInner("cObj_" + objName, m_chronoMap);
      this->setObjPtr(instance, &token);
      // Serialize object via ROOT
      RootType cltype(pool::DbReflex::forGuid(token.classID()));
      void* buffer = nullptr;
      std::size_t nbytes = 0;
      buffer = m_serializeSvc->serialize(instance, cltype, nbytes);
      sc = m_inputStreamingTool->putObject(buffer, nbytes, num);
      while (sc.isRecoverable()) {
         sc = m_inputStreamingTool->putObject(buffer, nbytes, num);
      }
      delete [] static_cast<char*>(buffer); buffer = nullptr;
      if (!sc.isSuccess()) {
         ATH_MSG_ERROR("Could not share object for: " << token.toString());
         return(StatusCode::FAILURE);
      }
      AuxDiscoverySvc auxDiscover;
      if (!auxDiscover.sendStore(m_serializeSvc.get(), m_inputStreamingTool.get(), instance, token.classID(), token.contID(), num).isSuccess()) {
         ATH_MSG_ERROR("Could not share dynamic aux store for: " << token.toString());
         return(StatusCode::FAILURE);
      }
      cltype.Destruct(instance); instance = nullptr;
      if (!m_inputStreamingTool->putObject(nullptr, 0, num).isSuccess()) {
         ATH_MSG_ERROR("Could not share object for: " << token.toString());
         return(StatusCode::FAILURE);
      }
   } else if (token.dbID() != Guid::null()) {
      std::string returnToken;
      const Token* metadataToken = m_poolSvc->getToken("FID:" + token.dbID().toString(), token.contID(), token.oid().first);
      if (metadataToken != nullptr) {
         returnToken = metadataToken->toString();
      } else {
         returnToken = token.toString();
      }
      delete metadataToken; metadataToken = nullptr;
      // Share token
      sc = m_inputStreamingTool->putObject(returnToken.c_str(), returnToken.size() + 1, num);
      if (!sc.isSuccess() || !m_inputStreamingTool->putObject(nullptr, 0, num).isSuccess()) {
         ATH_MSG_ERROR("Could not share token for: " << token.toString());
         return(StatusCode::FAILURE);
      }
   } else {
      return(StatusCode::RECOVERABLE);
   }
   return(StatusCode::SUCCESS);
}

//________________________________________________________________________________
StatusCode AthenaPoolCnvSvc::commitCatalog() {
   pool::IFileCatalog* catalog ATLAS_THREAD_SAFE =  // This is on the SharedWriter, after mother process finishes events
	   const_cast<pool::IFileCatalog*>(m_poolSvc->catalog());
   catalog->commit();
   catalog->start();
   return(StatusCode::SUCCESS);
}

//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::abortSharedWrClients(int client_n)
{
   ATH_MSG_ERROR("Sending ABORT to clients");
   // the master process will kill this process once workers abort
   // but it could be a time-limited loop
   auto streamingTool = m_outputStreamingTool[m_streamServer];
   StatusCode sc = StatusCode::SUCCESS;
   while (sc.isSuccess()) {
      if (client_n >= 0) {
         sc = streamingTool->lockObject("ABORT", client_n);
      }
      const char* dummy;
      sc = streamingTool->clearObject(&dummy, client_n);
      while (sc.isRecoverable()) {
         sc = streamingTool->clearObject(&dummy, client_n);
      }
   }
   return StatusCode::FAILURE;
}

//______________________________________________________________________________
void AthenaPoolCnvSvc::handle(const Incident& incident) {
   if (incident.type() == "StoreCleared" && m_outputStreamingTool[0]->isClient() && !m_parallelCompression) {
      m_outputStreamingTool[m_streamServer]->lockObject("release").ignore();
   }
   if (incident.type() == "EndEvent") {
      if (!processPoolAttributes(m_inputAttrPerEvent, m_lastInputFileName, IPoolSvc::kInputStream).isSuccess()) {
         ATH_MSG_DEBUG("handle EndEvent failed process POOL database attributes.");
      }
   }
}
//______________________________________________________________________________
AthenaPoolCnvSvc::AthenaPoolCnvSvc(const std::string& name, ISvcLocator* pSvcLocator) :
	::AthCnvSvc(name, pSvcLocator, POOL_StorageType),
	m_outputStreamingTool(this)
{
   declareProperty("OutputStreamingTool", m_outputStreamingTool);
}
//__________________________________________________________________________
void AthenaPoolCnvSvc::extractPoolAttributes(const StringArrayProperty& property,
		std::vector<std::vector<std::string> >* contAttr,
		std::vector<std::vector<std::string> >* dbAttr,
		std::vector<std::vector<std::string> >* domAttr) const {
   std::vector<std::string> opt;
   std::string attributeName, containerName, databaseName, valueString;
   for (std::vector<std::string>::const_iterator iter = property.value().begin(),
           last = property.value().end(); iter != last; ++iter) {
      opt.clear();
      attributeName.clear();
      containerName.clear();
      databaseName.clear();
      valueString.clear();
      using Gaudi::Utils::AttribStringParser;
      for (const AttribStringParser::Attrib& attrib : AttribStringParser (*iter)) {
         const std::string tag = attrib.tag;
         const std::string val = attrib.value;
         if (tag == "DatabaseName") {
            databaseName = val;
         } else if (tag == "ContainerName") {
            if (databaseName.empty()) {
               databaseName = "*";
            }
            containerName = val;
         } else {
            attributeName = tag;
            valueString = val;
         }
      }
      if (!attributeName.empty() && !valueString.empty()) {
         opt.push_back(attributeName);
         opt.push_back(valueString);
         if (!databaseName.empty()) {
            opt.push_back(databaseName);
            if (!containerName.empty()) {
               opt.push_back(containerName);
               if (containerName.compare(0, 6, "TTree=") == 0) {
                  dbAttr->push_back(opt);
               } else {
                  contAttr->push_back(opt);
               }
            } else {
               opt.push_back("");
               dbAttr->push_back(opt);
            }
         } else if (domAttr != 0) {
            domAttr->push_back(opt);
         } else {
            opt.push_back("*");
            opt.push_back("");
            dbAttr->push_back(opt);
         }
      }
   }
}
//__________________________________________________________________________
StatusCode AthenaPoolCnvSvc::processPoolAttributes(std::vector<std::vector<std::string> >& attr,
		const std::string& fileName,
		unsigned long contextId,
		bool doGet,
		bool doSet,
		bool doClear) const {
   bool retError = false;
   if (!m_inputStreamingTool.empty() && m_inputStreamingTool->isClient()) doGet = false;
   for (std::vector<std::vector<std::string> >::iterator iter = attr.begin(), last = attr.end();
		   iter != last; ++iter) {
      if (iter->size() == 2) {
         const std::string& opt = (*iter)[0];
         std::string data = (*iter)[1];
         if (data == "int" || data == "DbLonglong" || data == "double" || data == "string") {
            if (doGet) {
               if (!m_poolSvc->getAttribute(opt, data, pool::DbType(pool::ROOTTREE_StorageType).type(), contextId).isSuccess()) {
                  ATH_MSG_DEBUG("getAttribute failed for domain attr " << opt);
                  retError = true;
               }
            }
         } else if (doSet) {
            if (m_poolSvc->setAttribute(opt, data, pool::DbType(pool::ROOTTREE_StorageType).type(), contextId).isSuccess()) {
               ATH_MSG_DEBUG("setAttribute " << opt << " to " << data);
               if (doClear) {
                  iter->clear();
               }
            } else {
               ATH_MSG_DEBUG("setAttribute failed for domain attr " << opt << " to " << data);
               retError = true;
            }
         }
      }
      if (iter->size() == 4) {
         const std::string& opt = (*iter)[0];
         std::string data = (*iter)[1];
         const std::string& file = (*iter)[2];
         const std::string& cont = (*iter)[3];
         if (!fileName.empty() && (0 == fileName.compare(0, fileName.find('?'), file)
	         || (file[0] == '*' && file.find("," + fileName + ",") == std::string::npos))) {
            if (data == "int" || data == "DbLonglong" || data == "double" || data == "string") {
               if (doGet) {
                  if (!m_poolSvc->getAttribute(opt, data, pool::DbType(pool::ROOTTREE_StorageType).type(), fileName, cont, contextId).isSuccess()) {
                     ATH_MSG_DEBUG("getAttribute failed for database/container attr " << opt);
                     retError = true;
                  }
               }
            } else if (doSet) {
               if (m_poolSvc->setAttribute(opt, data, pool::DbType(pool::ROOTTREE_StorageType).type(), fileName, cont, contextId).isSuccess()) {
                  ATH_MSG_DEBUG("setAttribute " << opt << " to " << data << " for db: " << fileName << " and cont: " << cont);
                  if (doClear) {
                     if (file[0] == '*' && !m_persSvcPerOutput) {
                        (*iter)[2] += "," + fileName + ",";
                     } else {
                        iter->clear();
                     }
                  }
               } else {
                  ATH_MSG_DEBUG("setAttribute failed for " << opt << " to " << data << " for db: " << fileName << " and cont: " << cont);
                  retError = true;
               }
            }
         }
      }
   }
   for (std::vector<std::vector<std::string> >::iterator iter = attr.begin(); iter != attr.end(); ) {
      if (iter->empty()) {
         iter = attr.erase(iter);
      } else {
         ++iter;
      }
   }
   return(retError ? StatusCode::FAILURE : StatusCode::SUCCESS);
}
