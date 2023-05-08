/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <utility>



#include "MuonMatchingTool.h"
#include "xAODTrigger/MuonRoIContainer.h"

const static double ZERO_LIMIT = 1.e-5;

MuonMatchingTool :: MuonMatchingTool(const std::string& type, const std::string& name, const IInterface*  parent)
  : AthAlgTool(type, name, parent)
{}


StatusCode MuonMatchingTool :: initialize(){

  ATH_CHECK( m_trigDec.retrieve() );
  if(m_use_extrapolator){
    ATH_CHECK( m_extrapolator.retrieve() );
  }
  ATH_CHECK( m_MuonContainerKey.initialize() );
  ATH_CHECK( m_MuonRoIContainerKey.initialize() );
  ATH_CHECK( m_L2MuonSAContainerKey.initialize() );
  ATH_CHECK( m_L2muCombContainerKey.initialize() );
  ATH_CHECK( m_EFSAMuonContainerKey.initialize() );
  ATH_CHECK( m_EFCBMuonContainerKey.initialize() );
  ATH_CHECK( m_EFSAFSMuonContainerKey.initialize() );
  ATH_CHECK( m_EFCBFSMuonContainerKey.initialize() );
  ATH_CHECK( m_MStrackContainerKey.initialize() );
  ATH_CHECK( m_CBtrackContainerKey.initialize() );

  return StatusCode::SUCCESS;
}


const Amg::Vector3D MuonMatchingTool :: offlineMuonAtPivot(const xAOD::Muon* mu) const{
  const xAOD::TrackParticle* track = mu->primaryTrackParticle();
  std::unique_ptr<const Trk::TrackParameters> extPars(extTrackToPivot(track));
  return extPars ? extPars->position() : Amg::Vector3D(0.,0.,0.);
}


template<>
std::tuple<bool, double,double> MuonMatchingTool :: trigPosForMatch<xAOD::L2StandAloneMuon>(const xAOD::L2StandAloneMuon *trig){
  return std::forward_as_tuple(true, trig->roiEta(), trig->roiPhi());
}


std::tuple<bool, double,double> MuonMatchingTool :: trigPosForMatchSATrack(const xAOD::Muon *mu){
  return mu->muonType() == xAOD::Muon::MuonType::MuonStandAlone ? std::forward_as_tuple(true, mu->eta(), mu->phi()) : std::forward_as_tuple(false, 0., 0.);
}

std::tuple<bool, double,double> MuonMatchingTool :: trigPosForMatchCBTrack(const xAOD::Muon *mu){
  return mu->muonType() == xAOD::Muon::MuonType::Combined ? std::forward_as_tuple(true, mu->eta(), mu->phi()) : std::forward_as_tuple(false, 0., 0.);
}

std::tuple<bool, double,double> MuonMatchingTool :: PosForMatchSATrack(const xAOD::Muon *mu){
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle);
  return MuonTrack ? std::forward_as_tuple(true, MuonTrack->eta(), MuonTrack->phi()) : std::forward_as_tuple(false, 0., 0.);
}

std::tuple<bool, double,double> MuonMatchingTool :: PosForMatchCBTrack(const xAOD::Muon *mu){
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::CombinedTrackParticle);
  return MuonTrack ? std::forward_as_tuple(true, MuonTrack->eta(), MuonTrack->phi()) : std::forward_as_tuple(false, 0., 0.);
}


const xAOD::Muon* MuonMatchingTool :: matchEFSA(const xAOD::Muon *mu, std::string trig, bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSA()");
  const xAOD::TrackParticle* MuonTrack = nullptr;
  using Type = xAOD::Muon::TrackParticleType;
  std::vector<Type> types { Type::ExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MSOnlyExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MuonSpectrometerTrackParticle};
  for (Type type : types){
    MuonTrack = mu->trackParticle(type);
    ATH_MSG_DEBUG("HLT_Muons_RoI SA Muon");
    if (MuonTrack) break;
  }
  return MuonTrack ? match<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, "HLT_Muons_RoI.*", &MuonMatchingTool::trigPosForMatchSATrack) : nullptr;
}

const xAOD::Muon* MuonMatchingTool :: matchEFSA(const xAOD::TruthParticle *mu, std::string trig, bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSA() for truth particle");
  return mu ? match<xAOD::Muon>(mu, std::move(trig), m_EFreqdR, pass, "HLT_Muons_RoI.*", &MuonMatchingTool::trigPosForMatchSATrack) : nullptr;
}

