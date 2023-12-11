/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "JetMonitoring/JetContainerHistoFiller.h"

JetContainerHistoFiller::JetContainerHistoFiller(const std::string& n) : HistoGroupBase(n)
                                                                       , m_histoTools(this)
{
  declareInterface<JetContainerHistoFiller>(this);
  declareProperty("HistoTools", m_histoTools);

}

StatusCode JetContainerHistoFiller::initialize() {

  CHECK( m_histoTools.retrieve() );

  if(m_jetContainerName.empty()) {
    ATH_MSG_ERROR("Jet Container name not set. Please set the JetContainer property");
    return StatusCode::FAILURE;
  }
  CHECK( m_jetContainerName.initialize() );
  CHECK( m_EventInfoKey.initialize() );

  if(m_histoDir.empty()) m_histoDir =  m_jetContainerName.key()+"/";

  return StatusCode::SUCCESS;
}



int JetContainerHistoFiller::fillHistos(){

  ATH_MSG_DEBUG ("Filling hists " << name() << "..." << m_jetContainerName.key());

  SG::ReadHandle<xAOD::EventInfo> evtInfo{m_EventInfoKey};
  if (!evtInfo.isValid()) {
    ATH_MSG_DEBUG("Unable to retrieve xAOD::EventInfo");
    return 1;
  }

  //LAr event veto: skip events rejected by LAr
  if(evtInfo->errorState(xAOD::EventInfo::LAr)==xAOD::EventInfo::Error){
    ATH_MSG_DEBUG("SKIP for LAR error");
    return 1;
  }
  
  SG::ReadHandle<xAOD::JetContainer> jCont{m_jetContainerName};
  if( !jCont.isValid() ) {
    ATH_MSG_DEBUG (" No container  " << m_jetContainerName.key()<< " in Evt store. Returning.");
    return 0;
  }

  float weight = evtInfo->beamSpotWeight();

  /// simply call fillHistosFromContainer() for each tool...
  int count = 0;
  for( auto jtool : m_histoTools){
    ATH_MSG_DEBUG ("Filling hists " << jtool->name() << "..." << jCont);

    count += jtool->fillHistosFromContainer(*jCont, weight);
  }

  return count;    
}

void JetContainerHistoFiller::setInterval(Interval_t ityp, bool force ){
  // propagate interval to sub-tools
  for( auto jtool : m_histoTools){
    jtool->setInterval(ityp,force);
  }
}


int JetContainerHistoFiller::buildHistos(){
  int count=0;

  ATH_MSG_DEBUG ("Building hists " );

  // ask subtools to build their histos
  for( auto jtool : m_histoTools){
    count+=jtool->buildHistos();
    ATH_MSG_DEBUG (" *** Built hist :  "<<jtool->name()  );
    
    // keep a pointer to histos :
    const auto & hdata = jtool->bookedHistograms();
    for( const auto & hd : hdata ){ 
      m_vBookedHistograms.push_back(hd);
    }
  }
  return count;    
}


  int JetContainerHistoFiller::finalizeHistos(){
  int count = 0;
  for( auto jtool : m_histoTools){
    count+=jtool->finalizeHistos();
  }
  return count;
}

void JetContainerHistoFiller::prefixHistoDir(const std::string & preDir){
  for( auto jtool : m_histoTools){
    jtool->prefixHistoDir(preDir+m_histoDir);
  }
}
