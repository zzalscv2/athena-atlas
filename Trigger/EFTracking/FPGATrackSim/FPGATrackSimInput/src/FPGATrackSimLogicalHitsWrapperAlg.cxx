#include "FPGATrackSimLogicalHitsWrapperAlg.h"
#include "FPGATrackSimObjects/FPGATrackSimEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventOutputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimTowerInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimInput/IFPGATrackSimEventInputHeaderTool.h"
#include "FPGATrackSimInput/IFPGATrackSimEventOutputHeaderTool.h"

FPGATrackSimLogicalHitsWrapperAlg::FPGATrackSimLogicalHitsWrapperAlg (const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator)
{}


StatusCode FPGATrackSimLogicalHitsWrapperAlg::initialize()
{
  ATH_CHECK( m_hitInputTool.retrieve());
  ATH_CHECK( m_writeOutputTool.retrieve());
  ATH_CHECK( m_hitMapTool.retrieve());
  
  if (m_Clustering ) {
    ATH_CHECK( m_clusteringTool.retrieve());
    ATH_MSG_INFO ("Clustering is enabled");
  }

  return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimLogicalHitsWrapperAlg::BookHistograms(){
  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimLogicalHitsWrapperAlg::execute()
{
  FPGATrackSimEventInputHeader         eventHeader;
  
  bool last = false;
  ATH_CHECK (m_hitInputTool->readData(&eventHeader, last));
  if (last) return StatusCode::SUCCESS;

  ATH_MSG_DEBUG (eventHeader);

  // Map hits:
  FPGATrackSimLogicalEventInputHeader  logicEventHeader_1st;
  ATH_CHECK(m_hitMapTool->convert(1, eventHeader, logicEventHeader_1st));
  ATH_MSG_DEBUG (logicEventHeader_1st);

  // clustering:
  if (m_Clustering) {
    std::vector<FPGATrackSimCluster> clusters;
    ATH_CHECK(m_clusteringTool->DoClustering(logicEventHeader_1st, clusters));
    ATH_MSG_INFO ("Ending with " << clusters.size() << " clusters");
  }

  FPGATrackSimLogicalEventInputHeader  logicEventHeader_2nd;//fake empty
  FPGATrackSimLogicalEventOutputHeader logicEventOutputHeader;
  ATH_CHECK (m_writeOutputTool->writeData(&logicEventHeader_1st, &logicEventHeader_2nd, &logicEventOutputHeader));

  return StatusCode::SUCCESS;
}

