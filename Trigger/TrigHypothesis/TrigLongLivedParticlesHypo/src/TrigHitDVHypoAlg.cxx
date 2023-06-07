/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

  * Trigger Hypo Tool, that is aimed at triggering displaced vertex
  * author Kunihiro Nagano <kunihiro.nagano@cern.ch> - KEK
*/
#include "TrigHitDVHypoAlg.h"
#include "AthViews/ViewHelper.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "PathResolver/PathResolver.h"

#include <vector>
#include <unordered_map>

using TrigCompositeUtils::createAndStore;
using TrigCompositeUtils::DecisionContainer;
using TrigCompositeUtils::DecisionAuxContainer;
using TrigCompositeUtils::DecisionIDContainer;
using TrigCompositeUtils::decisionIDs;
using TrigCompositeUtils::newDecisionIn;
using TrigCompositeUtils::linkToPrevious;
using TrigCompositeUtils::viewString;
using TrigCompositeUtils::featureString;
using TrigCompositeUtils::hypoAlgNodeName;
using TrigCompositeUtils::findLink;
using TrigCompositeUtils::LinkInfo;
using TrigCompositeUtils::Decision;
using TrigCompositeUtils::allFailed;

using xAOD::JetContainer;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

TrigHitDVHypoAlg::TrigHitDVHypoAlg( const std::string& name,
				    ISvcLocator* pSvcLocator ) :
   ::HypoBase( name, pSvcLocator ),
   m_lumiBlockMuTool("LumiBlockMuTool/LumiBlockMuTool") {}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

