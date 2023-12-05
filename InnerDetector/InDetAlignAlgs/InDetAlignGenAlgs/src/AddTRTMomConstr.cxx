/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// @file AddTRTMomConstr.cxx
//
// @author Oleg Brandt
// many thanks to the TRTAlignAlgs designers and Pawel Bruckman, Markus Elsing, Thijs Cornelissen.
//

#include "InDetAlignGenAlgs/AddTRTMomConstr.h"
#include "GaudiKernel/ListItem.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkToolInterfaces/ITrackSummaryTool.h"
#include "TrkTrackSummary/TrackSummary.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "InDetRIO_OnTrack/TRT_DriftCircleOnTrack.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"
#include "InDetPrepRawData/TRT_DriftCircleContainer.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkPseudoMeasurementOnTrack/PseudoMeasurementOnTrack.h"
#include "Identifier/Identifier.h"
#include "InDetIdentifier/TRT_ID.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "IdDictDetDescr/IdDictManager.h"
#include "AthContainers/ConstDataVector.h"
#include <cmath>



AddTRTMomConstr::AddTRTMomConstr( const std::string& name, ISvcLocator* pSvcLocator )
  : AthAlgorithm   (name, pSvcLocator)
  , m_idHelper(nullptr)
  , m_trtid(nullptr)
  , m_trackListInput("ExtendedTracks")
  , m_trackListOutput("SiTracksWithTRTMomConstr")
  , m_trackSummaryTool("Trk::ITrackSummaryTool/ITrackSummaryTool")
  , m_trackFitter("Trk::KalmanFitter/TrkKalmanFitter")
  , m_useThetaCorrection(false)
  , m_thetaCorr(1.)
  , m_applyTrkSel(false)
  , m_selPtMin(0)
  , m_selEtaMin(-2.5)
  , m_selEtaMax(2.5)
  , m_selNHitPIXMin(0)
  , m_selNHitSCTMin(0)
  , m_selNHitTRTMin(20)
  , m_selNHitTRTMinCrack(15)
  , m_selEtaCrackMin(0.7)
  , m_selEtaCrackMax(1.)
  , m_selChiSqPerDOFMin(100)
  , m_nTracksProcessed(0)
  , m_nTracksPresel(0)
  , m_nTracksAccepted(0)
  , m_nRejectPIX(0)
  , m_nRejectSCT(0)
  , m_nRejectTRT(0)
  , m_nRejectPM(0)
{
  // declare algorithm parameters
  declareProperty("TrackListInput"     , m_trackListInput) ;
  declareProperty("TrackListOutput"    , m_trackListOutput) ;
  declareProperty("TrackFitter"        , m_trackFitter) ;
  declareProperty("TrackSummaryTool"   , m_trackSummaryTool) ;
  declareProperty("UseThetaCorrection" , m_useThetaCorrection) ;
  declareProperty("ApplyTrkSel"        , m_applyTrkSel) ;
  declareProperty("SelPtMin"           , m_selPtMin) ;
  declareProperty("SelEtaMin"          , m_selEtaMin) ;
  declareProperty("SelEtaMax"          , m_selEtaMax) ;
  declareProperty("SelNHitPIXMin"      , m_selNHitPIXMin) ;
  declareProperty("SelNHitSCTMin"      , m_selNHitSCTMin) ;
  declareProperty("SelNHitTRTMin"      , m_selNHitTRTMin) ;
  declareProperty("SelNHitTRTMinCrack" , m_selNHitTRTMinCrack) ;
  declareProperty("SelEtaCrackMin"     , m_selEtaCrackMin) ;
  declareProperty("SelEtaCrackMax"     , m_selEtaCrackMax) ;
  declareProperty("SelChiSqPerDOFMin"  , m_selChiSqPerDOFMin) ;
}

AddTRTMomConstr::~AddTRTMomConstr() {}

