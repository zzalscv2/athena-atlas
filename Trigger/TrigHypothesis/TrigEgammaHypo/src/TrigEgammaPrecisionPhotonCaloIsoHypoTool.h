/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGEGAMMAHYPO_TRIGPHOTONCALOISOHYPOTOOLINC_H
#define TRIGEGAMMAHYPO_TRIGPHOTONCALOISOHYPOTOOLINC_H 1

/*
 * =====================================================================================
 *
 *       Filename:  TrigEgammaPrecisionPhotonCaloIsoHypoTool.h
 *
 *    Description:  Hypo tool header for Calorimeter isolation applied HLT precision step for photon triggers
 *
 *        Created:  08/09/2022 11:19:55 AM
 *
 *         Author:  Fernando Monticelli (), Fernando.Monticelli@cern.ch
 *   Organization:  UNLP/IFLP/CONICET
 *
 * =====================================================================================
 */


#include "xAODEventInfo/EventInfo.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "ITrigEgammaPrecisionPhotonCaloIsoHypoTool.h"
#include "StoreGate/ReadDecorHandle.h"

/**
 * @class Implementation of the Egamma selection for Photons
 * @brief 
 **/

class TrigEgammaPrecisionPhotonCaloIsoHypoTool : public extends<AthAlgTool, ITrigEgammaPrecisionPhotonCaloIsoHypoTool> { 
  public: 
    TrigEgammaPrecisionPhotonCaloIsoHypoTool( const std::string& type,const std::string& name, const IInterface* parent );

    virtual StatusCode initialize() override;

    virtual StatusCode decide( std::vector<ITrigEgammaPrecisionPhotonCaloIsoHypoTool::PhotonInfo>& input )  const override;

    virtual bool decide( const ITrigEgammaPrecisionPhotonCaloIsoHypoTool::PhotonInfo& i ) const override;

  private:
    HLT::Identifier m_decisionId;
    
//    //Calorimeter electron ID  cuts
	Gaudi::Property< std::vector<float> > m_etabin { this, "EtaBins", {} , "Bins of eta" }; //!<  selection variable for PRECISION calo selection:eta bins
    Gaudi::Property< std::vector<float> > m_RelEtConeCut { this, "RelEtConeCut", {999., 999., 999.} , "Calo isolation cut on etcone20" };
    Gaudi::Property< std::vector<float> > m_RelTopoEtConeCut { this, "RelTopoEtConeCut", {999., 999., 999.}, "Calo isolation cut in [TopoEtcone20/pt, TopoEtcone30/pt, TopoEtcone40/pt]" };
    Gaudi::Property< std::vector<float> > m_CutOffset { this, "Offset", {0., 0., 0.} , "Calo isolation offset cut in [(Topo)Etcone20/pt, (Topo)Etcone30/pt, (Topo)Etcone40/pt]" };
    Gaudi::Property< std::string >        m_pidName {this, "PidName", "", "Pid name"}; 
   
    /* Accept all in case of noiso instance */	
    Gaudi::Property< bool >        m_acceptAll {this, "AcceptAll", false, "Force accept the event"}; 
    /* monitoring */
    ToolHandle< GenericMonitoringTool >   m_monTool { this, "MonTool", "", "Monitoring tool" };
  
    /*Luminosity info*/
    SG::ReadDecorHandleKey<xAOD::EventInfo> m_avgMuKey { this, "averageInteractionsPerCrossingKey", "EventInfo.averageInteractionsPerCrossing", "Decoration for Average Interaction Per Crossing" };
  
  
    int findCutIndex( float eta ) const;
}; 

#endif //> !TRIGEGAMMAHYPO_TRIGPRECISIONPHOTONHYPOTOOL_H
