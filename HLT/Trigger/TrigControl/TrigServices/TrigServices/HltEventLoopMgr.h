/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSERVICES_HLTEVENTLOOPMGR_H
#define TRIGSERVICES_HLTEVENTLOOPMGR_H

#include "TrigKernel/ITrigEventLoopMgr.h"
#include "EventInfo/EventID.h"  /* number_type */
#include "EventInfo/TriggerInfo.h"
#include "xAODEventInfo/EventInfo.h"
#include "eformat/eformat.h"

// Framework include files
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/MinimalEventLoopMgr.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/HistoProperty.h"
#include "AthenaKernel/Timeout.h"
#include "TrigKernel/HltOstreams.h"
#include "TrigKernel/HltAcceptFlag.h"
#include "TrigKernel/HltResultStatusCode.h"
#include "TrigKernel/HltPscErrorCode.h"
#include "TrigKernel/ITrigEventLoopMgr.h"
#include "TrigKernel/IHltTHistSvc.h"
#include "TrigROBDataProviderSvc/ITrigROBDataProviderSvc.h"

#include <tuple>
#include <utility>
#include <functional>
#include <memory>
#include <vector>
#include <set>
#include <stdint.h>

// Forward declarations
class IIncidentSvc;
class IAlgContextSvc;
class StoreGateSvc;
class IROBDataProviderSvc;
class ITHistSvc;
class TrigISHelper;
class TH1F;
class TH2I;
class TProfile;
class EventInfo;
class TrigCOOLUpdateHelper;
class CondAttrListCollection;
namespace coral {
  class AttributeList;
}
namespace TrigConf {
  class IHLTConfigSvc;
}
namespace HLT {
  class HLTResult;
}
namespace eformat {
  namespace write {
    class ROBFragment;
  }
}

