/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigTLAMonitorAlgorithm.h"


TrigTLAMonitorAlgorithm::TrigTLAMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
  : AthMonitorAlgorithm(name,pSvcLocator)
{}

TrigTLAMonitorAlgorithm::~TrigTLAMonitorAlgorithm() {}


StatusCode TrigTLAMonitorAlgorithm::initialize() {
  ATH_CHECK( m_jetContainerKey   .initialize() );
  ATH_CHECK( m_pfjetContainerKey .initialize() );
  ATH_CHECK( m_photonContainerKey.initialize() );
  ATH_CHECK( m_muonContainerKey  .initialize() );
  ATH_CHECK( m_tcEventInfoContainerKey  .initialize() );

  return AthMonitorAlgorithm::initialize();
}

StatusCode TrigTLAMonitorAlgorithm::fillHistograms( const EventContext& ctx ) const {
  using namespace Monitored;

  //
  // print the trigger chain names 
  std::string chainName;
  
  int size_AllChains = m_allChains.size();
  ATH_MSG_DEBUG(" Size of the AllChains trigger container: " << size_AllChains );
  for (int i =0; i<size_AllChains; i++){
    chainName = m_allChains[i];
    ATH_MSG_DEBUG("  Chain number: " << i << " AllChains Chain Name: " << chainName );
  }

  SG::ReadHandle<xAOD::TrigCompositeContainer> tcEventInfo = SG::makeHandle( m_tcEventInfoContainerKey, ctx );
  if (! tcEventInfo.isValid() ) {
    ATH_MSG_ERROR("evtStore() does not contain TrigCompositeContainer Collection with name "<< m_tcEventInfoContainerKey);
    return StatusCode::FAILURE;
  }
  SG::ReadHandle<xAOD::JetContainer> jets = SG::makeHandle( m_jetContainerKey, ctx );
  if (! jets.isValid() ) {
    ATH_MSG_ERROR("evtStore() does not contain JetContainer Collection with name "<< m_jetContainerKey);
    return StatusCode::FAILURE;
  }
  SG::ReadHandle<xAOD::JetContainer> pfjets = SG::makeHandle( m_pfjetContainerKey, ctx );
  if (! pfjets.isValid() ) {
    ATH_MSG_ERROR("evtStore() does not contain particle flow JetContainer Collection with name "<< m_pfjetContainerKey);
    return StatusCode::FAILURE;
  }


  // event-wide variables
  ANA_CHECK(fillEventInfoHistogram<float>(tcEventInfo, "AvgMu", "eventInfo" ));
  if (pfjets->size()>0){
    ATH_MSG_DEBUG("nEMPFlowJets = "<<pfjets->size());
    ANA_CHECK(fillEventInfoHistogram<int>(tcEventInfo, "NumPV", "eventInfo" ));
    ANA_CHECK(fillEventInfoHistogram<double>(tcEventInfo, "JetDensityEMPFlow", "eventInfo" ));
  }
  if (jets->size()>0){
    ATH_MSG_DEBUG("nEMTopoJets = "<<jets->size());
    ANA_CHECK(fillEventInfoHistogram<double>(tcEventInfo, "JetDensityEMTopo", "eventInfo" ));
  }

  const std::vector<std::string> jetCalibStates = {"JetConstitScaleMomentum_pt", "JetPileupScaleMomentum_pt", "JetEtaJESScaleMomentum_pt"};
  const std::vector<std::string> pfjetCalibStates = {"JetConstitScaleMomentum_pt", "JetPileupScaleMomentum_pt", "JetEtaJESScaleMomentum_pt", "JetGSCScaleMomentum_pt"};

  for ( const std::string& trigName : m_allChains ) {
    // TODO: Test if trigName has fired

    // proceed filling the histogram

    //
    // jets
    ANA_CHECK(fillParticleHistograms<xAOD::Jet>(jets, "jet", trigName));
    if (jets->size()>0){
      for (auto calibState: jetCalibStates){
        ANA_CHECK(fillJetPtCalibStatesHistograms(jets,calibState, "jet", trigName));
      }
      ANA_CHECK(fillJetVariableHistograms<float>(jets,"N90Constituents", "jet", trigName));
      ANA_CHECK(fillJetVariableHistograms<float>(jets,"Timing", "jet", trigName, -99));
      ANA_CHECK(fillJetVariableHistograms<float>(jets,"EMFrac", "jet", trigName, -99));
      ANA_CHECK(fillJetVariableHistograms<float>(jets,"HECFrac", "jet", trigName, -99));
    }

    //
    // particle flow jets
    ANA_CHECK(fillParticleHistograms<xAOD::Jet>(pfjets, "pfjet", trigName));
    if (pfjets->size()>0){
      for (auto calibState: pfjetCalibStates){
        ANA_CHECK(fillJetPtCalibStatesHistograms(pfjets, calibState, "pfjet", trigName));
      }
      ANA_CHECK(fillJetVariableHistograms<float>(pfjets,"ActiveArea", "pfjet", trigName));
      ANA_CHECK(fillJetTrackVariableHistograms<float>(pfjets,"TrackWidthPt1000", "pfjet", trigName));
      ANA_CHECK(fillJetTrackVariableHistograms<int>(pfjets,"NumTrkPt1000", "pfjet", trigName));
      ANA_CHECK(fillJetTrackVariableHistograms<float>(pfjets,"SumPtTrkPt500", "pfjet", trigName));
      ANA_CHECK(fillJetTrackVariableHistograms<float>(pfjets,"SumPtChargedPFOPt500", "pfjet", trigName));
      ANA_CHECK(fillJetVariableHistograms<float>(pfjets,"Jvt", "pfjet", trigName, -99));
      ANA_CHECK(fillJetVariableHistograms<float>(pfjets,"JvtRpt", "pfjet", trigName, -99));
      ANA_CHECK(fillJetVariableHistograms<float>(pfjets,"fastDIPS20211215_pu", "pfjet", trigName));
      ANA_CHECK(fillJetVariableHistograms<float>(pfjets,"fastDIPS20211215_pb", "pfjet", trigName));
      ANA_CHECK(fillJetVariableHistograms<float>(pfjets,"fastDIPS20211215_pc", "pfjet", trigName));
    }

    //
    // photons

    SG::ReadHandle<xAOD::PhotonContainer> phs = SG::makeHandle( m_photonContainerKey, ctx );
    if (! phs.isValid() ) {
      ATH_MSG_ERROR("evtStore() does not contain PhotonContainer Collection with name "<< m_photonContainerKey);
      return StatusCode::FAILURE;
    }

    ATH_CHECK(fillParticleHistograms<xAOD::Photon>(phs, "ph", trigName));

    //
    // muons

    SG::ReadHandle<xAOD::MuonContainer> muons = SG::makeHandle( m_muonContainerKey, ctx );
    if (! muons.isValid() ) {
      ATH_MSG_ERROR("evtStore() does not contain MuonContainer Collection with name "<< m_muonContainerKey);
      return StatusCode::FAILURE;
    }

    ANA_CHECK(fillParticleHistograms<xAOD::Muon>(muons, "muon", trigName));

    // Delta Angles
    if(jets->size()>=2) {
      ANA_CHECK(fillDeltaRHistograms(jets  ->at(0), jets  ->at(1), "jet0jet1"    , trigName));
    }

    if(pfjets->size()>=2) {
      ANA_CHECK(fillDeltaRHistograms(pfjets->at(0), pfjets->at(1), "pfjet0pfjet1", trigName));
    }

    if(jets->size()>=1 && phs->size()>=1) {
      ANA_CHECK(fillDeltaRHistograms(jets  ->at(0), phs   ->at(0), "jet0ph0"     , trigName));
    }

    if(pfjets->size()>=1 && phs->size()>=1) {
      ANA_CHECK(fillDeltaRHistograms(pfjets->at(0), phs   ->at(0), "pfjet0ph0"   , trigName));
    }

  } // for AllChains

  return StatusCode::SUCCESS;
}

