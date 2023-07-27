///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// PhysValBTag.cxx
// Implementation file for class PhysValBTag
// Author: E.Schopf<elisabeth.schopf@cern.ch>
// Updates: J.Hoefer <judith.hoefer@cern.ch>
///////////////////////////////////////////////////////////////////
// JetTagDQA includes
#include "PhysValBTag.h"

// STL includes
#include <vector>

// FrameWork includes
#include "GaudiKernel/IToolSvc.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"
#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingUtilities.h"

#include "AthenaBaseComps/AthCheckMacros.h"
#include "ParticleJetTools/JetFlavourInfo.h"

namespace JetTagDQA {

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

  // Constructors
  ////////////////

  PhysValBTag::PhysValBTag( const std::string& type,
                            const std::string& name,
                            const IInterface* parent ) :
    ManagedMonitorToolBase( type, name, parent ),
    m_muonAugmenter("Muons"),
    m_isData(false),
    m_antiKt4EMTopoPlots                     (0, "BTag/AntiKt4EMTopoJets/"                , "antiKt4EMTopoJets"),
    m_antiKt4EMPFlowJetsPlots                (0, "BTag/AntiKt4EMPFlowJets/"               , "antiKt4EMPFlowJets"),
    m_antiKtVR30Rmax4Rmin02PV0TrackJetsPlots (0, "BTag/AntiKtVR30Rmax4Rmin02PV0TrackJets/", "antiKtVR30Rmax4Rmin02PV0TrackJets"),
    m_nevents(0)
  {
 
    declareProperty( "isData", m_isData );

    declareProperty( "JetContainerEMTopo", m_jetNameEMTopo = "AntiKt4EMTopoJets" );
    declareProperty( "JetContainerPFlow", m_jetNamePFlow = "AntiKt4EMPFlowJets");
    declareProperty( "JetContainerTrackJets", m_jetNameTrackJet = "AntiKtVR30Rmax4Rmin02PV0TrackJets");

    declareProperty( "TrackContainerName", m_trackName = "InDetTrackParticles" );
    declareProperty( "VertexContainerName", m_vertexName = "PrimaryVertices" );

    declareProperty( "JetPtCutTtbar", m_jetPtCutTtbar = 20000);
    declareProperty( "JetPtCutZprime", m_jetPtCutZprime = 500000);
    declareProperty( "JetEtaCut", m_jetEtaCut = 2.5);
    declareProperty( "JVTCutAntiKt4EMTopoJets", m_JVTCutAntiKt4EMTopoJets = 0.59);
    declareProperty( "JVTCutLargerEtaAntiKt4EMTopoJets", m_JVTCutLargerEtaAntiKt4EMTopoJets = 0.11);
    declareProperty( "JVTCutAntiKt4EMPFlowJets", m_JVTCutAntiKt4EMPFlowJets = 0.2);
    declareProperty( "truthMatchProbabilityCut", m_truthMatchProbabilityCut = 0.75);

    declareProperty( "dipsTaggerName", m_dipsName = "dipsLoose20220314v2");
    declareProperty( "DL1dv00TaggerName", m_DL1dv00Name = "DL1dv00");
    declareProperty( "DL1dv01TaggerName", m_DL1dv01Name = "DL1dv01");
    declareProperty( "GN1TaggerName", m_GN1Name = "GN120220509");

  }

