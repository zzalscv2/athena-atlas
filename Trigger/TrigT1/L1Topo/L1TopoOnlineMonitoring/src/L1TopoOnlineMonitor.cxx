/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "L1TopoOnlineMonitor.h"

// Trigger includes
#include "xAODTrigger/L1TopoSimResults.h"
#include "TrigT1Result/CTP_Decoder.h"
#include "L1TopoRDO/Helpers.h"
#include "L1TopoRDO/L1TopoROD.h"
#include "L1TopoRDO/L1TopoFPGA.h"

// System includes
#include <utility>


// Local helper constants and functions
namespace {
  /// Return a vector of indices corresponding to bits set to 1 in the bitset
  template<size_t N>
  std::vector<size_t> bitsetIndices(const std::bitset<N>& bits) {
    std::vector<size_t> indices;
    indices.reserve(bits.count());
    for (size_t i=0; i<bits.size(); ++i) {
      if (bits[i]) indices.push_back(i);
    }
    return indices;
  }
}

// =============================================================================
// Standard constructor
// =============================================================================
L1TopoOnlineMonitor::L1TopoOnlineMonitor(const std::string& name, ISvcLocator* svcLoc)
  : AthMonitorAlgorithm(name, svcLoc),
    m_ctpIds(0)
{}

// =============================================================================
// Implementation of AthReentrantAlgorithm::initialize
// =============================================================================
StatusCode L1TopoOnlineMonitor::initialize() {

  m_rateHdwNotSim.reset(new float[s_nTopoCTPOutputs]);
  m_rateSimNotHdw.reset(new float[s_nTopoCTPOutputs]);
  m_rateHdwAndSim.reset(new float[s_nTopoCTPOutputs]);
  m_rateHdwSim.reset(new float[s_nTopoCTPOutputs]);
  m_countHdwNotSim.reset(new float[s_nTopoCTPOutputs]);
  m_countSimNotHdw.reset(new float[s_nTopoCTPOutputs]);
  m_countHdwSim.reset(new float[s_nTopoCTPOutputs]);
  m_countHdw.reset(new float[s_nTopoCTPOutputs]);
  m_countSim.reset(new float[s_nTopoCTPOutputs]);
  m_countAny.reset(new float[s_nTopoCTPOutputs]);

  for (size_t i=0;i<s_nTopoCTPOutputs;i++){
    m_rateHdwNotSim[i] = 0;
    m_rateSimNotHdw[i] = 0;
    m_rateHdwAndSim[i] = 0;
    m_rateHdwSim[i] = 0;
    m_countHdwNotSim[i] = 0;
    m_countSimNotHdw[i] = 0;
    m_countHdwSim[i] = 0;
    m_countHdw[i] = 0;
    m_countSim[i] = 0;
    m_countAny[i] = 0;
  }

  ATH_CHECK(m_l1topoKey.initialize());
  ATH_CHECK(m_ctpRdoKey.initialize(m_doHwMonCTP));
  ATH_CHECK(m_l1topoRawDataKey.initialize(m_doHwMon));
  ATH_CHECK(m_monTool.retrieve(DisableTool{m_monTool.name().empty()}));

  return AthMonitorAlgorithm::initialize();
}

StatusCode L1TopoOnlineMonitor::start() {

   const TrigConf::L1Menu * l1menu = nullptr;
   ATH_CHECK( m_detStore->retrieve(l1menu) ); 

   m_ctpIds = getCtpIds(*l1menu);

   m_startbit = getStartBits(*l1menu);

   return StatusCode::SUCCESS;
}