const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> MuonMatchingTool :: matchEFSALinkInfo(const xAOD::Muon *mu, std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSALinkInfo()");
  bool pass = false;
  TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> muonLinkInfo;
  const xAOD::TrackParticle* MuonTrack = nullptr;
  using Type = xAOD::Muon::TrackParticleType;
  std::vector<Type> types { Type::ExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MSOnlyExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MuonSpectrometerTrackParticle};
  for (Type type : types){
    MuonTrack = mu->trackParticle(type);
    if (MuonTrack) break;
  }
  return MuonTrack ? matchLinkInfo<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, "HLT_Muons_RoI.*", &MuonMatchingTool::trigPosForMatchSATrack) : muonLinkInfo;
}

const xAOD::Muon* MuonMatchingTool :: matchEFSAReadHandle( const EventContext& ctx, const xAOD::Muon *mu) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSAReadHandle()");
  const xAOD::TrackParticle* MuonTrack = nullptr;
  using Type = xAOD::Muon::TrackParticleType;
  std::vector<Type> types { Type::ExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MSOnlyExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MuonSpectrometerTrackParticle};
  for (Type type : types){
    MuonTrack = mu->trackParticle(type);
    if (MuonTrack) break;
  }
  return MuonTrack ? matchReadHandle<xAOD::Muon>( MuonTrack, m_EFreqdR, m_EFSAMuonContainerKey, ctx, &MuonMatchingTool::trigPosForMatchSATrack) : nullptr;
}

const xAOD::Muon* MuonMatchingTool :: matchEFCB(  const xAOD::TruthParticle *mu, std::string trig, bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCB() for TruthParticle");
  return mu ? match<xAOD::Muon>( mu, std::move(trig), m_EFreqdR, pass, "HLT_MuonsCB_RoI.*", &MuonMatchingTool::trigPosForMatchCBTrack) : nullptr;
}

const xAOD::Muon* MuonMatchingTool :: matchEFCB(  const xAOD::Muon *mu, std::string trig, bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCB()");
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);
  ATH_MSG_DEBUG("HLT_Muons_RoI CB Muon");
  return MuonTrack ? match<xAOD::Muon>( MuonTrack, std::move(trig), m_EFreqdR, pass, "HLT_MuonsCB_RoI.*", &MuonMatchingTool::trigPosForMatchCBTrack) : nullptr;
}

const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> MuonMatchingTool :: matchEFCBLinkInfo( const xAOD::Muon *mu, std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBLinkInfo()");
  bool pass = false;
  TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> muonLinkInfo;
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);
  return MuonTrack ? matchLinkInfo<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, "HLT_MuonsCB_RoI.*", &MuonMatchingTool::trigPosForMatchCBTrack) : muonLinkInfo;
}

const xAOD::Muon* MuonMatchingTool :: matchEFCBReadHandle( const EventContext& ctx, const xAOD::Muon *mu) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBReadHandle()");
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);
  return MuonTrack ? matchReadHandle<xAOD::Muon>( MuonTrack, m_EFreqdR, m_EFCBMuonContainerKey, ctx, &MuonMatchingTool::trigPosForMatchCBTrack) : nullptr;
}

const xAOD::Muon* MuonMatchingTool :: matchEFSAFS(const xAOD::Muon *mu, std::string trig, bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSAFS()");
  const xAOD::TrackParticle* MuonTrack = nullptr;
  using Type = xAOD::Muon::TrackParticleType;
  std::vector<Type> types { Type::ExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MSOnlyExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MuonSpectrometerTrackParticle};
  for (Type type : types){
    MuonTrack = mu->trackParticle(type);
    ATH_MSG_DEBUG("HLT_Muons_FS SA Muon");
    if (MuonTrack) break;
  }
  return MuonTrack ? match<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, "HLT_Muons_FS.*", &MuonMatchingTool::trigPosForMatchSATrack) : nullptr;
}

const xAOD::Muon* MuonMatchingTool :: matchEFSAFS(const xAOD::TruthParticle *mu, std::string trig, bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSAFS() for truth particle");
  return mu ? match<xAOD::Muon>(mu, std::move(trig), m_EFreqdR, pass, "HLT_Muons_FS.*", &MuonMatchingTool::trigPosForMatchSATrack) : nullptr;
}

