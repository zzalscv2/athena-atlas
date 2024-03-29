///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// StorePayloadMon.h 
// Header file for class StorePayloadMon
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef PERFMONCOMPS_PERFMON_STOREPAYLOADMON_H
#define PERFMONCOMPS_PERFMON_STOREPAYLOADMON_H 1

// C includes
#include <stdio.h>

// STL includes
#include <string>

// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "StoreGate/StoreGateSvc.h"

namespace PerfMon {

class ATLAS_NOT_THREAD_SAFE StorePayloadMon
//    ^ direct StoreGate access
  : public ::AthAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  StorePayloadMon( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~StorePayloadMon(); 

  // Assignment operator: 
  //StorePayloadMon &operator=(const StorePayloadMon &alg); 

  // Athena algorithm's Hooks
  virtual StatusCode  initialize();
  virtual StatusCode  execute();
  virtual StatusCode  finalize();


  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Default constructor: 
  StorePayloadMon();

  typedef ServiceHandle<StoreGateSvc> StoreGateSvc_t;
  /// handle to the store instance to monitor
  StoreGateSvc_t m_store;

  typedef ServiceHandle<IClassIDSvc> IClassIDSvc_t;
  /// handle to the class id svc
  IClassIDSvc_t m_clidsvc;

  /// payload-mon file descriptor
  int m_stream;

  /// Name of the output file where the monitoring data will be stored
  std::string m_stream_name;

  /// display mallinfos after each event
  bool m_displayMallinfos;
}; 

} //> end namespace PerfMon
#endif //> !PERFMONCOMPS_PERFMON_STOREPAYLOADMON_H
