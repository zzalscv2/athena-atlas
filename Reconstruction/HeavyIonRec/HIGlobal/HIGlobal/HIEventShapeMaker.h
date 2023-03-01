/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef __HIEVENTSHAPEMAKER_H__
#define __HIEVENTSHAPEMAKER_H__

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODHIEvent/HIEventShapeContainer.h"
#include "xAODHIEvent/HIEventShapeAuxContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"
#include "IHIEventShapeFiller.h"
#include "HIEventUtils/HIEventShapeSummaryTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <string>
#include <iostream> 
#include <sstream> 
#include <memory>

class CaloCellContainer;

class HIEventShapeMaker : public AthReentrantAlgorithm
{
public:

  /** Standard Athena-Algorithm Constructor */
  HIEventShapeMaker(const std::string& name, ISvcLocator* pSvcLocator);
  /** Default Destructor */
  ~HIEventShapeMaker() = default;


  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

private:
  SG::ReadHandleKey<xAOD::CaloClusterContainer>    m_towerContainerKey{ this, "InputTowerKey"     , "CombinedTower"   , "InputTowerKey" }; //TowerContainer key
  SG::ReadHandleKey<INavigable4MomentumCollection> m_naviContainerKey{ this, "NaviTowerKey"     ,  "CombinedTower"   , "InputTowerKey for Navigable Momentum collection" };
  SG::ReadHandleKey<CaloCellContainer>             m_cellContainerKey{ this, "InputCellKey"      , ""         , "InputCellKey" }; //CaloCellContainer key
  SG::ReadHandleKey<xAOD::HIEventShapeContainer>   m_readExistingKey{ this, "ReadExistingKey", "HIEventShapeContainer", "Read key to retrieve the shape im_summary_only is true" };
  SG::WriteHandleKey<xAOD::HIEventShapeContainer>  m_outputKey{ this, "OutputContainerKey"    , "HIEventShapeContainer", "Output Container Key" }; //Name of output container key
  SG::WriteHandleKey<xAOD::HIEventShapeContainer>  m_summaryKey{ this, "SummaryContainerKey"   , "", "SummaryContainerKey" };

  Gaudi::Property< bool > m_summaryOnly{ this, "SummaryOnly", false, "Summary Only boolean switch" };

  ToolHandle<IHIEventShapeFiller> m_HIEventShapeFillerTool{ this, "HIEventShapeFillerTool", "HIEventShapeFillerTool", "HIEventShapeFillerTool" };
  ToolHandle<IHIEventShapeSummaryTool> m_summaryTool{ this, "SummaryTool", "HIEventShapeSummaryTool", "Handle to IHIEventShapeSummaryTool" };

  std::string PrintHIEventShapeContainer(const xAOD::HIEventShapeContainer* Container) const;
};

#endif
