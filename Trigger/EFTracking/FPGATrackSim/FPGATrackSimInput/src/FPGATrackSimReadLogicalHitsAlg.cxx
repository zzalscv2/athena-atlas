#include "FPGATrackSimReadLogicalHitsAlg.h"
#include "FPGATrackSimObjects/FPGATrackSimTowerInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventOutputHeader.h"


FPGATrackSimReadLogicalHitsAlg::FPGATrackSimReadLogicalHitsAlg (const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator){}

StatusCode FPGATrackSimReadLogicalHitsAlg::initialize()
{
   ATH_CHECK( m_readOutputTool.retrieve());
   m_event=0;  
   return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimReadLogicalHitsAlg::execute() 
{
  FPGATrackSimLogicalEventInputHeader  eventInputHeader_1st;
  FPGATrackSimLogicalEventInputHeader  eventInputHeader_2nd;
  FPGATrackSimLogicalEventOutputHeader eventOutputHeader;
  bool last = false;
  ATH_CHECK(m_readOutputTool->readData(&eventInputHeader_1st, &eventInputHeader_2nd, &eventOutputHeader, last) );
  if (last) return StatusCode::SUCCESS;
  ATH_MSG_DEBUG (eventInputHeader_1st);
  m_event++;

  return StatusCode::SUCCESS;
}