  // Destructor
  ///////////////
  PhysValBTag::~PhysValBTag()
  {}

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode PhysValBTag::initialize()
  {
    ATH_MSG_INFO ("Initializing " << name() << "...");
    ATH_CHECK(ManagedMonitorToolBase::initialize());

    // initialize the truth-track-assoiation tool
    ATH_CHECK(m_trackTruthOriginTool.retrieve( EnableTool {true} ));

    // convert the HistogramDefinitions vector to a map 
    for(unsigned int i = 0; i < m_HistogramDefinitionsVector.size(); i++){
      std::string name = m_HistogramDefinitionsVector[i][0];
      m_HistogramDefinitionsMap.insert(std::pair< std::string, std::vector< std::string > >(name, m_HistogramDefinitionsVector[i]));
    }
    if(!m_jetNameEMTopo.empty()){
    m_btagplots.insert(std::make_pair(m_jetNameEMTopo, &m_antiKt4EMTopoPlots));
    }
    m_btagplots.insert(std::make_pair(m_jetNamePFlow, &m_antiKt4EMPFlowJetsPlots));
    m_btagplots.insert(std::make_pair(m_jetNameTrackJet, &m_antiKtVR30Rmax4Rmin02PV0TrackJetsPlots));

    for(const auto& [name, plot]: m_btagplots){
      plot->setDetailLevel(m_detailLevel);
      plot->setHistogramDefinitions(m_HistogramDefinitionsMap);
      plot->setIsDataJVTCutsAndTMPCut(m_isData,
				      m_JVTCutAntiKt4EMTopoJets,
				      m_JVTCutLargerEtaAntiKt4EMTopoJets,
				      m_JVTCutAntiKt4EMPFlowJets,
				      m_truthMatchProbabilityCut);
      plot->setTaggerNames(m_dipsName, m_DL1dv00Name, m_DL1dv01Name, m_GN1Name);
    }
   
    return StatusCode::SUCCESS;
  }

  StatusCode PhysValBTag::book(PlotBase& plots)
  {
    plots.initialize();
    std::vector<HistData> hists = plots.retrieveBookedHistograms();

    for (auto& hist : hists){
      ATH_MSG_INFO ("Initializing " << hist.first << " " << hist.first->GetName() << " " << hist.second << "...");
      ATH_CHECK(regHist(hist.first,hist.second,all));
    }
    return StatusCode::SUCCESS;
  }

  StatusCode PhysValBTag::bookHistograms()
  {
    ATH_MSG_INFO ("Booking hists " << name() << "...");

    if (m_detailLevel < 10) return StatusCode::SUCCESS;

    for(const auto& [name, plot] : m_btagplots ){
      ATH_CHECK(book(*plot));
    }
      
    return StatusCode::SUCCESS;
  }