StatusCode AddTRTMomConstr::initialize() {
  //retrieve the DetectorStore service
  ATH_CHECK(detStore().retrieve()) ;
  ATH_CHECK(detStore()->retrieve(m_trtid));
  ATH_CHECK(detStore()->retrieve(m_idHelper, "AtlasID"));
  // get TrackSummaryTool
  ATH_CHECK(m_trackSummaryTool.retrieve());
  // Set up fitter
  ATH_CHECK(m_trackFitter.retrieve());
  // Print input properties
  if( m_applyTrkSel ) {
    ATH_MSG_DEBUG(
             "Track selection will be applied:"
          << "\n " << m_selEtaMin << " < eta < " << m_selEtaMax
          << "\n trans. region: " << m_selEtaCrackMin << " < |eta| < " << m_selEtaCrackMax
          << "\n pT                            >  " << m_selPtMin/Gaudi::Units::GeV << " GeV"
          << "\n number_of_PIXEL_hits          >= " << m_selNHitPIXMin
          << "\n number_of_SCT_hits            >= " << m_selNHitSCTMin
          << "\n number_of_TRT_hits            >= " << m_selNHitTRTMin
          << "\n number_of_TRT_hits in crack   >= " << m_selNHitTRTMinCrack
          << "\n chi2/dof                      <  " << m_selChiSqPerDOFMin
                ) ;
  } else {
    ATH_MSG_DEBUG( "NO selection will be applied to tracks" ) ;
  }
  return StatusCode::SUCCESS;
} // initialize(.)

StatusCode AddTRTMomConstr::finalize() {
  ATH_MSG_DEBUG( (*this) ) ;
  return StatusCode::SUCCESS;
}

StatusCode AddTRTMomConstr::execute() {
  if( !m_trackListOutput.empty() && !m_trackListInput.empty() ) {
    const TrackCollection* inputtracks ;
    ATH_CHECK(evtStore()->retrieve( inputtracks, m_trackListInput) );
    auto outputtracks = std::make_unique<ConstDataVector<TrackCollection> >( SG::VIEW_ELEMENTS ) ;
    for (const auto it : *inputtracks){
      auto track=it;
      if( m_applyTrkSel && !accept( *track ) ) {
        ATH_MSG_DEBUG( "kinematic requirements not passed, skip track") ;
        continue ;
      }
      ++m_nTracksPresel ;
      track = addTRTMomentumConstraint( track ) ;
      if( track ) {
        ATH_MSG_VERBOSE ( "addTRTMomentumConstraint(.) produced new track" ) ;
        outputtracks->push_back( track ) ;
      } else {
        ATH_MSG_DEBUG( "addTRTMomentumConstraint(.) returned 0 pointer, skip track") ;
        ++m_nRejectPM ;
      }
    }

    m_nTracksProcessed += inputtracks->size() ;
    m_nTracksAccepted += outputtracks->size() ;

    if( evtStore()->record( std::move(outputtracks), m_trackListOutput ).isFailure() ) {
      ATH_MSG_ERROR( "Failed to record trackcollection with name " << m_trackListOutput );
    }
  }
  return StatusCode::SUCCESS;
} // execute(.)