class HltEventLoopMgr : public MinimalEventLoopMgr,
                        virtual public ITrigEventLoopMgr,
                        virtual public Athena::TimeoutMaster
{

public:
  /// Creator friend class
  friend class SvcFactory<HltEventLoopMgr>;

  /// Standard Constructor
  HltEventLoopMgr(const std::string& nam, ISvcLocator* svcLoc);
  /// Standard Destructor
  virtual ~HltEventLoopMgr();

  /// implementation of IInterface: queryInterface
  virtual StatusCode queryInterface(const InterfaceID& riid, void** ppvInterface);

  /// \name State transitions
  //@{
  virtual StatusCode sysInitialize();
  virtual StatusCode initialize();
  
  virtual StatusCode prepareForRun(const boost::property_tree::ptree & pt);

  virtual StatusCode hltUpdateAfterFork(const boost::property_tree::ptree & pt);

  virtual StatusCode stop();

  virtual StatusCode sysFinalize();
  virtual StatusCode finalize();

  virtual StatusCode sysReinitialize();
  virtual StatusCode reinitialize();  
  //@}
  
  /// Event processing
  virtual StatusCode executeEvent(void* par);
  
  virtual StatusCode
  processRoIs(const std::vector<eformat::ROBFragment<const uint32_t*> >& l1_result,
              hltinterface::HLTResult& hlt_result,
              const hltinterface::EventId& evId);

  /// Event timoeut reached
  virtual StatusCode timeOutReached();

  /// Update conditions data
  virtual StatusCode hltConditionsUpdate(std::vector<uint32_t>& hltCondCounter);

  /// Update HLT prescales
  virtual StatusCode hltPrescaleUpdate(uint32_t lumiBlock);
  
  /// return the application Name
  std::string applicationName() const { return m_applicationName.value(); }

    /// \name Obsolete methods for online running
  //@{
  virtual StatusCode nextEvent(int maxevt);          ///< Obsolete
  virtual StatusCode executeRun(int runNumber);      ///< Obsolete
  virtual StatusCode stopRun();                      ///< Obsolete
  virtual StatusCode start();                        ///< Obsolete
  //@}
  
private:

  /**
   * @brief Accessor method for the MsgStream.
   * @return handle to the MsgStream.
   */
  inline MsgStream& logStream() const { return *m_msg; }

  /**
   * @brief Accessor method for the message level variable.
   * @return value of the message level for this algorithm.
   */
  inline MSG::Level logLevel() const { return  (m_msg != 0) ? m_msg->level() : MSG::NIL; }

   /**
   * @brief Call execute method of algorithms
   * @return FAILURE in case one algorithm failed
   */
  StatusCode executeAlgorithms();

  /**
   * @brief Helper to fill histograms from HLT result
   * @param hlt_result          reference to dataflow HLTResult object
   */
  void fillHltResultHistograms(hltinterface::HLTResult& hlt_result);
  
  /// Check if running in partition
  bool validPartition() const {
    return (m_partitionName.value()!="None" && m_partitionName.value()!="NONE");  
  }

  /**
   * @brief Helper to set PSC error word
   * @param hlt_result    reference to dataflow HLTResult object; filled on return
   * @param pscErrorCode  PSC Error code which should be set
   */
  void HltSetPscError(hltinterface::HLTResult& hlt_result,
                      hltonl::PSCErrorCode pscErrorCode) const;

  /**
   * @brief Helper to build an empty dataflow HLT Result object
   * @param hlt_result          reference to dataflow HLTResult object; filled on return
   * @param run_no              for ROB fragment 
   * @param lvl1_id             for ROB fragment 
   * @param bunch_crossing_id   for ROB fragment
   * @param l1_Trigger_Type     for ROB fragment
   * @param l1_detev_type       for ROB fragment
   * @param pscErrorCode        PSC Error code which should be set
   */
  void HltEmptyResultROB(hltinterface::HLTResult& hlt_result,
			 uint32_t run_no, uint32_t lvl1_id, uint32_t bunch_crossing_id, 
			 uint32_t l1_Trigger_Type, uint32_t l1_detev_type,
			 hltonl::PSCErrorCode pscErrorCode);

  /**
   * @brief Helper to build the dataflow HLT Result object.
   * @return value              error code hltonl::PSCErrorCode 
   * @param hlt_result          reference to dataflow HLTResult object; filled on return
   * @param run_no              for ROB fragment 
   * @param lvl1_id             for ROB fragment 
   * @param bunch_crossing_id   for ROB fragment
   * @param l1_Trigger_Type     for ROB fragment
   * @param l1_detev_type       for ROB fragment
   * @param trigger_info        trigger info words for the HLT
   * @param stream_tags         stream tags generated during HLT processing
   */
  hltonl::PSCErrorCode HltResultROBs(hltinterface::HLTResult& hlt_result,
				       uint32_t run_no, uint32_t lvl1_id, uint32_t bunch_crossing_id, 
				       uint32_t l1_Trigger_Type, uint32_t l1_detev_type,
				       const std::vector<TriggerInfo::number_type>& trigger_info, 
				       const std::vector<TriggerInfo::StreamTag>& stream_tags,
				       const std::vector<xAOD::EventInfo::StreamTag>& xsts);

  /// Helper to build an empty HLTResult object
  void HltEmptyResultROB(hltinterface::HLTResult& hlt_result,
                         hltonl::PSCErrorCode pscErrorCode);
  /// Helper to build HLTResult object from Steering result
  void HltResult(hltinterface::HLTResult& hlt_result,
                 const EventInfo* pEvent,
                 const xAOD::EventInfo* xev);
  /// Helpers to book histograms
  void bookHistograms();
  void HltBookHistograms();
  void bookAllHistograms();
  void updateDFProps();
  // Call a simple IAlgorithm (usually member) function on all algorithms
  StatusCode callOnAlgs(const std::function<StatusCode(IAlgorithm&)> & func,
                        const std::string & fname, bool failureIsError = false);
  StatusCode prepareAlgs(const EventInfo& evinfo);
  /// Helper to reset necessary data at prepareForRun
  StatusCode internalPrepareResets();
  /// Helper to do whatever is necessary with RunParams (prepare) ptree
  const CondAttrListCollection *
  processRunParams(const boost::property_tree::ptree & pt);
  // Helper to update internally kept data from new sor
  void updInternal(const coral::AttributeList & sor_attrlist);
  // Helper to get update the metadata store with a new metadata record
  void updMetadaStore(const coral::AttributeList & sor_attrlist);
  /// Helper to clear per-event stores
  StatusCode clearTemporaryStores();
  /// Helper to update the detector mask
  void updDetMask(const std::pair<uint64_t, uint64_t>& dm);
  /// Helper to update the SOR time stamp
  void updSorTime(unsigned long long st);
  /// Helper to extract the single attr list off the SOR CondAttrListCollection
  const coral::AttributeList &
  getSorAttrList(const CondAttrListCollection * sor) const;
  /// Update the HLTConfigSvc (only required to succeed with DB based config)
  StatusCode updHLTConfigSvc();
  /// Set the EventInfo in the event store and return it
  const EventInfo * prepEventInfo() const;
  /// Set the xAOD::EventInfo in the event store
  StatusCode prepXAODEventInfo() const;
  /// Update the magnet field from IS when necessary and possible
  StatusCode updMagField() const;
  /// Set magnetic field from IS
  StatusCode setMagFieldFromIS() const;
  /// Reset selected proxies / IOV folders
  StatusCode resetCoolValidity();
  /// Helper to log a message with some details when ready to run
  void readyMsg() const;
  /// Helper to fill in HLTResult with stream tags
  void setStreamTags(hltinterface::HLTResult& hltr,
                  const std::vector<TriggerInfo::StreamTag>& stream_tags) const;
  void mergeStreamTags(hltinterface::HLTResult& hltr,
                     const std::vector<xAOD::EventInfo::StreamTag>& xsts) const;
  /// get the right hlt decision, setting debug stream tag if necessary
  hltonl::AcceptFlag processHltDecision(hltinterface::HLTResult& hltr) const;
  /// Serializes Steering's result into hltinterface's result robs
  hltonl::PSCErrorCode serializeRobs(hltinterface::HLTResult& hltr, bool& serOk,
                                     HLT::HLTResult* dobj, uint32_t rn,
                                     uint32_t l1id, uint32_t bcid,
                                     uint32_t l1tt, uint32_t dett);
  // Check whether a rob fits the space left. If not, issue an error
  bool checkRobSize(uint32_t robsize, uint32_t spaceleft, uint32_t maxsize);
  // Check consistency of the info received in the EventId and the CTP fragment
  bool checkEventIdConsistency(const hltinterface::EventId& evId) const;
  // serialize rob from steering's HLTResult
  bool serializeRob(std::unique_ptr<uint32_t>& tmpstor,
                    eformat::write::ROBFragment& rob,
                    HLT::HLTResult& dobj,
                    unsigned int robid,
                    int payload_max);
  // add rob to hltinterface::HLTResult
  void addRobToHLTResult(hltinterface::HLTResult& hltr,
                         eformat::write::ROBFragment& rob,
                         uint32_t*& fp, uint32_t& spaceleft);
  // Get monitoring information for navigation sizes of HLT EDM
  void recordEDMSizeInfo(size_t nav_size, bool serializationOk) const;
  // check if a ROB is enabled for readout in OKS
  bool isRobEnabled(uint32_t robid) const;
  // check if a Sub Detector is enabled for readout in OKS
  bool isSubDetectorEnabled(uint32_t subdetid) const;
  // filter a set of robs according to whether or not they are enabled
  std::set<uint32_t> filterRobs(const std::set<uint32_t> robs) const;
  // filter a set of dets according to whether or not they are enabled
  std::set<eformat::SubDetector>
  filterDets(const std::set<uint32_t> dets) const;

  /** Handles to required services/tools **/
  typedef ServiceHandle<IIncidentSvc> IIncidentSvc_t;
  IIncidentSvc_t         m_incidentSvc;

  typedef ServiceHandle<StoreGateSvc> StoreGateSvc_t;
  StoreGateSvc_t         m_evtStore;
  StoreGateSvc_t         m_detectorStore;
  StoreGateSvc_t         m_inputMetaDataStore;

  typedef ServiceHandle<IROBDataProviderSvc> IIROBDataProviderSvc_t;
  IIROBDataProviderSvc_t m_robDataProviderSvc;

  typedef ServiceHandle<ITHistSvc> ITHistSvc_t;
  ITHistSvc_t            m_THistSvc;

  ToolHandle<TrigISHelper>            m_isHelper;
  ToolHandle<TrigCOOLUpdateHelper>    m_coolHelper;

  /** Pointers to optional services/tools **/
  TrigConf::IHLTConfigSvc* m_hltConfigSvc;
  IAlgContextSvc*          m_algContextSvc;

  IntegerProperty          m_lvl1CTPROBid ;       // Source ID for CTP ROB fragment
  BooleanProperty          m_doMonitoring;        // Monitoring

  /// Helper classes for error codes
  hltonl::MapPscErrorCode  m_mapPscError;
  hltonl::MapResultStatusCode m_mapResultStatus;
  hltonl::MapAcceptFlag       m_mapAccept;

  MsgStream*                m_msg       ;   //!< Pointer to MsgStream
  EventID::number_type      m_currentRun;   //!< current run number
  EventID::number_type      m_currentLB;    //!< current lumiblock
  const EventInfo*          m_currentEvent; //!< current EventInfo object in StoreGate
  
  /// Start of Run Time: posix time in seconds since 1970/01/01
  /// Start of Run Time: time stamp ns - ns time offset for time_stamp, 32 bit unsigned
  std::vector<EventID::number_type> m_sorTime_stamp;

  /// detector mask0,1,2,3 - bit field indicating which TTC zones have been
  /// built into the event, one bit per zone, 128 bit total
  /// significance increases from first to last
  typedef EventID::number_type numt;
  std::tuple<numt, numt, numt, numt> m_detector_mask;

  typedef std::map<EventID::number_type, bool> CondUpdateMap;
  CondUpdateMap m_condUpdateOnLB;    //!< Lumiblocks to perform COOL updates and status

  uint32_t                  m_l1_hltCondUpdateLB;    //!< LB of prescale/conditions update from CTP fragment
  
  // --------------------------- Monitoring Histograms --------------------------
  TH1F*                     m_hist_eventAcceptFlags;            //!< Accept flags for processed events
  TH1F*                     m_hist_frameworkErrorCodes;         //!< PSC error codes
  TH1F*                     m_hist_Hlt_result_size;             //!< size of HLT result
  TH1F*                     m_hist_Hlt_result_status;           //!< Status flags in HLT result
  TH1F*                     m_hist_numStreamTags;               //!< Number of StreamTags from HLT
  TH1F*                     m_hist_streamTagTypes;              //!< StreamTag types from HLT   
  TH1F*                     m_hist_streamTagNames;              //!< StreamTag names from HLT
  TH1F*                     m_hist_num_partial_eb_robs;         //!< Number of ROBs for partial event building from HLT   
  TH1F*                     m_hist_num_partial_eb_SubDetectors; //!< Number of SubDetectors for partial event building from HLT 
  TH1F*                     m_hist_partial_eb_SubDetectors_ROBs;//!< SubDetectors for partial event building derived from ROB list
  TH1F*                     m_hist_partial_eb_SubDetectors_SDs; //!< SubDetectors for partial event building derived from SubDetector list

  TProfile*                 m_hist_HltEdmSizes_No_Truncation;   //!< HLT EDM sizes for all events without a truncated HLT result 
  TProfile*                 m_hist_HltEdmSizes_With_Truncation; //!< HLT EDM sizes for all events with a truncated HLT result 
  TProfile*                 m_hist_HltEdmSizes_TruncatedResult_Retained_Collections; //!< HLT EDM sizes for all collections which were retained in a truncated HLT result
  TProfile*                 m_hist_HltEdmSizes_TruncatedResult_Truncated_Collections;//!< HLT EDM sizes for all collections which were truncated in a truncated HLT result
  // --------------------------- Properties -------------------------------------
  BooleanProperty           m_setMagFieldFromIS; //!< Read magnet currents from IS
  StringProperty            m_applicationName;   //!< Application Name (="None" or "athenaHLT" for simulated data, "HLTMPPU-xx"? in online environment) */
  StringProperty            m_partitionName;     //!< Partition Name (="None" for offline, real partion name in online environment)
  BooleanProperty           m_forceHltReject;      // force reject of all events 
  BooleanProperty           m_forceHltAccept;      // force accept of all events
  StringProperty            m_HltResultName;       // name of HLT result in StoreGate
  StringProperty            m_HltDebugStreamName;  // stream name for Debug events 
  StringProperty            m_HltForcedStreamName; // stream name for forced accept events
  IntegerProperty           m_predefinedLumiBlock;
  IntegerProperty           m_prepareForRunSleep;    //!< max random sleep during prepareForRun in sec

  typedef SimpleProperty< std::vector<uint32_t> > Uint32ArrayProperty;
  Uint32ArrayProperty       m_enabledROBs;         //!< list of all enabled ROBs which can be retrieved
  Uint32ArrayProperty       m_enabledSubDetectors; //!< list of all enabled Sub Detectors which can be retrieved

  StringArrayProperty       m_hltEdmCollectionNames; //!< names of all HLT EDM collections for histogram label 
  
  StringProperty            m_jobOptionsType;        //!< JobOptions type (="NONE" or "DB", same as in PSC)

  Histo1DProperty           m_histProp_Hlt_result_size;  
  Histo1DProperty           m_histProp_numStreamTags;              
  Histo1DProperty           m_histProp_streamTagNames;  
  Histo1DProperty           m_histProp_num_partial_eb_robs;
  Histo1DProperty           m_histProp_Hlt_Edm_Sizes;       //!< HLT EDM sizes profile plots 

  int                       m_total_evt;
  int                       m_failed_evt;
  int                       m_invalid_lvl1_result;
  int                       m_invalid_hlt_result;

  // for CTP Lvl1 Rob
  std::vector<uint32_t>     m_ctpRobIdVec ;
  // event number - 32 bit unsigned
  uint32_t                  m_lvl1id  ;
  // run number - 32 bit unsigned
  EventID::number_type      m_run_no ;
  // bunch crossing ID,  32 bit unsigned
  EventID::number_type      m_bunch_crossing_id ;
  // time stamp - posix time in seconds from 1970, 32 bit unsigned
  EventID::number_type      m_time_stamp ;
  // time stamp ns - ns time offset for time_stamp, 32 bit unsigned
  EventID::number_type      m_time_stamp_ns_offset ;
  // luminosity block identifier, 32 bit unsigned
  EventID::number_type      m_lumi_block ;
  // status element
  TriggerInfo::number_type  m_l1_Status_Element ;
  // LVL1 trigger type
  TriggerInfo::number_type  m_l1_Trigger_Type ;
  // LVL1 trigger info
  std::vector<TriggerInfo::number_type> m_l1_Trigger_Info ;
  // LVL1 detev type
  uint32_t                  m_l1_detev_type ;

  /// Reference to a THistSvc which implements also the Hlt additions
  SmartIF<IHltTHistSvc>     m_hltTHistSvc;

  /// Reference to a ROBDataProviderSvc which implements also the Hlt additions
  SmartIF<ITrigROBDataProviderSvc> m_hltROBDataProviderSvc;

  /// Check integrity of CTP ROB and put event to debug stream when it fails
  BooleanProperty           m_lvl1CTPROBcheck ;

  /// Monitoring
  TH1F*                     m_hist_l1_robs;

  /// we need this maintain the data
  uint32_t                  m_status_words[3];

};

//=========================================================================
inline StatusCode HltEventLoopMgr::start()
{
  // This method might be called by ServiceManager::start and does nothing.
  // The real start method is prepareForRun and invoked by the PSC.
  return StatusCode::SUCCESS;
}

//=========================================================================
inline void
HltEventLoopMgr::HltSetPscError(hltinterface::HLTResult& hlt_result,
                                hltonl::PSCErrorCode pscErrorCode) const
{
  hlt_result.psc_errors.push_back(pscErrorCode);
}

#endif // TRIGSERVICES_HLTEVENTLOOPMGR_H
