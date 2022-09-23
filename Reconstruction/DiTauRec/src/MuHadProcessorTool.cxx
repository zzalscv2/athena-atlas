/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauRec/MuHadProcessorTool.h"

#include "xAODTau/TauJet.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauTrackContainer.h"
#include "xAODTau/TauDefs.h"
#include "xAODTau/TauTrackAuxContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

#include "xAODCore/ShallowAuxContainer.h"
#include "xAODCore/ShallowCopy.h"
#include "xAODMuon/MuonContainer.h"

//________________________________________
MuHadProcessorTool::MuHadProcessorTool(const std::string& type) :
  asg::AsgTool(type),
  m_tauContainerName("MuRmTauJets"),
  m_tauAuxContainerName("MuRmTauJetsAux."),
  m_configured(false),
  m_AODmode(false),
  m_data()
{
  declareProperty("TauContainer", m_tauContainerName);
  declareProperty("TauTrackContainer", m_tauTrackName="MuRmTauTracks" );
  declareProperty("RecTools", m_tools, "List of ITauToolBase tools");
  declareProperty("RNNTools", m_RNN_tools, "List of RNN tools");
  declareProperty("runOnAOD", m_AODmode); //AODS are input file
  declareProperty("seedJetConeSize", m_jetCone ) ;
  declareProperty("muonIDWP", m_MaxMuonIDWP = 2); // tight 0, medium 1, loose 2, veryloose 3
  declareProperty("MuonCalibrationToolHandle", m_thMuonCalibrationTool);
  declareProperty("MuonSelectionToolHandle", m_thMuonSelectionTool);
  declareProperty("SaveClusters", m_saveCluster );
  declareProperty("TrackClassificationDone", m_TrkClassifided = true );

}

//________________________________________
StatusCode MuHadProcessorTool::initialize(){

  ATH_CHECK( m_thMuonCalibrationTool.retrieve() );
  ATH_CHECK( m_thMuonSelectionTool.retrieve() );

  //-------------------------------------------------------------------------
  // No tools allocated!
  //-------------------------------------------------------------------------
  if (  m_tools.empty() || m_RNN_tools.empty() ) {
    ATH_MSG_ERROR("no tools given!");
    return StatusCode::FAILURE;
  }

  if ( m_TrkClassifided ) m_isoTrackType = xAOD::TauJetParameters::classifiedIsolation ;
  else m_isoTrackType = xAOD::TauJetParameters::modifiedIsolationTrack ;

  //-------------------------------------------------------------------------
  // Allocate tools
  //-------------------------------------------------------------------------
  ATH_MSG_INFO("List of tools in execution sequence:");
  ATH_MSG_INFO("------------------------------------");

  unsigned int tool_count = 0;

  for ( ToolHandle<ITauToolBase> & itT : m_tools ) 
  {
    ATH_CHECK( itT.retrieve() ) ;
    ++tool_count;
    ATH_MSG_INFO( itT->name() );
    itT->setTauEventData(&m_data) ;
  }

  for ( ToolHandle<ITauToolBase> & itT : m_RNN_tools ) 
  {
    ATH_CHECK( itT.retrieve() ) ;
    ++tool_count;
    ATH_MSG_INFO( itT.name() );
    itT->setTauEventData(&m_data) ;
  }

  ATH_MSG_INFO(" ");
  ATH_MSG_INFO("------------------------------------");

  if (tool_count == 0) {
    ATH_MSG_ERROR("could not allocate any tool!");
    return StatusCode::FAILURE;
  }

  ///////////////////////////////////////////////////////////////////////////

  return StatusCode::SUCCESS;
}

//________________________________________
StatusCode MuHadProcessorTool::execute(){
  
  StatusCode sc;
  m_data.clear();

  const xAOD::TauJetContainer*     pContainerOriginal = nullptr ;

  //-------------------------------------------------------------------------
  // retrieve Tau Containers from StoreGate
  //-------------------------------------------------------------------------
  sc = evtStore()->retrieve(pContainerOriginal, "TauJets" );
  if (sc.isFailure()) {
    if (m_AODmode) {
      ATH_MSG_WARNING("Failed to retrieve " << m_tauContainerName << "! Will exit TauProcessor now!!");
      return StatusCode::SUCCESS;
    }
    else {
      ATH_MSG_FATAL("Failed to retrieve " << m_tauContainerName);
      return StatusCode::FAILURE;
    }
  } 

  xAOD::TauJetContainer* pContainer = new xAOD::TauJetContainer()  ;
  sc = evtStore()->record( pContainer, m_tauContainerName ) ;
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Unable to record TauContainer in TDS");
    delete pContainer;
    return StatusCode::FAILURE;
  }

  xAOD::TauJetAuxContainer* pAuxContainer = new xAOD::TauJetAuxContainer() ;
  sc = evtStore()->record( pAuxContainer, m_tauContainerName + "Aux." ) ;
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Unable to record TauContainer in TDS");
    delete pAuxContainer ;
    return StatusCode::FAILURE;
  }

  pContainer->setStore( pAuxContainer );
  ATH_MSG_DEBUG( "Recorded xAOD tau jets with key: " << m_tauAuxContainerName );

