/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SICLUSTERIZATIONTOOL_HITSTOXAODCOPIER_H
#define SICLUSTERIZATIONTOOL_HITSTOXAODCOPIER_H

// Framework includes
#include <InDetIdentifier/PixelID.h>
#include <InDetRawData/PixelRDO_Container.h>
#include <StoreGate/ReadHandleKey.h>
#include <StoreGate/WriteHandleKey.h>
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "SiClusterizationTool/PixelRDOTool.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODCore/BaseContainer.h"

// STL includes
#include <string>

/**
 * @class HitsToxAODCopier
 * @brief Algorithm to copy RDO hits into xAOD writable format
 **/
namespace InDet {
class HitsToxAODCopier : public AthReentrantAlgorithm {
 public:
  HitsToxAODCopier(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~HitsToxAODCopier() override = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;

 private:
  ToolHandle<InDet::PixelRDOTool> m_pixelRDOTool{this, "PixelRDOTool",
                                                 "InDet::PixelRDOTool"};
  SG::ReadHandleKey<PixelRDO_Container> m_rdoContainerKey{
      this, "PixelRDOContainerKey", "PixelRDOs"};
  SG::WriteHandleKey<xAOD::BaseContainer> m_outputKey{
      this, "OutputCollectionKey", "PixelHits", "name of output container"};

  const PixelID* m_idHelper = nullptr;
};
}  // namespace InDet
#endif  // SICLUSTERIZATIONTOOL_HITSTOXAODCOPIER_H
