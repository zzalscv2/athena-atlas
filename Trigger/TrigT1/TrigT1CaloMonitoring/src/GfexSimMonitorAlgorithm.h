/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_GFEXSIMMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_GFEXSIMMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"

#include "xAODTrigger/gFexSRJetRoIContainer.h"
#include "xAODTrigger/gFexLRJetRoIContainer.h"
#include "xAODTrigger/gFexTauRoIContainer.h"
#include "xAODTrigger/gFexFwdElRoIContainer.h"
#include "xAODTrigger/gFexMETRoIContainer.h"
#include "xAODTrigger/gFexSumETRoIContainer.h"

#include "xAODTrigL1Calo/gFexTowerContainer.h"

class GfexSimMonitorAlgorithm : public AthMonitorAlgorithm {
    public:
        GfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
        virtual ~GfexSimMonitorAlgorithm()=default;
        virtual StatusCode initialize() override;
        virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

    private:

        StringProperty m_Grouphist   {this,"Grouphist"       ,"GfexSimMonitor","group name for histograming"};
        
        ToolHandle<GenericMonitoringTool> m_monTool{this,"jFEXMonTool","","Monitoring tool"};
        void  genError(const std::string& location, const std::string& title) const;
        
        
        // container keys for jfex input data
        SG::ReadHandleKey<xAOD::gFexTowerContainer> m_gFexTowerKey{this, "gFexTowerContainer","L1_gFexDataTowers","SG key of the input gFex Tower container"};

        // container keys for Data tobs
        SG::ReadHandleKey< xAOD::gFexSRJetRoIContainer > m_data_key_jJ   {this,"gFexSRJetRoIContainer","L1_gFexSRJetRoI","SG key of the gFex SR Jet Roi container"};
        SG::ReadHandleKey< xAOD::gFexLRJetRoIContainer > m_data_key_jLJ  {this,"gFexLRJetRoIContainer","L1_gFexLRJetRoI","SG key of the gFex LR Jet Roi container"};
        SG::ReadHandleKey< xAOD::gFexTauRoIContainer   > m_data_key_jTau {this,"gFexTauRoIContainer"  ,"L1_gFexTauRoI"  ,"SG key of the gFex Tau Roi container"   };
        SG::ReadHandleKey< xAOD::gFexFwdElRoIContainer > m_data_key_jEM  {this,"gFexFwdElRoIContainer","L1_gFexFwdElRoI","SG key of the gFex EM Roi container"    };
        SG::ReadHandleKey< xAOD::gFexMETRoIContainer   > m_data_key_jXE  {this,"gFexMETRoIContainer"  ,"L1_gFexMETRoI"  ,"SG key of the gFex MET Roi container"   };
        SG::ReadHandleKey< xAOD::gFexSumETRoIContainer > m_data_key_jTE  {this,"gFexSumETRoIContainer","L1_gFexSumETRoI","SG key of the gFex SumEt Roi container" };

        // container keys for Simulation tobs
        SG::ReadHandleKey< xAOD::gFexSRJetRoIContainer > m_simu_key_jJ   {this,"gFexSRJetRoISimContainer","L1_gFexSRJetRoISim","SG key of the Sim gFex SR Jet Roi container"};
        SG::ReadHandleKey< xAOD::gFexLRJetRoIContainer > m_simu_key_jLJ  {this,"gFexLRJetRoISimContainer","L1_gFexLRJetRoISim","SG key of the Sim gFex LR Jet Roi container"};
        SG::ReadHandleKey< xAOD::gFexTauRoIContainer   > m_simu_key_jTau {this,"gFexTauRoISimContainer"  ,"L1_gFexTauRoISim"  ,"SG key of the Sim gFex Tau Roi container"   };
        SG::ReadHandleKey< xAOD::gFexFwdElRoIContainer > m_simu_key_jEM  {this,"gFexFwdElRoISimContainer","L1_gFexFwdElRoISim","SG key of the Sim gFex EM Roi container"    };
        SG::ReadHandleKey< xAOD::gFexMETRoIContainer   > m_simu_key_jXE  {this,"gFexMETRoISimContainer"  ,"L1_gFexMETRoISim"  ,"SG key of the Sim gFex MET Roi container"   };
        SG::ReadHandleKey< xAOD::gFexSumETRoIContainer > m_simu_key_jTE  {this,"gFexSumETRoISimContainer","L1_gFexSumETRoISim","SG key of the Sim gFex SumEt Roi container" };

        
        template <typename T> std::vector<std::array<float,5> > tobMatching(const SG::ReadHandleKey<T>& tobs1Key, const SG::ReadHandleKey<T>& tobs2Key, const EventContext& ctx, std::vector< std::array<float,5> > & unmatched ) const;
        void fillHist(const std::string & pkg, const std::string & item, const std::string & input, const std::string & LB, const bool fillError, std::vector< std::array<float,5> > & elem ) const;
        
        template <typename T> std::vector<std::array<int,3> >  tobMatchingGlobals(const SG::ReadHandleKey<T>& tobs1Key, const SG::ReadHandleKey<T>& tobs2Key, const EventContext& ctx, std::vector< std::array<int,3> > & unmatched ) const;
        void fillHistGlobals(const std::string & pkg, const std::string & item, const std::string & input, const std::string & LB, const bool fillError, std::vector< std::array<int,3> > & elem ) const;
};       
#endif
