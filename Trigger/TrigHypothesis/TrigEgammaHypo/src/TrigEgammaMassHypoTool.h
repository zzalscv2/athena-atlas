/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGEGAMMAHYPO_TRIGEGAMMAMASSHYPOTOOL_H
#define TRIGEGAMMAHYPO_TRIGEGAMMAMASSHYPOTOOL_H

/**************************************************************************
 **
 **   File: Trigger/TrigHypothesis/TrigEgammaHypo/src/TrigEgammaMassHypoTool.h
 **
 **   Description: - Hypothesis Tool: search for electron pairs with 
 **                invariant mass in some interval; intended for Z->ee
 **                - Modified from TrigL2DielectronMassHypo by R. Goncalo
 **                - Modified from TrigEFDielectronMassHypo by
 **                  Debottam Bakshi Gupta
 **
 **   Author: T. Hrynova  <thrynova@mail.cern.ch>
 **
 **   Created:   Nov 13 2009
 **
 **************************************************************************/ 

#include <string>
#include <vector>

#include "DecisionHandling/ComboHypoToolBase.h"

#include "xAODTracking/TrackParticleContainer.h"

#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"

#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"


/**
 * \class TrigEgammaMassHypoTool
 * \brief TrigEgammaMassHypoTool is a ComboHypoTool that calculates the inv mass
 * Apply inv mass cuts (upper and lower cut) to the two electrons and accepts the event if condition is 
 * satisfied
 *
 */


class TrigEgammaMassHypoTool:  public ComboHypoToolBase {

 public:
  
  TrigEgammaMassHypoTool(const std::string& type,
                    const std::string& name,
                    const IInterface* parent);

  virtual StatusCode initialize() override;


 private:
  
  virtual bool executeAlg(std::vector<LegDecision>& thecomb) const override;
  
  // flags
  Gaudi::Property< bool > m_acceptAll {this, "AcceptAll", false, "Ignore selection" };
  
  // cuts
  Gaudi::Property<float> m_lowerMassElectronClusterCut {this,"LowerMassElectronClusterCut", 50000.0, "Lower mass cut for electron-cluster pair"}; //!<  lower inv mass cut (e,cluster)
  Gaudi::Property<float> m_upperMassElectronClusterCut {this,"UpperMassElectronClusterCut", 130000.0, "Upper mass cut for electron-cluster pair"}; //!<  upper inv mass cut (e,cluster)
  
  // monitoring
  ToolHandle<GenericMonitoringTool> m_monTool { this, "MonTool", "", "Monitoring tool" };


}; // TRIGEGAMMAHYPO_TRIGEGAMMAMASSHYPOTOOL_H
#endif



