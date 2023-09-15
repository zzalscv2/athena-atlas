/* emacs: this is -*- c++ -*- */
/**
 **     @file    T_AnalysisConfigR3_Tier0.h
 **
 **     @brief   baseclass template so that we can use in different contexts 
 **              in different ways in the monitoring
 ** 
 **       NB: this will be configured to run *either* a standard 
 **       analysis, or a "purity" analysis. If a purity analysis, 
 **       the trigger tracks become the reference (with all the 
 **       selection) and the offline or truth the "test" tracks
 **       This would be a simple switch if the reference tracks
 **       were in the RoI, but as they are not we need to move the 
 **       RoI filtering to the test filter and *not* the reference 
 **       filter grrrrrr  
 **
 **     @author  mark sutton
 **     @date    Tue 16 May 2017 09:28:55 CEST 
 **
 **     Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef TrigInDetAnalysisExample_T_AnalysisConfigR3_Tier0_H
#define TrigInDetAnalysisExample_T_AnalysisConfigR3_Tier0_H

#include "TrigInDetAnalysis/TIDAEvent.h"
#include "TrigInDetAnalysis/TIDAVertex.h"
#include "TrigInDetAnalysis/TrackSelector.h"     
#include "TrigInDetAnalysisUtils/T_AnalysisConfig.h"
#include "TrigInDetAnalysisUtils/TagNProbe.h"

#include "TrigInDetAnalysisExample/AnalysisR3_Tier0.h"
#include "TrigInDetAnalysisExample/VtxAnalysis.h"
#include "TrigInDetAnalysisExample/ChainString.h"
#include "TrigInDetAnalysisExample/TIDATools.h"

#include "TTree.h"
#include "TFile.h"

#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
 

// McParticleEvent includes
#include "McParticleEvent/TruthParticleContainer.h"

#include "GeneratorObjects/McEventCollection.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"

#include "xAODEventInfo/EventInfo.h"



#include "TrigInDetAnalysis/TIDDirectory.h"

#include "TrigInDetAnalysis/Filter_AcceptAll.h"

#include "TrigInDetAnalysisUtils/TIDARoiDescriptorBuilder.h"
#include "TrigInDetAnalysisUtils/Filter_etaPT.h"
#include "TrigInDetAnalysisUtils/Filter_RoiSelector.h"
#include "TrigInDetAnalysisUtils/Associator_BestMatch.h"
#include "TrigInDetAnalysisUtils/Filters.h"

                            

#include "VxVertex/VxContainer.h"

#include "muonEvent/MuonContainer.h"

#include "egammaEvent/ElectronContainer.h"

#include "tauEvent/TauJetContainer.h"


#include "TrigSteeringEvent/HLTResult.h"
#include "TrigDecisionTool/ExpertMethods.h"

// #include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"

#include "TrigCompositeUtils/TrigCompositeUtils.h"


template<typename T>
std::ostream& operator<<( std::ostream& s, const std::vector<T>& v) { 
  for ( size_t i=0 ; i<v.size() ; i++ ) s << "  " << v[i];
  return s;
}


template<typename T, typename A=AnalysisR3_Tier0>
class T_AnalysisConfigR3_Tier0 : public T_AnalysisConfig<T> {

public:

  // Full constructor: test/reference/selection
  // - analysisInstanceName: the name of the analysis chain being created
  // - xxxChainName: the name of the chain to be used as test/reference/selection; must be "StoreGate" in case of direct access to SG containers
  // - xxxType: the type of tracks to be retrieved from the test/reference/selection chain or container
  // - xxxKey:  the key for tracks to be retrieved from the test/reference/selection chain or container
  // - roiInfo: in case the test chain is a real chain, this is used to specify RoI widths; in case the test chain is a fake chain, this is used for RoI position too
  // - all standard operations are performed in loops over 0=test 1=reference 2=selection
  T_AnalysisConfigR3_Tier0(const std::string& analysisInstanceName,
			   const std::string& testChainName,      const std::string& testType,      const std::string& testKey,
			   const std::string& referenceChainName, const std::string& referenceType, const std::string& referenceKey,
			   TrackFilter*     testFilter,  TrackFilter*     referenceFilter, 
			   TrackAssociator* associator,
			   TrackAnalysis*   analysis,
			   TagNProbe*      TnP_tool = 0) :
    T_AnalysisConfig<T>( analysisInstanceName,
			 testChainName,      testType,      testKey,
			 referenceChainName, referenceType, referenceKey,
			 testFilter, referenceFilter,
			 associator,
			 analysis),
    m_useBeamCondSvc(false),
    m_doOffline(true),
    m_doMuons(false),
    m_doElectrons(false),
    m_doTaus(false),
    m_doBjets(false),
    m_pdgID(0),
    m_parent_pdgID(0),
    m_NRois(0),
    m_NRefTracks(0),
    m_NTestTracks(0),
    m_runPurity(false),
    m_shifter(false),
    m_pTthreshold(0),
    m_first(true),
    m_containTracks(false), 
    m_tnp_flag(false),
    m_invmass(0),
    m_invmass_obj(0)
  {

    /// leave in for development
    ///    std::cout << "chain size: " << m_chainNames.size() << "(" << this << ")" << std::endl; 

    m_chainNames.push_back(testChainName);
 
    ///    std::cout << "chain size: " << m_chainNames.size() << "(" << this << ")" << std::endl; 

    m_TnP_tool = TnP_tool;
    
#if 0
    /// leave this code here for debugging purposes ...
    ChainString& chain = m_chainNames.back(); 

    std::cout << "\nT_AnalysisConfigR3_Tier0::name:                " << name() << "\t" << this << std::endl;
    std::cout <<  "T_AnalysisConfigR3_Tier0::chain specification: " << testChainName << " -> " << chain << "\t" << chain.raw() << std::endl;
    std::cout << "\tchain: " << chain.head()    << std::endl;
    std::cout << "\tkey:   " << chain.tail()    << std::endl;
    std::cout << "\troi:   " << chain.roi()     << std::endl;
    std::cout << "\tvtx:   " << chain.vtx()     << std::endl;
    std::cout << "\tte:    " << chain.element() << std::endl;
    std::cout << "\textra: " << chain.extra()   << std::endl;

    std::cout << "\tpost:  " << chain.post()          << std::endl; 
    std::cout << "\tpt:    " << chain.postvalue("pt") << std::endl;

    std::cout << "\tcontainTracks: " << m_containTracks << std::endl;

#endif
    
    m_testType = testType;

    if ( m_TnP_tool ) m_tnp_flag = true;

  }


  virtual ~T_AnalysisConfigR3_Tier0() {
    if ( m_TnP_tool )    delete m_TnP_tool;
    if ( m_invmass )     delete m_invmass;
    if ( m_invmass_obj ) delete m_invmass_obj;
  }


  void initialise() { 

    if ( m_tnp_flag ) {
      if ( m_invmass==0 )     m_invmass     = new TIDA::Histogram<float>( monTool(), "invmass" );
      if ( m_invmass_obj==0 ) m_invmass_obj = new TIDA::Histogram<float>( monTool(), "invmass_obj" );
    }

    for ( size_t it=0 ; it<m_types.size() ; it++ ) {
      if ( m_types[it]=="" ) m_offline_types.push_back( "InDetTrackParticles" );
      else                   m_offline_types.push_back( m_types[it] );      
    }

    if ( m_offline_types.empty() ) m_offline_types.push_back( "InDetTrackParticles" );

  }


  void setRunPurity( bool b ) { m_runPurity=b; }

  void setShifter( bool b )    { m_shifter=b; }

  void useBeamCondSvc( bool b ) { m_useBeamCondSvc = b; }

  void containTracks( bool b ) { m_containTracks = b; }

  void setPdgID( int i=0 ) { m_pdgID=i; }

  void setParentPdgID( int i=0 ) { m_parent_pdgID=i; }

  void setMCTruthRef( bool b ) { m_mcTruth=b; }

  void setOfflineRef( bool b ) { m_doOffline=b; }

  void setTypes( const std::vector<std::string>& t ) { m_types=t; }

  void set_monTool( ToolHandle<GenericMonitoringTool>* m ) { m_monTool=m; }

  ToolHandle<GenericMonitoringTool>* monTool() { return m_monTool; }

public:

  A* m_manalysis;

  using T_AnalysisConfig<T>::name;

protected:

  using T_AnalysisConfig<T>::m_provider;
  using T_AnalysisConfig<T>::m_tdt;
  using T_AnalysisConfig<T>::m_mcTruth;

  using T_AnalysisConfig<T>::m_analysis;

  using T_AnalysisConfig<T>::m_selectorTest;
  using T_AnalysisConfig<T>::m_selectorRef;
  using T_AnalysisConfig<T>::m_associator;
  using T_AnalysisConfig<T>::m_filters;
  
  virtual void loop() {

    const TagNProbe* pTnP_tool = m_TnP_tool;

    if( m_provider->msg().level() <= MSG::VERBOSE) {
      m_provider->msg(MSG::VERBOSE) <<  "AnalysisConfigR3_Tier0::loop() for " << T_AnalysisConfig<T>::m_analysisInstanceName <<  endmsg;
    }

    /// this isn't working yet - will renable once we have a workaround
    // get (offline) beam position
    //    double xbeam = 0;
    //    double ybeam = 0;

#if 0

    if ( m_first ) {      

      m_first = false;
      
      if ( m_provider->msg().level() <= MSG::VERBOSE ) {
	m_provider->msg(MSG::VERBOSE) << " using beam position\tx=" << xbeam << "\ty=" << ybeam << endmsg;
	
	std::vector<std::string> configuredChains  = (*(m_tdt))->getListOfTriggers("L2_.*, EF_.*, HLT_.*");
	
       	for ( unsigned i=0 ; i<configuredChains.size() ; i++ ) {
	  /// for debugging ...
	  //	std::cout << "Configured chain " << configuredChains[i]  << std::endl;
	  m_provider->msg(MSG::VERBOSE)  << "Chain " << configuredChains[i]  << endmsg;
	}
      }
    
      //      std::cout << "\tloop() analyse chains " << m_chainNames.size() << std::endl;
	
    }

#endif

    Filter_True filter;
    
    Filter_etaPT    filter_etaPT( 5, 200 );
    Filter_Combined filter_truth( &filter_etaPT,   &filter_etaPT);
    
    /// will need to add a vertex filter at some point probably
    // Filter_Combined filterRef (&filter_offline, &filter_vertex);

    int iRefFilter  = 1;
    int iTestFilter = 0;

    if ( m_runPurity ) { 
      iRefFilter  = 0;
      iTestFilter = 1;
    }
    
    
    TrackFilter* rfilter = m_filters[iRefFilter][0];
    TrackFilter* tfilter = m_filters[iTestFilter][0];

    Filter_Combined filterRef(   rfilter, &filter );
    Filter_Combined filterTest(  tfilter, &filter );

    TrigTrackSelector  selectorTruth( &filter_truth, m_pdgID, m_parent_pdgID );

    TrigTrackSelector  selectorRef( &filterRef );
    TrigTrackSelector* pselectorRef = &selectorRef;

    TrigTrackSelector  selectorTest( &filterTest );
    TrigTrackSelector* pselectorTest = &selectorTest;
    
    /// switch reference selector to truth if requested
    if ( m_mcTruth ) pselectorRef = &selectorTruth;

    /// this isn't working yet - will renable once we have a workaround
    /// if ( xbeam!=0 || ybeam!=0 ) {
    ///      std::cerr << "Oh no ! setBeamLine() : " << xbeam << " " << ybeam << std::endl;
    ///     pselectorRef->setBeamline(  xbeam, ybeam );
    /// }  

    /// now start everything going for this event properly ...

    TIDA::Event   event;
    TIDA::Event*  eventp = &event;

    double beamline[4] = { 0, 0, 0, 0 }; /// the fourth value will store the number of vertices
	 
    // clear the ntuple TIDA::Event class
    eventp->clear();

    /// (obviously) get the event info

    const xAOD::EventInfo* pEventInfo = 0;

    unsigned           run_number        = 0;
    unsigned long long event_number      = 0;
    unsigned           lumi_block        = 0;
    unsigned           bunch_crossing_id = 0;
    unsigned           time_stamp        = 0;
    double             mu_val            = 0;

    /// for debugging ...
    //    std::cout << "\tloop() get EventInfo" << std::endl;

    if ( this->template retrieve( pEventInfo, "EventInfo" ).isFailure() ) {
      m_provider->msg(MSG::WARNING) << "Failed to get EventInfo " << endmsg;
    } else {

      run_number        = pEventInfo->runNumber();
      event_number      = pEventInfo->eventNumber();
      lumi_block        = pEventInfo->lumiBlock();
      time_stamp        = pEventInfo->timeStamp();
      bunch_crossing_id = pEventInfo->bcid();
      mu_val            = pEventInfo->averageInteractionsPerCrossing();
    }
  
    if(m_provider->msg().level() <= MSG::VERBOSE){
      m_provider->msg(MSG::VERBOSE) << "run "     << run_number
                                    << "\tevent " << event_number
                                    << "\tlb "    << lumi_block << endmsg;
    }

    // clear the ntuple TIDA::Event class
    eventp->clear();

    eventp->run_number(run_number);
    eventp->event_number(event_number);
    eventp->lumi_block(lumi_block);
    eventp->time_stamp(time_stamp);
    eventp->bunch_crossing_id(bunch_crossing_id);
    eventp->mu(mu_val);

    /// first check whether the chains have actually run, otherwise there's no point
    /// doing anything

    bool analyse = false;
  
    // Check HLTResult

    for ( unsigned ichain=0 ; ichain<m_chainNames.size() ; ichain++ ) {

      const std::string& chainname = m_chainNames[ichain].head();
 
      if ( chainname == "" ) analyse = true;
      else { 

	//Only for trigger chains
	if ( chainname.find("L2")  == std::string::npos &&
	     chainname.find("EF")  == std::string::npos &&
	     chainname.find("HLT") == std::string::npos ) continue;
	
	if ( m_provider->msg().level() <= MSG::DEBUG ) {
	  m_provider->msg(MSG::DEBUG) << "Chain "  << chainname
				      << "\tpass " << (*m_tdt)->isPassed(chainname)
				      << "\tpres " << (*m_tdt)->getPrescale(chainname) << endmsg;
	}

	/// for debugging ...
	//	std::cout << "\tChain "  << chainname << "\tpass " << (*m_tdt)->isPassed(chainname)
	//		  << "\tpres " << (*m_tdt)->getPrescale(chainname) << std::endl;
	
	if ( (*(m_tdt))->isPassed(chainname) ) analyse = true;
	
      }

    }
    
    
    if ( !this->m_keepAllEvents && !analyse ) {
      if(m_provider->msg().level() <= MSG::VERBOSE) {
        m_provider->msg(MSG::VERBOSE) << "No chains passed unprescaled - not processing this event" << endmsg;
      }
      return;
    }

    
    /// for Monte Carlo get the truth particles if requested to do so

    selectorTruth.clear();

    if(m_provider->msg().level() <= MSG::VERBOSE)
      m_provider->msg(MSG::VERBOSE) << "MC Truth flag " << m_mcTruth << endmsg;

    /// get the offline vertices into our structure

    std::vector<TIDA::Vertex> vertices;
    std::vector<TIDA::Vertex> vertices_rec;

    std::vector<double> refbeamspot;
    std::vector<double> testbeamspot;

    /// fetch offline vertices ...

    m_provider->msg(MSG::VERBOSE) << "fetching AOD Primary vertex container" << endmsg;

    if ( !this->select( vertices, "PrimaryVertices" ) ) { 
      m_provider->msg(MSG::VERBOSE) << "could not retrieve the 'PrimaryVertices' vertex collection" << std::endl;
    }

    /// add the truth particles if needed

    if ( m_mcTruth ) {
      eventp->addChain( "Truth" );
      eventp->back().addRoi(TIDARoiDescriptor());
      eventp->back().back().addTracks(selectorTruth.tracks());
    }

    /// now add the vertices

    if ( m_doOffline ) {
      for ( unsigned i=0 ; i<vertices.size() ; i++ )  {
        if(m_provider->msg().level() <= MSG::VERBOSE)
          m_provider->msg(MSG::VERBOSE) << "vertex " << i << " " << vertices[i] << endmsg;
        eventp->addVertex(vertices[i]);
      }
    }



    /// now add the offline tracks and reco objects

    std::vector<TIDA::Track*> offline_tracks;
    std::vector<TIDA::Track*> electron_tracks;
    std::vector<TIDA::Track*> muon_tracks;

    std::vector<TIDA::Track*> ref_tracks;
    std::vector<TIDA::Track*> test_tracks;

    offline_tracks.clear();
    electron_tracks.clear();
    muon_tracks.clear();

    ref_tracks.clear();
    test_tracks.clear();

    // offline track retrieval now done once for each chain rather than each roi
    if ( m_provider->msg().level() <= MSG::VERBOSE )
      m_provider->msg(MSG::VERBOSE) << "MC Truth flag " << m_mcTruth << endmsg;

    bool foundTruth = false;

    /// FIXME: most of the different truth selection can go  

    if ( m_mcTruth ) {

      filter_truth.setRoi( 0 ); // don't filter on RoI yet (or until needed)  

      selectorTruth.clear();

      if ( m_provider->msg().level() <= MSG::VERBOSE )
	m_provider->msg(MSG::VERBOSE) << "getting Truth" << endmsg;

      if ( m_provider->evtStore()->template contains<TruthParticleContainer>("INav4MomTruthEvent") ) {
	//ESD
	this->template selectTracks<TruthParticleContainer>( &selectorTruth, "INav4MomTruthEvent" );
	foundTruth = true;
      }
      else if ( m_provider->evtStore()->template contains<TruthParticleContainer>("SpclMC") ) {
	/// AOD
	this->template selectTracks<TruthParticleContainer>( &selectorTruth, "SpclMC");
	foundTruth = true;
      }
      else if ( m_provider->evtStore()->template contains<xAOD::TruthParticleContainer>("TruthParticles") ) {
	/// xAOD::TruthParticles
	this->template selectTracks<xAOD::TruthParticleContainer>( &selectorTruth, "TruthParticles");
	foundTruth = true;
      }
      else if ( m_provider->evtStore()->template contains<TruthParticleContainer>("") ) {
	/// anything else?
	this->template selectTracks<TruthParticleContainer>( &selectorTruth, "");
	foundTruth = true;
      }
      else
	if ( m_provider->msg().level() <= MSG::VERBOSE ) {
	  m_provider->msg(MSG::VERBOSE) << "Truth not found - none whatsoever!" << endmsg;
	}
    }
      
    if ( m_mcTruth && !foundTruth ) {
          
      if ( m_provider->msg().level() <= MSG::VERBOSE ) { 
	m_provider->msg(MSG::VERBOSE) << "getting Truth" << endmsg;
      }

      /// selectTracks<TruthParticleContainer>( &selectorTruth, "INav4MomTruthEvent" );

      const McEventCollection* mcevent = nullptr;

      /// now as a check go through the GenEvent collection

      std::string keys[4] = { "GEN_AOD", "TruthEvent", "", "G4Truth" };

      std::string key = "";

      bool foundcollection = false;

      for ( int ik=0 ; ik<4 ; ik++ ) {
         
	if ( m_provider->msg().level() <= MSG::VERBOSE ) {
	  m_provider->msg(MSG::VERBOSE) << "Try McEventCollection: " << keys[ik] << endmsg;
	}

	if ( !m_provider->evtStore()->template contains<McEventCollection>(keys[ik]) ) {
	  if( m_provider->msg().level() <= MSG::VERBOSE )
	    m_provider->msg(MSG::VERBOSE) << "No McEventCollection: " << keys[ik] << endmsg;
	  continue;
	}

	if ( m_provider->msg().level() <= MSG::VERBOSE )
	  m_provider->msg(MSG::VERBOSE) << "evtStore()->retrieve( mcevent, " << keys[ik] << " )" << endmsg;

	if ( this->template retrieve( mcevent, keys[ik] ).isFailure() ) {
	  if ( m_provider->msg().level() <= MSG::VERBOSE )
	    m_provider->msg(MSG::VERBOSE) << "Failed to get McEventCollection: " << keys[ik] << endmsg;
	}
	else {
	  /// found this key
	  key = keys[ik];
	  if(m_provider->msg().level() <= MSG::VERBOSE)
	    m_provider->msg(MSG::VERBOSE) << "Found McEventCollection: " << key << endmsg;
	  foundcollection = true;
	  break;
	}
      }

      /// not found any truth collection
      if ( !foundcollection ) {
	if(m_provider->msg().level() <= MSG::VERBOSE)
	  m_provider->msg(MSG::WARNING) << "No MC Truth Collections of any sort, whatsoever!!!" << endmsg;

	//    m_tree->Fill();
	//    return StatusCode::FAILURE;

	return;
      }

      if ( m_provider->msg().level() <= MSG::VERBOSE ) {
	m_provider->msg(MSG::VERBOSE) << "Found McEventCollection: " << key << "\tNevents " << mcevent->size() << endmsg;
      }

      McEventCollection::const_iterator evitr = mcevent->begin();
      McEventCollection::const_iterator evend = mcevent->end();

      unsigned ie = 0; /// count of "events" - or interactions
      unsigned ip = 0; /// count of particles

      unsigned ie_ip = 0; /// count of "events with some particles"

      while ( evitr!=evend ) {

	int ipc = 0; /// count of particles in this interaction

	int pid = HepMC::signal_process_id((*evitr));

	//The logic should be clarified here
	if ( pid!=0 ) { /// hooray! actually found a sensible event

          // For HepMC2-based builds the following two functions return
          // GenEvent::particle_const_iterator
          // while for HepMC3-based builds they return
          // std::vector<HepMC3::ConstGenParticlePtr>::const_iterator
          // see AtlasHepMC/GenEvent.h for function definitions.
          auto pitr(HepMC::begin(**evitr));
          const auto pend(HepMC::end(**evitr));

	  while ( pitr!=pend ) {

	    selectorTruth.selectTrack( *pitr++ );

	    ++ipc;
                
	  }

	}
	++ie;
	++evitr;

	if ( ipc>0 ) {
	  /// if there were some particles in this interaction ...
	  //      m_provider->msg(MSG::VERBOSE) << "Found " << ie << "\tpid " << pid << "\t with " << ip << " TruthParticles (GenParticles)" << endmsg;
	  ++ie_ip;
	  ip += ipc;
	}
      }

      if(m_provider->msg().level() <= MSG::VERBOSE){
	m_provider->msg(MSG::VERBOSE) << "Found " << ip << " TruthParticles (GenParticles) in " << ie_ip << " GenEvents out of " << ie << endmsg;
	m_provider->msg(MSG::VERBOSE) << "selected " << selectorTruth.size() << " TruthParticles (GenParticles)" << endmsg;
      }

      if(selectorTruth.size() > 0) foundTruth = true;

      if ( !(ip>0) ) {
	if (m_provider->msg().level() <= MSG::VERBOSE) m_provider->msg(MSG::WARNING) << "NO TRUTH PARTICLES - returning" << endmsg;
	return; /// need to be careful here, if not requiring truth *only* should not return
      }
	  
    }

    if ( m_doOffline && !m_mcTruth) {

      bool found_offline = false;

      for ( size_t it=0 ; it<m_offline_types.size() ; it++ ) {
	if ( m_provider->evtStore()->template contains<xAOD::TrackParticleContainer>( m_offline_types[it] ) ) {
	  this->template selectTracks<xAOD::TrackParticleContainer>( pselectorRef, m_offline_types[it] );
	  refbeamspot = this->template getBeamspot<xAOD::TrackParticleContainer>( m_offline_types[it] );
	  found_offline = true;
	}
	else { 
	  m_provider->msg(MSG::WARNING) << "Offline tracks not found: " << m_offline_types[it] << endmsg;
	}
      }
      
      if ( !found_offline ) { 
	if (m_provider->evtStore()->template contains<Rec::TrackParticleContainer>("TrackParticleCandidate") ) {
	  /// do we still want to support Rec::TrackParticles ? Would this even still work ?
	  this->template selectTracks<Rec::TrackParticleContainer>( pselectorRef, "TrackParticleCandidate" );
	}
	else { 
	  m_provider->msg(MSG::WARNING) << "Offline tracks not found: " << "TrackParticleCandidate" << endmsg;
	}
      }

    }

    /// clone the asociator

    TrackAssociator* associator = m_associator->clone();

    //    std::cout << "\tloop() loop over chains proper ..." << std::endl;

    /// now loop over all relevant chains to get the trigger tracks...
    for ( unsigned ichain=0 ; ichain<m_chainNames.size() ; ichain++ ) {

      /// create chains for ntpl

      // std::string& chainname = chains[ichain];
      const std::string& chainname = m_chainNames[ichain].head();
      const std::string&       key = m_chainNames[ichain].tail();
      const std::string&  vtx_name = m_chainNames[ichain].vtx();
  
      // Not used left just in case
      // const std::string&  roi_name = m_chainNames[ichain].roi();
      // const std::string&  te_name = m_chainNames[ichain].element();
      m_pTthreshold = 0;  /// why does this need to be a class variable ???

      if ( m_chainNames[ichain].postcount() ) { 
        std::string ptvalue = m_chainNames[ichain].postvalue("pt");
	if ( ptvalue!="" ) m_pTthreshold = std::stod(ptvalue);
      }


      unsigned decisiontype = TrigDefs::Physics;
    
      if ( !m_chainNames[ichain].passed() ) decisiontype = TrigDefs::includeFailedDecisions;

      /// useful debug information to be kept in for the time being 
      //      if ( decisiontype==TrigDefs::requireDecision ) std::cout << "\tSUTT TrigDefs::requireDecision " << decisiontype << std::endl;
      //      if ( decisiontype==TrigDefs::Physics )         std::cout << "\tSUTT TrigDefs::Physics "         << decisiontype << std::endl;

      if ( chainname!="" && m_provider->msg().level() <= MSG::VERBOSE ) {

        m_provider->msg(MSG::VERBOSE) << "status for chain " << chainname
                                      << "\tpass "           << (*m_tdt)->isPassed(chainname)
                                      << "\tprescale "       << (*m_tdt)->getPrescale(chainname) << endmsg;

        m_provider->msg(MSG::VERBOSE) << "fetching features for chain " << chainname << endmsg;
    
        m_provider->msg(MSG::VERBOSE) << chainname << "\tpassed: " << (*m_tdt)->isPassed( chainname ) << endmsg;
      }

      /// useful debug information to be kept in
      //      std::cout << "\tstatus for chain " << chainname
      //		<< "\tpass "           << (*m_tdt)->isPassed( chainname )
      //		<< "\tpassdt "         << (*m_tdt)->isPassed( chainname, decisiontype )
      //		<< "\tprescale "       << (*m_tdt)->getPrescale( chainname ) << std::endl;
	

      //   m_provider->msg(MSG::INFO) << chainname << "\tpassed: " << (*m_tdt)->isPassed( chainname ) << "\t" << m_chainNames[ichain] << "\trun " << run_number << "\tevent " << event_number << endmsg;


      if ( chainname!="" && !this->m_keepAllEvents && !(*m_tdt)->isPassed( chainname, decisiontype ) ) continue;

      /// Get chain combinations and loop on them
      /// - loop made on chain selected as the one steering RoI creation
      // Trig::FeatureContainer f = (*m_tdt)->features( chainname, TrigDefs::alsoDeactivateTEs);
     
      /// only use the TDT for extracting collections if this was a trigger analysis 
      /// for fullscan "offline" type analyses (ie fullscan FTK) do not use this

      // tag and probe analysis processes multiple chains passed in the tag and probe tool at once so loop over vector of chains
      std::vector<std::string> chainNames ;
            
      if ( !m_tnp_flag ) {
	chainNames.push_back(m_chainNames[ichain].raw()) ;
      }
      else {
	chainNames.push_back(pTnP_tool->tag());
	chainNames.push_back(pTnP_tool->probe());
      }

      // loop over new chainNames vector but doing the same stuff
      for ( size_t i=0 ; i<chainNames.size() ; i++ ) {

	ChainString chainConfig = chainNames[i] ;
	std::string chainName   = chainConfig.head();
	
	eventp->addChain( chainNames[i] ); 
	
	TIDA::Chain& chain = eventp->back();
	
	if ( chainName == "" ) { 

	  /// do we still want the blind chain access for track collections ???

	  pselectorTest->clear();
	
	  /// dummy full scan chain 

	  //	  TIDARoiDescriptor roifs(true);
		
	  // chain.addRoi( roifs );

	  chain.addRoi( TIDARoiDescriptor(true) );
	
	  if ( m_provider->evtStore()->template contains<xAOD::TrackParticleContainer>(key) ) {
	    this->template selectTracks<xAOD::TrackParticleContainer>( pselectorTest, key );
	    refbeamspot = this->template getBeamspot<xAOD::TrackParticleContainer>( key );
	  }

	  const std::vector<TIDA::Track*>& testtracks = pselectorTest->tracks();

	  chain.back().addTracks(testtracks);
	
	  if ( vtx_name!="" ) { 
	  
	    /// MT Vertex access

	    m_provider->msg(MSG::VERBOSE) << "\tFetch xAOD::VertexContainer with key " << vtx_name << endmsg;

	    std::vector<TIDA::Vertex> tidavertices;

	    if ( this->select( tidavertices, vtx_name ) ) chain.back().addVertices( tidavertices );	 
	  }
	

	  //	  if ( roiInfo ) delete roiInfo;

	}
	else {

	  /// new Roi based feature access
	  
	  //std::string roi_key = m_chainNames[ichain].roi();

	  std::string roi_key = chainConfig.roi();
	
	  unsigned feature_type =TrigDefs::lastFeatureOfType;

	  if ( roi_key!="" ) feature_type= TrigDefs::allFeaturesOfType;

	  /// new FeatureRequestDescriptor with leg access
	  
	  int leg = -1;
	  
	  if ( chainConfig.element()!="" ) { 
	    leg = std::atoi(chainConfig.element().c_str());
	  }
	  
	  std::string rgex = roi_key;

	  std::vector< TrigCompositeUtils::LinkInfo<TrigRoiDescriptorCollection> > rois = 
	    (*m_tdt)->template features<TrigRoiDescriptorCollection>( Trig::FeatureRequestDescriptor( chainName,  
												      decisiontype,
												      rgex, 
												      feature_type,
												      "roi", 
												      leg ) );
	  
	  /// a hack to fetch back the Rois with "_probe" in the name if the standard named
	  /// RoiDescriptors are not actually present for this chain ...

	  if ( rois.empty() ) {
	    if ( !rgex.empty() ) {
	      rgex += "_probe";
	      rois = (*m_tdt)->template features<TrigRoiDescriptorCollection>( Trig::FeatureRequestDescriptor( chainName,
													       decisiontype,
													       rgex,
													       feature_type,
													       "roi",
													       leg ) );
	    }
	  }

	  int iroi = 0; /// count of how many rois processed so far
	  
	  for ( const TrigCompositeUtils::LinkInfo<TrigRoiDescriptorCollection>& roi_info : rois ) {

	    iroi++;

	    /// don't extract any additional rois if a superRoi is requested: 
	    /// In this case, the superRoi would be shared between the different 
	    /// chains - this shouldn't be needed now ...

	    if ( roi_key=="SuperRoi" && iroi>1 ) continue; 
	
	    //	  std::cout << "\troi: get link " << roi_key << " ..." << std::endl;

	    const ElementLink<TrigRoiDescriptorCollection> roi_link = roi_info.link;

	    /// check this is not a spurious TDT match
	    if ( roi_key!="" && roi_link.dataID()!=rgex ) continue;

        /// invalid feature links can happen for missing (or truncated) Aux containers
        if ( !roi_link.isValid() ) continue;

	    const TrigRoiDescriptor* const* roiptr = roi_link.cptr();

	    if ( roiptr == 0 ) continue;
	    

	    /// for debug ...
	    //	  std::cout << "\troi: link deref ..." << *roiptr << std::endl;

	    if (m_provider->msg().level() <= MSG::VERBOSE) {
	      m_provider->msg(MSG::VERBOSE) << " RoI descriptor for seeded chain " << chainname << " " << **roiptr << endmsg;
	    }
	 
	    TIDARoiDescriptor* roiInfo = new TIDARoiDescriptor( TIDARoiDescriptorBuilder(**roiptr) );
	 
	    //	  if ( dbg ) std::cout << "\troi " << iroi << " " << *roiInfo << std::endl;

	    /// get the tracks 

	    pselectorTest->clear();

	    if ( this->template selectTracks<xAOD::TrackParticleContainer>( pselectorTest, roi_link,  key ) ) { } 

	    // beamspot stuff not needed for xAOD::TrackParticles

	    /// create analysis chain

	    chain.addRoi( *roiInfo );
	
	    /// get tracks 

	    const std::vector<TIDA::Track*>& testtracks = pselectorTest->tracks();

	    chain.back().addTracks(testtracks);
	

	    /// now get the vertices 
	  
	    if ( vtx_name!="" ) { 

	      std::vector<TIDA::Vertex> tidavertices;

	      this->select( tidavertices, roi_link, vtx_name );

	      chain.back().addVertices( tidavertices );
	    
	    } /// retrieve online vertices
	  

#if 0
	    if ( dbg ) { 
	      std::cout << "\tTIDA analysis for chain: " << chainname << "\t key: " << key << "\t" << **roiptr << std::endl;
	      std::cout << "\tcollections: " << chain.back() << std::endl; 
	    }
#endif
	  
	    if ( roiInfo ) delete roiInfo;
	  
	  }


	} /// "offline" of "roi" type chains

      } // end of loop chainNames vector loop
	
	
      if ( m_provider->msg().level() <= MSG::VERBOSE ) {
	m_provider->msg(MSG::VERBOSE) << "event: " << *eventp << endmsg;
      }

    }

    // close previous loop over chains and open new one
    
    for ( unsigned ichain=0 ; ichain<eventp->size() ; ichain++ ) {
      
      TIDA::Chain& chain = (*eventp)[ichain];
      ChainString chainConfig(chain.name());        
      const std::string&  vtx_name = chainConfig.vtx();

      // skip tag chains to avoid performing standard analysis on them (done for tnp at the same time as probes)
      if ( m_tnp_flag && chainConfig.extra().find("_tag")!=std::string::npos ) continue ;
      
      std::vector<TIDA::Roi*> rois ;
      
      if (m_tnp_flag) {
	// needs to be done AFTER retrieving offline tracks as pselectorRef passed as arguement, hence restructuring
	rois = pTnP_tool->GetRois( eventp->chains(), pselectorRef, &filterRef, m_invmass, m_invmass_obj );
      }
      else {
        rois.reserve( chain.size() );
        for ( size_t ir=0 ; ir<chain.size() ; ir++ ) {
	  rois.push_back( &(chain.rois()[ir]) );
	}
      }

      // now loop over the rois (again)

      for ( unsigned iroi=0 ; iroi<rois.size() ; iroi++ ) {

	if ( this->filterOnRoi() ) { 
	  filterRef.setRoi( &(rois.at(iroi)->roi() ) ); 
	  filterRef.containtracks( m_containTracks ); 
	}
	else filterRef.setRoi( 0 );
	
	test_tracks.clear();
	
	// this block is before the track retrieval in the original, is it working the same here?
	
	/// This is nonsense and needs restructuring - why is the truth and offline selection 
	/// done within this RoI loop? It means the complete offline and truth tracks will be 
	/// retrieved for every RoI ! really we should have the structure 
	///   
	///   - check_a_trigger_chain_has_passed
	///   - get_offline_or_truth_particles
	///   - loop_over_rois
	///     - get_trigger_tracks
	///     - filter_offline_or_truth_reference
	///     - match_tracks
	///     - call_analyis_routine
	///
	/// will leave as it is for the time being

        if ( m_provider->msg().level() <= MSG::VERBOSE )
          m_provider->msg(MSG::VERBOSE) << "MC Truth flag " << m_mcTruth << endmsg;

	if ( m_mcTruth ) {
	  if ( this->filterOnRoi() )  filter_truth.setRoi( &(rois.at(iroi)->roi() ) );
	  ref_tracks = pselectorRef->tracks(&filter_truth);
	}
	else { // ie. if ( m_doOffline )
	  ref_tracks = pselectorRef->tracks(&filterRef) ; 
	}
	
	if ( m_provider->msg().level() <= MSG::VERBOSE ) {
	  m_provider->msg(MSG::VERBOSE) << "ref tracks.size() " << pselectorRef->tracks().size() << endmsg;
	  for ( int ii=pselectorRef->tracks().size() ; ii-- ; ) {
	    m_provider->msg(MSG::VERBOSE) << "  ref track " << ii << " " << *pselectorRef->tracks()[ii] << endmsg;
	  }
	}

        test_tracks.clear();


        for ( unsigned itrk=0 ; itrk<rois.at(iroi)->tracks().size() ; itrk++ ) {
          test_tracks.push_back(&(rois.at(iroi)->tracks().at(itrk)));
        }	
	
	/// debug ...
	//	std::cout << "sutt track multiplicities: offline " << offline_tracks.size() << "\ttest " << test_tracks.size() << std::endl;

	/// get the beamline
	
	beamline[0] = pselectorTest->getBeamX();
	beamline[1] = pselectorTest->getBeamY();
	beamline[2] = pselectorTest->getBeamZ();

	beamline[3] = vertices.size();

	/// the original code sets a class variable on the fill class
	/// this will not be reentrant, as another instance could overwrite
	/// it, but will keep it in for now. MAybe we could clone the 
	/// analysis class each time we have an instance. Need to 
	/// investigate whether such an event-by-event might be fast 
	/// enough. Foe the time being, leave the original code here, 
	/// but commented
	//	m_manalysis->setvertices( vertices.size() );  /// what is this for ??? /// is this thread safe ??? 

	/// if we want a purity, we need to swap round which tracks are the 
	/// reference tracks and which the test tracks

	if ( m_runPurity ) {

	  if ( this->getUseHighestPT() ) HighestPTOnly( test_tracks );

	  if ( m_pTthreshold>0 ) FilterPT( test_tracks, m_pTthreshold );

	  /// stats book keeping 
	  m_NRois++;
	  m_NRefTracks  += test_tracks.size();
	  m_NTestTracks += ref_tracks.size();

	  /// match test and reference tracks
	  associator->match( test_tracks, ref_tracks );
	  
	  m_manalysis->execute( test_tracks, ref_tracks, associator, eventp, beamline );

	}
	else { 

	  /// filter on highest pt track only if required
	  if ( this->getUseHighestPT() ) HighestPTOnly( ref_tracks );

	  /// ignore all tracks belong the specific analysis pt threshold if set
	  
	  if ( m_pTthreshold>0 )         FilterPT( ref_tracks, m_pTthreshold );

	  /// should be included - but leave commented out here to allow
	  /// study of rois with *no relevant truth particles !!!   
	  // if ( ref_tracks.size()==0 ) continue;

	  /// stats book keeping 
	  m_NRois++;
	  m_NRefTracks  += ref_tracks.size();
	  m_NTestTracks += test_tracks.size();

	  /// match test and reference tracks
	  associator->match( ref_tracks, test_tracks );
	
	  /// this setting of the roi is not thread safe so it has been diabled for the time being
	  /// in principle we may be able to remove it completely, but we need to check whether
	  /// we can do without this functionality, so leave the code in place until we either 
	  /// fix it properly in the future, or determine that it is not needed 
	  //	  m_manalysis->setroi( &rois.at(iroi)->roi() );  
	  m_manalysis->execute( ref_tracks, test_tracks, associator, eventp, beamline );

	  if ( vtx_name!="" ) { 
	    /// get vertices for this roi - have to copy to a vector<Vertex*>
	    std::vector<TIDA::Vertex> vr = rois.at(iroi)->vertices();
	    std::vector<TIDA::Vertex*> vtx_rec;    
	    for ( unsigned iv=0 ; iv<vr.size() ; iv++ ) vtx_rec.push_back( &vr[iv] );

	    std::vector<TIDA::Vertex*> vtx;
	    if ( this->getVtxIndex()<0 ) { 
	      for ( unsigned iv=0 ; iv<vertices.size() ; iv++ ) vtx.push_back( &vertices[iv] );
	    }
	    else { 
	      if ( vertices.size()>unsigned(this->getVtxIndex()) ) vtx.push_back( &vertices[this->getVtxIndex()] );
	    }

	    m_manalysis->execute_vtx( vtx, vtx_rec, eventp );
	  }

	}
 
	if ( m_manalysis->debug() ) { 
	  m_provider->msg(MSG::INFO) << "Missing track for " << m_chainNames[ichain]  
				     << "\trun "             << run_number 
				     << "\tevent "           << event_number 
				     << "\tlb "              << lumi_block << endmsg;     
	}

      }

    }
    
    delete associator;

    if ( m_provider->msg().level() <= MSG::VERBOSE ) {
      m_provider->msg(MSG::VERBOSE) << "\n\nEvent " << *eventp << endmsg;
    }

  }



  virtual void book() {
    
    if(m_provider->msg().level() <= MSG::VERBOSE)
      m_provider->msg(MSG::VERBOSE) << "AnalysisConfigR3_Tier0::book() " << name() << endmsg;

    // get the TriggerDecisionTool

    if( m_tdt->retrieve().isFailure() ) {
      if(m_provider->msg().level() <= MSG::ERROR)
        m_provider->msg(MSG::ERROR) << " Unable to retrieve the TrigDecisionTool: Please check job options file" << endmsg;
      return;
    }

    if(m_provider->msg().level() <= MSG::VERBOSE) {
      m_provider->msg(MSG::VERBOSE) << " Successfully retrived the TrigDecisionTool"  << endmsg;
    }


    /// get list of configured triggers
    if (m_provider->msg().level() <= MSG::VERBOSE) {
      std::vector<std::string> configuredChains  = (*(m_tdt))->getListOfTriggers("L2_.*, EF_.*, HLT_.*");

      m_provider->msg(MSG::VERBOSE)  << "Configured chains" << endmsg;
      for ( unsigned i=0 ; i<configuredChains.size() ; i++ ) {
        if( m_provider->msg().level() <= MSG::VERBOSE)
          m_provider->msg(MSG::VERBOSE)  << " Chain " << configuredChains[i]  << endmsg;
      }
    }


    for ( unsigned ic=0 ; ic<m_chainNames.size() ; ic++ ) {

      if ( ic>0 ) { 
	m_provider->msg(MSG::WARNING) << "more than one chain configured for this analysis - skipping " << m_chainNames[ic] << endmsg;
	continue;
      }

      m_provider->msg(MSG::VERBOSE) << "Analyse chain " << m_chainNames[ic] << endmsg;

      // m_provider->msg(MSG::VERBOSE)  << "--------------------------------------------------" << endmsg;
      
      std::string folder_name = "";
      
      if ( name()!="" )  folder_name = name(); 
      else               folder_name = "HLT/TRIDT/IDMon";  
      
      /// don't use test_type now ? 
      if( m_testType != "" ) folder_name = folder_name + "/" + m_testType;
      
      std::string mongroup;
      
#if 0   
      /// this isn;t working correctly at the moment, but we don;t want to 
      /// remove it, so leave it here until we can fix it 
   
      if ( name().find("Shifter")!=std::string::npos || m_shifter ) {
	/// shifter histograms - do not encode chain names
	if      ( m_chainNames.at(ic).tail().find("_FTF") != std::string::npos )              mongroup = folder_name + "/FTF";
	else if ( m_chainNames.at(ic).tail().find("_IDTrig") != std::string::npos || 
		  m_chainNames.at(ic).tail().find("_EFID") != std::string::npos )             mongroup = folder_name + "/EFID";
	else if ( m_chainNames.at(ic).tail().find("InDetTrigParticle") != std::string::npos ) mongroup = folder_name + "/EFID_RUN1";
	else if ( m_chainNames.at(ic).tail().find("_GSF")      != std::string::npos )         mongroup = folder_name + "/GSF";
	else                                                                                  mongroup = folder_name + "/Unknown";

	if ( m_chainNames.at(ic).vtx()!="" ) mongroup += "/" + m_chainNames.at(ic).vtx();

      }
#endif 
      //      else {
	/// these are the Expert / non-Shifter histograms - encode the full chain names

	if ( m_chainNames[ic].head() == "" ) mongroup = folder_name + "/Fullscan";
	else                                 mongroup = folder_name + "/" + m_chainNames[ic].head();

	std::string track_collection = ""; 

	if ( m_chainNames.at(ic).tail()!="" )  { 
	  track_collection =  "/" + m_chainNames.at(ic).tail();
	  if ( m_chainNames.at(ic).extra()!="" ) track_collection += "_" + m_chainNames.at(ic).extra();
	}

	if ( m_chainNames.at(ic).roi()!="" ) { 
	  if ( track_collection!="" ) track_collection += "_" + m_chainNames[ic].roi();
	  else                        track_collection = "/" + m_chainNames[ic].roi();
	}

	if ( m_chainNames.at(ic).vtx()!="" ) { 
	  if ( track_collection!="" ) track_collection += "_" + m_chainNames[ic].vtx();
	  else                        track_collection  = "/" + m_chainNames[ic].vtx();
	}

	/// add trigger element and roi descriptor names
	if ( m_chainNames.at(ic).element()!="" ) { 
	  if ( track_collection!="" ) track_collection += "_" + m_chainNames[ic].element();
	  else                        track_collection  = "/" + m_chainNames[ic].element();
	}
	
	if ( track_collection!="" )  mongroup += track_collection;

	if ( !m_chainNames.at(ic).passed() )      mongroup += "/DTE";

	//      }
      
      m_provider->msg(MSG::VERBOSE) << " book mongroup " << mongroup << endmsg;
      
      m_manalysis = dynamic_cast<A*>(m_analysis);
 
      if ( monTool() ) m_manalysis->set_monTool( monTool() );

      m_analysis->initialise();
      
      if(m_provider->msg().level() <= MSG::VERBOSE) {
	m_provider->msg(MSG::VERBOSE) << "AnalysisConfigR3_Tier0::book() done" << endmsg;
      }
    }

  }



  virtual void finalize() {

    if(m_provider->msg().level() <= MSG::VERBOSE){
      m_provider->msg(MSG::VERBOSE) << "AnalysisConfigR3_Tier0::finalise() " << m_provider->name() << endmsg;
    }
    
    m_analysis->finalise();

    m_provider->msg(MSG::INFO) << m_provider->name() << " " << m_chainNames[0] << "   \tNRois processed: " << m_NRois << "\tRef tracks: " << m_NRefTracks << "\tTestTracks: " << m_NTestTracks << endmsg;

    if(m_provider->msg().level() <= MSG::VERBOSE) {
      m_provider->msg(MSG::VERBOSE) << m_provider->name() << " finalised" << endmsg;
    }
  }


protected:

  bool      m_useBeamCondSvc;

  std::vector<ChainString>  m_chainNames;
  std::vector<A*>           m_analyses;
  std::string               m_testType;

  bool m_doOffline;
  bool m_doMuons;
  bool m_doElectrons;
  bool m_doTaus;
  bool m_doBjets;
  bool m_doTauThreeProng;
  bool m_tauEtCutOffline;

  std::vector<std::string> m_offline_types;
  std::vector<std::string> m_types;

  std::string m_outputFileName;

  int m_pdgID;
  int m_parent_pdgID;

  /// output stats
  int m_NRois;
  int m_NRefTracks;
  int m_NTestTracks;

  bool m_runPurity;

  bool m_shifter;

  double m_pTthreshold;

  bool   m_first;
  
  bool   m_containTracks;

  TagNProbe* m_TnP_tool ; 

  bool       m_tnp_flag;

  ToolHandle<GenericMonitoringTool>* m_monTool;

  TIDA::Histogram<float>*  m_invmass;
  TIDA::Histogram<float>*  m_invmass_obj;

};



#endif  // TrigInDetAnalysisExample_T_AnalysisConfigR3_Tier0_H

