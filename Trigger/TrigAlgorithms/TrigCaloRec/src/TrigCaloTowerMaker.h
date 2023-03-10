// Hi Emacs ! this is  -*- C++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
 *
 * NAME:      TrigCaloTowerMaker
 * PACKAGE:   Trigger/TrigAlgorithms/TrigCaloRec
 *
 * AUTHOR:    P.A. Delsart
 * CREATED:   August 2006
 *
 *********************************************************************/
#ifndef TRIGCALOREC_TRIGCALOTOWERMAKER_H
#define TRIGCALOREC_TRIGCALOTOWERMAKER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CaloEvent/CaloTowerContainer.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "CaloUtils/CaloTowerBuilderToolBase.h"



class TrigCaloTowerMaker : public AthReentrantAlgorithm {

 public:

  /**  constructor */
  TrigCaloTowerMaker(const std::string& name, ISvcLocator* pSvcLocator);

  /** HLT method to initialize */
  virtual StatusCode initialize() override;

  virtual StatusCode execute(const EventContext& ctx) const override;
 private:



  /** Number of eta segments in which we divide the calorimeter */
  Gaudi::Property<unsigned int> m_nEtaTowers{this, "NumberOfEtaTowers", 50};

  /** Number of phi segments in which we divide the calorimeter */
  Gaudi::Property<unsigned int> m_nPhiTowers{this, "NumberOfPhiTowers", 64};

  /** Eta limits of the region where the towers are built */
  Gaudi::Property<double> m_minEta{this, "EtaMin", -2.5};
  Gaudi::Property<double> m_maxEta{this, "EtaMax", 2.5};
  // TODO find out meaning of these, seems relevan for RoI operation mode only
  Gaudi::Property<double> m_deta{this, "DeltaEta", 0.5};
  Gaudi::Property<double> m_dphi{this, "DeltaPhi", 0.5};


  ToolHandleArray<CaloTowerBuilderToolBase> m_towerMakerTools{this, "TowerMakerTools", {}};

  SG::ReadHandleKey<TrigRoiDescriptorCollection> m_inputRoiKey{ this,
      "RoIs",
      "rois",
      "input RoI collection name"};

  SG::ReadHandleKey<CaloCellContainer> m_inputCellsKey{ this,
      "Cells",
      "cells",
      "input CaloCellContainer "};

  SG::WriteHandleKey<CaloTowerContainer> m_outputTowerKey{ this,
      "CaloTowers",
      "calotowers",
      "output CaloTowerContainer"};

  SG::WriteHandleKey< INavigable4MomentumCollection> m_caloTowerNav4LinkKey{this,
      "CaloTowersSymLinkName",
      "calotowers",
      "Calo Towers SymLink Name - don't set this"};



  /** To help structure Tower container */
  bool m_includeFcal;

  ToolHandle< GenericMonitoringTool > m_monTool { this, "MonTool", "", "Monitoring tool" };

};
#endif