const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> MuonMatchingTool :: matchEFSAFSLinkInfo(const xAOD::Muon *mu, std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSAFSLinkInfo()");
  bool pass = false;
  TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> muonLinkInfo;
  const xAOD::TrackParticle* MuonTrack = nullptr;
  using Type = xAOD::Muon::TrackParticleType;
  std::vector<Type> types { Type::ExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MSOnlyExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MuonSpectrometerTrackParticle};
  for (Type type : types){
    MuonTrack = mu->trackParticle(type);
    if (MuonTrack) break;
  }
  return MuonTrack ? matchLinkInfo<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, "HLT_Muons_FS.*", &MuonMatchingTool::trigPosForMatchSATrack) : muonLinkInfo;
}

const xAOD::Muon* MuonMatchingTool :: matchEFSAFSReadHandle( const EventContext& ctx, const xAOD::Muon *mu) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFSAFSReadHandle()");
  const xAOD::TrackParticle* MuonTrack = nullptr;
  using Type = xAOD::Muon::TrackParticleType;
  std::vector<Type> types { Type::ExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MSOnlyExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MuonSpectrometerTrackParticle};
  for (Type type : types){
    MuonTrack = mu->trackParticle(type);
    if (MuonTrack) break;
  }
  return MuonTrack ? matchReadHandle<xAOD::Muon>( MuonTrack, m_EFreqdR, m_EFSAFSMuonContainerKey, ctx, &MuonMatchingTool::trigPosForMatchSATrack) : nullptr;
}

const xAOD::Muon* MuonMatchingTool :: matchEFCBFS(  const xAOD::TruthParticle *mu, std::string trig, bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBFS() for TruthParticle");
  return mu ? match<xAOD::Muon>( mu, std::move(trig), m_EFreqdR, pass, "HLT_MuonsCB_FS.*", &MuonMatchingTool::trigPosForMatchCBTrack) : nullptr;
}

const xAOD::Muon* MuonMatchingTool :: matchEFCBFS( const xAOD::Muon *mu, std::string trig, bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBFS()");
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);
  ATH_MSG_DEBUG("HLT_Muons_FS CB Muon");
  return MuonTrack ? match<xAOD::Muon>( MuonTrack, std::move(trig), m_EFreqdR, pass, "HLT_MuonsCB_FS.*", &MuonMatchingTool::trigPosForMatchCBTrack) : nullptr;
}

const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> MuonMatchingTool :: matchEFCBFSLinkInfo( const xAOD::Muon *mu, std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBFSLinkInfo()");
  bool pass = false;
  TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> muonLinkInfo;
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);
  return MuonTrack ? matchLinkInfo<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, "HLT_MuonsCB_FS.*", &MuonMatchingTool::trigPosForMatchCBTrack) : muonLinkInfo;
}

const xAOD::Muon* MuonMatchingTool :: matchEFCBFSReadHandle( const EventContext& ctx, const xAOD::Muon *mu) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBFSReadHandle()");
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);
  return MuonTrack ? matchReadHandle<xAOD::Muon>( MuonTrack, m_EFreqdR, m_EFCBFSMuonContainerKey, ctx, &MuonMatchingTool::trigPosForMatchCBTrack) : nullptr;
}

const xAOD::Muon* MuonMatchingTool :: matchEFIso( const xAOD::Muon *mu, std::string trig, bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFIso()");
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);
  return MuonTrack ? match<xAOD::Muon>( MuonTrack, std::move(trig), m_EFreqdR, pass, "HLT_MuonsIso", &MuonMatchingTool::trigPosForMatchCBTrack) : nullptr;
}

const xAOD::Muon* MuonMatchingTool :: matchEFIso( const xAOD::TruthParticle *mu, std::string trig, bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFIso() for truth particle");
  return mu ? match<xAOD::Muon>( mu, std::move(trig), m_EFreqdR, pass, "HLT_MuonsIso", &MuonMatchingTool::trigPosForMatchCBTrack) : nullptr;
}


const xAOD::TrackParticle* MuonMatchingTool :: SearchEFTrack(const EventContext &ctx, const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer>& muLinkInfo, const SG::ReadHandleKey<xAOD::TrackParticleContainer>& ReadHandleKey) const {
  const xAOD::TrackParticle* MatchedTrack = nullptr;
  const ElementLink<xAOD::MuonContainer> muEL = muLinkInfo.link;
  float EFEta = (*muEL)->eta();
  float EFPhi = (*muEL)->phi();
  float mindR = 999.;

  SG::ReadHandle<xAOD::TrackParticleContainer> trackHandle(ReadHandleKey, ctx);
  if ( !trackHandle.isValid() ) return MatchedTrack;

  const auto track = m_trigDec->associateToEventView<xAOD::TrackParticleContainer>(trackHandle, muLinkInfo);
  const xAOD::TrackParticleContainer::const_iterator begin = track.first;
  const xAOD::TrackParticleContainer::const_iterator end  = track.second;

  for (xAOD::TrackParticleContainer::const_iterator it = begin; it != end; ++it) {

    float deta = EFEta - (*it)->eta();
    float dphi = xAOD::P4Helpers::deltaPhi(EFPhi, (*it)->phi() );
    float dR = std::sqrt(deta*deta + dphi*dphi);

    if( dR< mindR ){
      mindR = dR;
      MatchedTrack = (*it);
    }
  }

  return MatchedTrack;

}

