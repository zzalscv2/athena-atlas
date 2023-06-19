/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "StripClusterizationAlg.h"

#include <AthenaMonitoringKernel/Monitored.h>
#include <xAODInDetMeasurement/StripClusterAuxContainer.h>

namespace ActsTrk {

StripClusterizationAlg::StripClusterizationAlg(const std::string& name,
                                               ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode StripClusterizationAlg::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << "... ");

  ATH_CHECK(m_rdoContainerKey.initialize());
  ATH_CHECK(m_stripDetElStatus.initialize(!m_stripDetElStatus.empty()));
  ATH_CHECK(m_stripDetEleCollKey.initialize());
  ATH_CHECK(m_clusterContainerKey.initialize());
  ATH_CHECK(m_clusteringTool.retrieve());
  ATH_CHECK(detStore()->retrieve(m_idHelper, "SCT_ID"));

  bool disableSmry =
      !m_stripDetElStatus.empty() && !VALIDATE_STATUS_ARRAY_ACTIVATED;
  ATH_CHECK(m_summaryTool.retrieve(DisableTool{disableSmry}));

  if (!m_monTool.empty())
    ATH_CHECK(m_monTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode StripClusterizationAlg::execute(const EventContext& ctx) const {
  Monitored::Timer<std::chrono::milliseconds> timer("TIME_execute");
  Monitored::Group mon(m_monTool, timer);

  SG::ReadHandle<SCT_RDO_Container> rdoContainer =
      SG::makeHandle(m_rdoContainerKey, ctx);
  ATH_CHECK(rdoContainer.isValid());

  SG::WriteHandle<xAOD::StripClusterContainer> clusterHandle =
      SG::makeHandle(m_clusterContainerKey, ctx);

  SG::ReadHandle<InDet::SiDetectorElementStatus> status;
  if (!m_stripDetElStatus.empty()) {
    status = SG::makeHandle(m_stripDetElStatus, ctx);
    if (!status.isValid()) {
      ATH_MSG_FATAL("Invalid SiDetectorelementStatus");
      return StatusCode::FAILURE;
    }
  }

  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> stripDetEleHandle(
      m_stripDetEleCollKey, ctx);
  const InDetDD::SiDetectorElementCollection* stripDetEle(*stripDetEleHandle);
  if (!stripDetEleHandle.isValid() || stripDetEle == nullptr) {
    ATH_MSG_FATAL("Invalid SiDetectorElementCollection");
    return StatusCode::FAILURE;
  }

  std::unique_ptr<xAOD::StripClusterContainer> stripContainer =
      std::make_unique<xAOD::StripClusterContainer>();
  std::unique_ptr<xAOD::StripClusterAuxContainer> stripAuxContainer =
      std::make_unique<xAOD::StripClusterAuxContainer>();
  stripContainer->setStore(stripAuxContainer.get());

  // Reserve space, estimate of mean clusters to reduce re-allocations
  stripContainer->reserve(m_expectedClustersPerRDO.value() *
                          rdoContainer->size());

  for (const InDetRawDataCollection<SCT_RDORawData>* rdos : *rdoContainer) {
    if (rdos == nullptr or rdos->empty()) {
      ATH_MSG_DEBUG("No input strip RDOs for this container element");
      continue;
    }
    IdentifierHash idHash = rdos->identifyHash();

    bool goodModule = true;
    if (m_checkBadModules.value()) {
      if (!m_stripDetElStatus.empty()) {
        goodModule = status->isGood(idHash);
      } else {
        goodModule = m_summaryTool->isGood(idHash, ctx);
      }
    }
    VALIDATE_STATUS_ARRAY(
        m_checkBadModules.value() && !m_stripDetElStatus.empty(),
        status->isGood(idHash), m_summaryTool->isGood(idHash));

    if (!goodModule) {
      ATH_MSG_DEBUG("Strip module failed status check");
      continue;
    }

    ATH_CHECK(m_clusteringTool->clusterize(
        *rdos, *m_idHelper, stripDetEle->getDetectorElement(idHash),
        m_stripDetElStatus.empty() ? nullptr : status.cptr(),
        *stripContainer.get()));
  }

  ATH_CHECK(clusterHandle.record(std::move(stripContainer),
                                 std::move(stripAuxContainer)));
  return StatusCode::SUCCESS;
}

}  // namespace ActsTrk
