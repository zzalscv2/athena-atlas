/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef SICLUSTERIZATIONTOOL_HITSTOXAODCOPIER_H
#define SICLUSTERIZATIONTOOL_HITSTOXAODCOPIER_H

// Framework includes
#include <InDetIdentifier/PixelID.h>
#include "InDetIdentifier/SCT_ID.h"
#include <InDetRawData/PixelRDO_Container.h>
#include <InDetRawData/SCT_RDO_Container.h>
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
  SG::ReadHandleKey<PixelRDO_Container> m_pixelRdoContainerKey{
      this, "PixelRDOContainerKey", "ITkPixelRDOs"};
  SG::ReadHandleKey<SCT_RDO_Container> m_stripRdoContainerKey{
      this, "StripRDOContainerKey", "ITkStripRDOs"};

  SG::WriteHandleKey<xAOD::BaseContainer> m_pixelOutputKey{
      this, "PixelOutputCollectionKey", "PixelHits", "name of output container"};

  SG::WriteHandleKey<xAOD::BaseContainer> m_stripOutputKey{
      this, "StripOutputCollectionKey", "StripHits", "name of output container"};

  const PixelID* m_pixelIdHelper = nullptr;
  const SCT_ID* m_stripIdHelper = nullptr;


  StatusCode exportPixel(const EventContext& context) const;
  StatusCode exportStrip(const EventContext& context) const;

};
}  // namespace InDet
#endif  // SICLUSTERIZATIONTOOL_HITSTOXAODCOPIER_H
