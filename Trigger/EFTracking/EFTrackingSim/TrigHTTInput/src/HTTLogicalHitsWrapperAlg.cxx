#include "HTTLogicalHitsWrapperAlg.h"
#include "TrigHTTObjects/HTTEventInputHeader.h"
#include "TrigHTTObjects/HTTLogicalEventInputHeader.h"
#include "TrigHTTObjects/HTTLogicalEventOutputHeader.h"
#include "TrigHTTObjects/HTTTowerInputHeader.h"
#include "TrigHTTObjects/HTTCluster.h"
#include "TrigHTTInput/IHTTEventInputHeaderTool.h"
#include "TrigHTTInput/IHTTEventOutputHeaderTool.h"

HTTLogicalHitsWrapperAlg::HTTLogicalHitsWrapperAlg (const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator)
{}


StatusCode HTTLogicalHitsWrapperAlg::initialize()
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

StatusCode HTTLogicalHitsWrapperAlg::BookHistograms(){
  return StatusCode::SUCCESS;
}


StatusCode HTTLogicalHitsWrapperAlg::execute()
{
  HTTEventInputHeader         eventHeader;
  
  bool last = false;
  ATH_CHECK (m_hitInputTool->readData(&eventHeader, last));
  if (last) return StatusCode::SUCCESS;

  ATH_MSG_DEBUG (eventHeader);

  // Map hits:
  HTTLogicalEventInputHeader  logicEventHeader_1st;
  ATH_CHECK(m_hitMapTool->convert(1, eventHeader, logicEventHeader_1st));
  ATH_MSG_DEBUG (logicEventHeader_1st);

  // clustering:
  if (m_Clustering) {
    std::vector<HTTCluster> clusters;
    ATH_CHECK(m_clusteringTool->DoClustering(logicEventHeader_1st, clusters));
    ATH_MSG_INFO ("Ending with " << clusters.size() << " clusters");
  }

  HTTLogicalEventInputHeader  logicEventHeader_2nd;//fake empty
  HTTLogicalEventOutputHeader logicEventOutputHeader;
  ATH_CHECK (m_writeOutputTool->writeData(&logicEventHeader_1st, &logicEventHeader_2nd, &logicEventOutputHeader));

  return StatusCode::SUCCESS;
}

