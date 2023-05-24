/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "HitsToxAODCopier.h"
namespace InDet {
HitsToxAODCopier::HitsToxAODCopier(const std::string& name,
                                   ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}


StatusCode HitsToxAODCopier::initialize() {
  ATH_CHECK(m_pixelRDOTool.retrieve());
  ATH_CHECK(m_rdoContainerKey.initialize());
  ATH_CHECK(m_outputKey.initialize());
  ATH_CHECK(detStore()->retrieve(m_idHelper, "PixelID"));

  return StatusCode::SUCCESS;
}

StatusCode HitsToxAODCopier::execute(const EventContext& context) const {

  SG::ReadHandle<PixelRDO_Container> rdoContainer =
      SG::makeHandle(m_rdoContainerKey, context);
  ATH_CHECK(rdoContainer.isValid());

  auto output = std::make_unique<xAOD::BaseContainer>();
  auto outputAux = std::make_unique<xAOD::AuxContainerBase>();
  output->setStore(outputAux.get());
  static const SG::AuxElement::Accessor<int> col("col");
  static const SG::AuxElement::Accessor<int> row("row");
  static const SG::AuxElement::Accessor<int> tot("tot");
  static const SG::AuxElement::Accessor<int> eta_module("eta_module");
  static const SG::AuxElement::Accessor<int> phi_module("phi_module");
  static const SG::AuxElement::Accessor<int> layer_disk("layer_disk");
  static const SG::AuxElement::Accessor<int> barrel_ec("barrel_ec");
  static const SG::AuxElement::Accessor<uint64_t> id("detid");

  for (auto collection : *rdoContainer) {
    const InDetDD::SiDetectorElement* element =
        m_pixelRDOTool->checkCollection(*collection, context);
    if (element != nullptr) {
      std::vector<InDet::UnpackedPixelRDO> hits =
          m_pixelRDOTool->getUnpackedPixelRDOs(*collection, *m_idHelper,
                                               element, context);
      for (auto hit : hits) {
        auto* item = new SG::AuxElement();
        output->push_back(item);
        col(*item) = hit.COL;
        row(*item) = hit.ROW;
        tot(*item) = hit.TOT;
        eta_module(*item) = m_idHelper->eta_module(hit.ID);
        phi_module(*item) = m_idHelper->phi_module(hit.ID);
        layer_disk(*item) = m_idHelper->layer_disk(hit.ID);
        barrel_ec(*item) = m_idHelper->barrel_ec(hit.ID);
        id(*item) = hit.ID.get_compact();
      }
    }
  }
  auto outputHandle = SG::makeHandle(m_outputKey, context);
  ATH_CHECK(outputHandle.record(std::move(output), std::move(outputAux)));

  return StatusCode::SUCCESS;
}

}  // namespace InDet