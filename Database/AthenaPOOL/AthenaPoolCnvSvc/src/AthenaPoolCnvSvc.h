/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLCNVSVC_ATHENAPOOLCNVSVC_H
#define ATHENAPOOLCNVSVC_ATHENAPOOLCNVSVC_H

/** @file AthenaPoolCnvSvc.h
 *  @brief This file contains the class definition for the AthenaPoolCnvSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaPoolCnvSvc/IAthenaPoolCnvSvc.h"

#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIoComponent.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

#include "StorageSvc/DbType.h"
#include "AthenaBaseComps/AthCnvSvc.h"
#include "AthenaKernel/IAthenaIPCTool.h"
#include "PmbCxxUtils/BasicStopWatch.h"
#include "PoolSvc/IPoolSvc.h"

#include <vector>
#include <map>
#include <mutex>
#include <limits>

// Forward declarations
class IAthenaSerializeSvc;
class Guid;

template <class TYPE> class SvcFactory;

/** @class AthenaPoolCnvSvc
 *  @brief This class provides the interface between Athena and PoolSvc.
 **/
class AthenaPoolCnvSvc : public ::AthCnvSvc,
		public virtual IAthenaPoolCnvSvc,
		public virtual IIncidentListener,
		public virtual IIoComponent {
   // Allow the factory class access to the constructor
   friend class SvcFactory<AthenaPoolCnvSvc>;

public:

   /// Required of all Gaudi Services
   StatusCode initialize();
   StatusCode io_reinit();
   /// Required of all Gaudi Services
   StatusCode finalize();
   StatusCode io_finalize();
   /// Required of all Gaudi services:  see Gaudi documentation for details
   StatusCode queryInterface(const InterfaceID& riid, void** ppvInterface);

   /// Implementation of IConversionSvc: Create the transient representation of an object from persistent state.
   /// @param pAddress [IN] pointer to IOpaqueAddress of the representation.
   /// @param refpObject [OUT] pointer to DataObject to be created.
   StatusCode createObj(IOpaqueAddress* pAddress, DataObject*& refpObject);

   /// Implementation of IConversionSvc: Convert the transient object to the requested representation.
   /// @param pObject [IN] pointer to DataObject.
   /// @param refpAddress [OUT] pointer to IOpaqueAddress of the representation to be created.
   StatusCode createRep(DataObject* pObject, IOpaqueAddress*& refpAddress);

   /// Implementation of IConversionSvc: Resolve the references of the converted object.
   /// @param pAddress [IN] pointer to IOpaqueAddress of the representation to be resolved.
   /// @param pObject [IN] pointer to DataObject to be created.
   StatusCode fillRepRefs(IOpaqueAddress* pAddress, DataObject* pObject);

   /// Implementation of IConversionSvc: Connect to the output connection specification with open mode.
   /// @param outputConnectionSpec [IN] the name of the output connection specification as string.
   /// @param openMode [IN] the open mode of the file as string.
   StatusCode connectOutput(const std::string& outputConnectionSpec,
		   const std::string& openMode);

   /// Implementation of IConversionSvc: Connect to the output connection specification with open mode.
   /// @param outputConnectionSpec [IN] the name of the output
   /// connection specification as string.
   StatusCode connectOutput(const std::string& outputConnectionSpec);

   /// Implementation of IConversionSvc: Commit pending output.
   /// @param doCommit [IN] boolean to force full commit
   StatusCode commitOutput(const std::string& outputConnectionSpec, bool doCommit);

   /// Disconnect to the output connection.
   StatusCode disconnectOutput(const std::string& outputConnectionSpec);

   /// @return pointer to PoolSvc instance.
   IPoolSvc* getPoolSvc();

   /// @return a string token to a Data Object written to Pool
   /// @param placement [IN] pointer to the placement hint
   /// @param obj [IN] pointer to the Data Object to be written to Pool
   /// @param classDesc [IN] pointer to the Seal class description for the Data Object.
   Token* registerForWrite(Placement* placement, const void* obj, const RootType& classDesc);

   /// @param obj [OUT] pointer to the Data Object.
   /// @param token [IN] string token of the Data Object for which a Pool Ref is filled.
   void setObjPtr(void*& obj, const Token* token);

   /// @return a boolean for using detailed time and size statistics.
   bool useDetailChronoStat() const;

   /// Create a Generic address using explicit arguments to identify a single object.
   /// @param svcType [IN] service type of the address.
   /// @param clid [IN] class id for the address.
   /// @param par [IN] string containing the database name.
   /// @param ip [IN] object identifier.
   /// @param refpAddress [OUT] converted address.
   StatusCode createAddress(long svcType,
		   const CLID& clid,
		   const std::string* par,
		   const unsigned long* ip,
		   IOpaqueAddress*& refpAddress);

   /// Create address from string form
   /// @param svcType [IN] service type of the address.
   /// @param clid [IN] class id for the address.
   /// @param refAddress [IN] string form to be converted.
   /// @param refpAddress [OUT] converted address.
   StatusCode createAddress(long svcType,
		   const CLID& clid,
		   const std::string& refAddress,
		   IOpaqueAddress*& refpAddress);

   /// Convert address to string form
   /// @param pAddress [IN] address to be converted.
   /// @param refAddress [OUT] converted string form.
   StatusCode convertAddress(const IOpaqueAddress* pAddress, std::string& refAddress);

   /// Extract/deduce the DB technology from the connection
   /// string/file specification
   StatusCode decodeOutputSpec(std::string& connectionSpec, int& outputTech) const;

   /// Implement registerCleanUp to register a IAthenaPoolCleanUp to be called during cleanUp.
   StatusCode registerCleanUp(IAthenaPoolCleanUp* cnv);

   /// Implement cleanUp to call all registered IAthenaPoolCleanUp cleanUp() function.
   StatusCode cleanUp(const std::string& connection);

   /// Set the input file attributes, if any are requested from jobOpts
   /// @param fileName [IN] name of the input file
   StatusCode setInputAttributes(const std::string& fileName);

   /// Make this a server.
   virtual StatusCode makeServer(int num);

   /// Make this a client.
   virtual StatusCode makeClient(int num);

   /// Read the next data object
   virtual StatusCode readData();

   /// Commit Catalog
   virtual StatusCode commitCatalog();

   /// Send abort to SharedWriter clients if the server quits on error
   /// @param client_n [IN] number of the current client, -1 if no current
   StatusCode abortSharedWrClients(int client_n);

   /// Implementation of IIncidentListener: Handle for EndEvent incidence
   void handle(const Incident& incident);

   /// Standard Service Constructor
   AthenaPoolCnvSvc(const std::string& name, ISvcLocator* pSvcLocator);
   /// Destructor
   virtual ~AthenaPoolCnvSvc() = default;

private: // member functions
   /// Extract POOL ItechnologySpecificAttributes for Domain, Database and Container from property.
   void extractPoolAttributes(const StringArrayProperty& property,
	   std::vector<std::vector<std::string> >* contAttr,
	   std::vector<std::vector<std::string> >* dbAttr,
	   std::vector<std::vector<std::string> >* domAttr = 0) const;

   /// Set/get technology dependent POOL attributes
   StatusCode processPoolAttributes(std::vector<std::vector<std::string> >& attr,
	   const std::string& fileName,
	   unsigned long contextId,
	   bool doGet = true,
	   bool doSet = true,
	   bool doClear = true) const;

private: // data
   /// decoded storage tech requested in "StorageTechnology" property
   pool::DbType                  m_dbType;  
   std::string                   m_lastInputFileName;
   ServiceHandle<IPoolSvc>       m_poolSvc{this,"PoolSvc","PoolSvc"};
   ServiceHandle<IClassIDSvc>    m_clidSvc{this,"ClassIDSvc","ClassIDSvc"};
   ServiceHandle<IAthenaSerializeSvc> m_serializeSvc{this,"AthenaRootSerializeSvc","AthenaRootSerializeSvc"};
   ToolHandle<IAthenaIPCTool>    m_inputStreamingTool{this,"InputStreamingTool",{}};
   ToolHandleArray<IAthenaIPCTool>    m_outputStreamingTool;
   //The following doesn't work because of Gaudi issue #122
   //ToolHandleArray<IAthenaIPCTool>    m_outputStreamingTool{this,"OutputStreamingTool", {} };
   std::size_t     m_streamServer=0;
   int m_metadataClient=0;

   /// Map that holds chrono information
   PMonUtils::BasicStopWatchResultMap_t m_chronoMap{};

private: // properties
   /// UseDetailChronoStat, enable detailed output for time and size statistics for AthenaPOOL:
   /// default = false.
   BooleanProperty m_useDetailChronoStat{this,"UseDetailChronoStat",false};

   /// Default Storage Tech for containers (ROOTTREE, ROOTTREEINDEX, ROOTRNTUPLE)
   StringProperty  m_storageTechProp{this,"StorageTechnology", "ROOTTREEINDEX"};
   //StringProperty  m_storageTechProp{this,"StorageTechnology", "ROOTRNTUPLE"};
   /// PoolContainerPrefix, prefix for top level POOL container: default = "POOLContainer"
   StringProperty  m_containerPrefixProp{this,"PoolContainerPrefix","CollectionTree"};
   /// TopLevelContainerName, naming hint policy for top level POOL container: default = "<type>"
   StringProperty  m_containerNameHintProp{this,"TopLevelContainerName",""};
   /// SubLevelBranchName, naming hint policy for POOL branching: default = "" (no branching)
   StringProperty  m_branchNameHintProp{this,"SubLevelBranchName", "<type>/<key>"};

   /// Output PoolAttributes, vector with names and values of technology specific attributes for POOL
   StringArrayProperty m_poolAttr{this,"PoolAttributes",{},"Pool Attributes","OrderedSet<std::string>"};
   std::vector<std::vector<std::string> > m_domainAttr;
   std::vector<std::vector<std::string> > m_databaseAttr;
   std::vector<std::vector<std::string> > m_containerAttr;
   std::vector<unsigned int> m_contextAttr;
   std::map<std::string, int> m_fileCommitCounter;
   std::map<std::string, int> m_fileFlushSetting;
   /// Input PoolAttributes, vector with names and values of technology specific attributes for POOL
   StringArrayProperty m_inputPoolAttr{this,"InputPoolAttributes",{}};
   std::vector<std::vector<std::string> > m_inputAttr;
   /// Print input PoolAttributes per event, vector with names of technology specific attributes for POOL
   /// to be printed each event
   StringArrayProperty m_inputPoolAttrPerEvent{this,"PrintInputAttrPerEvt",{}};
   std::vector<std::vector<std::string> > m_inputAttrPerEvent;

   /// Output FileNames to be associated with Stream Clients
   std::vector<std::string>   m_streamClientFiles;

   /// MaxFileSizes, vector with maximum file sizes for Athena POOL output files
   StringArrayProperty m_maxFileSizes{this,"MaxFileSizes",{}};
   long long m_domainMaxFileSize=std::numeric_limits<long long>::max();
   std::map<std::string, long long> m_databaseMaxFileSize;

   /// PersSvcPerOutput, boolean property to use multiple persistency services, one per output stream.
   /// default = true.
   BooleanProperty m_persSvcPerOutput{this,"PersSvcPerOutput",true};
   unsigned outputContextId(const std::string& outputConnection);

   /// PersSvcPerInputType, string property, tree name to use multiple persistency services, one per input type.
   /// default = "", no tree name results in a single persistency service.
   StringProperty m_persSvcPerInputType{this,"PersSvcPerInputType",""};
   std::mutex  m_mutex;
  
   /// For SharedWriter:
   /// To use MetadataSvc to merge data placed in a certain container
   StringProperty  m_metadataContainerProp{this,"OutputMetadataContainer","MetaData"};
   /// Make this instance a Streaming Client during first connect/write automatically
   IntegerProperty m_makeStreamingToolClient{this,"MakeStreamingToolClient",0};
   /// Use Streaming for selected technologies only
   IntegerProperty m_streamingTechnology{this,"StreamingTechnology",-1};
   /// Use Athena Object sharing for metadata only, event data is collected and send via ROOT TMemFile
   BooleanProperty m_parallelCompression{this,"ParallelCompression",true};
   /// Extension to use ROOT TMemFile for event data, "?pmerge=<host>:<port>"
   StringProperty  m_streamPortString{this,"StreamPortString","?pmerge=localhost:0"};
   /// When using TMemFile call Write on number of Events, respecting CollectionTree auto_flush
   IntegerProperty m_numberEventsPerWrite{this,"NumberEventsPerWrite",-1};

   /// Property for DataHeaderCnv input DHForm cache size
   IntegerProperty m_DHFormCacheSize { this, "maxDHFormCacheSize", 100 };
   /// Flag to control SG alias filtering when writing out DataHeader (see DataHeaderCnv_p6)
   BooleanProperty m_DHFilterAliases { this, "doFilterDHAliases", true };

};

#endif
