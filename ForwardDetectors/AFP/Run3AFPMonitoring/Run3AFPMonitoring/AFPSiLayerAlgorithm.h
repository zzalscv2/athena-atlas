/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFPSILAYERALGORITHM_H
#define AFPSILAYERALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
//#include "TrigAnalysisInterfaces/IBunchCrossingTool.h"
#include "xAODForward/AFPSiHitContainer.h"
#include "xAODForward/AFPSiHit.h"
//#include "LumiBlockData/BunchCrossingCondData.h"

#include "TRandom3.h"

class BunchCrossingCondData;

class AFPSiLayerAlgorithm : public AthMonitorAlgorithm {
public:
	AFPSiLayerAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
	virtual ~AFPSiLayerAlgorithm();
	virtual StatusCode initialize() override;
	virtual StatusCode fillHistograms( const EventContext& ctx ) const override;
	virtual StatusCode execute(const EventContext& ctx) const override;
	
private:
<<<<<<< HEAD
	std::map<std::string,std::map<std::string,int>> m_HitmapGroups;
	std::map<std::string,int> m_TrackGroup; 
	SG::ReadHandleKey<xAOD::AFPSiHitContainer> m_afpHitContainerKey;
//	ToolHandle<Trig::IBunchCrossingTool> m_bcTool;
	//SG::ReadCondHandleKey<BunchCrossingCondData>  m_bunchCrossingKey{this, "BunchCrossingKey", "BunchCrossingData", "Key BunchCrossing CDO" };
	//BunchCrossingCondData m_bunchCrossingCondTool;
	
	//SG::ReadCondHandleKey<BunchCrossingCondData> m_bunchCrossingCondTool {this, "BunchCrossingCondDataKey", "BunchCrossingData" ,"SG Key of BunchCrossing CDO"};
	//SG::ReadHandleKey<BunchCrossingCondData> m_bunchCrossingCondTool;
	//ToolHandle<Trig::IBunchCrossingTool> m_bunchCrossingTool;
	//ToolHandle<Trig::IBunchCrossingTool> m_bunchCrossingTool{this, "BunchCrossingTool",""};
	
=======
    std::map<std::string,std::map<std::string,int>> m_HitmapGroups;
    std::map<std::string,int> m_TrackGroup; 
    SG::ReadHandleKey<xAOD::AFPSiHitContainer> m_afpHitContainerKey;
>>>>>>> upstream/master

protected:
	std::vector<std::string> m_pixlayers = { "P0", "P1", "P2", "P3"};
	std::vector<std::string> m_stationnames = { "farAside", "nearAside" , "nearCside" , "farCside"};

};
#endif