bool AddTRTMomConstr::accept( const Trk::Track& track ) {
  // selection for alignment quality tracks
  ATH_MSG_DEBUG( "Inside accept(.)") ;

  bool rc=true ;
  const Trk::Track* pTrack = &track ;
  const Trk::Perigee* mesp = pTrack->perigeeParameters();
  if( !mesp ) {
    ATH_MSG_WARNING("MeasuredPerigee pointer 0, track not accepted") ;
    return false ;
  }

  float theta = mesp->parameters()[Trk::theta] ;
  float pt = 0. ;
  if( theta != 0 ){
    float ptInv = std::abs( mesp->parameters()[Trk::qOverP] ) / std::sin( theta ) ;
    if( ptInv != 0 ) {
      pt = 1./ptInv ;
      if( m_selPtMin > 0 && pt < m_selPtMin ) rc = false ;
    }
  } else {
    ATH_MSG_WARNING( "theta = 0, q/p conversion to p_T failed!" );
    return false ;
  }

  float eta = -std::log( std::tan( theta/2. ) ) ;
  if( m_selEtaMin < m_selEtaMax && (eta < m_selEtaMin || eta > m_selEtaMax ) ) {
    rc = false ;
  }

  float chisqpdof = track.fitQuality()->chiSquared() / track.fitQuality()->numberDoF() ;
  if( m_selChiSqPerDOFMin > 0 && chisqpdof > m_selChiSqPerDOFMin ) rc = false ;
  std::unique_ptr<const Trk::TrackSummary> summary
    = m_trackSummaryTool->summary( *pTrack ) ;
  if( summary->get(Trk::numberOfPixelHits) < m_selNHitPIXMin ) {
    ++m_nRejectPIX ;
    rc = false;
  }
  if( summary->get(Trk::numberOfSCTHits) < m_selNHitSCTMin ) {
    ++m_nRejectSCT ;
    rc = false ;
  }
  // different treatment for TRT in transition region
  bool isInTransitionRegion = std::abs(eta) > m_selEtaCrackMin && std::abs(eta) < m_selEtaCrackMax ;
  if( isInTransitionRegion ) {
    if( int(summary->get(Trk::numberOfTRTHits)) < m_selNHitTRTMinCrack ) {
      ++m_nRejectTRT ;
      rc = false ;
    }
  } else {
    if( int(summary->get(Trk::numberOfTRTHits)) < m_selNHitTRTMin ) {
      ++m_nRejectTRT ;
      rc = false ;
    }
  }

  // All cuts passed...
  ATH_MSG_DEBUG( "Track selection:"
        << "\n\t passed : " << rc
        << "\n\t Pt     = " << pt
        << "\n\t phi    = " << mesp->parameters()[Trk::phi]
        << "\n\t eta    = " << eta
        << "\n\t nPIX   = " << summary->get(Trk::numberOfPixelHits)
        << "\n\t nSCT   = " << summary->get(Trk::numberOfSCTHits)
        << "\n\t nTRT   = " << summary->get(Trk::numberOfTRTHits)
               ) ;
  ATH_MSG_DEBUG( "leaving accept(.)" ) ;
  return rc ;
} // accept(.)

const Trk::TrackStateOnSurface* AddTRTMomConstr::findouterscthit( const Trk::Track* track ) {
  ATH_MSG_VERBOSE ("Inside findouterscthit: " << track->trackStateOnSurfaces()->size());
  double rmax=0. ;
  const Trk::TrackStateOnSurface* rc=nullptr ;
  for (const Trk::TrackStateOnSurface* tsos : *track->trackStateOnSurfaces()) {
    if( !tsos->type(Trk::TrackStateOnSurface::Outlier) &&
        tsos->measurementOnTrack() &&
        tsos->trackParameters() ) {
      ATH_MSG_VERBOSE ( "Found good measuredtrackparameters" );
      const InDet::SCT_ClusterOnTrack* sctclus = dynamic_cast<const InDet::SCT_ClusterOnTrack*>(tsos->measurementOnTrack());
      if(sctclus) {
        ATH_MSG_VERBOSE ( "Found SCT_ClusterOnTrack");
        const Amg::Vector3D& pos = sctclus->globalPosition() ;
        double r = std::sqrt(pos.x()*pos.x() + pos.y()*pos.y()) ;
        if(rc==nullptr || r>rmax) {
          rc = tsos ;
          rmax = r ;
        }
      }
    }
  }
  ATH_MSG_VERBOSE ( "Returning surface with adress: " << rc << " at radius " << rmax);
  return rc ;
}

const Trk::TrackStateOnSurface* AddTRTMomConstr::findinnertrthit( const Trk::Track* track ) {
  ATH_MSG_VERBOSE ( "Inside findinnerscthit: " << track->trackStateOnSurfaces()->size() );
  double rmin=9999999. ;
  const Trk::TrackStateOnSurface* rc=nullptr ;
  for (const Trk::TrackStateOnSurface* tsos : *track->trackStateOnSurfaces()) {
    if( !tsos->type(Trk::TrackStateOnSurface::Outlier) &&
         tsos->measurementOnTrack() &&
         tsos->trackParameters() ) {
      ATH_MSG_VERBOSE ( "Found good measuredtrackparameters" );
      const InDet::TRT_DriftCircleOnTrack* trthit = dynamic_cast<const InDet::TRT_DriftCircleOnTrack*>(tsos->measurementOnTrack());
      if(trthit) {
        ATH_MSG_VERBOSE ( "Found TRT_DriftCircleOnTrack" );
        const Amg::Vector3D& pos = trthit->globalPosition() ;
        double r = sqrt(pos.x()*pos.x() + pos.y()*pos.y()) ;
        if(rc==nullptr || r<rmin) {
          rc = tsos ;
          rmin = r ;
        }
      }
    }
  }
  ATH_MSG_VERBOSE ( "Returning surface with adress: " << rc << " at radius " << rmin );
  return rc ;
}

