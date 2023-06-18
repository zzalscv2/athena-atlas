#include "FPGATrackSimDumpOutputStatAlg.h"

#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventOutputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "TH2F.h"


FPGATrackSimDumpOutputStatAlg::FPGATrackSimDumpOutputStatAlg (const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator) {}


StatusCode FPGATrackSimDumpOutputStatAlg::initialize()
{
  ATH_MSG_INFO ( "FPGATrackSimDumpOutputStatAlg::initialize()");  
  ATH_CHECK( m_readOutputTool.retrieve());
  ATH_CHECK( m_writeOutputTool.retrieve());

  // eventually add some histograms for monitoring
  ATH_CHECK( BookHistograms());
  
   // TO DO: must register the histograms to the TFile in the Tool
  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimDumpOutputStatAlg::BookHistograms(){
  //m_hits_r_vs_z = new TH2F("h_hits_r_vs_z", "r/z ITK hit map; z[mm];r[mm]", 3500, 0., 3500., 450, 0., 450.);
  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimDumpOutputStatAlg::execute() {
  ATH_MSG_DEBUG ("Running on event ");   
  
  FPGATrackSimLogicalEventInputHeader  eventInputHeader_1st;
  FPGATrackSimLogicalEventInputHeader  eventInputHeader_2nd;
  FPGATrackSimLogicalEventOutputHeader eventOutputHeader;

  bool last=false;
  ATH_CHECK (m_readOutputTool->readData(&eventInputHeader_1st, &eventInputHeader_2nd, &eventOutputHeader, last));
  if (last) return StatusCode::SUCCESS;

  
  ATH_MSG_VERBOSE (eventInputHeader_1st);
  ATH_MSG_VERBOSE (eventInputHeader_2nd);

  // fill histograms
  for (const auto& track : eventOutputHeader.getFPGATrackSimTracks_1st()) {
    ATH_MSG_DEBUG (track);
    //commented out for future debugging
    //float r= std::sqrt(hit.getX()*hit.getX() + hit.getY()*hit.getY());
    //m_hits_r_vs_z->Fill(hit.getZ(), r);
  }

  for (const auto &track : eventOutputHeader.getFPGATrackSimTracks_2nd()) {
    ATH_MSG_DEBUG (track);
  }

  ATH_CHECK (m_writeOutputTool->writeData(&eventInputHeader_1st, &eventInputHeader_2nd, &eventOutputHeader) );
  return StatusCode::SUCCESS;
}

