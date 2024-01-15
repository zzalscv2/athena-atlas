/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SPACEPOINT_PERSISTIFICATION_H
#define SPACEPOINT_PERSISTIFICATION_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

namespace InDet {

  class SpacePointPersistification :
    public AthReentrantAlgorithm {
  public:
    /// Constructor with parameters:
    SpacePointPersistification(const std::string &name, ISvcLocator *pSvcLocator);
    
    //@name Usual algorithm methods
    //@{
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    //@{
    
  private:
    SG::ReadHandleKey< xAOD::SpacePointContainer > m_inSpacepoints {this, "InputSpacePointsName", "",
	"Input space points container"};

    SG::WriteDecorHandleKey< xAOD::SpacePointContainer > m_clusterLink {this, "ClusterLinkName", "",
      "name of the link to the xAOD::Cluster collection"};
  };

}

#endif