  StatusCode PhysValBTag::fillHistograms()
  {
    ATH_MSG_INFO ("Filling hists " << name() << "...");
    
    if (m_detailLevel < 10) return StatusCode::SUCCESS;
    
    ++m_nevents;
    //std::cout << "Number of proccessed events = " << m_nevents << std::endl;

    // event info
    const xAOD::EventInfo* event(0);
    ATH_CHECK(evtStore()->retrieve(event, "EventInfo"));

    // determine if the sample is ttbar or Zprime (on the first event, where the jetPtCut is still initial -1)
    if(m_jetPtCut < 0){
      // get the DSID
      int dsid = event->mcChannelNumber();

      // check if it is a ttbar or Zprime sample
      if(dsid == 410000 || dsid == 601229){
        m_jetPtCut = m_jetPtCutTtbar;
      }
      else if(dsid == 427080 || dsid == 427081 ||dsid == 801271 || dsid == 800030) {
        m_jetPtCut = m_jetPtCutZprime;
        m_onZprime = true;
      }
      // if none applies give a warning and use the default cut
      else {
        ATH_MSG_WARNING("It is checked if the sample is ttbar (has dsid 410000) or Zprime (has dsid 427080). None applies (read dsid is " << dsid << "). Applying default pT cut of 20000 MeV now.");
        m_jetPtCut = 20000;
      }

      // do not do the track-truth association on this Sherpa Z' sample
      if(dsid == 361405){
        m_doTrackTruth = false;
      }
    }

    // get the primary vertex
    const xAOD::VertexContainer *vertices = 0;
    CHECK( evtStore()->retrieve(vertices, "PrimaryVertices") );
    int npv(0);
    size_t indexPV = 0;
    bool has_pv = false;
    xAOD::VertexContainer::const_iterator vtx_itr = vertices->begin();
    xAOD::VertexContainer::const_iterator vtx_end = vertices->end();
    int count = -1;
    // loop over the vertices
    for (; vtx_itr != vtx_end; ++vtx_itr) {
      count++;
      if ((*vtx_itr)->nTrackParticles() >= 2) {
        npv++;
        if ((*vtx_itr)->vertexType() == 1) {
          if (m_PV_x != -999.) ATH_MSG_WARNING( ".... second PV in the events ...!!!!!!");
          indexPV = count;
          has_pv = true;
          m_PV_x = (*vtx_itr)->x();
          m_PV_y = (*vtx_itr)->y();
          m_PV_z = (*vtx_itr)->z();
        }
      }
    }
    if (!has_pv) {
      //ATH_MSG_WARNING( ".... rejecting the event due to missing PV!!!!");
      return StatusCode::SUCCESS;
    }
    const xAOD::Vertex *myVertex = vertices->at(indexPV); // the (reco?) primary vertex
    //std::cout<<"z coordinate of PV: "<< myVertex->z() <<std::endl;

    // get the tracks
    const xAOD::TrackParticleContainer* tracks(0);
    ATH_CHECK(evtStore()->retrieve(tracks, m_trackName));

    // loop over the jet collections
    for(const auto& [name, plot] : m_btagplots){
      
      // get the jets
      const xAOD::JetContainer* jets(0);
      StatusCode jetException = evtStore()->retrieve(jets, name);
      // If StatusCode is not SUCCESS, no jet collection with this name, continue loop
      if (jetException != StatusCode::SUCCESS) continue;

      int nJets_withCut = 0;
      int nJets_containing_moun = 0; 
      int nJets_containing_SV = 0; 
      std::map<std::string, int> nJetsThatPassedWPCuts;
      plot->initializeNJetsThatPassedWPCutsMap(nJetsThatPassedWPCuts);

      // check if the muon info is available on the first jet (since isAvailable gives the same result on all jets)
      bool muon_info_available = false;
      if(jets->size() > 0){
        const xAOD::BTagging* btag = xAOD::BTaggingUtilities::getBTagging( *(jets->at(0)) );
        if(btag && btag->isAvailable< ElementLink<xAOD::MuonContainer> >("softMuon_link") ){
          muon_info_available = true;
        }
      }

      // loop over the jets
      for (auto jet : *jets) {

        // apply the jet pT eta and jvt cuts
        if(jet->pt() <= m_jetPtCut) continue;
        if(std::abs(jet->eta()) >= m_jetEtaCut) continue;
        //Arnaud: JVT cut to remove horns in jet eta 
        if (plot->m_JVT_defined && jet->getAttribute<float>("Jvt") < plot->m_JVT_cut
	    && jet->pt() > 20e3 && jet->pt() < 60e3
	    && std::abs(jet->eta()) < 2.4 ) continue;
        if (plot->m_JVTLargerEta_defined && jet->getAttribute<float>("Jvt") < plot->m_JVTLargerEta_cut
	    && jet->pt() > 20e3 && jet->pt() < 60e3
	    && std::abs(jet->eta()) > 2.4 && std::abs(jet->eta()) < m_jetEtaCut ) continue;

        // get the btagging
        const xAOD::BTagging* btag = xAOD::BTaggingUtilities::getBTagging( *jet );

        // count the jets that pass the cuts
        nJets_withCut++;

        // get the jet truth label
        int truth_label(1000);
        if(!m_isData){
          if(jet->isAvailable<int>("HadronConeExclTruthLabelID")) jet->getAttribute("HadronConeExclTruthLabelID", truth_label);
          else jet->getAttribute("TruthLabelID",truth_label);    
        }

        // fill the jet related histograms
        plot->fillJetKinVars(jet, truth_label, m_onZprime, event);

        // fill the jet, btag & vertex related plots
        if (btag){
          // augment with muon information
          if(! muon_info_available){
            m_muonAugmenter.augment(*btag);
          }

          // fill other variables
          bool contains_muon;
          double jet_Lxy = -1;
          plot->fillOther(jet, btag, contains_muon, jet_Lxy, truth_label, event);
          if(contains_muon) nJets_containing_moun++;

          // get the track to truth associations
          std::map<const xAOD::TrackParticle*, int> track_truth_associations = getTrackTruthAssociations(btag);

          // fill track related variables
          int num_HF_tracks_in_jet;
          plot->fillTrackVariables(jet, btag, myVertex, track_truth_associations, contains_muon, truth_label, num_HF_tracks_in_jet, event);
          // fill SV related vars
          bool contains_SV;
          plot->fillSVVariables(btag, track_truth_associations, contains_muon, truth_label, num_HF_tracks_in_jet, contains_SV, event);
          if(contains_SV) nJets_containing_SV++;
          // fill discriminant related vars
          plot->fillDiscriminantVariables(btag, jet, jet_Lxy, truth_label, contains_muon, m_onZprime, nJetsThatPassedWPCuts, event);
        }
        else{
          ATH_MSG_WARNING("btag (obtained by xAOD::BTaggingUtilities::getBTagging(*jet)) is a null pointer.");
        }
      }

      // fill multiplicities
      plot->fillMultiplicities(nJets_withCut, tracks->size(), npv, myVertex->nTrackParticles(), nJets_containing_moun, nJets_containing_SV, nJetsThatPassedWPCuts, event);
      // fill PV variables
      plot->fillPVVariables(m_PV_x, m_PV_y, m_PV_z, event);

    }

    return StatusCode::SUCCESS;
  }

