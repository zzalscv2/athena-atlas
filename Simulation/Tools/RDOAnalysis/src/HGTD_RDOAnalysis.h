/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HGTD_RDO_ANALYSIS_H
#define HGTD_RDO_ANALYSIS_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/LockedHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "StoreGate/ReadHandleKey.h"

#include "GeneratorObjects/McEventCollection.h"
#include "InDetSimData/InDetSimDataCollection.h"

#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"
#include "HGTD_RawData/HGTD_RDO_Container.h"

#include <string>
#include <vector>
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"

class TTree;
class HGTD_ID;

namespace InDetDD {
  class HGTD_DetectorManager;
}

class HGTD_RDOAnalysis : public AthAlgorithm
{

struct SdoInfo {
  float time;
  int truth = -1; // signal=0, delta = 1; pileup=2
  bool operator<(const SdoInfo& rhs) const { return time < rhs.time; }
};

public:
  HGTD_RDOAnalysis(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override final;
  virtual StatusCode execute() override final;

private:

  bool IsHSGoodParticle(HepMC3::ConstGenParticlePtr particlePtr, const HepMC::GenEvent* hardScatterEvent, float min_pt_cut = 1000.);

  SG::ReadHandleKey<HGTD_RDO_Container> m_inputKey {this, "CollectionName", "HGTD_RDOs", "Input HGTD RDO collection name"}; 
  SG::ReadHandleKey<InDetSimDataCollection> m_inputTruthKey {this, "SDOCollectionName", "HGTD_SDO_Map", "Input HGTD SDO collection name"};
  SG::ReadHandleKey<McEventCollection> m_inputMcEventCollectionKey {this, "McEventCollectionName", "TruthEvent", "Input McEventCollection name"};

  const HGTD_DetectorManager *m_HGTD_Manager{};
  Gaudi::Property<std::string> m_HGTD_Name
  {this, "DetectorName", "HGTD", "HGTD detector name"};
  const HGTD_ID *m_HGTD_ID{};
  Gaudi::Property<std::string> m_HGTDID_Name
  {this, "PixelIDName", "HGTD_ID", "HGTD ID name"};

  Gaudi::Property<std::string> m_histPath {this, "HistPath", "/RDOAnalysis/HGTD/", ""};
  Gaudi::Property<std::string> m_sharedHistPath {this, "SharedHistPath", "/RDOAnalysis/histos/", ""};
  Gaudi::Property<std::string> m_ntuplePath {this, "NtuplePath", "/RDOAnalysis/ntuples/", ""};
  Gaudi::Property<std::string> m_ntupleName {this, "NtupleName", "HGTD", ""};
  Gaudi::Property<bool> m_doPosition {this, "DoPosition", true, ""};

  ServiceHandle<ITHistSvc> m_thistSvc {this, "HistSvc", "THistSvc", ""};

  TTree* m_tree{};
  std::vector<int>   m_rdo_hit_module_layer; //0, 1, 2, 3
  std::vector<float> m_rdo_hit_module_x;  //in mm
  std::vector<float> m_rdo_hit_module_y;  //in mm
  std::vector<float> m_rdo_hit_module_z;  //in mm
  std::vector<float> m_rdo_hit_x;  //in mm
  std::vector<float> m_rdo_hit_y;  //in mm
  std::vector<float> m_rdo_hit_z;  //in mm
  std::vector<float> m_rdo_hit_toa;  //in ns
  std::vector<float> m_rdo_hit_sdo_deposit_time;  //in ns
  std::vector<int>   m_rdo_hit_truth; // signal=1, pileup=2, secondary=3
};

#endif // HGTD_RDO_ANALYSIS_H
