/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/*************************************************************************************************
 **
 **   File: Trigger/TrigHypothesis/TrigEgammaHypo/src/combos/TrigEgammaTopoHypoTool.h
 **
 **   Description: - Hypothesis Tool: search for electron/photon pairs with
 **                invariant mass or deltaPhi in some interval; intended for e.g Z->ee, H->gg, etc
 **
 **
 **   Author: Debottam Bakshi Gupta <debottam.bakshi.gupta@cern.ch>
 **
 **   Created:   May 15 2021
 **
 **************************************************************************************************/

#include "TrigEgammaTopoHypoTool.h"
#include "xAODBase/IParticleContainer.h"

namespace TCU = TrigCompositeUtils;

TrigEgammaTopoHypoTool::TrigEgammaTopoHypoTool(const std::string& type, const std::string& name, const IInterface* parent)
    : ComboHypoToolBase(type, name, parent) {}


StatusCode TrigEgammaTopoHypoTool::initialize()
{
  ATH_MSG_DEBUG("AcceptAll            = " << m_acceptAll );
  ATH_MSG_DEBUG("ApplyMassCut         = " << m_applyMassCut );
  ATH_MSG_DEBUG("ApplyDPhiCut         = " << m_applyDPhiCut );
  ATH_MSG_DEBUG("LowerMassCut         = " << m_lowerMassEgammaClusterCut );
  ATH_MSG_DEBUG("UpperMassCut         = " << m_upperMassEgammaClusterCut );

  if ( not m_monTool.name().empty() ) {
        ATH_CHECK( m_monTool.retrieve() );
        ATH_MSG_DEBUG("m_monTool name: " << m_monTool);
  }

  return StatusCode::SUCCESS;
}

bool TrigEgammaTopoHypoTool::executeAlg(std::vector<LegDecision> &combination) const {
  auto massOfAccepted = Monitored::Scalar( "MassOfAccepted"   , -1.0 );
  auto dphiOfAccepted = Monitored::Scalar( "DphiOfAccepted"   , -99 );
  auto monitorIt    = Monitored::Group( m_monTool, massOfAccepted, dphiOfAccepted);
//retrieve the elements
  std::vector<ElementLink<xAOD::IParticleContainer>> selected_electrons;
  for (auto el: combination){
    ATH_MSG_DEBUG("found Combination: "<<combination);
    auto EL= el.second;    
    auto electronLink = TCU::findLink<xAOD::IParticleContainer>( *EL, TCU::featureString() ).link;
    selected_electrons.push_back(electronLink);
  }
  auto electronLink1=selected_electrons[0];
  auto electronLink2=selected_electrons[1];
  TLorentzVector hlv1 = (*electronLink1)->p4();
  TLorentzVector hlv2 = (*electronLink2)->p4();
  massOfAccepted = (hlv1+hlv2).M();
  dphiOfAccepted = hlv1.DeltaPhi(hlv2);
  
  ATH_MSG_DEBUG("Found two Electrons/Photons with deltaPhi " <<dphiOfAccepted);

  ATH_MSG_DEBUG("Found two Electrons/Photons with mass " <<massOfAccepted);

  // apply the cut
  bool pass=true;
 
  if(m_acceptAll)
    ATH_MSG_DEBUG("Applying no selections");

  if(m_applyMassCut){
   bool FailsMassCut = massOfAccepted<m_lowerMassEgammaClusterCut;
   if (m_upperMassEgammaClusterCut>=0) FailsMassCut |= massOfAccepted>m_upperMassEgammaClusterCut;
   if (FailsMassCut){
    pass = false;
    ATH_MSG_DEBUG("Combination failed mass cut: " << massOfAccepted << " not in [" << m_lowerMassEgammaClusterCut << "," <<  m_upperMassEgammaClusterCut << "]");
   }
   else
     ATH_MSG_DEBUG( " Invariant mass " << massOfAccepted << " is  within [" <<m_lowerMassEgammaClusterCut<< "," << m_upperMassEgammaClusterCut << "] This selection passed! ");
  }

  if (m_applyDPhiCut){
   bool FailsDPhiCut = dphiOfAccepted < m_thresholdDPhiCut;
   if (FailsDPhiCut){
    ATH_MSG_DEBUG("Combination failed deltaPhi cut: " << dphiOfAccepted << " is below " << m_thresholdDPhiCut);
    pass = false;
   }
   else
    ATH_MSG_DEBUG( " deltaPhi " << dphiOfAccepted << " is above the threshold "<<m_thresholdDPhiCut<<" This selection passed! ");
  }
    
  return pass;

}
