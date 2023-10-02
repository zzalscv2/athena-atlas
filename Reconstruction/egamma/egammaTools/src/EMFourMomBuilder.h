/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EGAMMATOOLS_EMFOURMOMBUILDER_H
#define EGAMMATOOLS_EMFOURMOMBUILDER_H

/**
  @class EMFourMomBuilder
  sets the fourmomentum : energy is taken from the cluster and angles either
  from tracking or cluster. In case the egamma object is a conversion :
  - if it is a single/double track conversion with TRT only tracks
  - (i.e no more than 4 hits in pixel+SCT), take the cluster info, not the track
  info
  -
  @author  Zerwas,Anastopoulos
  */

#include "egammaInterfaces/IEMFourMomBuilder.h"
#include "egammaBaseTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ToolHandle.h"

#include "xAODCaloEvent/CaloClusterFwd.h"
#include "xAODEgamma/EgammaFwd.h"
#include "xAODEgamma/ElectronFwd.h"
#include "xAODEgamma/PhotonFwd.h"

#include "EventPrimitives/EventPrimitives.h"
#include "egammaRecEvent/egammaRec.h"
#include "egammaUtils/eg_resolution.h"

class EMFourMomBuilder final
  : public egammaBaseTool
  , virtual public IEMFourMomBuilder
{

public:
  /** @brief Default constructor*/
  EMFourMomBuilder(const std::string& type,
                   const std::string& name,
                   const IInterface* parent);

  /** @brief Destructor*/
  virtual ~EMFourMomBuilder() = default;

  /** @brief initialize method*/
  StatusCode initialize() override final;
  /** @brief execute method*/
  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::Egamma* eg) const override final;

private:

  /** @brief Resolution configuration*/
  Gaudi::Property<std::string> m_ResolutionConfiguration{
    this,
    "ResolutionConfiguration",
    "run2_pre",
    "Resolution Configuration"
  };

  std::unique_ptr<eg_resolution> m_eg_resol;
};

#endif