StatusCode TrigTLAMonitorAlgorithm::fillDeltaRHistograms(const xAOD::IParticle* p0, const xAOD::IParticle* p1, const std::string& prefix, const std::string& trigName) const
{
  // histograms
  Monitored::Scalar<double> dr(prefix+"dr_"  +trigName,0.0);

  // fill
  dr = p0->p4().DeltaR(p1->p4());
  fill("TrigTLAMonitor", dr);

  return StatusCode::SUCCESS;
}

template <typename T>
StatusCode TrigTLAMonitorAlgorithm::getEventInfoDetail(const xAOD::TrigComposite_v1* tcEI, const std::string& varname, T& variable) const{

  auto status = tcEI->getDetail<T>(varname, variable);
  if (!status) ATH_MSG_WARNING("No "<<varname<<" for this event");
  else {
    ATH_MSG_DEBUG("Retrieved EventInfo variable: "<<varname<<" = "<<variable);
  }

  return StatusCode::SUCCESS;
}

StatusCode TrigTLAMonitorAlgorithm::fillJetPtCalibStatesHistograms(SG::ReadHandle<xAOD::JetContainer> jets,  const std::string& calibState,  const std::string& prefix,  const std::string& trigName) const {

  Monitored::Scalar<float> ptCalibScale  (prefix+calibState+"_"+trigName,-1.0);

  unsigned cnt(0);
  for(auto jet : *jets) {
    auto status = jet->getAttribute<float>(calibState, ptCalibScale);
    if (!status){
      ATH_MSG_WARNING("Failed retrieving "<<calibState<<" for "<<prefix);
    }
    ptCalibScale = ptCalibScale*1e-3;
    fill("TrigTLAMonitor",ptCalibScale);
    if (cnt < 3) ATH_MSG_DEBUG(prefix<<" "<<calibState<<" = "<<ptCalibScale<<" GeV");
    cnt++;
  }

  return StatusCode::SUCCESS;
}

