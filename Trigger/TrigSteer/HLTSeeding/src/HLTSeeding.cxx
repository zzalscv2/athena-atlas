
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "HLTSeeding.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigConfxAOD/IKeyWriterTool.h"
#include "xAODTrigger/TrigCompositeAuxContainer.h"

#include "StoreGate/WriteHandle.h"
#include "GaudiKernel/EventContext.h"

#include "RoiDescriptor/RoiDescriptor.h"

HLTSeeding::HLTSeeding(const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator) {}


StatusCode HLTSeeding::initialize() {
  ATH_MSG_INFO( "Reading RoIB information from: " << m_RoIBResultKey.objKey() << " : "
                << m_RoIBResultKey.fullKey() << " : " << m_RoIBResultKey.key() );

  if ( m_RoIBResultKey.empty() && m_l1TriggerResultKey.empty() ) {
    ATH_MSG_INFO( "RoIBResult and L1TriggerResult keys both empty: assume we're running with CTP emulation" );
  }

  ATH_CHECK( m_RoIBResultKey.initialize(!m_RoIBResultKey.empty()) );
  ATH_CHECK( m_l1TriggerResultKey.initialize(!m_l1TriggerResultKey.empty()) );

  ATH_CHECK( m_summaryKey.initialize() );
  ATH_CHECK( m_startStampKey.initialize() );

  ATH_CHECK( m_ctpUnpacker.retrieve() );
  ATH_CHECK( m_roiUnpackers_roib.retrieve() );
  ATH_CHECK( m_roiUnpackers_xaod.retrieve() );
  ATH_CHECK( m_prescaler.retrieve() );

  if ( !m_keyWriterTool.empty() ) {
    ATH_CHECK( m_keyWriterTool.retrieve() );
  }

  if ( !m_consistencyChecker.empty() ) {
    ATH_CHECK( m_consistencyChecker.retrieve() );
  }

  if (m_doCostMonitoring) {
    ATH_CHECK( m_trigCostSvcHandle.retrieve() );
  }

  if ( m_roiZedWidthDefault!=0 ) { 
    ATH_MSG_INFO( "CHANGING THE DEFAULT ROI Z WIDTH: " << m_roiZedWidthDefault );
    RoiDescriptor::circumvent_cppchecker( m_roiZedWidthDefault );
  }

  return StatusCode::SUCCESS;
}


