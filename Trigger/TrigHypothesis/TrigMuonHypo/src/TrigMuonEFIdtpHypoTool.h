/*
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONHYPO_TRIGMUONEFIDTPHYPOTOOL_H 
#define TRIGMUONHYPO_TRIGMUONEFIDTPHYPOTOOL_H

#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h" 
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "xAODMuon/MuonContainer.h"
#include "GaudiKernel/SystemOfUnits.h"

// include Muon SelectionTool for quality criteria 
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"

class StoreGateSvc;

class TrigMuonEFIdtpHypoTool: public ::AthAlgTool {
   enum { MaxNumberTools = 20 };  

public:
   
   TrigMuonEFIdtpHypoTool(const std::string& type, const std::string & name, const IInterface* parent);
   ~TrigMuonEFIdtpHypoTool() = default;
   
   struct MuonEFIdperfInfo {
      MuonEFIdperfInfo( TrigCompositeUtils::Decision* d, 
			const xAOD::Muon* m,
			const xAOD::TrackParticle* pt,
			const xAOD::TrackParticle* ftf,
			const TrigCompositeUtils::Decision* previousDecision )
	 : decision( d ), 
	   muon( m ),
	   ptTrack( pt ),
	   ftfTrack( ftf ),
	   previousDecisionIDs(TrigCompositeUtils::decisionIDs( previousDecision ).begin(), 
			       TrigCompositeUtils::decisionIDs( previousDecision ).end() )
      {}
      
      TrigCompositeUtils::Decision* decision;
      const xAOD::Muon* muon;
      const xAOD::TrackParticle* ptTrack;
      const xAOD::TrackParticle* ftfTrack;
      const TrigCompositeUtils::DecisionIDContainer previousDecisionIDs;
   };

   virtual StatusCode initialize() override;    
   StatusCode decide(std::vector<TrigMuonEFIdtpHypoTool::MuonEFIdperfInfo>& toolInput) const ;

 private:

   bool passedQualityCuts(const xAOD::Muon* muon) const;
   bool decideOnSingleObject(TrigMuonEFIdtpHypoTool::MuonEFIdperfInfo& input, size_t cutIndex) const;
   StatusCode inclusiveSelection(std::vector<TrigMuonEFIdtpHypoTool::MuonEFIdperfInfo>& toolInput) const;
   StatusCode multiplicitySelection(std::vector<TrigMuonEFIdtpHypoTool::MuonEFIdperfInfo>& toolInput) const;

   HLT::Identifier m_decisionId;
   // Properties:
   Gaudi::Property< bool > m_muonqualityCut {
      this, "MuonQualityCut", false, "Ignore selection" };
   Gaudi::Property< std::vector<std::vector<double>> > m_ptBins {
      this, "PtBins", { {0, 2.5} }, "Bins range of each pT threshold" };
   Gaudi::Property< std::vector<std::vector<double>> > m_ptThresholds {
      this, "PtThresholds", { {5.49*Gaudi::Units::GeV} }, "Track pT requirement ( separate threshold for each muon )" };
   Gaudi::Property< bool > m_acceptAll {
      this, "AcceptAll", false, "Ignore selection" };
   
   // Other members:   
   std::vector<size_t> m_bins={0};
   ToolHandle< GenericMonitoringTool > m_monTool { this, "MonTool", "", "Monitoring tool" };
   ToolHandle<CP::IMuonSelectionTool> m_muonSelTool{this, "MuonSelectionTool", "CP::MuonSelectionTool/MuonSelectionTool", "Tool for muon quality selection"};
};

#endif
