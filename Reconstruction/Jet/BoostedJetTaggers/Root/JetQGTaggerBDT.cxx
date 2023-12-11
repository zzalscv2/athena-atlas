/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "BoostedJetTaggers/JetQGTaggerBDT.h"

#include <TSystem.h>

#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"

#include "xAODTracking/VertexContainer.h"

namespace CP {

  JetQGTaggerBDT::JetQGTaggerBDT( const std::string& name ) :
    JSSTaggerBase( name ),
    m_BDTmethod("BDT_method"),
    m_trkSelectionTool(name+"_trackselectiontool", this)
  {

    /// Jet kinematics
    declareProperty( "JetPtMin",              m_jetPtMin = 20000.0);
    declareProperty( "JetPtMax",              m_jetPtMax = 1500000.0);
    m_jetEtaMax = 2.5; /// Replace base class default value for JetEtaMax

    /// Tagger configuration
    m_calibArea = "BoostedJetTaggers/JetQGTaggerBDT/Oct18/"; /// Overwrite base class default
    declareProperty( "UseJetVars",  m_mode = 1); /// 0 uses the tracks. 1 uses variables from the jets (default)

    m_tmvaConfigFileName="TMVAClassification_BDTQGTagger_Oct18_BDT.weights.xml";

  }

  StatusCode JetQGTaggerBDT::initialize() {

    ATH_MSG_INFO( "Initializing JetQGTaggerBDT tool" );

    if( ! m_configFile.empty() ) {

      /// Get configReader
      ATH_CHECK( getConfigReader() );

      // get the name/path of the JSON config
      m_tmvaConfigFileName = m_configReader.GetValue("TMVAConfigFile" ,"");

    }

    /// if the calibArea is specified to be "Local" then it looks in the same place as the top level configs
    if( m_calibArea.empty() ){
      ATH_MSG_ERROR( "You need to specify where the calibArea is as either being Local or on CVMFS" );
      return StatusCode::FAILURE;
    }
    else if(m_calibArea.compare("Local")==0){
      std::string localCalibArea = "BoostedJetTaggers/share/JetQGTaggerBDT/";
      ATH_MSG_INFO( "Using Local calibArea " << localCalibArea );
      /// convert the JSON config file name to the full path
      m_tmvaConfigFilePath = PathResolverFindCalibFile(localCalibArea+m_tmvaConfigFileName);
    }
    else{
      ATH_MSG_INFO( "Using CVMFS calibArea" );
      /// get the config file from CVMFS
      /// necessary because xml files are too large to house on the data space
      m_tmvaConfigFilePath = PathResolverFindCalibFile( ("BoostedJetTaggers/"+m_calibArea+"/"+m_tmvaConfigFileName).c_str() );
    }

    /// Make sure score cut string is not empty
    m_strScoreCut = m_configReader.GetValue("ScoreCut" ,"");
    if(m_strScoreCut.empty()){
      ATH_MSG_ERROR( "Score cut function is empty!" );
      return StatusCode::FAILURE;
    }
    else  ATH_MSG_INFO( "scoreCut: " << m_strScoreCut );
    
    /// set up InDet selection tool
    ANA_CHECK( ASG_MAKE_ANA_TOOL( m_trkSelectionTool,  InDet::InDetTrackSelectionTool ) );
    ANA_CHECK( m_trkSelectionTool.setProperty( "CutLevel", "Loose" ) );
    ANA_CHECK( m_trkSelectionTool.retrieve() );

    /// read json file for DNN weights
    ATH_MSG_INFO( "BDT Tagger configured with: " << m_tmvaConfigFilePath );

    /// -- Initialize TMVA for BDTs for each slot
    TMVA::Tools::Instance();
    for (auto& tagger : m_bdtTagger) {
      tagger.tmva = std::make_unique<TMVA::Reader>( "!Color:!Silent" );

      tagger.tmva->AddVariable( "NTracks", &tagger.ntracks );
      tagger.tmva->AddVariable( "TrackWidth", &tagger.trackwidth );
      tagger.tmva->AddVariable( "JetPt",  &tagger.pt );
      tagger.tmva->AddVariable( "JetEta", &tagger.eta );
      tagger.tmva->AddVariable( "TrackC1", &tagger.trackC1 );

      /// configure the bdt
      tagger.tmva->BookMVA( m_BDTmethod.c_str(), m_tmvaConfigFilePath.c_str() );
    }

    /// Initialize WriteDecorHandle keys
    m_decScoreKey = m_containerName + "." + m_decScoreKey.key();

    ATH_CHECK( m_decScoreKey.initialize() );

    /// Call base class initialize
    ATH_CHECK( JSSTaggerBase::initialize() );

    return StatusCode::SUCCESS;
  
  }