StatusCode HLTSeeding::execute (const EventContext& ctx) const {
  {
    auto timeStampHandle = SG::makeHandle( m_startStampKey, ctx );
    ATH_CHECK( timeStampHandle.record( std::make_unique<TrigTimeStamp>() ) );
  }
  using namespace TrigCompositeUtils;
  const bool decodeRoIB = !m_RoIBResultKey.empty();
  const bool decodexAOD = !m_l1TriggerResultKey.empty();

  const ROIB::RoIBResult* roib = nullptr;
  if (decodeRoIB) {
    SG::ReadHandle<ROIB::RoIBResult> roibH( m_RoIBResultKey, ctx );
    roib = roibH.cptr();
    ATH_MSG_DEBUG( "Obtained RoIBResult" );
  }

  const xAOD::TrigComposite* l1TriggerResult = nullptr;
  if (decodexAOD) {
    auto l1TriggerResultCont = SG::makeHandle(m_l1TriggerResultKey, ctx);
    if (!l1TriggerResultCont.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve L1TriggerResult with key " << m_l1TriggerResultKey.key());
      return StatusCode::FAILURE;
    }
    if (l1TriggerResultCont->size() != 1) {
      ATH_MSG_ERROR("Size of the L1TriggerResultContainer is " << l1TriggerResultCont->size() << " but 1 expected");
      return StatusCode::FAILURE;
    }
    l1TriggerResult = l1TriggerResultCont->at(0);
    if (msgLvl(MSG::DEBUG)) {
      const std::vector<std::string>& linkNames = l1TriggerResult->linkColNames();
      const std::vector<uint32_t>& linkClids = l1TriggerResult->linkColClids();
      ATH_MSG_DEBUG("L1TriggerResult has " << linkNames.size() << " links:");
      for (size_t i=0; i<linkNames.size(); ++i) {
        ATH_MSG_DEBUG("--> " << linkNames.at(i) << " CLID: " << linkClids.at(i));
      }
    }
  }

  SG::WriteHandle<DecisionContainer> handle = TrigCompositeUtils::createAndStore( m_summaryKey, ctx );
  auto *chainsInfo = handle.ptr();

  HLT::IDVec l1SeededChains;
  if (decodeRoIB) {
    ATH_CHECK( m_ctpUnpacker->decode( *roib, l1SeededChains ) );
  } else if (m_ctpUnpacker->isEmulated()) {
    ATH_CHECK( m_ctpUnpacker->decode( ROIB::RoIBResult{}, l1SeededChains ) );
  }

  // important: sorting of the list of seeded chains is needed so that the deduplication and following set difference are correct
  std::sort( l1SeededChains.begin(), l1SeededChains.end() ); 

  // Multiple items can seed some chains, remove duplicates from the sorted vector
  HLT::IDVec::iterator removeFrom = std::unique(l1SeededChains.begin(), l1SeededChains.end());
  l1SeededChains.erase(removeFrom, l1SeededChains.end());

  HLT::IDVec activeChains; // Chains which are activated to run in the first pass (seeded and pass prescale)
  HLT::IDVec prescaledChains; // Chains which are activated but do not run in the first pass (seeded but prescaled out)

  ATH_CHECK( m_prescaler->prescaleChains( ctx, l1SeededChains, activeChains ) );

  // important: sorting of the list of active chains is needed so that the set difference is correct
  std::sort( activeChains.begin(), activeChains.end() ); 

  std::set_difference(l1SeededChains.begin(), l1SeededChains.end(),
                      activeChains.begin(), activeChains.end(),
                      std::back_inserter(prescaledChains));

  // Validation
  for (const HLT::Identifier& id : prescaledChains) {
    if (std::find(activeChains.begin(), activeChains.end(), id) != activeChains.end()) {
      ATH_MSG_ERROR("Prescaled chain cannot also be an active chain (" << id << ")");
    }
  }

  ATH_CHECK( saveChainsInfo( l1SeededChains, chainsInfo, "l1seeded" ) );
  ATH_CHECK( saveChainsInfo( activeChains, chainsInfo, "unprescaled" ) );
  ATH_CHECK( saveChainsInfo( prescaledChains, chainsInfo, "prescaled" ) );
  // Note: 'prescaled' is deduced from 'l1seeded' and 'unprescaled'.

  // Do cost monitoring, this utilises the HLT_costmonitor chain
  if (m_doCostMonitoring) {
    const static HLT::Identifier costMonitorChain(m_costMonitoringChain);
    const auto activeCostMonIt = std::find(activeChains.begin(), activeChains.end(), costMonitorChain);
    if (activeCostMonIt == activeChains.end()){
      ATH_CHECK(m_trigCostSvcHandle->discardEvent(ctx));
    }
  }

  ATH_MSG_DEBUG( "Unpacking RoIs" );
  HLT::IDSet activeChainSet( activeChains.begin(), activeChains.end() );
  if (decodeRoIB) {
    for ( auto unpacker: m_roiUnpackers_roib ) {
      ATH_CHECK( unpacker->unpack( ctx, *roib, activeChainSet ) );
    }
  } else if (m_ctpUnpacker->isEmulated()) {
    ROIB::RoIBResult roib{};
    for ( auto unpacker: m_roiUnpackers_roib ) {
      ATH_CHECK( unpacker->unpack( ctx, roib, activeChainSet ) );
    }
  }
  if (decodexAOD) {
    for ( auto unpacker: m_roiUnpackers_xaod ) {
      try {
        ATH_CHECK( unpacker->unpack( ctx, *l1TriggerResult, activeChainSet ) );
      } catch (const std::exception& ex) {
        ATH_MSG_ERROR("Exception in " << unpacker->name() << "::unpack: " << ex.what());
        return StatusCode::FAILURE;
      }
    }
  }

  if ( !m_keyWriterTool.empty() ) {
    ATH_CHECK( m_keyWriterTool->writeKeys(ctx) );
  }

  if ( !m_consistencyChecker.empty() ) {
    ATH_CHECK( m_consistencyChecker->consistencyCheck(l1SeededChains, ctx) );
  }

  return StatusCode::SUCCESS;
}


StatusCode HLTSeeding::saveChainsInfo(const HLT::IDVec& chains,
                                      xAOD::TrigCompositeContainer* storage,
                                      const std::string& type) {
  using namespace TrigCompositeUtils;
  Decision* d = newDecisionIn( storage, type );
  for ( auto c: chains) {
    addDecisionID(c.numeric(), d);
  }
  return StatusCode::SUCCESS;
}