template <typename T>
StatusCode TrigTLAMonitorAlgorithm::fillJetVariableHistograms(SG::ReadHandle<xAOD::JetContainer> jets,  const std::string& varname,  const std::string& prefix,  const std::string& trigName, T default_val) const {

  Monitored::Scalar<T> variable  (prefix+varname+"_"+trigName,default_val);

  unsigned cnt(0);
  for(auto jet : *jets) {
    auto status = jet->getAttribute<T>(varname, variable);
    if (!status){
      ATH_MSG_WARNING("Failed retrieving "<<varname<<" for "<<prefix);
    }
    fill("TrigTLAMonitor",variable);
    if (cnt < 3) ATH_MSG_DEBUG(prefix<<" "<<varname<<" = "<<variable);
    cnt++;
  }

  return StatusCode::SUCCESS;
}


template <typename T>
StatusCode TrigTLAMonitorAlgorithm::fillJetTrackVariableHistograms(SG::ReadHandle<xAOD::JetContainer> jets,  const std::string& varname,  const std::string& prefix,  const std::string& trigName, T default_val) const {

  std::vector<T> variable_vec;
  Monitored::Scalar<T> variable  (prefix+varname+"_"+trigName,default_val);

  unsigned cnt(0);
  for(auto jet : *jets) {
    auto status = jet->getAttribute<std::vector<T>>(varname, variable_vec);
    if (!status){
      ATH_MSG_WARNING("Failed retrieving "<<varname<<" for "<<prefix);
    }
    variable = variable_vec.at(0);
    fill("TrigTLAMonitor",variable);
    if (cnt < 3) ATH_MSG_DEBUG(prefix<<" "<<varname<<" = "<<variable);
    cnt++;
  }

  return StatusCode::SUCCESS;
}

template <typename U, typename T>
StatusCode TrigTLAMonitorAlgorithm::fill2DHistogram(const std::string& varname1, U var1, const std::string& varname2, T var2, const std::string& prefix) const{

  Monitored::Scalar<U> mon_var1(prefix+"_"+varname1, var1);
  Monitored::Scalar<T> mon_var2(prefix+"_"+varname2, var2);
  ATH_MSG_DEBUG("Filling 2D histogram. "<<varname1<<" = "<<var1<<" ; "<<varname2<<" = "<<var2);
  fill("TrigTLAMonitor", mon_var1, mon_var2);

  return StatusCode::SUCCESS;

}

