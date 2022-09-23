/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_MUHADCLUSTERSUBSTRUCTUREBUILDER_H
#define DITAUREC_MUHADCLUSTERSUBSTRUCTUREBUILDER_H

#include <vector>
#include "xAODCaloEvent/CaloVertexedTopoCluster.h"

#include "tauRecTools/TauRecToolBase.h"
#include "CxxUtils/fpcompare.h"
#include "xAODTau/TauJet.h"
#include "TLorentzVector.h"

/**
 *  @brief merged and modified of tauRecTools/SubstructureVariables, tauRecTools/TauCommonCalcVars, tauRecTools/TauCellVariables
 *  @author Lianyou SHAN
 *
 *  **/

class MuHadClusterSubStructVariables : public TauRecToolBase
{
    public: 
        
        static const double DEFAULT;

        MuHadClusterSubStructVariables(const std::string& name="MuHadClusterSubStructVariables");
	ASG_TOOL_CLASS2(MuHadClusterSubStructVariables, TauRecToolBase, ITauToolBase)
	
        ~MuHadClusterSubStructVariables() override = default ;

        StatusCode execute(xAOD::TauJet& pTau) override ;
        StatusCode initialize() override ;

    private:

        TLorentzVector calculateTauCentroid(int nConst, const std::vector< xAOD::CaloVertexedTopoCluster >& );

	std::string m_configPath;
        double m_maxPileUpCorrection; 
        double m_pileUpAlpha;         //  slope of the pileup correction
        double m_cellCone;  // outer cone for cells used in calculations
        
        bool m_doVertexCorrection;

        bool m_onlyCore ;
        std::string m_MuonClusName ;
        std::string m_MuonTrkPartName ;
        bool m_classifierDone ;
        xAOD::TauJetParameters::TauTrackFlag m_isoTrackType;
};
#endif