StatusCode L1TopoOnlineMonitor::fillHistograms( const EventContext& ctx ) const {
  
  DecisionBits decisionBits{};
  enum class MonFunction : uint8_t {doSimMon=0, doHwMonCTP, doHwMon, doComp, doSummary};
  std::vector<uint8_t> failedMonFunctions;

  
  if (m_doHwMon) {
    StatusCode sc = doHwMon(decisionBits,ctx);
    ATH_MSG_DEBUG("Executed doHWMon: " << (sc.isFailure() ? "failed" : "ok"));
    if (sc.isFailure()) {
      failedMonFunctions.push_back(static_cast<uint8_t>(MonFunction::doHwMon));
    }    
  }

  if (m_doHwMonCTP) {
    StatusCode sc = doHwMonCTP(decisionBits,ctx);
    ATH_MSG_DEBUG("Executed doHWMonCTP: " << (sc.isFailure() ? "failed" : "ok"));
    if (sc.isFailure()) {
      failedMonFunctions.push_back(static_cast<uint8_t>(MonFunction::doHwMonCTP));
    }    
  }
  
  if (m_doSimMon) {
    StatusCode sc = doSimMon(decisionBits,ctx);
    ATH_MSG_DEBUG("Executed doSimMon: " << (sc.isFailure() ? "failed" : "ok"));
    if (sc.isFailure()) {
      failedMonFunctions.push_back(static_cast<uint8_t>(MonFunction::doSimMon));
    }    
  }

  if (m_doComp) {
    StatusCode sc = doComp(decisionBits);
    ATH_MSG_DEBUG("Executed doComp: " << (sc.isFailure() ? "failed" : "ok"));
    if (sc.isFailure()) {
      failedMonFunctions.push_back(static_cast<uint8_t>(MonFunction::doComp));
    }    
  }
  
  auto monFailedMonFunctions = Monitored::Collection("MonitoringFailures", failedMonFunctions);
  Monitored::Group(m_monTool, monFailedMonFunctions);

  return StatusCode::SUCCESS;
}