  StatusCode PhysValBTag::procHistograms()
  {
    ATH_MSG_INFO ("Finalising hists " << name() << "...");

    for(const auto& [name, plot] : m_btagplots ){
      plot->finalize();

    }
    return StatusCode::SUCCESS;
  }

  ///////////////////////////////////////////////////////////////////
  // Const methods:
  ///////////////////////////////////////////////////////////////////

  std::map<const xAOD::TrackParticle*, int> PhysValBTag::getTrackTruthAssociations(const xAOD::BTagging* btag) const {

    // define the return vector
    std::map<const xAOD::TrackParticle*, int> truthValues;

    // get the track links from te btag
    std::vector< ElementLink< xAOD::TrackParticleContainer > > assocTracks = btag->auxdata<std::vector<ElementLink<xAOD::TrackParticleContainer> > >("BTagTrackToJetAssociator");

    // loop over the tracks associated to the btag and get the truth values
    for(unsigned int i = 0; i < assocTracks.size(); i++) {
      if (!assocTracks.at(i).isValid()) continue;

      // get the curent track
      const xAOD::TrackParticle* track = *(assocTracks.at(i));  

      // only try accessing the truth values if not on data
      int origin = 0;
      if(!m_isData && m_doTrackTruth){
        origin = m_trackTruthOriginTool->getTrackOrigin(track);
      }

      // add the truth values to the vector
      truthValues.insert( std::make_pair( track, origin ) );
    }

    // also loop over the tracks associated to the MSV vertices -> can be missing in the other track list
    // get the MSV vertices
    std::vector< ElementLink< xAOD::VertexContainer > > MSV_vertices;
    try { MSV_vertices = btag->auxdata<std::vector< ElementLink< xAOD::VertexContainer > > >("MSV_vertices"); }
    catch(std::exception& exception) {  }

    // loop over the MSV vertices
    for (unsigned int i = 0; i < MSV_vertices.size(); i++) {
      if (!MSV_vertices.at(i).isValid()) continue;
      const xAOD::Vertex* vtx = *(MSV_vertices.at(i));

      // get the track links
      std::vector<ElementLink<DataVector<xAOD::TrackParticle> > > MSV_assocTracks = vtx->trackParticleLinks();

      // loop over the tracks
      for(unsigned int i = 0; i < MSV_assocTracks.size(); i++) {
        if (!MSV_assocTracks.at(i).isValid()) continue;

        // get the curent track
        const xAOD::TrackParticle* track = *(MSV_assocTracks.at(i));  
        
        // only try accessing the truth values if not on data
        int origin = 0;
        if(!m_isData && m_doTrackTruth){
          origin = m_trackTruthOriginTool->getTrackOrigin(track);
        }

        // add the truth values to the vector
        truthValues.insert( std::make_pair( track, origin ) );
      }
    }

    // return
    return truthValues;
  }


}

//  LocalWords:  str