const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> MuonMatchingTool :: matchEFIsoLinkInfo( const xAOD::Muon *mu, std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchEFCBLinkInfo()");
  bool pass = false;
  TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> muonLinkInfo;
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);
  return MuonTrack ? matchLinkInfo<xAOD::Muon>(MuonTrack, std::move(trig), m_EFreqdR, pass, "HLT_MuonsIso", &MuonMatchingTool::trigPosForMatchCBTrack) : muonLinkInfo;
}


const xAOD::L2StandAloneMuon* MuonMatchingTool :: matchL2SA(  const xAOD::Muon *mu, const std::string& trig, bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchL2SA()");
  float reqdR = m_L2SAreqdR;
  if(m_use_extrapolator){
    reqdR = reqdRL1byPt(mu->pt());
    const Amg::Vector3D extPos = offlineMuonAtPivot(mu);
    if(extPos.norm()>ZERO_LIMIT){
      return match<xAOD::L2StandAloneMuon>( &extPos, trig, reqdR, pass);
    }
  }
  return match<xAOD::L2StandAloneMuon>( mu, trig, reqdR, pass, "HLT_MuonL2SAInfo");
}

const TrigCompositeUtils::LinkInfo<xAOD::L2StandAloneMuonContainer> MuonMatchingTool :: searchL2SALinkInfo(  const xAOD::Muon *mu, std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::searchL2SALinkInfo()");
  bool pass = false;
  return matchLinkInfo<xAOD::L2StandAloneMuon>( mu, std::move(trig), 1000., pass, "HLT_MuonL2SAInfo");
}

const xAOD::L2StandAloneMuon* MuonMatchingTool :: matchL2SAReadHandle( const EventContext& ctx, const xAOD::Muon *mu) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchL2SAReadHandle()");
  float reqdR = m_L2SAreqdR;
  if(m_use_extrapolator){
    reqdR = reqdRL1byPt(mu->pt());
    const Amg::Vector3D extPos = offlineMuonAtPivot(mu);
    if(extPos.norm()>ZERO_LIMIT){
      return matchReadHandle<xAOD::L2StandAloneMuon>( &extPos, reqdR, m_L2MuonSAContainerKey, ctx);
    }
  }
  const xAOD::TrackParticle* MuonTrack = nullptr;
  using Type = xAOD::Muon::TrackParticleType;
  std::vector<Type> types { Type::ExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MSOnlyExtrapolatedMuonSpectrometerTrackParticle,
                            Type::MuonSpectrometerTrackParticle};
  for (Type type : types){
    MuonTrack = mu->trackParticle(type);
    if (MuonTrack) break;
  }
  return MuonTrack ? matchReadHandle<xAOD::L2StandAloneMuon>( MuonTrack, reqdR, m_L2MuonSAContainerKey, ctx) : nullptr;
}


const xAOD::L2CombinedMuon* MuonMatchingTool :: matchL2CB(  const xAOD::Muon *mu, std::string trig, bool &pass) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchL2CB()");
  return match<xAOD::L2CombinedMuon>( mu, std::move(trig), m_L2CBreqdR, pass, "HLT_MuonL2CBInfo");
}

const TrigCompositeUtils::LinkInfo<xAOD::L2CombinedMuonContainer> MuonMatchingTool :: searchL2CBLinkInfo(  const xAOD::Muon *mu, std::string trig) const {
  ATH_MSG_DEBUG("MuonMonitoring::searchL2CBLinkInfo()");
  bool pass = false;
  return matchLinkInfo<xAOD::L2CombinedMuon>( mu, std::move(trig),  1000., pass, "HLT_MuonL2CBInfo");
}