const Trk::PseudoMeasurementOnTrack* AddTRTMomConstr::createPMfromSi ( const Trk::Perigee* mp ) {
  Trk::DefinedParameter z0   ( mp->parameters()[Trk::z0],    Trk::z0    ) ;
  Trk::DefinedParameter theta( mp->parameters()[Trk::theta], Trk::theta ) ;
  std::array<Trk::DefinedParameter, 2> defPar = {z0, theta};
  if( !mp->covariance() )  return nullptr;
  Trk::LocalParameters  parFromSi( defPar ) ;
  AmgSymMatrix(2) covFromSi;

  covFromSi( 0, 0 ) = (*mp->covariance())( Trk::z0,Trk::z0 ) ;
  covFromSi( 1, 1 ) = (*mp->covariance())( Trk::theta,Trk::theta ) ;
  covFromSi( 1, 0 ) = (*mp->covariance())( Trk::z0, Trk::theta ) ;
  covFromSi( 0, 1 ) = (*mp->covariance())( Trk::z0, Trk::theta ) ;

  const Trk::Surface& mpSurf = mp->associatedSurface() ;

  Trk::PseudoMeasurementOnTrack *pm = new Trk::PseudoMeasurementOnTrack( std::move(parFromSi)
                                                                       , std::move(covFromSi)
                                                                       , mpSurf
                                                                       ) ;
  return pm ;
}

const Trk::PseudoMeasurementOnTrack* AddTRTMomConstr::createPMfromTRT( const Trk::Perigee* mpSi
                                                                     , const Trk::Perigee* mpTRT ) {

  if( !mpSi->covariance() || !mpTRT->covariance() )  return nullptr;

  // since z0, theta not measured by TRT, take back initial Si values with blown up errors.
  Trk::DefinedParameter z0TRT    ( mpSi->parameters()[Trk::z0],                 Trk::z0     ) ; //!< from Si
  Trk::DefinedParameter thetaTRT ( mpSi->parameters()[Trk::theta],              Trk::theta  ) ; //!< from Si
  Trk::DefinedParameter qOvewPTRT( mpTRT->parameters()[Trk::qOverP]*m_thetaCorr, Trk::qOverP ) ; //!< from TRT
  std::array<Trk::DefinedParameter, 3> parFromTRT_vec = {z0TRT, thetaTRT,
                                                         qOvewPTRT};
  Trk::LocalParameters parFromTRT(parFromTRT_vec);
  double A = 1e6 ; // scale up the errorfor z0 and theta to avoid double-counting when including the PM


  AmgSymMatrix(3) covFromTRT;

  // take the z0 theta part from extended track (correl. messed up by TRT fit) and scale it up with A
  covFromTRT( 0, 0 ) = (*mpSi->covariance())( Trk::z0, Trk::z0      ) * A * A ;
  covFromTRT( 1, 1 ) = (*mpSi->covariance())( Trk::theta, Trk::theta  ) * A * A ;
  covFromTRT( 1, 0 ) = (*mpSi->covariance())( Trk::z0   , Trk::theta  ) * A * A ;
  covFromTRT( 0, 1 ) = (*mpSi->covariance())( Trk::z0   , Trk::theta  ) * A * A ;
  // scale up the cov terms dependant on z0 and theta by A, and m_thetaCorr from q/p. Same m_thetaCorr for theta
  covFromTRT( 2, 2 ) = (*mpTRT->covariance())( Trk::qOverP, Trk::qOverP ) * m_thetaCorr * m_thetaCorr ;
  covFromTRT( 2, 0 ) = (*mpTRT->covariance())( Trk::z0   , Trk::qOverP ) * A * m_thetaCorr ;
  covFromTRT( 2, 1 ) = (*mpTRT->covariance())( Trk::theta, Trk::qOverP ) * A * m_thetaCorr * m_thetaCorr ;
  covFromTRT( 0, 2 ) = (*mpTRT->covariance())( Trk::z0   , Trk::qOverP ) * A * m_thetaCorr ;
  covFromTRT( 1, 2 ) = (*mpTRT->covariance())( Trk::theta, Trk::qOverP ) * A * m_thetaCorr * m_thetaCorr ;

  const Trk::Surface& mpTRTSurf = mpSi->associatedSurface()  ;

  Trk::PseudoMeasurementOnTrack *pm = new Trk::PseudoMeasurementOnTrack( std::move(parFromTRT)
                                                                       , std::move(covFromTRT)
                                                                       , mpTRTSurf
                                                                       ) ;
  return pm ;
}

