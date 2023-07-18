/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonABLineJsonDumpAlg.h"
#include "StoreGate/ReadCondHandle.h"
#include "nlohmann/json.hpp"
#include <fstream>

MuonABLineJsonDumpAlg::MuonABLineJsonDumpAlg(const std::string& name, ISvcLocator* pSvcLocator):
      AthAlgorithm{name, pSvcLocator} {}

StatusCode MuonABLineJsonDumpAlg::initialize() {
    ATH_CHECK(m_readALineKey.initialize());
    ATH_CHECK(m_readBLineKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
  }
StatusCode MuonABLineJsonDumpAlg::execute(){
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadCondHandle<ALineContainer> aLineContainer{m_readALineKey, ctx};
  if (!aLineContainer.isValid()){
    ATH_MSG_FATAL("Failed to load ALine container "<<m_readALineKey.fullKey());
    return StatusCode::FAILURE;
  }
  SG::ReadCondHandle<BLineContainer> bLineContainer{m_readBLineKey, ctx};
  if (!aLineContainer.isValid()){
    ATH_MSG_FATAL("Failed to load BLine container "<<m_readBLineKey.fullKey());
    return StatusCode::FAILURE;
  }
  
  std::ofstream ostr{m_jsonFile};
  if (!ostr.good()) {
     ATH_MSG_FATAL("Failed to create output file "<<m_jsonFile);
     return StatusCode::FAILURE;
  }
  ostr<<"["<<std::endl;
  unsigned int nLines{0};
  for (const ALinePar& aLine : **aLineContainer) {
    ++nLines;
    ostr<<"    {"<<std::endl;
    /// Identifier of the A Line
    ostr<<"     \"typ\": \""<<aLine.AmdbStation()<<"\","<<std::endl;
    ostr<<"     \"jzz\": "<<aLine.AmdbEta()<<", "<<std::endl;
    ostr<<"     \"jff\": "<<aLine.AmdbPhi()<<", "<<std::endl;
    ostr<<"     \"job\": "<<aLine.AmdbJob()<<", "<<std::endl;
    /// ALine parameter
    using APar = ALinePar::Parameter;
    ostr<<"     \"svalue\": "<<aLine.getParameter(APar::transS)<<", "<<std::endl;
    ostr<<"     \"zvalue\": "<<aLine.getParameter(APar::transZ)<<", "<<std::endl;
    ostr<<"     \"tvalue\": "<<aLine.getParameter(APar::transT)<<", "<<std::endl;
    ostr<<"     \"tsv\": "<<aLine.getParameter(APar::rotS)<<", "<<std::endl;
    ostr<<"     \"tzv\": "<<aLine.getParameter(APar::rotZ)<<", "<<std::endl;
    ostr<<"     \"ttv\": "<<aLine.getParameter(APar::rotT);
    /// BLine parameter
    BLineContainer::const_iterator itr = bLineContainer->find(aLine.identify());
    if (itr == bLineContainer->end()) {
      /// Check that the last entry does not have a comma
      ostr<<std::endl<<"    }"<< (nLines != aLineContainer->size() ? "," : "")<<std::endl;
      continue;
    }
    ostr<<","<<std::endl;
    using BPar = BLinePar::Parameter;
    const BLinePar& bLine = (*itr);
    ostr<<"     \"bz\": "<<bLine.getParameter(BPar::bz)<<","<<std::endl;
    ostr<<"     \"bp\": "<<bLine.getParameter(BPar::bp)<<","<<std::endl;
    ostr<<"     \"bn\": "<<bLine.getParameter(BPar::bn)<<","<<std::endl;
    ostr<<"     \"sp\": "<<bLine.getParameter(BPar::sp)<<","<<std::endl;
    ostr<<"     \"sn\": "<<bLine.getParameter(BPar::sn)<<","<<std::endl;
    ostr<<"     \"tw\": "<<bLine.getParameter(BPar::tw)<<","<<std::endl;
    ostr<<"     \"pg\": "<<bLine.getParameter(BPar::pg)<<","<<std::endl;
    ostr<<"     \"tr\": "<<bLine.getParameter(BPar::tr)<<","<<std::endl;
    ostr<<"     \"eg\": "<<bLine.getParameter(BPar::eg)<<","<<std::endl;
    ostr<<"     \"ep\": "<<bLine.getParameter(BPar::ep)<<","<<std::endl;
    ostr<<"     \"en\": "<<bLine.getParameter(BPar::en)<<","<<std::endl;
    /// No idea what xAtlas & yAtlas are in this parameter book
    ostr<<"     \"xAtlas\": 0 ,"<<std::endl;
    ostr<<"     \"yAtlas\": 0 ,"<<std::endl;
    ostr<<"     \"hwElement\": \"";
    ostr<<m_idHelperSvc->stationNameString(bLine.identify());
    const int stEta = m_idHelperSvc->stationEta(bLine.identify());
    ostr<<std::abs(stEta)<<(stEta > 0 ? "A" : "C");
    ostr<<m_idHelperSvc->stationPhi(bLine.identify());
    ostr<<"\""<<std::endl;
    ostr<<"    }"<< (nLines != aLineContainer->size() ? "," : "")<<std::endl;
  }
  ostr<<"]"<<std::endl;
  ostr.close();
  
  
  return StatusCode::SUCCESS;
}