/**
 *    This ProcessorTool ( so that all the related tools ) should NOT be (direct) merged into R22, 
 *    untill a ShallowCopy version (for TauJets) can be developed and tested toward physics analysis users.
 *
  std::pair< xAOD::TauJetContainer* , xAOD::ShallowAuxContainer* > pContainer 
                                          = xAOD::shallowCopyContainer( * pContainerOriginal ) 
**/
  
  const xAOD::MuonContainer* muonsCont = nullptr ;
  ATH_CHECK( evtStore()->retrieve( muonsCont, "Muons" ) ) ;

  std::pair<xAOD::MuonContainer*, xAOD::ShallowAuxContainer*> shallowMuon = xAOD::shallowCopyContainer( * muonsCont );

  std::map< int, overlapMuon > foundMuons ;

  static const SG::AuxElement::Decorator< int > decLooseMuon( "IDqlt" );
  
  for( const auto xMuon : * shallowMuon.first ) 
  {
    decLooseMuon( *xMuon ) = 4 ;
    if(  (*m_thMuonCalibrationTool).applyCorrection( *xMuon ) == CP::CorrectionCode::OutOfValidityRange )
    {
      ATH_MSG_WARNING( "MuonCalibrationTool can not really apply calibration nor smearing" );
      continue;
    }

    xAOD::Muon::Quality quality = (*m_thMuonSelectionTool).getQuality( * xMuon );
    // make a decoration
    decLooseMuon( *xMuon ) = static_cast<int>( quality ) ;

    if( quality > m_MaxMuonIDWP ) continue ;

    const ElementLink< xAOD::TrackParticleContainer > muIDTrack = xMuon->inDetTrackParticleLink();
    if (  ! muIDTrack.isValid() )  continue ;

    overlapMuon olmu ;
    olmu.IDtrkP4 = (*muIDTrack)->p4();
    olmu.Qlt = static_cast<int>( quality ) ;

    if ( xMuon->cluster() != nullptr )
    {
      olmu.ClustP4 = xMuon->cluster()->p4() ;
    }

    int idxTau = -1 ;
    int cntTau = 0 ;
    double mindR = 9.9 ;

    for ( auto tau : * pContainerOriginal )
    {
      double dR =  ( tau->p4() ).DeltaR( olmu.IDtrkP4 ) ;
      if (  dR < mindR ) 
      {
        mindR = dR ;
        idxTau = cntTau ;
      }
      cntTau ++ ;
    }

    if ( mindR < 9.9 )  foundMuons.insert(  std::pair< int, overlapMuon > ( idxTau, olmu ) ) ;
  }
  delete shallowMuon.first ;
  delete shallowMuon.second ;

  m_data.xAODTauContainer = pContainer;
  m_data.tauAuxContainer = pAuxContainer;
  
  typedef std::vector< ElementLink< xAOD::TauTrackContainer > >  TauTrackLinks_t;
  typedef std::vector< ElementLink< xAOD::IParticleContainer > >  IParticleLinks_t;

  xAOD::TauTrackContainer* tauTrackCont = new xAOD::TauTrackContainer();
  xAOD::TauTrackAuxContainer* tauTrackContAux = new xAOD::TauTrackAuxContainer();
  ATH_CHECK( evtStore()->record( tauTrackCont, m_tauTrackName ));
  ATH_CHECK( evtStore()->record( tauTrackContAux, m_tauTrackName + "Aux."));
  tauTrackCont->setStore( tauTrackContAux );

  const xAOD::CaloClusterContainer* caloClusterCont = nullptr;
  if ( m_saveCluster ) 
    ATH_CHECK( evtStore()->retrieve( caloClusterCont, "CaloCalTopoClusters" ) );

  //-------------------------------------------------------------------------
  // Initialize tools for this event
  //-------------------------------------------------------------------------
  for ( ToolHandle<ITauToolBase> & itT : m_tools ) {
    sc = itT->eventInitialize();
    if (sc != StatusCode::SUCCESS)
    {
      ATH_MSG_ERROR ("failed to call `eventInitialize` on sub-tool " << itT->name());
      return StatusCode::FAILURE;
    }
  }

  for ( ToolHandle<ITauToolBase> & itT : m_RNN_tools ) {
    sc = itT->eventInitialize();
    if (sc != StatusCode::SUCCESS)
    {
      ATH_MSG_ERROR ("failed to call `eventInitialize` on sub-tool " << itT->name());
      return StatusCode::FAILURE;
    }
  }

  ////////////////////////////////////////////////////////

  //loop over taus