const Trk::MeasurementSet AddTRTMomConstr::addPM( Trk::MeasurementSet& ms, const Trk::PseudoMeasurementOnTrack* pm ) {
  Trk::MeasurementSet sortedMS ;
  sortedMS.push_back( pm ) ;
  for( int i=0, i_max=ms.size() ; i!=i_max ; ++i ) {
    sortedMS.push_back( ms[i] ) ;
  }
  return sortedMS ;
}

Trk::Track* AddTRTMomConstr::addTRTMomentumConstraint(const Trk::Track* track) {
  Trk::MeasurementSet setSi ;
  Trk::MeasurementSet setTRT ;
  //store all silicon measurements into the measurementset
  for (const auto it:*(track->measurementsOnTrack())){
    const Trk::RIO_OnTrack* rio = dynamic_cast <const Trk::RIO_OnTrack*>(it);
    if (rio) {
      const Identifier& surfaceID = (rio->identify()) ;
      if( m_idHelper->is_sct(surfaceID) || m_idHelper->is_pixel(surfaceID) ) {
        setSi.push_back ( it ) ;
      }
      if( m_idHelper->is_trt(surfaceID) ) {
        setTRT.push_back ( it ) ;
      }
    }
  }
  ATH_MSG_DEBUG("TRTMomConstr() : Found " << setSi.size()  << " Si  measurm's!" ) ;
  ATH_MSG_DEBUG("TRTMomConstr() : Found " << setTRT.size() << " TRT measurm's!") ;
  if( int(setTRT.size()) < m_selNHitTRTMin ) {
    ATH_MSG_DEBUG( "TRTMomConstr() : fewer TRT measurements than required: "
               << setTRT.size() << " < " << m_selNHitTRTMin << ", skip track" ) ;
    return nullptr ;
  }

  ATH_MSG_DEBUG( std::setiosflags( std::ios::scientific )) ;
  ATH_MSG_DEBUG (std::setprecision( 7 )) ;
  ATH_MSG_VERBOSE  ( std::setiosflags( std::ios::scientific )) ;
  ATH_MSG_VERBOSE  ( std::setprecision( 7 )) ;
  const Trk::Perigee* perTrk = track->perigeeParameters();
  if( !perTrk ) {
    ATH_MSG_WARNING("TRTMomConstr() : No Perigee parameter on track!");
    return nullptr ;
  }

  // now add z_0 and theta from original track as PseudoMeas to the TRT MeasurementSet
  const Trk::PseudoMeasurementOnTrack *pmFromSi = createPMfromSi( perTrk ) ;
  if( !pmFromSi ) {
    ATH_MSG_ERROR( "TRTMomConstr() : PseudoMeasurementOnTrack creation failed! " );
    return nullptr ;
  }
  ATH_MSG_DEBUG( "TRTMomConstr() : pmFromSi " << *pmFromSi) ;
  Trk::MeasurementSet setTRTPM = addPM( setTRT, pmFromSi ) ;

   ATH_MSG_VERBOSE ( "TRTMomConstr() : TRT+PM MeasurementSet : " );
  for( int i=0, i_max=setTRTPM.size() ; i!=i_max ; ++i ) {
    ATH_MSG_VERBOSE ("============== i=" << i << " =============");
    ATH_MSG_VERBOSE ( *(setTRTPM[i]));
  }
   ATH_MSG_VERBOSE ("==========================================");

  // fit TRT part of the track with PseudoMeas on z_0, theta
   Trk::Track* trkTRT = (m_trackFitter->fit(
     Gaudi::Hive::currentContext(), setTRTPM, *perTrk, true, Trk::pion
     //, Trk::muon
     //, Trk::nonInteracting
   )).release();
   if (!trkTRT) {
     ATH_MSG_DEBUG("TRTMomConstr() : Fit of TRT part of the track failed! ");
     return nullptr;
   }
  const Trk::Perigee* perTRT = trkTRT->perigeeParameters();
  ATH_MSG_DEBUG( "TRTMomConstr() : perTRT " << *perTRT) ;
  // the theta value after TRT+PM fit can be different from the initial one by < o(10e-4). Correct q/p optionally
  if( m_useThetaCorrection ) m_thetaCorr = std::sin( perTrk->parameters()[Trk::theta] )
                                         / std::sin( perTRT->parameters()[Trk::theta] ) ;
  ATH_MSG_DEBUG( "TRTMomConstr() : Scalefactor to correct q/p: " << m_thetaCorr) ;

  // define new PM with the momentum constraint from the TRT to pass to Si
  const Trk::PseudoMeasurementOnTrack *pmFromTRT = createPMfromTRT( perTrk, perTRT ) ;
  if( !pmFromTRT ) {
    ATH_MSG_ERROR( "TRTMomConstr() : PseudoMeasurementOnTrack creation failed! " );
    return nullptr ;
  }
  ATH_MSG_DEBUG("TRTMomConstr() : pmFromTRT " << *pmFromTRT ) ;
  Trk::MeasurementSet setSiPM = addPM( setSi, pmFromTRT ) ;

  // fit Si part of the track with PM from TRT
  Trk::Track* fittedTrack =
    m_trackFitter
      ->fit(Gaudi::Hive::currentContext(), setSiPM, *perTrk, true, Trk::pion
            //, Trk::muon
            //, Trk::nonInteracting
            ).release();
  if (!fittedTrack) {
    ATH_MSG_DEBUG( "TRTMomConstr() : Si+TRT-p_T Track fit failed !" ) ;
  } else {
    const Trk::Perigee* perSi = fittedTrack->perigeeParameters();

    ATH_MSG_DEBUG( "TRTMomConstr() : Si+PM(TRT) track parameteres @ perigee: " << *perSi ) ;
  }

  return fittedTrack;
} // TRTMomentumConstraintForExtTracks