const xAOD::L2CombinedMuon* MuonMatchingTool :: matchL2CBReadHandle( const EventContext& ctx, const xAOD::Muon *mu) const {
  ATH_MSG_DEBUG("MuonMonitoring::matchL2CBReadHandle()");
  const xAOD::TrackParticle* MuonTrack = mu->trackParticle(xAOD::Muon::TrackParticleType::Primary);
  return MuonTrack ? matchReadHandle<xAOD::L2CombinedMuon>( MuonTrack, m_L2CBreqdR, m_L2muCombContainerKey, ctx) : nullptr;
}

const xAOD::MuonRoI* MuonMatchingTool :: matchL1( double refEta, double refPhi, double reqdR, const std::string& trig, bool &pass) const {
  ATH_MSG_DEBUG("Chain: " << trig);
  pass = false;
  const xAOD::MuonRoI *closest = nullptr;
  Trig::FeatureRequestDescriptor featureRequestDescriptor(trig,TrigDefs::includeFailedDecisions);
  auto l2muonFeatures = m_trigDec->features<xAOD::L2StandAloneMuonContainer>(featureRequestDescriptor); 
  for( const auto& linkInfo : l2muonFeatures){ // loop on L2 muon features
    // get L1 muon associated with this L2 muon
    auto l1muonLinkInfo = TrigCompositeUtils::findLink<xAOD::MuonRoIContainer>(linkInfo.source, "initialRecRoI");
    auto l1muonLink = l1muonLinkInfo.link;
    if(!l1muonLink.isValid()){
      ATH_MSG_ERROR("Invalid link to L1 muon");
      continue;
    }
    const xAOD::MuonRoI* l1muon = *l1muonLink;
    double l1muonEta = l1muon->eta();
    double l1muonPhi = l1muon->phi();
    
    double deta = refEta - l1muonEta;
    double dphi = xAOD::P4Helpers::deltaPhi(refPhi, l1muonPhi);
    double dR = std::sqrt(deta*deta + dphi*dphi);
    ATH_MSG_DEBUG("L1 muon candidate eta=" << l1muonEta << " phi=" << l1muonPhi << " dR=" << dR);
    if( dR<reqdR ){
      reqdR = dR;
      pass = true;
      closest = l1muon;
      ATH_MSG_DEBUG("*** L1 muon eta=" << l1muonEta << " phi=" << l1muonPhi << " dR=" << dR <<  " isPassed=true" ); 
    }
    else{
      ATH_MSG_DEBUG("*** L1 muon eta=" << l1muonEta << " phi=" << l1muonPhi << " dR=" << dR <<  " isPassed=false" );
    }
  }
  return closest;
}

const xAOD::MuonRoI* MuonMatchingTool :: matchL1( const xAOD::Muon *mu, const std::string& trig, bool &pass) const {
  double refEta = mu->eta();
  double refPhi = mu->phi();
  double reqdR = 0.25;

  if(m_use_extrapolator){
    reqdR = reqdRL1byPt(mu->pt());
    const Amg::Vector3D extPos = offlineMuonAtPivot(mu);
    if(extPos.norm()>ZERO_LIMIT){
      refEta = extPos.eta();
      refPhi = extPos.phi();
    }
  }
  return matchL1(refEta, refPhi, reqdR, trig, pass); 
}

const xAOD::MuonRoI* MuonMatchingTool :: matchL1( const xAOD::TruthParticle *mu, const std::string& trig, bool &pass) const {
  double refEta = mu->eta();
  double refPhi = mu->phi();
  double reqdR = 0.25;
  return matchL1(refEta, refPhi, reqdR, trig, pass);
}


const xAOD::Muon* MuonMatchingTool :: matchL2SAtoOff( const EventContext& ctx, const xAOD::L2StandAloneMuon* samu) const {
  return matchOff(ctx, samu, m_L2SAreqdR, &MuonMatchingTool::PosForMatchSATrack);
}

const xAOD::Muon* MuonMatchingTool :: matchL2CBtoOff( const EventContext& ctx, const xAOD::L2CombinedMuon* cbmu) const {
  return matchOff(ctx, cbmu, m_L2CBreqdR, &MuonMatchingTool::PosForMatchCBTrack);
}


bool MuonMatchingTool :: isMatchedL2SA(const xAOD::L2StandAloneMuon* samu, const xAOD::Muon* mu) const{
  float offlEta = mu->eta();
  float offlPhi = mu->phi();
  float trigEta = samu->roiEta();
  float trigPhi = samu->roiPhi();

  float deta = offlEta - trigEta;
  float dphi = xAOD::P4Helpers::deltaPhi(offlPhi, trigPhi);
  float dR = sqrt(deta*deta + dphi*dphi);
  return dR < m_L2SAreqdR;
}