  StatusCode JetQGTaggerBDT::tag( const xAOD::Jet& jet ) const {
    
    ATH_MSG_DEBUG( "Obtaining BDT QG result" );

    /// Create asg::AcceptData object
    asg::AcceptData acceptData( &m_acceptInfo );

    /// Reset the AcceptData cut results
    ATH_CHECK( resetCuts( acceptData ) );

    /// Check basic kinematic selection
    ATH_CHECK( checkKinRange( jet, acceptData ) );

    /// Create WriteDecorHandles
    SG::WriteDecorHandle<xAOD::JetContainer, bool> decTagged(m_decTaggedKey);
    SG::WriteDecorHandle<xAOD::JetContainer, float> decScore(m_decScoreKey);

    /// TODO: Is this actually needed?
    if ( !passKinRange(jet) ) {
      decTagged(jet) = false;
      return StatusCode::SUCCESS;
    }

    /// Get BDT score
    float jet_score = getScore( jet, acceptData );
    ATH_MSG_DEBUG(TString::Format("jet score %g",jet_score) );

    /// Get cut from cut function
    float cut = m_funcScoreCut->Eval(jet.pt()/1000.);

    /// Decorate score to jet
    decScore(jet) = jet_score;

    /// Check score against cut and decorate result
    if ( jet_score < cut ) acceptData.setCutResult("QuarkJetTag", true);
    decTagged(jet) = acceptData.getCutResult( "QuarkJetTag" );

    return StatusCode::SUCCESS;

  }

  float JetQGTaggerBDT::getScore( const xAOD::Jet& jet, asg::AcceptData &acceptData ) const {

    /// Load the new values of the variables for this jet
    bool validVars = getJetProperties( jet, acceptData );

    /// evaluate bdt
    float bdt_score(-666.);
    if ( !validVars ) {
      ATH_MSG_WARNING( "One (or more) tagger input variable has an undefined value (NaN), setting score to -666" );
      return bdt_score;
    }
    bdt_score = m_bdtTagger->tmva->EvaluateMVA( m_BDTmethod.c_str() );

    return bdt_score;
  }

  bool JetQGTaggerBDT::getJetProperties( const xAOD::Jet& jet, asg::AcceptData &acceptData ) const {
    /* Update the jet substructure variables for this jet */

    m_bdtTagger->pt  = jet.pt()/1000.0;
    m_bdtTagger->eta = jet.eta();

    ATH_MSG_DEBUG( TString::Format("pT: %g, eta: %g", m_bdtTagger->pt, m_bdtTagger->eta) );

    m_bdtTagger->ntracks = -1.;
    m_bdtTagger->trackwidth = -1.;
    m_bdtTagger->trackC1 = -1.;

    bool validVars = true;

    if ( m_mode == 1 ) {
      validVars = getPrecomputedVariables( jet, acceptData );
    }
    else if( m_mode == 0 ) {
      validVars = calculateVariables( jet, acceptData );
    }

    if ( !validVars ) {
      ATH_MSG_ERROR( "Can't determine QG tagging variables! Try different mode." );
    }
    
    return validVars;
  
  }