StatusCode TrigHitDVHypoAlg::initialize()
{
   m_tools_lowest_jetEt = 1000;
   m_tools_loosest_wp   = 0;
   CHECK( m_hypoTools.retrieve() );
   for ( auto & tool: m_hypoTools ) {
      ATH_MSG_VERBOSE( "+++++ Hypo Tool name: " << tool->name() );
      std::cmatch results;
      if( std::regex_search(tool->name().c_str(),results,std::regex("hitdvjet(\\d+)_(\\w+)_")) ) {
	 std::string sth = results[1].str();
	 std::string swp = results[2].str();
	 ATH_MSG_VERBOSE( "   thres = " << sth << ", wp = " << swp );
	 int thres = std::stoi(sth);
	 if( thres < m_tools_lowest_jetEt ) m_tools_lowest_jetEt = thres;
	 if( swp == "loose"  && m_tools_loosest_wp >= 1 ) m_tools_loosest_wp = 1;
	 if( swp == "medium" && m_tools_loosest_wp >= 2 ) m_tools_loosest_wp = 2;
	 if( swp == "tight"  && m_tools_loosest_wp >= 3 ) m_tools_loosest_wp = 3;
      }
   }
   ATH_MSG_INFO( "Lowest jetEt used in hypo tools = " << m_tools_lowest_jetEt << " (GeV)" );
   ATH_MSG_INFO( "Loosest WP used in hypo tools = " << m_tools_loosest_wp << " (loose=1,medium=2,tight=3)");
   // loose : eta<2.0, SP seed=true,  BDT eff=0.9
   // medium: eta<2.0, SP seed=false, BDT eff=0.75
   // tight : eta<1.0, SP seed=false, BDT eff=0.75

   CHECK( m_jetsKey.initialize() );
   CHECK( m_hitDVSeedKey.initialize() );
   CHECK( m_hitDVTrkKey.initialize() );
   CHECK( m_hitDVSPKey.initialize() );
   CHECK( m_hitDVKey.initialize());
   CHECK( m_lumiDataKey.initialize(!m_isMC) );

   if ( !m_monTool.empty() ) CHECK( m_monTool.retrieve() );

   for (auto& reader : m_tmva_reader) {
      // Create two instances with same variables
      auto tmva = std::array{std::make_unique<TMVA::Reader>( "!Color:!Silent" ),
                             std::make_unique<TMVA::Reader>( "!Color:!Silent" )};
      for (auto& t : tmva) {
         t->AddVariable("n_track_qual", &reader.n_track_qual);
         t->AddVariable("ly0_sp_frac",  &reader.ly0_sp_frac);
         t->AddVariable("ly1_sp_frac",  &reader.ly1_sp_frac);
         t->AddVariable("ly2_sp_frac",  &reader.ly2_sp_frac);
         t->AddVariable("ly3_sp_frac",  &reader.ly3_sp_frac);
         t->AddVariable("ly4_sp_frac",  &reader.ly4_sp_frac);
         t->AddVariable("ly5_sp_frac",  &reader.ly5_sp_frac);
         t->AddVariable("ly6_sp_frac",  &reader.ly6_sp_frac);
         t->AddVariable("ly7_sp_frac",  &reader.ly7_sp_frac);
      };
      reader.tmva_0eta1 = std::move(tmva[0]);
      reader.tmva_1eta2 = std::move(tmva[1]);

      // --- Book the MVA methods specific to eta range
      const std::string tuningVer  = "v22a"; // "v21a";
      const std::string methodName = "BDT method";

      const std::string weightfile_0eta1 = PathResolver::find_calib_file(
         "TrigHitDVHypo/HitDV.BDT.weights.0eta1." + tuningVer + ".xml");
      const std::string weightfile_1eta2 = PathResolver::find_calib_file(
         "TrigHitDVHypo/HitDV.BDT.weights.1eta2." + tuningVer + ".xml");
      ATH_MSG_DEBUG("opening weightfile = " << weightfile_0eta1);
      ATH_MSG_DEBUG("opening weightfile = " << weightfile_1eta2);
      reader.tmva_0eta1->BookMVA(methodName, weightfile_0eta1);
      reader.tmva_1eta2->BookMVA(methodName, weightfile_1eta2);
   }

   return StatusCode::SUCCESS;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

StatusCode TrigHitDVHypoAlg::execute( const EventContext& context ) const
{
   // monitoring
   auto mon_n_dvtrks     = Monitored::Scalar<int>( "n_dvtrks",     0 );
   auto mon_n_dvsps      = Monitored::Scalar<int>( "n_dvsps",      0 );
   auto mon_n_jetseeds   = Monitored::Scalar<int>( "n_jetseeds",   0 );
   auto mon_n_jetseedsdel= Monitored::Scalar<int>( "n_jetseedsdel",0 );
   auto mon_n_spseeds    = Monitored::Scalar<int>( "n_spseeds",    0 );
   auto mon_n_spseedsdel = Monitored::Scalar<int>( "n_spseedsdel", 0 );
   auto mon_average_mu   = Monitored::Scalar<float>( "average_mu", 0.);
   auto monitorIt        = Monitored::Group( m_monTool, mon_n_dvtrks, mon_n_dvsps, mon_n_jetseeds, mon_n_jetseedsdel, mon_n_spseeds, mon_n_spseedsdel, mon_average_mu );

   // previous decisions
   ATH_MSG_DEBUG( "Retrieving pervious decision: \"" << decisionInput().key() << "\"" );
   auto previousDecisionsHandle = SG::makeHandle( decisionInput(), context );
   ATH_CHECK( previousDecisionsHandle.isValid() );

   ATH_MSG_DEBUG( "Running with " << previousDecisionsHandle->size() << " previous decisions" );
   if( previousDecisionsHandle->size()!=1 ) {
      ATH_MSG_ERROR( "Previous decision handle size is not 1. It is" << previousDecisionsHandle->size() );
      return StatusCode::FAILURE;
   }
   const Decision * previousDecision = previousDecisionsHandle->at(0);

   TrigCompositeUtils::DecisionIDContainer previousDecisionIDs;
   TrigCompositeUtils::decisionIDs(previousDecision, previousDecisionIDs);
   ATH_MSG_DEBUG( "IDs of active legs:" );
   for(auto decisionID: previousDecisionIDs) { ATH_MSG_DEBUG( "    " << decisionID ); }

   // new output decisions
   ATH_MSG_DEBUG( "Creating new output decision handle" );
   SG::WriteHandle<DecisionContainer> outputHandle = createAndStore(decisionOutput(), context );
   auto outputDecisions = outputHandle.ptr();

   // input objects

   // jets
   auto jetsHandle = SG::makeHandle(m_jetsKey, context );
   ATH_CHECK( jetsHandle.isValid() );
   ATH_MSG_DEBUG( "jet handle size: " << jetsHandle->size() );

   const xAOD::JetContainer* jetsContainer = jetsHandle.get();
   if( jetsContainer == nullptr ) {
      ATH_MSG_ERROR( "ERROR Cannot get jet container" );
      return StatusCode::FAILURE;
   }
   bool isJetEtPassToolsCut  = false;
   float jetEtaToolsCut = 2.0;
   if( m_tools_loosest_wp >= 3 ) jetEtaToolsCut = 1.0;
   for ( const xAOD::Jet* jet : *jetsContainer ) {
      float jet_pt  = static_cast<float>(jet->pt() / Gaudi::Units::GeV );
      float jet_eta = static_cast<float>(jet->eta());
      if( jet_pt >= m_tools_lowest_jetEt && std::abs(jet_eta)<=jetEtaToolsCut ) {
	 isJetEtPassToolsCut = true;
	 break;
      }
   }

   // hitDV objects
   auto hitDVSeedHandle = SG::makeHandle(m_hitDVSeedKey, context );
   ATH_CHECK( hitDVSeedHandle.isValid() );
   ATH_MSG_DEBUG( "hitDVSeed handle size: " << hitDVSeedHandle->size() );

   auto hitDVTrkHandle = SG::makeHandle(m_hitDVTrkKey, context );
   ATH_CHECK( hitDVTrkHandle.isValid() );
   ATH_MSG_DEBUG( "hitDVTrk handle size: " << hitDVTrkHandle->size() );

   auto hitDVSPHandle = SG::makeHandle(m_hitDVSPKey, context );
   ATH_CHECK( hitDVSPHandle.isValid() );
   ATH_MSG_DEBUG( "hitDVSP handle size: " << hitDVSPHandle->size() );

   const xAOD::TrigCompositeContainer * hitDVSeedsContainer = hitDVSeedHandle.get();
   const xAOD::TrigCompositeContainer * hitDVTrksContainer  = hitDVTrkHandle.get();
   const xAOD::TrigCompositeContainer * hitDVSPsContainer   = hitDVSPHandle.get();

   if( hitDVSeedsContainer == nullptr ) {
      ATH_MSG_ERROR( "ERROR Cannot get hitDVSeeds container" );
      return StatusCode::FAILURE;
   }
   if( hitDVTrksContainer == nullptr ) {
      ATH_MSG_ERROR( "ERROR Cannot get hitDVTrks container" );
      return StatusCode::FAILURE;
   }
   if( hitDVSPsContainer == nullptr ) {
      ATH_MSG_ERROR( "ERROR Cannot get hitDVSPs container" );
      return StatusCode::FAILURE;
   }

   mon_n_dvtrks = hitDVTrksContainer->size();
   mon_n_dvsps  = hitDVSPsContainer->size();
   const unsigned int N_MAX_SP_STORED = 100000;
   bool isSPOverflow = false;
   if( hitDVSPsContainer->size() >= N_MAX_SP_STORED ) isSPOverflow = true;
   ATH_MSG_DEBUG( "hitDVSP size=" << mon_n_dvsps );

   // average mu
   float averageMu = 0;
   if( m_isMC ) {
      if( m_lumiBlockMuTool ) {
	 averageMu = static_cast<float>(m_lumiBlockMuTool->averageInteractionsPerCrossing());
	 ATH_MSG_DEBUG( "offline averageMu = " << averageMu );
      }
   }
   else {
      SG::ReadCondHandle<LuminosityCondData> lcd (m_lumiDataKey,context);
      averageMu = lcd.cptr()->lbAverageInteractionsPerCrossing();
      ATH_MSG_DEBUG( "online averageMu = " << averageMu );
   }
   mon_average_mu = averageMu;

   // find seeds based on HLT jets
   std::vector<float> jetSeeds_pt;
   std::vector<float> jetSeeds_eta;
   std::vector<float> jetSeeds_phi;
   ATH_CHECK( findJetSeeds(jetsContainer, m_jetSeed_ptMin, m_jetSeed_etaMax, jetSeeds_pt, jetSeeds_eta, jetSeeds_phi) );
   int n_alljetseeds = jetSeeds_eta.size();
   ATH_CHECK( selectSeedsNearby(hitDVSeedsContainer, jetSeeds_eta, jetSeeds_phi, jetSeeds_pt) );
   mon_n_jetseeds = jetSeeds_eta.size();
   mon_n_jetseedsdel = n_alljetseeds - jetSeeds_eta.size();

   // find seeds based on SP frac itself
   std::vector<float> spSeeds_eta;
   std::vector<float> spSeeds_phi;
   std::vector<float> void_pt;
   int n_allspseeds = 0;
   if( m_tools_loosest_wp <= 1 ) {
      ATH_CHECK( findSPSeeds(context, hitDVSPsContainer, spSeeds_eta, spSeeds_phi) );
      n_allspseeds  = spSeeds_eta.size();
      ATH_CHECK( selectSeedsNearby(hitDVSeedsContainer, spSeeds_eta, spSeeds_phi, void_pt) );
      mon_n_spseeds = spSeeds_eta.size();
      mon_n_spseedsdel = n_allspseeds - spSeeds_eta.size();
   }
   else {
      mon_n_spseeds    = 0;
      mon_n_spseedsdel = 0;
   }

   // output EDM object
   auto hitDVContainer    = std::make_unique<xAOD::TrigCompositeContainer>();
   auto hitDVContainerAux = std::make_unique<xAOD::TrigCompositeAuxContainer>();
   hitDVContainer->setStore(hitDVContainerAux.get());

   xAOD::TrigCompositeContainer* dvContainer = hitDVContainer.get();
   std::vector<TrigHitDVHypoTool::HitDVHypoInfo>  hitDVHypoInputs;
   std::unordered_map<Decision*, size_t>          mapDecIdx;

   // calculate BDT and create hitDVContainer EDM
   if( isJetEtPassToolsCut ) {
      const float preselBDTthreshold = -0.6;

      int n_passed_jet = 0;
      int seed_type = SeedType::HLTJet;
      ATH_CHECK( calculateBDT(context, hitDVSPsContainer, hitDVTrksContainer, jetSeeds_pt, jetSeeds_eta, jetSeeds_phi, preselBDTthreshold, seed_type, dvContainer, n_passed_jet) );

      int n_passed_sp = 0;
      if( m_tools_loosest_wp <= 1 ) {
	 seed_type = SeedType::SP;
	 ATH_CHECK( calculateBDT(context, hitDVSPsContainer, hitDVTrksContainer, void_pt, spSeeds_eta, spSeeds_phi, preselBDTthreshold, seed_type, dvContainer, n_passed_sp) );
      }

      ATH_MSG_DEBUG( "nr of dv container / jet-seeded / sp-seed candidates = " << dvContainer->size() << " / " << n_passed_jet << " / " << n_passed_sp );

      // Prepare inputs to HypoTool
      for ( auto dv : *dvContainer ) {
	 Decision* newDecision = TrigCompositeUtils::newDecisionIn( outputDecisions, previousDecision, TrigCompositeUtils::hypoAlgNodeName(), context);
	 mapDecIdx.emplace( newDecision, dv->index() );
	 TrigHitDVHypoTool::HitDVHypoInfo hypoInfo{ newDecision, isSPOverflow, averageMu, dv, previousDecisionIDs };
	 hitDVHypoInputs.push_back( hypoInfo );
      }
   }

   // monitor
   ATH_CHECK( doMonitor(dvContainer) );

   // Loop over all hypoToolinputs and get their decisions
   for ( auto & tool: m_hypoTools ) {
      ATH_MSG_DEBUG( "+++++ Now computing decision for " << tool->name() );
      ATH_CHECK( tool->decide( hitDVHypoInputs ) );
   }

   // record hitDV object
   SG::WriteHandle<xAOD::TrigCompositeContainer> hitDVHandle(m_hitDVKey, context);
   ATH_CHECK( hitDVHandle.record( std::move( hitDVContainer ), std::move( hitDVContainerAux ) ) );
   ATH_MSG_DEBUG( "recorded hitDV object to SG" );

   DecisionContainer::iterator it = outputDecisions->begin();
   while(it != outputDecisions->end()) {
      ATH_MSG_DEBUG( "+++++ outputDecision: " << *it << " +++++" );
      if ( allFailed( *it ) ) {
         ATH_MSG_DEBUG( "---> all failed, erasing" );
         it = outputDecisions->erase(it);
      } else {
         ATH_MSG_DEBUG( "---> not all failed" );

         // Link hitDV object
         auto     decision = *it;
         size_t   idx = mapDecIdx.at(*it);

         ElementLink<xAOD::TrigCompositeContainer> dvEL = ElementLink<xAOD::TrigCompositeContainer>(*hitDVHandle, idx, context);
         ATH_CHECK( dvEL.isValid() );

         ATH_CHECK( decision->setObjectLink<xAOD::TrigCompositeContainer>(featureString(), dvEL) );

         ATH_MSG_DEBUG(*decision);
         ++it;
      }
   }

   //
   ATH_CHECK( hypoBaseOutputProcessing(outputHandle) );

   //
   return StatusCode::SUCCESS;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

float TrigHitDVHypoAlg::deltaR2(float eta_1, float phi_1, float eta_2, float phi_2) const
{
   float dPhi = phi_1 - phi_2;
   if (dPhi < -TMath::Pi()) dPhi += 2*TMath::Pi();
   if (dPhi >  TMath::Pi()) dPhi -= 2*TMath::Pi();
   float dEta = eta_1 - eta_2;
   return (dPhi*dPhi+dEta*dEta);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

int TrigHitDVHypoAlg::getSPLayer(int layer, float eta) const
{
   float abseta = std::fabs(eta);

   // if Pixel/SCT barrel, layer number is as it is
   if( 0<=layer && layer <=7 ) {
      ATH_MSG_VERBOSE("layer=" << layer << ", eta=" << abseta);
      return layer;
   }

   // for Pixel/SCT endcap, assign layer number of 0-7 depending on eta range

   int base = 0;

   //
   const float PixBR6limit = 1.29612;
   const float PixBR5limit = 1.45204;
   const float PixBR4limit = 1.64909;
   const float PixBR3limit = 1.90036;
   const float PixBR2limit = 2.2146;

   // Pixel Endcap #1
   base = 8;
   if( layer==base || layer==(base+12) ) {
      ATH_MSG_VERBOSE("Pix EC1, eta=" << abseta);
      if( abseta > PixBR2limit ) return 2;
      return 3;
   }

   // Pixel Endcap #2
   base = 9;
   if( layer==base || layer==(base+12) ) {
      ATH_MSG_VERBOSE("Pix EC2, eta=" << abseta);
      if( abseta > PixBR2limit ) return 2;
      return 3;
   }

   // Pixel Endcap #3
   base = 10;
   if( layer==base || layer==(base+12) ) {
      ATH_MSG_VERBOSE("Pix EC3, eta=" << abseta);
      return 3;
   }

   // SCT Endcap #1
   base = 11;
   if( layer==base || layer==(base+12) ) {
      ATH_MSG_VERBOSE("Sct EC1, eta=" << abseta);
      if( abseta < PixBR6limit )      return 7;
      else if( abseta < PixBR5limit ) return 6;
      return 5;
   }

   // SCT Endcap #2
   base = 12;
   if( layer==base || layer==(base+12) ) {
      ATH_MSG_VERBOSE("Sct EC2, eta=" << abseta);
      if( abseta < PixBR5limit )      return 7;
      else if( abseta < PixBR4limit ) return 6;
      return 4;
   }

   // SCT Endcap #3
   base = 13;
   if( layer==base || layer==(base+12) ) {
      ATH_MSG_VERBOSE("Sct EC3, eta=" << abseta);
      if( abseta < PixBR4limit ) return 7;
      return 5;
   }

   // SCT Endcap #4
   base = 14;
   if( layer==base || layer==(base+12) ) {
      ATH_MSG_VERBOSE("Sct EC4, eta=" << abseta);
      if( abseta < PixBR4limit ) return 6;
      else if( abseta < PixBR3limit ) return 6;
      return 4;
   }

   // SCT Endcap #5
   base = 15;
   if( layer==base || layer==(base+12) ) {
      ATH_MSG_VERBOSE("Sct EC5, eta=" << abseta);
      if( abseta < PixBR3limit ) return 7;
      return 5;
   }

   // SCT Endcap #6
   base = 16;
   if( layer==base || layer==(base+12) ) {
      ATH_MSG_VERBOSE("Sct EC6, eta=" << abseta);
      if( abseta < PixBR3limit ) return 6;
      return 4;
   }

   // SCT Endcap #7
   base = 17;
   if( layer==base || layer==(base+12) ) {
      ATH_MSG_VERBOSE("Sct EC7, eta=" << abseta);
      if( abseta < PixBR3limit ) return 7;
      return 5;
   }

   // SCT Endcap #8
   base = 18;
   if( layer==base || layer==(base+12) ) {
      ATH_MSG_VERBOSE("Sct EC8, eta=" << abseta);
      if( abseta < PixBR3limit ) return 7;
      return 6;
   }

   // SCT Endcap #9
   base = 19;
   if( layer==base || layer==(base+12) ) {
      ATH_MSG_VERBOSE("Sct EC9, eta=" << abseta);
      return 7;
   }

   return 0;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

StatusCode TrigHitDVHypoAlg::doMonitor(const xAOD::TrigCompositeContainer* dvContainer ) const
{
   std::vector<float> mnt_eta1_ly0_spfr;
   std::vector<float> mnt_eta1_ly1_spfr;
   std::vector<float> mnt_eta1_ly2_spfr;
   std::vector<float> mnt_eta1_ly3_spfr;
   std::vector<float> mnt_eta1_ly4_spfr;
   std::vector<float> mnt_eta1_ly5_spfr;
   std::vector<float> mnt_eta1_ly6_spfr;
   std::vector<float> mnt_eta1_ly7_spfr;
   std::vector<int>   mnt_eta1_n_qtrk;
   std::vector<float> mnt_eta1_bdtscore;
   std::vector<float> mnt_1eta2_ly0_spfr;
   std::vector<float> mnt_1eta2_ly1_spfr;
   std::vector<float> mnt_1eta2_ly2_spfr;
   std::vector<float> mnt_1eta2_ly3_spfr;
   std::vector<float> mnt_1eta2_ly4_spfr;
   std::vector<float> mnt_1eta2_ly5_spfr;
   std::vector<float> mnt_1eta2_ly6_spfr;
   std::vector<float> mnt_1eta2_ly7_spfr;
   std::vector<int>   mnt_1eta2_n_qtrk;
   std::vector<float> mnt_1eta2_bdtscore;
   auto mon_eta1_ly0_spfr  = Monitored::Collection("eta1_ly0_spfr",   mnt_eta1_ly0_spfr);
   auto mon_eta1_ly1_spfr  = Monitored::Collection("eta1_ly1_spfr",   mnt_eta1_ly1_spfr);
   auto mon_eta1_ly2_spfr  = Monitored::Collection("eta1_ly2_spfr",   mnt_eta1_ly2_spfr);
   auto mon_eta1_ly3_spfr  = Monitored::Collection("eta1_ly3_spfr",   mnt_eta1_ly3_spfr);
   auto mon_eta1_ly4_spfr  = Monitored::Collection("eta1_ly4_spfr",   mnt_eta1_ly4_spfr);
   auto mon_eta1_ly5_spfr  = Monitored::Collection("eta1_ly5_spfr",   mnt_eta1_ly5_spfr);
   auto mon_eta1_ly6_spfr  = Monitored::Collection("eta1_ly6_spfr",   mnt_eta1_ly6_spfr);
   auto mon_eta1_ly7_spfr  = Monitored::Collection("eta1_ly7_spfr",   mnt_eta1_ly7_spfr);
   auto mon_eta1_n_qtrk    = Monitored::Collection("eta1_n_qtrk",     mnt_eta1_n_qtrk);
   auto mon_eta1_bdtscore  = Monitored::Collection("eta1_bdtscore",   mnt_eta1_bdtscore);
   auto mon_1eta2_ly0_spfr = Monitored::Collection("1eta2_ly0_spfr",  mnt_1eta2_ly0_spfr);
   auto mon_1eta2_ly1_spfr = Monitored::Collection("1eta2_ly1_spfr",  mnt_1eta2_ly1_spfr);
   auto mon_1eta2_ly2_spfr = Monitored::Collection("1eta2_ly2_spfr",  mnt_1eta2_ly2_spfr);
   auto mon_1eta2_ly3_spfr = Monitored::Collection("1eta2_ly3_spfr",  mnt_1eta2_ly3_spfr);
   auto mon_1eta2_ly4_spfr = Monitored::Collection("1eta2_ly4_spfr",  mnt_1eta2_ly4_spfr);
   auto mon_1eta2_ly5_spfr = Monitored::Collection("1eta2_ly5_spfr",  mnt_1eta2_ly5_spfr);
   auto mon_1eta2_ly6_spfr = Monitored::Collection("1eta2_ly6_spfr",  mnt_1eta2_ly6_spfr);
   auto mon_1eta2_ly7_spfr = Monitored::Collection("1eta2_ly7_spfr",  mnt_1eta2_ly7_spfr);
   auto mon_1eta2_n_qtrk   = Monitored::Collection("1eta2_n_qtrk",    mnt_1eta2_n_qtrk);
   auto mon_1eta2_bdtscore = Monitored::Collection("1eta2_bdtscore",  mnt_1eta2_bdtscore);
   auto monitorIt = Monitored::Group( m_monTool,
				      mon_eta1_ly0_spfr, mon_eta1_ly1_spfr, mon_eta1_ly2_spfr, mon_eta1_ly3_spfr,
				      mon_eta1_ly4_spfr, mon_eta1_ly5_spfr, mon_eta1_ly6_spfr, mon_eta1_ly7_spfr,
				      mon_eta1_n_qtrk, mon_eta1_bdtscore,
				      mon_1eta2_ly0_spfr, mon_1eta2_ly1_spfr, mon_1eta2_ly2_spfr, mon_1eta2_ly3_spfr,
				      mon_1eta2_ly4_spfr, mon_1eta2_ly5_spfr, mon_1eta2_ly6_spfr, mon_1eta2_ly7_spfr,
				      mon_1eta2_n_qtrk, mon_1eta2_bdtscore);

   //
   for ( auto dv : *dvContainer ) {
      int   seed_type   = dv->getDetail<int>  ("hitDV_seed_type");
      // do not fill sp-seeded candidates
      if( seed_type == SeedType::SP ) continue;
      float seed_eta    = dv->getDetail<float>("hitDV_seed_eta");
      int   n_track_qual= dv->getDetail<int>  ("hitDV_n_track_qual");
      float bdt_score   = dv->getDetail<float>("hitDV_bdt_score");
      float ly0_sp_frac = dv->getDetail<float>("hitDV_ly0_sp_frac");
      float ly1_sp_frac = dv->getDetail<float>("hitDV_ly1_sp_frac");
      float ly2_sp_frac = dv->getDetail<float>("hitDV_ly2_sp_frac");
      float ly3_sp_frac = dv->getDetail<float>("hitDV_ly3_sp_frac");
      float ly4_sp_frac = dv->getDetail<float>("hitDV_ly4_sp_frac");
      float ly5_sp_frac = dv->getDetail<float>("hitDV_ly5_sp_frac");
      float ly6_sp_frac = dv->getDetail<float>("hitDV_ly6_sp_frac");
      float ly7_sp_frac = dv->getDetail<float>("hitDV_ly7_sp_frac");
      if( std::abs(seed_eta) < 1.0 ) {
	 mnt_eta1_ly0_spfr.push_back(ly0_sp_frac);
	 mnt_eta1_ly1_spfr.push_back(ly1_sp_frac);
	 mnt_eta1_ly2_spfr.push_back(ly2_sp_frac);
	 mnt_eta1_ly3_spfr.push_back(ly3_sp_frac);
	 mnt_eta1_ly4_spfr.push_back(ly4_sp_frac);
	 mnt_eta1_ly5_spfr.push_back(ly5_sp_frac);
	 mnt_eta1_ly6_spfr.push_back(ly6_sp_frac);
	 mnt_eta1_ly7_spfr.push_back(ly7_sp_frac);
	 mnt_eta1_n_qtrk.push_back(n_track_qual);
	 mnt_eta1_bdtscore.push_back(bdt_score);
      }
      else if( std::abs(seed_eta) < 2.0 ) {
	 mnt_1eta2_ly0_spfr.push_back(ly0_sp_frac);
	 mnt_1eta2_ly1_spfr.push_back(ly1_sp_frac);
	 mnt_1eta2_ly2_spfr.push_back(ly2_sp_frac);
	 mnt_1eta2_ly3_spfr.push_back(ly3_sp_frac);
	 mnt_1eta2_ly4_spfr.push_back(ly4_sp_frac);
	 mnt_1eta2_ly5_spfr.push_back(ly5_sp_frac);
	 mnt_1eta2_ly6_spfr.push_back(ly6_sp_frac);
	 mnt_1eta2_ly7_spfr.push_back(ly7_sp_frac);
	 mnt_1eta2_n_qtrk.push_back(n_track_qual);
	 mnt_1eta2_bdtscore.push_back(bdt_score);
      }
   }

   //
   return StatusCode::SUCCESS;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

StatusCode TrigHitDVHypoAlg::calculateBDT(const EventContext& context,
					  const xAOD::TrigCompositeContainer* spsContainer,
					  const xAOD::TrigCompositeContainer* trksContainer,
					  const std::vector<float>& seeds_pt,
					  const std::vector<float>& seeds_eta, const std::vector<float>& seeds_phi,
					  const float& cutBDTthreshold, const int seed_type,
					  xAOD::TrigCompositeContainer* dvContainer, int& n_passed) const
{
   if( seeds_eta.size() != seeds_phi.size() ) return StatusCode::SUCCESS;
   n_passed = 0;

   for(unsigned int iseed=0; iseed<seeds_eta.size(); iseed++) {

      float seed_eta = seeds_eta[iseed];
      float seed_phi = seeds_phi[iseed];

      ATH_MSG_VERBOSE("+++++ seed eta: " << seed_eta << ", phi:" << seed_phi << " +++++");

      // loop on space points
      const int   N_LAYER = 8;
      const float DR_SQUARED_TO_REF_CUT = 0.16; // const float DR_TO_REF_CUT = 0.4;

      int n_sp_injet = 0;
      int n_sp_injet_usedByTrk = 0;
      int v_n_sp_injet[N_LAYER];
      int v_n_sp_injet_usedByTrk[N_LAYER];
      for(int i=0; i<N_LAYER; i++) { v_n_sp_injet[i]=0; v_n_sp_injet_usedByTrk[i]=0; }

      for ( auto spData : *spsContainer ) {
	 // match within dR
	 float sp_eta = spData->getDetail<float>("hitDVSP_eta");
	 float sp_phi = spData->getDetail<float>("hitDVSP_phi");
	 float dR2 = deltaR2(sp_eta,sp_phi,seed_eta,seed_phi);
	 if( dR2 > DR_SQUARED_TO_REF_CUT ) continue;

	 //
	 int sp_layer = (int)spData->getDetail<int16_t>("hitDVSP_layer");
	 int sp_trkid = (int)spData->getDetail<int16_t>("hitDVSP_usedTrkId");
	 bool isUsedByTrk = (sp_trkid != -1);

	 int ilayer = getSPLayer(sp_layer,sp_eta);

	 if( ilayer<=7  ) { // Pixel barrel or Sct barrel
	    n_sp_injet++;
	    v_n_sp_injet[ilayer]++;
	    if( isUsedByTrk ) {
	       n_sp_injet_usedByTrk++;
	       v_n_sp_injet_usedByTrk[ilayer]++;
	    }
	 }
      }
      ATH_MSG_VERBOSE("nr of SPs in jet: usedByTrk / all = " << n_sp_injet_usedByTrk << " / " << n_sp_injet);
      float v_ly_sp_frac[N_LAYER];
      for(int i=0; i<N_LAYER; i++) {
	 float frac = 0.;
	 if( v_n_sp_injet[i] > 0 ) frac = 1.0 - static_cast<float>(v_n_sp_injet_usedByTrk[i]) / static_cast<float>(v_n_sp_injet[i]);
	 v_ly_sp_frac[i] = frac;
	 ATH_MSG_VERBOSE("Layer " << i << ": frac=" << v_ly_sp_frac[i] << ", n used / all = " << v_n_sp_injet_usedByTrk[i] << " / " << v_n_sp_injet[i]);
      }

      // loop on tracks
      const float TRK_PT_GEV_CUT = 2.0;

      unsigned int n_qtrk_injet = 0;
      for ( auto trk : *trksContainer ) {
	 float trk_ptGeV  = trk->getDetail<float>("hitDVTrk_pt");
	 trk_ptGeV /= Gaudi::Units::GeV;
	 if( trk_ptGeV < TRK_PT_GEV_CUT ) continue;
	 float trk_eta = trk->getDetail<float>("hitDVTrk_eta");
	 float trk_phi = trk->getDetail<float>("hitDVTrk_phi");
	 float dR2 = deltaR2(trk_eta,trk_phi,seed_eta,seed_phi);
	 if( dR2 > DR_SQUARED_TO_REF_CUT ) continue;
	 n_qtrk_injet++;
      }
      ATH_MSG_DEBUG("nr of all / quality tracks matched = " << trksContainer->size() << " / " << n_qtrk_injet);

      // evaluate BDT
      bool isSeedOutOfRange = false;
      if( n_qtrk_injet == 0 ) {
	 isSeedOutOfRange = true;
	 for(int i=0; i<N_LAYER; i++) {
	    if( std::fabs(v_ly_sp_frac[i]) > 1e-3 ) {
	       isSeedOutOfRange = false; break;
	    }
	 }
      }
      float bdt_score = -2.0;
      if( ! isSeedOutOfRange ) {
         auto& reader = *m_tmva_reader.get(context);
         reader.n_track_qual = static_cast<float>(n_qtrk_injet);
         reader.ly0_sp_frac  = v_ly_sp_frac[0];
         reader.ly1_sp_frac  = v_ly_sp_frac[1];
         reader.ly2_sp_frac  = v_ly_sp_frac[2];
         reader.ly3_sp_frac  = v_ly_sp_frac[3];
         reader.ly4_sp_frac  = v_ly_sp_frac[4];
         reader.ly5_sp_frac  = v_ly_sp_frac[5];
         reader.ly6_sp_frac  = v_ly_sp_frac[6];
         reader.ly7_sp_frac  = v_ly_sp_frac[7];

         if ( std::abs(seed_eta) < 1 ) {
            bdt_score = reader.tmva_0eta1->EvaluateMVA("BDT method");
         } else if ( std::abs(seed_eta) < 2 ) {
            bdt_score = reader.tmva_1eta2->EvaluateMVA("BDT method");
         }
      }

      // BDT threshold
      if( bdt_score < cutBDTthreshold ) continue;

      // passed selection
      ATH_MSG_VERBOSE("Passed selection");
      n_passed++;

      // create EDM object
      xAOD::TrigComposite *dv = new xAOD::TrigComposite();
      dv->makePrivateStore();
      dvContainer->push_back(dv);

      float seed_pt = 0;
      if ( seed_type == SeedType::HLTJet ) seed_pt = seeds_pt[iseed];
      dv->setDetail<float>("hitDV_seed_pt",      seed_pt);
      dv->setDetail<float>("hitDV_seed_eta",     seed_eta);
      dv->setDetail<float>("hitDV_seed_phi",     seed_phi);
      dv->setDetail<int>  ("hitDV_seed_type",    seed_type);
      dv->setDetail<int>  ("hitDV_n_track_qual", n_qtrk_injet);
      dv->setDetail<float>("hitDV_ly0_sp_frac",  v_ly_sp_frac[0]);
      dv->setDetail<float>("hitDV_ly1_sp_frac",  v_ly_sp_frac[1]);
      dv->setDetail<float>("hitDV_ly2_sp_frac",  v_ly_sp_frac[2]);
      dv->setDetail<float>("hitDV_ly3_sp_frac",  v_ly_sp_frac[3]);
      dv->setDetail<float>("hitDV_ly4_sp_frac",  v_ly_sp_frac[4]);
      dv->setDetail<float>("hitDV_ly5_sp_frac",  v_ly_sp_frac[5]);
      dv->setDetail<float>("hitDV_ly6_sp_frac",  v_ly_sp_frac[6]);
      dv->setDetail<float>("hitDV_ly7_sp_frac",  v_ly_sp_frac[7]);
      dv->setDetail<float>("hitDV_bdt_score",    bdt_score);

      ATH_MSG_VERBOSE("Created a new entry EDM");
   }
   ATH_MSG_DEBUG("nr of BDT passed = " << n_passed);

   //
   return StatusCode::SUCCESS;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

StatusCode TrigHitDVHypoAlg::findJetSeeds(const xAOD::JetContainer* jetsContainer, const float cutJetPt, const float cutJetEta,
					  std::vector<float>& jetSeeds_pt, std::vector<float>& jetSeeds_eta, std::vector<float>& jetSeeds_phi) const
{
   std::vector<float> mnt_jet_pt;
   std::vector<float> mnt_jet_eta;
   auto mon_jet_pt  = Monitored::Collection("jet_pt",  mnt_jet_pt);
   auto mon_jet_eta = Monitored::Collection("jet_eta", mnt_jet_eta);
   auto monitorIt   = Monitored::Group( m_monTool, mon_jet_pt, mon_jet_eta );

   ATH_MSG_VERBOSE("looking for jet seed with pt cut=" << cutJetPt << ", eta cut=" << cutJetEta);
   for ( const xAOD::Jet* jet : *jetsContainer ) {
      float jet_pt  = static_cast<float>(jet->pt() / Gaudi::Units::GeV );
      if( jet_pt < cutJetPt ) {
	 ATH_MSG_VERBOSE("Fails jet pt cut, pt = " << jet_pt);
	 continue;
      }
      mnt_jet_pt.push_back(jet_pt);
      float jet_eta = static_cast<float>(jet->eta());
      mnt_jet_eta.push_back(jet_eta);
      if( std::fabs(jet_eta) > cutJetEta ) {
	 ATH_MSG_VERBOSE("Fails jet eta cut, eta = " << jet_eta);
	 continue;
      }
      float jet_phi = static_cast<float>(jet->phi());
      jetSeeds_pt.push_back(jet_pt);
      jetSeeds_eta.push_back(jet_eta);
      jetSeeds_phi.push_back(jet_phi);
   }
   ATH_MSG_VERBOSE("nr of jet seeds=" << jetSeeds_eta.size());

   return StatusCode::SUCCESS;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

StatusCode TrigHitDVHypoAlg::findSPSeeds( const EventContext& ctx, const xAOD::TrigCompositeContainer* spsContainer,
					  std::vector<float>& seeds_eta, std::vector<float>& seeds_phi ) const
{
   seeds_eta.clear();
   seeds_phi.clear();

   const int   NBINS_ETA = 50;
   const float ETA_MIN   = -2.5;
   const float ETA_MAX   =  2.5;

   const int   NBINS_PHI = 80;
   const float PHI_MIN   = -4.0;
   const float PHI_MAX   =  4.0;

   char hname[64];

   unsigned int slotnr    = ctx.slot();
   unsigned int subSlotnr = ctx.subSlot();

   sprintf(hname,"hitdv_s%u_ss%u_ly6_h2_nsp",slotnr,subSlotnr);
   std::unique_ptr<TH2F> ly6_h2_nsp = std::make_unique<TH2F>(hname,hname,NBINS_ETA,ETA_MIN,ETA_MAX,NBINS_PHI,PHI_MIN,PHI_MAX);
   sprintf(hname,"hitdv_s%u_ss%u_ly7_h2_nsp",slotnr,subSlotnr);
   std::unique_ptr<TH2F> ly7_h2_nsp = std::make_unique<TH2F>(hname,hname,NBINS_ETA,ETA_MIN,ETA_MAX,NBINS_PHI,PHI_MIN,PHI_MAX);

   sprintf(hname,"hitdv_s%u_ss%u_ly6_h2_nsp_notrk",slotnr,subSlotnr);
   std::unique_ptr<TH2F> ly6_h2_nsp_notrk = std::make_unique<TH2F>(hname,hname,NBINS_ETA,ETA_MIN,ETA_MAX,NBINS_PHI,PHI_MIN,PHI_MAX);
   sprintf(hname,"hitdv_s%u_ss%u_ly7_h2_nsp_notrk",slotnr,subSlotnr);
   std::unique_ptr<TH2F> ly7_h2_nsp_notrk = std::make_unique<TH2F>(hname,hname,NBINS_ETA,ETA_MIN,ETA_MAX,NBINS_PHI,PHI_MIN,PHI_MAX);

   for ( auto spData : *spsContainer ) {
      int sp_layer = (int)spData->getDetail<int16_t>("hitDVSP_layer");
      float sp_eta = spData->getDetail<float>("hitDVSP_eta");
      int ilayer = getSPLayer(sp_layer,sp_eta);
      if( ilayer<6 ) continue;

      int sp_trkid = (int)spData->getDetail<int16_t>("hitDVSP_usedTrkId");
      bool isUsedByTrk = (sp_trkid != -1);
      float sp_phi = spData->getDetail<float>("hitDVSP_phi");

      bool fill_out_of_pi = false;
      float sp_phi2 = 0;
      if( sp_phi < 0 ) {
	 sp_phi2 = 2*TMath::Pi() + sp_phi;
	 if( sp_phi2 < PHI_MAX ) fill_out_of_pi = true;
      }
      else {
	 sp_phi2 = -2*TMath::Pi() + sp_phi;
	 if( PHI_MIN < sp_phi2 ) fill_out_of_pi = true;
      }
      if( ilayer==6 ) {
	                      ly6_h2_nsp->Fill(sp_eta,sp_phi);
	 if( fill_out_of_pi ) ly6_h2_nsp->Fill(sp_eta,sp_phi2);
	 if( ! isUsedByTrk )                  ly6_h2_nsp_notrk->Fill(sp_eta,sp_phi);
	 if( ! isUsedByTrk && fill_out_of_pi) ly6_h2_nsp_notrk->Fill(sp_eta,sp_phi2);
      }
      if( ilayer==7 ) {
	                      ly7_h2_nsp->Fill(sp_eta,sp_phi);
	 if( fill_out_of_pi ) ly7_h2_nsp->Fill(sp_eta,sp_phi2);
	 if( ! isUsedByTrk )                  ly7_h2_nsp_notrk->Fill(sp_eta,sp_phi);
	 if( ! isUsedByTrk && fill_out_of_pi) ly7_h2_nsp_notrk->Fill(sp_eta,sp_phi2);
      }
   }

   ATH_MSG_VERBOSE("looking for ly6/ly6 doublet seeds");

   // (idx, sort/weight, eta, phi)
   std::vector<std::tuple<int,float,float,float>> QT;

   for(int ly6_ieta=1; ly6_ieta<=NBINS_ETA; ly6_ieta++) {
      float ly6_eta = (ly6_h2_nsp->GetXaxis()->GetBinLowEdge(ly6_ieta) + ly6_h2_nsp->GetXaxis()->GetBinUpEdge(ly6_ieta))/2.0;
      for(int ly6_iphi=1; ly6_iphi<=NBINS_PHI; ly6_iphi++) {
	 float ly6_phi = (ly6_h2_nsp->GetYaxis()->GetBinLowEdge(ly6_iphi) + ly6_h2_nsp->GetYaxis()->GetBinUpEdge(ly6_iphi))/2.0;

	 float ly6_nsp       = ly6_h2_nsp      ->GetBinContent(ly6_ieta,ly6_iphi);
	 float ly6_nsp_notrk = ly6_h2_nsp_notrk->GetBinContent(ly6_ieta,ly6_iphi);
	 float ly6_frac      = ( ly6_nsp > 0 ) ? ly6_nsp_notrk / ly6_nsp : 0;
	 if( ly6_nsp < 10 || ly6_frac < 0.85 ) continue;

	 float ly7_frac_max = 0;
	 float ly7_eta_max  = 0;
	 float ly7_phi_max  = 0;
	 for(int ly7_ieta=std::max(1,ly6_ieta-1); ly7_ieta<std::min(NBINS_ETA,ly6_ieta+1); ly7_ieta++) {
	    for(int ly7_iphi=std::max(1,ly6_iphi-1); ly7_iphi<=std::min(NBINS_PHI,ly6_iphi+1); ly7_iphi++) {
	       float ly7_nsp       = ly7_h2_nsp      ->GetBinContent(ly7_ieta,ly7_iphi);
	       float ly7_nsp_notrk = ly7_h2_nsp_notrk->GetBinContent(ly7_ieta,ly7_iphi);
	       float ly7_frac      = ( ly7_nsp > 0 ) ? ly7_nsp_notrk / ly7_nsp : 0;
	       if( ly7_nsp < 10 ) continue;
	       if( ly7_frac > ly7_frac_max ) {
		  ly7_frac_max = ly7_frac;
		  ly7_eta_max  = (ly7_h2_nsp->GetXaxis()->GetBinLowEdge(ly7_ieta) + ly7_h2_nsp->GetXaxis()->GetBinUpEdge(ly7_ieta))/2.0;
		  ly7_phi_max  = (ly7_h2_nsp->GetXaxis()->GetBinLowEdge(ly7_iphi) + ly7_h2_nsp->GetXaxis()->GetBinUpEdge(ly7_iphi))/2.0;
	       }
	    }
	 }
	 if( ly7_frac_max < 0.85 ) continue;
	 //
	 float wsum = ly6_frac + ly7_frac_max;
	 float weta = (ly6_eta*ly6_frac + ly7_eta_max*ly7_frac_max) / wsum;
	 float wphi = (ly6_phi*ly6_frac + ly7_phi_max*ly7_frac_max) / wsum;
	 float w = wsum / 2.0;
	 QT.push_back(std::make_tuple(-1,w,weta,wphi));
      }
   }
   ATH_MSG_VERBOSE("nr of ly6/ly7 doublet candidate seeds=" << QT.size() << ", doing clustering...");

   // sort
   std::sort(QT.begin(), QT.end(),
	     [](const std::tuple<int,float,float,float>& lhs, const std::tuple<int,float,float,float>& rhs) {
		return std::get<1>(lhs) > std::get<1>(rhs); } );

   // clustering
   const double CLUSTCUT_DIST_SQUARED = 0.04; // const double CLUSTCUT_DIST = 0.2;
   const double CLUSTCUT_SEED_FRAC    = 0.9;

   std::vector<float> seeds_wsum;

   for(unsigned int i=0; i<QT.size(); i++) {
      float phi  = std::get<3>(QT[i]);
      float eta  = std::get<2>(QT[i]);
      float w    = std::get<1>(QT[i]);
      if(i==0) {
	 seeds_eta.push_back(w*eta); seeds_phi.push_back(w*phi);
	 seeds_wsum.push_back(w);
	 continue;
      }
      const int IDX_INITIAL = 100;
      float dist2_min = 100.0;
      int idx_min     = IDX_INITIAL;
      for(unsigned j=0; j<seeds_eta.size(); j++) {
	 float ceta = seeds_eta[j]/seeds_wsum[j];
	 float cphi = seeds_phi[j]/seeds_wsum[j];
	 // intentionally calculate in this way as phi is defined beyond -Pi/Pi (no boundary)
	 float deta  = std::fabs(ceta-eta);
	 float dphi  = std::fabs(cphi-phi);
	 float dist2 = dphi*dphi+deta*deta;
	 if( dist2 < dist2_min ) {
	    dist2_min = dist2;
	    idx_min  = j;
	 }
      }
      int match_idx = IDX_INITIAL;
      if( idx_min != IDX_INITIAL ) {
	 if( dist2_min < CLUSTCUT_DIST_SQUARED ) { match_idx = idx_min; }
      }
      if( match_idx == IDX_INITIAL ) {
	 if( w > CLUSTCUT_SEED_FRAC && dist2_min > CLUSTCUT_DIST_SQUARED ) {
	    seeds_eta.push_back(w*eta); seeds_phi.push_back(w*phi);
	    seeds_wsum.push_back(w);
	 }
	 continue;
      }
      float new_eta   = seeds_eta[match_idx]  + w*eta;
      float new_phi   = seeds_phi[match_idx]  + w*phi;
      float new_wsum  = seeds_wsum[match_idx] + w;
      seeds_eta[match_idx]   = new_eta;
      seeds_phi[match_idx]   = new_phi;
      seeds_wsum[match_idx]  = new_wsum;
   }
   QT.clear();
   for(unsigned int i=0; i<seeds_eta.size(); i++) {
      float eta = seeds_eta[i] / seeds_wsum[i];
      float phi = seeds_phi[i] / seeds_wsum[i];
      seeds_eta[i] = eta;
      seeds_phi[i] = phi;
      if( phi < -TMath::Pi() ) phi =  2*TMath::Pi() + phi;
      if( phi >  TMath::Pi() ) phi = -2*TMath::Pi() + phi;
      seeds_phi[i] = phi;
   }
   ATH_MSG_VERBOSE("after clustering, nr of seeds = " << seeds_eta.size());

   // delete overlap (can happen at phi=-Pi/Pi bounadry)
   std::vector<unsigned int> idx_to_delete;
   for(unsigned int i=0; i<seeds_eta.size(); i++) {
      if( std::find(idx_to_delete.begin(),idx_to_delete.end(),i) != idx_to_delete.end() ) continue;
      float eta_i = seeds_eta[i];
      float phi_i = seeds_phi[i];
      for(unsigned int j=i+1; j<seeds_eta.size(); j++) {
	 if( std::find(idx_to_delete.begin(),idx_to_delete.end(),j) != idx_to_delete.end() ) continue;
	 float eta_j = seeds_eta[j];
	 float phi_j = seeds_phi[j];
	 float dR2 = deltaR2(eta_i,phi_i,eta_j,phi_j);
	 if( dR2 < CLUSTCUT_DIST_SQUARED ) idx_to_delete.push_back(j);
      }
   }
   ATH_MSG_VERBOSE("nr of duplicated seeds to be removed = " << idx_to_delete.size());
   if( idx_to_delete.size() > 0 ) {
      std::sort(idx_to_delete.begin(),idx_to_delete.end());
      for(unsigned int j=idx_to_delete.size(); j>0; j--) {
	 unsigned int idx = idx_to_delete[j-1];
	 seeds_eta.erase(seeds_eta.begin()+idx);
	 seeds_phi.erase(seeds_phi.begin()+idx);
      }
   }

   ATH_MSG_VERBOSE("nr of ly6/ly7 seeds=" << seeds_eta.size());

   // return
   return StatusCode::SUCCESS;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

StatusCode TrigHitDVHypoAlg::selectSeedsNearby(const xAOD::TrigCompositeContainer* hitDVSeedsContainer,
					       std::vector<float>& jetSeeds_eta, std::vector<float>& jetSeeds_phi, std::vector<float>& jetSeeds_pt) const
{
   std::vector<unsigned int> idx_to_delete;
   for(unsigned int idx=0; idx<jetSeeds_eta.size(); ++idx) {
      const float DR_SQUARED_CUT_TO_FTFSEED = 0.09; // const float DR_CUT_TO_FTFSEED = 0.3;
      float eta = jetSeeds_eta[idx];
      float phi = jetSeeds_phi[idx];
      float dR2min = 9999;
      for ( auto seed : *hitDVSeedsContainer ) {
	 float seed_eta  = seed->getDetail<float>("hitDVSeed_eta");
	 float seed_phi  = seed->getDetail<float>("hitDVSeed_phi");
	 float dR2 = deltaR2(eta,phi,seed_eta,seed_phi);
	 if( dR2 < dR2min ) dR2min = dR2;
      }
      if( dR2min > DR_SQUARED_CUT_TO_FTFSEED ) idx_to_delete.push_back(idx);
   }
   if( idx_to_delete.size() > 0 ) {
      std::sort(idx_to_delete.begin(),idx_to_delete.end());
      for(unsigned int j=idx_to_delete.size(); j>0; j--) {
	 unsigned int idx = idx_to_delete[j-1];
	 jetSeeds_eta.erase(jetSeeds_eta.begin()+idx);
	 jetSeeds_phi.erase(jetSeeds_phi.begin()+idx);
	 if( jetSeeds_pt.size() > 0 ) jetSeeds_pt.erase(jetSeeds_pt.begin()+idx);
      }
   }
   return StatusCode::SUCCESS;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