bool MuonMatchingTool :: isMatchedL2CB(const xAOD::L2CombinedMuon* cbmu, const xAOD::Muon* mu) const{
  float dR = xAOD::P4Helpers::deltaR(cbmu, mu, false);
  return dR < m_L2CBreqdR;
}

bool MuonMatchingTool :: isMatchedL2InsideOut(const xAOD::L2CombinedMuon* cbiomu, const xAOD::Muon* mu) const{
  float dR = xAOD::P4Helpers::deltaR(cbiomu, mu, false);
  return dR < m_L2InsideOutreqdR;
}

double MuonMatchingTool :: FermiFunction(double x, double x0, double w) {
  return 1/(1+TMath::Exp(-10*(x-x0)/w));
}


  
const Trk::TrackParameters* MuonMatchingTool :: extTrackToPivot(const xAOD::TrackParticle* track) const {

  const Trk::TrackParameters *extRPC = nullptr;
  const Trk::TrackParameters *extTGC = nullptr;

  if(!track) return extTGC;
  double trkEta = track->eta();
  double extEta =0.;
  bool isBarrel = true;

  if( fabs(trkEta)<1.05){
    extRPC = extTrackToRPC(track);
    if(!extRPC){
      isBarrel = false;
      extTGC = extTrackToTGC(track);
    }
    else{
      isBarrel = true;
      extEta = extRPC->position().eta();
      if(fabs(extEta)>=1.05){
	extTGC = extTrackToTGC(track);
	isBarrel = (extTGC) == nullptr; 
      }
    }
  }
  else if( fabs(trkEta)>=1.05 ){
    extTGC = extTrackToTGC(track);
    if(!extTGC){
      isBarrel = true;
      extRPC = extTrackToRPC(track);
    }
    else{
      isBarrel = false;
      extEta = extTGC->position().eta();
      if(fabs(extEta)<1.05){
	extRPC = extTrackToRPC(track);
	isBarrel = (extRPC) != nullptr;
      }
    }
  }

  ATH_MSG_DEBUG("extTGC=" << extTGC << " extRPC=" << extRPC << " isBarrel=" << isBarrel);
  if(   isBarrel) delete extTGC;
  else if(!isBarrel) delete extRPC;
  return (isBarrel) ? extRPC : extTGC;
}

  

const Trk::TrackParameters* MuonMatchingTool :: extTrackToTGC( const xAOD::TrackParticle* trk ) const {
  ATH_MSG_DEBUG("extTrackToTGC");
  if(!trk) return nullptr;
  double TGC_Z = ( trk->eta()>0 )? 15153.0:-15153.0;
  Amg::Transform3D matrix = Amg::Transform3D( Amg::Vector3D( 0.,0.,TGC_Z ) );
  //object pointed by matrix will be deleted in destructer of DiscSurface, therefore release it
  std::unique_ptr<Trk::DiscSurface> disc(new Trk::DiscSurface( matrix, 0., 15000.));
  const bool boundaryCheck = true;

  const Trk::TrackParameters* param = m_extrapolator->extrapolate(Gaudi::Hive::currentContext(),
                                                                  trk->perigeeParameters(),
                                                                  *disc,
                                                                  Trk::anyDirection,
                                                                  boundaryCheck,
                                                                  Trk::muon).release();

  ATH_MSG_DEBUG("param=" << param 
		<< " eta=" << ((param) ? param->position().eta() : 0) 
		<< " phi=" << ((param) ? param->position().phi() : 0));;
  return param;
}



const Trk::TrackParameters* MuonMatchingTool :: extTrackToRPC( const xAOD::TrackParticle* trk ) const {
  ATH_MSG_DEBUG("extTrackToRPC");
  if(!trk) return nullptr;
  std::unique_ptr<Trk::CylinderSurface> barrel(new Trk::CylinderSurface(  7478., 15000. ));
  const bool boundaryCheck = true;

  const Trk::TrackParameters* param = m_extrapolator->extrapolate(Gaudi::Hive::currentContext(),
                                                                  trk->perigeeParameters(),
                                                                  *barrel,
                                                                  Trk::anyDirection,
                                                                  boundaryCheck,
                                                                  Trk::muon).release();
  return param;
}



double MuonMatchingTool :: reqdRL1byPt( double mupt){
  double dR = 0.08;
  if( mupt < 10000. ) {
    dR = -0.00001*mupt + 0.18;
  } 
  return dR;
}