  bool JetQGTaggerBDT::getPrecomputedVariables( const xAOD::Jet& jet, asg::AcceptData &acceptData ) const {

    bool validVars = true;

    int ntrk = -1;
    float trkWidth = -1.;
    float trkC1 = -1.;

    if ( !jet.getAttribute<int>("DFCommonJets_QGTagger_NTracks", ntrk) ) {
      if ( m_nWarnVar++ < m_nWarnMax ) ATH_MSG_WARNING( "Unable to retrieve DFCommonJets_QGTagger_NTracks" );
      else ATH_MSG_DEBUG( "Unable to retrieve DFCommonJets_QGTagger_NTracks" );
      acceptData.setCutResult("ValidEventContent", false);
      validVars = false;
    }
    if ( !jet.getAttribute<float>("DFCommonJets_QGTagger_TracksWidth", trkWidth) ) {
      if ( m_nWarnVar++ < m_nWarnMax )ATH_MSG_WARNING( "Unable to retrieve DFCommonJets_QGTagger_TracksWidth" );
      else ATH_MSG_DEBUG( "Unable to retrieve DFCommonJets_QGTagger_TracksWidth" );
      acceptData.setCutResult("ValidEventContent", false);
      validVars = false;
    }
    if ( !jet.getAttribute<float>("DFCommonJets_QGTagger_TracksC1", trkC1) ) {
      if ( m_nWarnVar++ < m_nWarnMax ) ATH_MSG_WARNING( "Unable to retrieve DFCommonJets_QGTagger_TracksC1" );
      else ATH_MSG_DEBUG( "Unable to retrieve DFCommonJets_QGTagger_TracksC1" );
      acceptData.setCutResult("ValidEventContent", false);
      validVars = false;
    }

    m_bdtTagger->ntracks = (float) ntrk;
    m_bdtTagger->trackwidth = trkWidth;
    m_bdtTagger->trackC1 = trkC1;

    return validVars;
  
  }

