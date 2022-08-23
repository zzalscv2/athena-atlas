/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef HIGLOBAL_SPACEPOINTCOPIER_H
#define HIGLOBAL_SPACEPOINTCOPIER_H

#include "TrkSpacePoint/SpacePointContainer.h"
#include "xAODCore/BaseContainer.h"
#include "xAODCore/AuxContainerBase.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include <string>

/**
 * @class SpacePointCopier
 * @brief 
 **/
class SpacePointCopier : public AthReentrantAlgorithm {
public:
  SpacePointCopier(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~SpacePointCopier() override;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& context) const override;
  virtual StatusCode finalize() override;

private:
  SG::ReadHandleKey<SpacePointContainer> m_pixelSPKey{this, "PixelsSPKey", "PixelSpacePoints", "Pixel SP collection name"};
  SG::ReadHandleKey<SpacePointContainer> m_SCTSPKey{this, "SCTSPKey", "SCT_SpacePoints", "SCT SP collection name"};
  SG::WriteHandleKey<xAOD::BaseContainer> m_outputKey{this, "OutputCollectionKey", "SpacePoints", "name of output container"};

  Gaudi::Property<size_t> m_maxPixSP{this, "maxPixelSP", std::numeric_limits<size_t>::max(), "Skip conversion in events that have more than this pixel SP"};
  Gaudi::Property<size_t> m_maxSCTSP{this, "maxSCTSP", std::numeric_limits<size_t>::max(), "Skip conversion in events that have more than this SCT SP"};
  Gaudi::Property<size_t> m_maxTotalSP{this, "maxTotalSP", std::numeric_limits<size_t>::max(), "Skip conversion in events that have more than this totasl SP"};

};

#endif // HIGLOBAL_SPACEPOINTCOPIER_H