//  unfortunately   TLorentzVector was not easily accessible in downStream ( like EventLooop ) yet
  static const SG::AuxElement::Decorator< std::vector<double> > decMuonTrack( "overlapMuonTrack" );
  static const SG::AuxElement::Decorator< std::vector<double> > decMuonCluster( "overlapMuonCluster" );
  static const SG::AuxElement::Decorator< int > decMuonQlt( "overlapMuonQuality" );

  int tauIdx = 0 ;
  for ( auto tau : * pContainerOriginal )
  {
    xAOD::TauJet *newTau = new xAOD::TauJet()  ;

    pContainer->push_back( newTau ) ;

    *newTau = *tau ;

    //-----------------------------------------------------------------
    // set tau candidate data for easy handling
    //-----------------------------------------------------------------

    TLorentzVector muTrack( 0., 0., 0., 0. ) ;
    TLorentzVector muCluster( 0., 0., 0., 0. ) ;
    float muOLdR = 9.9 ;

    int muQlt = 9 ;
    std::map< int, overlapMuon >::iterator itr = foundMuons.find( tauIdx ) ;
    if ( itr != foundMuons.end() )
    {
      overlapMuon mymuon = itr->second ;

      muOLdR =  ( mymuon.IDtrkP4 ).DeltaR( newTau->p4() ) ;  
      if ( muOLdR < m_jetCone ) 
      {
        muTrack = mymuon.IDtrkP4 ;
        muQlt = mymuon.Qlt ;
        muCluster = mymuon.ClustP4 ;
      }
    } else  // one more loop to test whether more taus overlapped with one muon
    {
      for ( std::pair< int, overlapMuon >  itr : foundMuons ) 
      {
        overlapMuon mymuon = itr.second ;
        
        float dR = ( mymuon.IDtrkP4 ).DeltaR( newTau->p4() ) ;

        if ( dR < muOLdR ) 
        {
          muOLdR = dR ;
          if ( muOLdR <  m_jetCone ) 
          {
            muTrack = mymuon.IDtrkP4 ;
            muQlt = mymuon.Qlt ;
            muCluster = mymuon.ClustP4 ;
          } 
        }
      }
    }

    std::vector< double > muTrack_v4 = { muTrack.Pt(), muTrack.Eta(), muTrack.Phi(), muTrack.E() } ;

    decMuonTrack( * newTau ) = muTrack_v4 ;
    decMuonQlt( * newTau ) = muQlt ;
    std::vector< double > muCluster_v4 = { muCluster.Pt(), muCluster.Eta(), muCluster.Phi(), muCluster.E() } ;
    decMuonCluster( * newTau ) = muCluster_v4 ;

    newTau->auxdata< float >( "muOLdR" ) = muOLdR  ;

    
    ATH_MSG_DEBUG( " #tau : " << tauIdx <<" pt "<< newTau->pt() <<" dRmu "<< muOLdR ) ;

    //-----------------------------------------------------------------
    // Process the candidate
    //-----------------------------------------------------------------

    if (  muOLdR <= m_jetCone  ) 
    {
      for ( ToolHandle<ITauToolBase> & itT : m_tools ) 
      {
        ATH_MSG_DEBUG("Invoking tauRec tool " << itT->name() );

        ATH_CHECK( itT->execute( *newTau ) ) ;
      }
    } 
 
    if ( muOLdR > m_jetCone )  newTau->clearTauTrackLinks();

    for ( ToolHandle<ITauToolBase> & itT : m_RNN_tools )
    {

      if (    (    ( itT->name() ).find( "TrackFinder" ) !=  std::string::npos 
                || ( itT->name() ).find( "TrackClassifier" ) !=  std::string::npos 
              )  &&  muOLdR <= m_jetCone  
         ) 
      {
        continue ;
      }

      ATH_MSG_DEBUG("Invoking RNN tool " << itT->name() );

      ATH_CHECK( itT->execute( *newTau ) ) ;
    }

    //  re set tauCluster  Link 
    if ( m_saveCluster )
    {
      const IParticleLinks_t tauClusterLinks = newTau->clusterLinks() ;
      IParticleLinks_t new_tauClusterLinks ;
      for( const auto& link : tauClusterLinks )
      {
        ElementLink<xAOD::IParticleContainer> new_link ;
        new_link.toContainedElement( *caloClusterCont, caloClusterCont->at( link.index() ) ) ;
        new_tauClusterLinks.push_back( new_link );
      }
      newTau->setClusterLinks( new_tauClusterLinks ) ; 
    }

    tauIdx ++ ;

    if (sc.isSuccess()) {
          
      ATH_MSG_VERBOSE("The tau candidate has been modified");

    } else {
      ATH_MSG_WARNING( " Check failure tool execute " ) ;
    }
  }

  //-------------------------------------------------------------------------
  // Finalize tools for this event
  //-------------------------------------------------------------------------

  for (auto tool : m_tools)
    ATH_CHECK(tool->eventFinalize());

  for (auto tool : m_RNN_tools)
    ATH_CHECK(tool->eventFinalize());

  ///////////////////////////////////////////////////////
  // locking of containers is moved to separate tau tool

  return StatusCode::SUCCESS;
}

StatusCode MuHadProcessorTool::finalize(){

  return StatusCode::SUCCESS;
}