MsgStream& operator<<( MsgStream& outst, const AddTRTMomConstr& alg ) {
  return alg.dump( outst ) ;
}

MsgStream& AddTRTMomConstr::dump( MsgStream& outst ) const {
  outst << "\n|-------------------------------------------------------------------";
  outst << "-----------------------------|\n" ;
  outst << "|  processed                      : "
        << std::setw(7) << m_nTracksProcessed
        << " tracks                                               |\n";
  outst << "|  accepted by track presel.      : "
        << std::setw(7) << m_nTracksPresel
        << " tracks                                               |\n";
  outst << "|  accepted by track presel. + PM : "
        << std::setw(7) << m_nTracksAccepted
        << " tracks                                               |\n";
  outst << "| ------------------------------------------------------------------";
  outst << "---------------------------- |\n" ;
  outst << "|  reject by # PIX hits           : "
        << std::setw(7) << m_nRejectPIX
        << " tracks                                               |\n";
  outst << "|  reject by # SCT hits           : "
        << std::setw(7) << m_nRejectSCT
        << " tracks                                               |\n";
  outst << "|  reject by # TRT hits           : "
        << std::setw(7) << m_nRejectTRT
        << " tracks                                               |\n";
  outst << "| ------------------------------------------------------------------";
  outst << "---------------------------- |\n" ;
  outst << "|  reject by exist. PM(TRT)       : "
        << std::setw(7) << m_nRejectPM
        << " tracks                                               |\n";

  outst << "|-------------------------------------------------------------------";
  outst << "-----------------------------|\n" ;
  return outst ;
}