  bool JetQGTaggerBDT::calculateVariables( const xAOD::Jet& jet, asg::AcceptData &acceptData ) const {
    ///calculate q/g tagging variables from GhostTracks associated to jet
    ///some derivations apply slimming to these tracks, which would lead to wrong values.
    ///so we compare the number of GhostTracks to NumTrkPt500 (i.e. nTracks)
    ///      if they are "close enough" we can proceed

    bool validVars = true;
    bool isValid = true;
    const xAOD::Vertex* primvertex {nullptr};

    const xAOD::VertexContainer* vxCont = nullptr;
    if ( evtStore()->retrieve( vxCont, "PrimaryVertices" ).isFailure() ) {
      if ( m_nWarnVar++ < m_nWarnMax ) ATH_MSG_WARNING( "Unable to retrieve primary vertex container PrimaryVertices" );
      else ATH_MSG_DEBUG( "Unable to retrieve primary vertex container PrimaryVertices" );
      acceptData.setCutResult("ValidEventContent", false);
      isValid = false;
    }
    else if ( vxCont->empty() ) {
      if ( m_nWarnVar++ < m_nWarnMax ) ATH_MSG_WARNING( "Event has no primary vertices!" );
      ATH_MSG_DEBUG( "Event has no primary vertices!" );
      acceptData.setCutResult("ValidEventContent", false);
      isValid = false;
    }
    else {
      for ( const auto *vx : *vxCont ) {
        /// take the first vertex in the list that is a primary vertex
        if ( vx->vertexType()==xAOD::VxType::PriVtx ) {
          primvertex = vx;
          break;
        }
      }
    }
    if ( !primvertex ) isValid = false;

    if ( !isValid ) {
      validVars = false;
      return validVars;
    }

    ///NTracks
    std::vector<int> nTrkVec;
    if(jet.getAttribute(xAOD::JetAttribute::NumTrkPt500, nTrkVec)){
      ATH_MSG_DEBUG(nTrkVec.size());
      m_bdtTagger->ntracks = (float) nTrkVec[primvertex->index()];
    }
    else
      ///if NumTrkPt500 is not available, I can't confirm that the number of GhostTracks is correct (i.e. unslimmed)
      validVars = false;

    ///TrackWidth
    bool undefTrackWidth = false;
    std::vector<float> trkWidthVec;
    if(jet.getAttribute(xAOD::JetAttribute::TrackWidthPt500, trkWidthVec)){
      ATH_MSG_DEBUG(trkWidthVec.size());
      m_bdtTagger->trackwidth = trkWidthVec[primvertex->index()];
    }
    else
      ///if TrackWidthPt500 is not available, we can maybe calculate it from tracks
      undefTrackWidth = true;
    float weightedwidth = 0.;

    ///TrackC1
    float beta = 0.2;
    float weightedwidth2 = 0.;
    float sumPt = 0.;

    std::vector<const xAOD::TrackParticle*> trackParttmp;
    if(!jet.getAssociatedObjects("GhostTrack",trackParttmp)){
      ATH_MSG_ERROR("This jet has no associated objects");
      validVars = false;
    }
    ///track selection
    for(unsigned i=trackParttmp.size();i>0; i--){
      if(!trackParttmp[i-1]){
        trackParttmp.erase(trackParttmp.begin()+i-1);
        continue;
      }
      const xAOD::TrackParticle* trk = static_cast<const xAOD::TrackParticle*>(trackParttmp[i-1]);
      bool accept = (trk->pt()>500 &&
          m_trkSelectionTool->accept(*trk)
      );
      if (!accept){
        trackParttmp.erase(trackParttmp.begin()+i-1);
      }
    }

    if(! isCorrectNumberOfTracks(m_bdtTagger->ntracks,trackParttmp.size())){
      ATH_MSG_ERROR("Number of ghost associated tracks wrong!");
      validVars = false;
    }

    ///calculate TrackC1 (and TrackWidth if necessary)
    for(unsigned i=0; i<trackParttmp.size(); i++){
      double ipt = trackParttmp.at(i)->pt();
      double ieta = trackParttmp.at(i)->eta();
      double iphi = trackParttmp.at(i)->phi();
      sumPt += ipt;
      if(undefTrackWidth){
        double deta_i = trackParttmp.at(i)->eta() - jet.eta();
        double dphi_i = TVector2::Phi_mpi_pi(trackParttmp.at(i)->phi() - jet.phi());
        double dR_i = sqrt( deta_i*deta_i + dphi_i*dphi_i );
        weightedwidth += ipt * dR_i;
      }

      for(unsigned j=i+1; j<trackParttmp.size(); j++){
        double deta = ieta - trackParttmp.at(j)->eta();
        double dphi = TVector2::Phi_mpi_pi(iphi - trackParttmp.at(j)->phi());
        double dR = sqrt( deta*deta + dphi*dphi );
        weightedwidth2 += ipt * trackParttmp.at(j)->pt() * pow(dR,beta);
      }
    }

    if(undefTrackWidth) {
      m_bdtTagger->trackwidth = sumPt>0 ? weightedwidth/sumPt : -0.1;
    }
    m_bdtTagger->trackC1 = sumPt>0 ? weightedwidth2/(sumPt*sumPt) : -0.1;

    return validVars;
  }

  bool JetQGTaggerBDT::isCorrectNumberOfTracks(int expectedNTracks, int nTracksFromGhostTracks) const{
    ///some derivations do not store all tracks associated to the jet.
    ///In this case the calculation of the tagging variables will be wrong.
    ///The requirements are fairly loose, because a few tracks may get lost in the derivation production.
    ///But it will fail quickly if the too many tracks were slimmed away.
    if(nTracksFromGhostTracks == 0){
      if(expectedNTracks == 0)
        return true;
      return abs(expectedNTracks-nTracksFromGhostTracks) < 3;
    }else if(expectedNTracks/nTracksFromGhostTracks < 0.5 && abs(expectedNTracks-nTracksFromGhostTracks) > 5){
      return false;
    }
    return true;
  }

} /* namespace CP */

/// the end