StatusCode L1TopoOnlineMonitor::doSimMon( DecisionBits& decisionBits, const EventContext& ctx ) const {
  
  
  SG::ReadHandle<xAOD::L1TopoSimResultsContainer> cont(m_l1topoKey, ctx);
  if(!cont.isValid()){
    ATH_MSG_FATAL("Could not retrieve L1Topo EDM Container from the Simulation.");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("----got L1Topo container: " << cont.key());

  std::bitset<s_nTopoCTPOutputs>& triggerBitsSim = DecisionBits::createBits(decisionBits.triggerBitsSim);
  for(const auto l1topo_dec : * cont){
    ATH_MSG_DEBUG( "Reading L1Topo EDM:: Connection ID: " << l1topo_dec->connectionId() << " Clock: " << l1topo_dec->clock() << " Bit-length: " << l1topo_dec->bitWidth() << " Word: " << l1topo_dec->topoWord() << " Word64: " << l1topo_dec->topoWord64() );
    
    std::vector<unsigned> topoword;

    if (l1topo_dec->bitWidth() == 32) {
      for(unsigned int i=0; i<32; ++i) {
	uint32_t mask = 0x1; mask <<= i;
	if ((l1topo_dec->topoWord() & mask) !=0)
	  {
	    topoword.push_back(32*l1topo_dec->clock()+i);
	    uint32_t pos = 32*(l1topo_dec->clock()+(l1topo_dec->connectionId()==2 ? 0 : 2))+i;
	    triggerBitsSim[pos] = ((!decisionBits.triggerBits.has_value() || m_forceCTPasHdw) && m_ctpIds[pos]>=512) ? false : true;
	  }
      }
      std::string name = "CableElec_";
      name += std::to_string(l1topo_dec->connectionId());
      auto monTopoDec = Monitored::Collection(name, topoword);
      Monitored::Group(m_monTool,monTopoDec);
    }
    else if (l1topo_dec->bitWidth() == 64) {
      for(auto startbit : m_startbit[l1topo_dec->connectionId() - 4]) {
	      uint64_t mask = 0x11; mask <<= startbit;
	      if ((l1topo_dec->topoWord64() & mask) !=0) { 
          topoword.push_back(64*l1topo_dec->clock() + startbit);
        }
      }
      std::string name = "CableOpti_";
      name += std::to_string(l1topo_dec->connectionId());
      auto monTopoDec = Monitored::Collection(name, topoword);
      Monitored::Group(m_monTool,monTopoDec);
    }
    else {
      ATH_MSG_DEBUG( "Unknown Bit-length: " << l1topo_dec->bitWidth() );
      return StatusCode::FAILURE;
    }
  }
  std::vector<size_t> triggerBitIndicesSim = bitsetIndices(triggerBitsSim);
  auto monTopoSim = Monitored::Collection("TopoSim", triggerBitIndicesSim);
  Monitored::Group(m_monTool,monTopoSim);

  
  return StatusCode::SUCCESS;
}

StatusCode L1TopoOnlineMonitor::doHwMonCTP( DecisionBits& decisionBits, const EventContext& ctx ) const {
  
  // Retrieve CTP DAQ data for comparison
  SG::ReadHandle<CTP_RDO> ctpRdo{m_ctpRdoKey, ctx};
  if (!ctpRdo.isValid()) {
    ATH_MSG_DEBUG("Failed to retrieve CTP_RDO object (converted from CTP DAQ ROB) with key \""
                  << m_ctpRdoKey.key() << "\". Skipping CTP hardware comparison");
    return StatusCode::FAILURE;
  }

  // CTP RDO contains 17 TBP words for a number of BCs, so use CTP_Decoder to access accepted BC
  CTP_Decoder ctp;
  ctp.setRDO(ctpRdo.cptr());
  const uint32_t l1aPos = ctpRdo->getL1AcceptBunchPosition();
  if (l1aPos >= ctp.getBunchCrossings().size()) {
    ATH_MSG_DEBUG("CTP_RDO gave invalid l1aPos. Skipping CTP hardware comparison");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("CTP l1aPos, size: " << l1aPos << ", " << ctp.getBunchCrossings().size());
  const CTP_BC& ctpL1a = ctp.getBunchCrossings().at(l1aPos);

  // Fill decision bits from CTP RDO
  std::bitset<s_nTopoCTPOutputs>& triggerBitsCtp = DecisionBits::createBits(decisionBits.triggerBitsCtp);
  static constexpr size_t ctpTBPSize{512};
  const std::bitset<ctpTBPSize>& tbp = ctpL1a.getTBP();
  ATH_MSG_VERBOSE("CTP TBP bits: " << tbp.to_string());

  for (size_t i=0; i<s_nTopoCTPOutputs; ++i) {
    if (m_ctpIds[i] < 512)
      {triggerBitsCtp[i] = tbp.test(m_ctpIds[i]);}
    else
      {triggerBitsCtp[i] = false;}
  }

  std::vector<size_t> triggerBitIndicesCtp = bitsetIndices(triggerBitsCtp);
  auto monTopoCtp = Monitored::Collection("TopoCTP", triggerBitIndicesCtp);
  Monitored::Group(m_monTool,monTopoCtp);

  return StatusCode::SUCCESS;
}

StatusCode L1TopoOnlineMonitor::doHwMon( DecisionBits& decisionBits, const EventContext& ctx ) const {
  
  SG::ReadHandle<xAOD::L1TopoRawDataContainer> cont(m_l1topoRawDataKey, ctx);
  if(!cont.isValid()){
    ATH_MSG_FATAL("Could not retrieve L1Topo RAW Data Container from the BS data.");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("----got L1Topo Raw Data container: " << cont.key());

  std::bitset<s_nTopoCTPOutputs>& triggerBits = DecisionBits::createBits(decisionBits.triggerBits);
  std::bitset<s_nTopoCTPOutputs>& overflowBits = DecisionBits::createBits(decisionBits.overflowBits);
  
  std::unique_ptr<L1Topo::L1TopoFPGA> l1topoFPGA;
  
  for(const xAOD::L1TopoRawData* l1topo_raw : *cont) {
    const std::vector<uint32_t>& dataWords = l1topo_raw->dataWords();
    size_t nWords = dataWords.size();
    uint32_t rodTrailer2 = dataWords[--nWords];
    uint32_t rodTrailer1 = dataWords[--nWords];

    L1Topo::L1TopoROD l1topoROD(rodTrailer1, rodTrailer2);

    ATH_MSG_VERBOSE(l1topoROD);

    for (size_t i = nWords; i --> 0;) {
      if ((i+1)%8==0) {
	uint32_t fpgaTrailer2 = dataWords[i];
	uint32_t fpgaTrailer1 = dataWords[--i];

	l1topoFPGA.reset(new L1Topo::L1TopoFPGA(fpgaTrailer1, fpgaTrailer2));

	ATH_MSG_VERBOSE(*l1topoFPGA.get());
    
      }
      else {
	if (l1topoFPGA->topoNumber() != 1) {
	  i-=3;
	  uint32_t overflowWord = dataWords[--i];
	  uint32_t triggerWord = dataWords[--i];
	  for (size_t iBit=0;iBit<32;iBit++) {
	    uint32_t topo = l1topoFPGA->topoNumber();
	    uint32_t fpga = l1topoFPGA->fpgaNumber();
	    unsigned int index = L1Topo::triggerBitIndexPhase1(topo, fpga, iBit);
	    overflowBits[index] = (overflowWord>>iBit)&1;
	    triggerBits[index] = (triggerWord>>iBit)&1;
	  }
	  ATH_MSG_DEBUG("trigger word: " << std::hex << std::showbase << triggerWord << std::dec);
	  ATH_MSG_DEBUG("overflow word: " << std::hex << std::showbase << overflowWord << std::dec);
	}
      }
    }
  }

  triggerBits = triggerBits & (~overflowBits);
  const std::vector<size_t> triggerBitIndicesHdw = bitsetIndices(triggerBits);
  const std::vector<size_t> overflowBitIndicesHdw = bitsetIndices(overflowBits);
  
  ATH_MSG_VERBOSE("trigger bits: " << triggerBits.to_string() );
  ATH_MSG_VERBOSE("overflow bits: " << overflowBits.to_string() );
 
  auto monHdw = Monitored::Collection("HdwResults", triggerBitIndicesHdw);
  auto monOverflow = Monitored::Collection("OverflowResults", overflowBitIndicesHdw);

  Monitored::Group(m_monTool, monHdw, monOverflow);

  return StatusCode::SUCCESS;
}

StatusCode L1TopoOnlineMonitor::doComp( DecisionBits& decisionBits ) const {
  if (!decisionBits.triggerBitsSim.has_value()) {
    ATH_MSG_DEBUG("Simulation bits not set. Skipping simulation to hardware comparison");
    return StatusCode::FAILURE;
  }

  std::bitset<s_nTopoCTPOutputs> triggerBitsSim = decisionBits.triggerBitsSim.value(); // Alias
  std::bitset<s_nTopoCTPOutputs> triggerBitsHdw;

  if (decisionBits.triggerBits.has_value() && !m_forceCTPasHdw)
    {triggerBitsHdw = decisionBits.triggerBits.value();}
  else if (decisionBits.triggerBitsCtp.has_value())
    {triggerBitsHdw = decisionBits.triggerBitsCtp.value();}
  else {
    ATH_MSG_DEBUG("Hardware bits not set. Skipping simulation to hardware comparison");
    return StatusCode::FAILURE;
  }

  std::bitset<s_nTopoCTPOutputs> triggerBitsSimNotHdw = triggerBitsSim & (~triggerBitsHdw);
  std::bitset<s_nTopoCTPOutputs> triggerBitsHdwNotSim = triggerBitsHdw & (~triggerBitsSim);
  std::bitset<s_nTopoCTPOutputs> triggerBitsHdwSim = triggerBitsHdw & triggerBitsSim;
  std::bitset<s_nTopoCTPOutputs> triggerBitsAny = triggerBitsHdw | triggerBitsSim;

  std::vector<size_t> triggerBitIndicesSimNotHdw = bitsetIndices(triggerBitsSimNotHdw);
  std::vector<size_t> triggerBitIndicesHdwNotSim = bitsetIndices(triggerBitsHdwNotSim);
  auto monSimNotHdw = Monitored::Collection("SimNotHdwL1TopoResult", triggerBitIndicesSimNotHdw);
  auto monHdwNotSim = Monitored::Collection("HdwNotSimL1TopoResult", triggerBitIndicesHdwNotSim);

  Monitored::Group(m_monTool, monSimNotHdw, monHdwNotSim);

  float rate=0;
  for (size_t i=0;i<4;i++) {
    auto mon_trig = Monitored::Scalar<unsigned>("Phase1TopoTrigger_"+std::to_string(i));
    auto mon_match = Monitored::Scalar<unsigned>("Phase1TopoMissMatch_"+std::to_string(i));
    auto mon_weight = Monitored::Scalar<float>("Phase1TopoWeight_"+std::to_string(i));
    for (size_t j=0;j<32;j++) {
      m_countHdwNotSim[32*i+j]+=triggerBitsHdwNotSim[32*i+j];
      m_countSimNotHdw[32*i+j]+=triggerBitsSimNotHdw[32*i+j];
      m_countHdwSim[32*i+j]+=triggerBitsHdwSim[32*i+j];
      m_countHdw[32*i+j]+=triggerBitsHdw[32*i+j];
      m_countSim[32*i+j]+=triggerBitsSim[32*i+j];
      m_countAny[32*i+j]+=triggerBitsAny[32*i+j];
      
      rate = m_countHdw[32*i+j]>0 ? m_countHdwNotSim[32*i+j]/m_countHdw[32*i+j] : 0;
      if (rate != m_rateHdwNotSim[32*i+j]) {
	mon_trig = static_cast<unsigned>(j);
	mon_match = 0;
	mon_weight = rate-m_rateHdwNotSim[32*i+j];
	m_rateHdwNotSim[32*i+j] = rate;
	Monitored::Group(m_monTool, mon_trig, mon_match, mon_weight);
      }
      rate = m_countSim[32*i+j]>0 ? m_countSimNotHdw[32*i+j]/m_countSim[32*i+j] : 0;
      if (rate != m_rateSimNotHdw[32*i+j]) {
	mon_trig = static_cast<unsigned>(j);
	mon_match = 1;
	mon_weight = rate-m_rateSimNotHdw[32*i+j];
	m_rateSimNotHdw[32*i+j] = rate;
	Monitored::Group(m_monTool, mon_trig, mon_match, mon_weight);
      }
      rate = m_countAny[32*i+j]>0 ? m_countHdwSim[32*i+j]/m_countAny[32*i+j] : 0;
      if (rate != m_rateHdwAndSim[32*i+j]) {
	mon_trig = static_cast<unsigned>(j);
	mon_match = 2;
	mon_weight = rate-m_rateHdwAndSim[32*i+j];
	m_rateHdwAndSim[32*i+j] = rate;
	Monitored::Group(m_monTool, mon_trig, mon_match, mon_weight);
      }
      rate = m_countSim[32*i+j]>0 ? m_countHdw[32*i+j]/m_countSim[32*i+j] : 0;
      if (rate != m_rateHdwSim[32*i+j]) {
	mon_trig = static_cast<unsigned>(j);
	mon_match = 3;
	mon_weight = rate-m_rateHdwSim[32*i+j];
	m_rateHdwSim[32*i+j] = rate;
	Monitored::Group(m_monTool, mon_trig, mon_match, mon_weight);
      }
    }
  }

  return StatusCode::SUCCESS;
}

std::vector<std::vector<unsigned>> L1TopoOnlineMonitor::getStartBits( const TrigConf::L1Menu& l1menu ) {

  std::vector<std::vector<unsigned>> startbit_vec;
  std::vector<std::string> connNames = l1menu.connectorNames();
  for( const std::string connName : {"Topo1Opt0", "Topo1Opt1", "Topo1Opt2", "Topo1Opt3"}) {
    if( find(connNames.begin(), connNames.end(), connName) == connNames.end() ) {
      continue;
    }
    std::vector<unsigned> startbit;
	  for(auto & t1 : l1menu.connector(connName).triggerLines()) {
      startbit.push_back(t1.startbit());
    }
    startbit_vec.push_back(startbit);
  }
  return startbit_vec;
}
// 

std::vector<unsigned> L1TopoOnlineMonitor::getCtpIds( const TrigConf::L1Menu& l1menu ) {
  
  // Topo
  std::vector<std::string> connNames = l1menu.connectorNames();
  std::vector<std::string> labelsTopoEl(s_nTopoCTPOutputs);
  for( const std::string connName : {"Topo2El", "Topo3El"}) {
    if( find(connNames.begin(), connNames.end(), connName) == connNames.end() ) {
      continue;
    }
    for(uint fpga : {0,1}) {
      for(uint clock : {0,1}) {
	for(auto & tl : l1menu.connector(connName).triggerLines(fpga,clock)) {
	  uint flatIndex = tl.flatindex() + 64 * (connName == "Topo3El"); 
	  labelsTopoEl[flatIndex] = tl.name();
	}
      }
    }
  }

  ATH_MSG_DEBUG("Obtaining CTPIds for Phase1 L1Topo Monitoring");
  std::vector<unsigned> ctpIds(s_nTopoCTPOutputs,999);
  for( const auto & item : l1menu ) {
    std::string definition = item.definition();
    if (definition.substr(0,5) == "TOPO_" &&
	definition.find(' ') == std::string::npos) {
      std::string trigger = definition.substr(0, definition.find('['));
      auto pos = std::find(labelsTopoEl.begin(),labelsTopoEl.end(),trigger);
      if (pos != labelsTopoEl.end()) {
	ATH_MSG_DEBUG("Found one CTP; ,CTPId: " << item.ctpId() << " ,Name: " << item.name() << " ,Definition: " << definition);
	unsigned index = std::distance(labelsTopoEl.begin(),pos);
	ctpIds[index]=item.ctpId();
      }
    }
  }

  return ctpIds;
}
