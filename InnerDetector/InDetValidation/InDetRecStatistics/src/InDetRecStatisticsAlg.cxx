/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//  file:   InDetRecStatisticsAlg.cxx
//  author: Sven Vahsen (sevahsen AT lbl DOT gov), with contributions from Andrei Gaponenko and Laurent Vacavant
//
//  to do-list:
//     o write out percentage of tracks with bad tracksummary
//     o add energy of mctracks for Michael
//     o don't save intermediate newTracking track's to ntuple
//     o statistics prints hit association purity, (holes in sct/pixels/b-layer, outliers, etc...)
//     o navigation between hits and tracks
//     X check Propagator defaults (check with Andrei regarding Tool)
//     o count tracks with without associated hits, without truth, truth without beginvertex
//     o improved navigation between truth and reconstructed tracks
//     o follow atlas naming conventions for all variable and method names

#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "HepPDT/ParticleData.hh"
#include "CLHEP/Units/SystemOfUnits.h"
#include "TruthUtils/HepMCHelpers.h"
#include <cmath>
#include <memory>
#include <ostream>
#include <iostream>
#include <sstream>

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/TRT_ID.h"
#include "TrkEventUtils/RoT_Extractor.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkEventPrimitives/FitQualityOnSurface.h"
#include "TrkEventPrimitives/ResidualPull.h"
#include "TrkSurfaces/Surface.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTrack/Track.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkSurfaces/PerigeeSurface.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkTruthData/TrackTruth.h"
#include "TrkTruthData/TrackTruthCollection.h"
#include "TrkEventPrimitives/JacobianThetaPToCotThetaPt.h"
#include "TrkParameters/TrackParameters.h"       //vv
#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include "TrkToolInterfaces/IUpdator.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"
#include "TrkToolInterfaces/ITruthToTrack.h"
#include "TrkToolInterfaces/ITrackSelectorTool.h"

// Other
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "IdDictDetDescr/IdDictManager.h"
#include "GeneratorObjects/McEventCollection.h"
#include "InDetRIO_OnTrack/TRT_DriftCircleOnTrack.h"
#include "VxVertex/VxContainer.h"
#include "VxVertex/RecVertex.h"

#include "InDetRecStatistics/InDetRecStatisticsAlg.h"
#include "InDetRecStatistics/TrackStatHelper.h"
#include "AtlasHepMC/GenParticle.h"
#include "InDetRecStatistics//PileUpType.h"




static const char * const s_linestr = "----------------------------------------------------------------------------------------------------------------------------------------------";
static const char * const s_linestr2 = "..............................................................................................................................................";

InDet::InDetRecStatisticsAlg::InDetRecStatisticsAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator),
  m_particleDataTable          (nullptr),
  m_trtID                      (nullptr),
  m_idDictMgr                  (nullptr),
  m_truthToTrack               ("Trk::TruthToTrack"),
  m_trkSummaryTool             ("Trk::TrackSummaryTool/InDetTrackSummaryTool"),
  m_updatorHandle              ("Trk::KalmanUpdator/TrkKalmanUpdator"),
  m_updator                    (nullptr),
  m_residualPullCalculator     ("Trk::ResidualPullCalculator/ResidualPullCalculator"),
  m_McTrackCollection_key      ("TruthEvent"),
  m_trackSelectorTool          ("InDet::InDetDetailedTrackSelectorTool"),
  m_UseTrackSummary            (true),
  m_printSecondary             (false),
  m_minPt                      (1000),
  m_maxEta                     (4.2),
  m_maxEtaBarrel               (0.8),
  m_maxEtaTransition           (1.6),
  m_maxEtaEndcap               (2.5),
  m_fakeTrackCut               (0.9),
  m_fakeTrackCut2              (0.7),
  m_matchTrackCut              (0.5),
  m_maxRStartPrimary           (  25.0*CLHEP::mm),
  m_maxRStartSecondary         ( 360.0*CLHEP::mm),
  m_maxZStartPrimary           ( 200.0*CLHEP::mm),
  m_maxZStartSecondary         (2000.0*CLHEP::mm),
  m_minREndPrimary             ( 400.0*CLHEP::mm),
  m_minREndSecondary           (1000.0*CLHEP::mm),
  m_minZEndPrimary             (2300.0*CLHEP::mm),
  //m_maxZIndet                (),
  m_minZEndSecondary           (3200.0*CLHEP::mm),
  m_useTrackSelection          (false),
  m_doTruth                    (true),
  m_minEtaFORWARD              (2.5),
  m_maxEtaFORWARD              (4.2),
  m_isUnbiased                 (0),
  m_events_processed           (0)
{
  // m_RecTrackCollection_keys.push_back(std::string("Tracks"));
  // m_TrackTruthCollection_keys.push_back(std::string("TrackTruthCollection"));

  // Algorithm properties
  declareProperty("SummaryTool",                m_trkSummaryTool);
  declareProperty("TruthToTrackTool",           m_truthToTrack);
  declareProperty("UpdatorTool",                m_updatorHandle,
		  "Measurement updator to calculate unbiased track states");
  declareProperty("ResidualPullCalculatorTool", m_residualPullCalculator,
		  "Tool to calculate residuals and pulls");
  declareProperty("TrackCollectionKeys",        m_RecTrackCollection_keys);
  declareProperty("McTrackCollectionKey",       m_McTrackCollection_key);
  declareProperty("TrackTruthCollectionKeys",   m_TrackTruthCollection_keys);
  declareProperty("UseTrackSelection"       ,   m_useTrackSelection);
  declareProperty("DoTruth"                 ,   m_doTruth);
  declareProperty("TrackSelectorTool"       ,   m_trackSelectorTool);
  declareProperty("UseTrackSummary",            m_UseTrackSummary);
  declareProperty("PrintSecondary",             m_printSecondary);
  declareProperty("minPt",		        m_minPt);
  declareProperty("maxEta",		        m_maxEta);
  declareProperty("maxEtaBarrel",               m_maxEtaBarrel  );
  declareProperty("maxEtaTransition",           m_maxEtaTransition);
  declareProperty("maxEtaEndcap",               m_maxEtaEndcap);
  declareProperty("maxEtaFORWARD", m_maxEtaFORWARD);
  declareProperty("minEtaFORWARD", m_minEtaFORWARD);
  declareProperty("fakeTrackCut",               m_fakeTrackCut);
  declareProperty("fakeTrackCut2",              m_fakeTrackCut2);
  declareProperty("matchTrackCut",              m_matchTrackCut);
  declareProperty("maxRStartPrimary",	        m_maxRStartPrimary);
  declareProperty("maxRStartSecondary",	        m_maxRStartSecondary);
  declareProperty("maxZStartPrimary",	        m_maxZStartPrimary);
  declareProperty("maxZStartSecondary",         m_maxZStartSecondary);
  declareProperty("minREndPrimary",		m_minREndPrimary);
  declareProperty("minREndSecondary",		m_minREndSecondary);
  declareProperty("minZEndPrimary",		m_minZEndPrimary);
  declareProperty("minZEndSecondary",	        m_minZEndSecondary);
  m_idHelper  = nullptr;
  m_pixelID   = nullptr;
  m_sctID     = nullptr;
  m_UpdatorWarning = false;
  m_pullWarning = false;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode InDet::InDetRecStatisticsAlg::initialize(){

  // Part 1: Get the messaging service, print where you are
  ATH_MSG_DEBUG("initialize()");

  StatusCode sc1 = getServices();           // retrieve store gate service etc
  if (sc1.isFailure()) {
    ATH_MSG_FATAL("Error retrieving services !");
    return StatusCode::FAILURE;
  }

  if (m_RecTrackCollection_keys.empty()) {
    ATH_MSG_ERROR("No reco track collection specified! Aborting.");
    return StatusCode::FAILURE;
  }

  if (m_doTruth && m_RecTrackCollection_keys.size() != m_TrackTruthCollection_keys.size()) {
    ATH_MSG_ERROR("You have specified "
		  << m_RecTrackCollection_keys.size()
		  << " TrackCollection keys, and " <<  m_TrackTruthCollection_keys.size()
		  << " TrackTruthCollection keys."
		  << " You have to specify one TrackTruthCollection for each"
		  << " TrackCollection! Exiting."
		  );
    return StatusCode::FAILURE;
  }

  // ----------------------------------
  // use updator to get unbiased states
  if ( ! m_updatorHandle.empty() ) {
    if (m_updatorHandle.retrieve().isFailure()) {
      ATH_MSG_FATAL("Could not retrieve measurement updator tool: "
	  << m_updatorHandle);
      return StatusCode::FAILURE;
    }
    m_updator = &(*m_updatorHandle);
  } else {
    ATH_MSG_DEBUG(
      "No Updator for unbiased track states given, use normal states!");
    m_updator = nullptr;
  }


  //get residual and pull calculator
  if (m_residualPullCalculator.empty()) {
    ATH_MSG_INFO(
      "No residual/pull calculator for general hit residuals configured."
	);
    ATH_MSG_INFO(
      "It is recommended to give R/P calculators to the det-specific tool"
	<< " handle lists then.");
  } else if (m_residualPullCalculator.retrieve().isFailure()) {
    ATH_MSG_FATAL("Could not retrieve "<< m_residualPullCalculator
	<<" (to calculate residuals and pulls) ");

  } else {
    ATH_MSG_INFO("Generic hit residuals&pulls will be calculated in one or both "
		 << "available local coordinates");
  }

  // create one TrackStatHelper object of each trackCollection --- this is used to accumulate track and hit statistics

  struct cuts ct;
  ct.maxEtaBarrel=  m_maxEtaBarrel;
  ct.maxEtaTransition= m_maxEtaTransition;
  ct.maxEtaEndcap=        m_maxEtaEndcap;
  ct.fakeTrackCut=	  m_fakeTrackCut;
  ct.fakeTrackCut2=	  m_fakeTrackCut2;
  ct.matchTrackCut	=  m_matchTrackCut;
  ct.maxRStartPrimary	 = m_maxRStartPrimary;
  ct.maxRStartSecondary	 = m_maxRStartSecondary;
  ct.maxZStartPrimary	 = m_maxZStartPrimary;
  ct.maxZStartSecondary	 = m_maxZStartSecondary;
  ct.minREndPrimary	 = m_minREndPrimary;
  ct.minREndSecondary	 = m_minREndSecondary;
  ct.minZEndPrimary	 = m_minZEndPrimary;
  ct.minZEndSecondary  	 = m_minZEndSecondary;
  ct.minPt               = m_minPt;
  ct.minEtaFORWARD = m_minEtaFORWARD;
  ct.maxEtaFORWARD = m_maxEtaFORWARD;

  unsigned int nCollections = 0;
  for (SG::ReadHandleKeyArray<TrackCollection>::const_iterator
       it = m_RecTrackCollection_keys.begin();
       it < m_RecTrackCollection_keys.end(); ++ it) {
    InDet::TrackStatHelper * collection =
      new TrackStatHelper(it->key(),(m_doTruth ? m_TrackTruthCollection_keys[nCollections].key() : ""), m_doTruth);
    nCollections ++;
    collection->SetCuts(ct);
    m_SignalCounters.push_back(collection);
  }

  StatusCode sc3 = resetStatistics();     // reset all statistic counters
  if (sc3.isFailure()) {
    ATH_MSG_FATAL("Error in resetStatistics !");
    return StatusCode::FAILURE;
  }

  ATH_CHECK( m_RecTrackCollection_keys.initialize() );
  ATH_CHECK( m_McTrackCollection_key.initialize(m_doTruth && !m_McTrackCollection_key.key().empty()) );
  ATH_CHECK( m_TrackTruthCollection_keys.initialize() );

  return StatusCode :: SUCCESS;

}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode InDet::InDetRecStatisticsAlg::execute(const EventContext &ctx)  const {

    ATH_MSG_DEBUG("entering execute()");

    // Get reconstructed tracks , generated tracks, and truth from storegate

    SG::ReadHandle<McEventCollection> SimTracks;

    if (m_doTruth) {
      SimTracks=SG::ReadHandle<McEventCollection>(m_McTrackCollection_key,ctx);
      if (!SimTracks.isValid()) {
        // @TODO warning ?
        ATH_MSG_WARNING("Error retrieving collections !");
        return StatusCode::SUCCESS;
      }
    }

    // Doesn't take account of pileup:
    //m_gen_tracks_processed += (*(SimTracks->begin()))->particles_size();
    m_events_processed ++;
    CounterLocal counter;

    // select charged and stable generated tracks
    // apply pt, eta etc cuts to generated tracks
    // devide generated tracks into primary, truncated, secondary

    std::vector <std::pair<HepMC::ConstGenParticlePtr,int> > GenSignal;
    //     GenSignalPrimary, GenSignalTruncated, GenSignalSecondary;
    unsigned int inTimeStart = 0;
    unsigned int inTimeEnd   = 0;
    if (m_doTruth) selectGenSignal ((SimTracks.isValid() ? &(*SimTracks) : nullptr), GenSignal, inTimeStart, inTimeEnd, counter);

    // step through the various reconstructed TrackCollections and
    // corresponding TrackTruthCollections and produce statistics for each

    if (m_SignalCounters.empty()) {
      ATH_MSG_ERROR("No reco track collection specified! Aborting.");
      return StatusCode::FAILURE;
    }

    std::vector< SG::ReadHandle<TrackCollection> > rec_track_collections = m_RecTrackCollection_keys.makeHandles(ctx);
    std::vector< SG::ReadHandle<TrackTruthCollection> > truth_track_collections;
    if (m_doTruth && !m_TrackTruthCollection_keys.empty()) {
      truth_track_collections = m_TrackTruthCollection_keys.makeHandles(ctx);
      if (truth_track_collections.size() != rec_track_collections.size()) {
        ATH_MSG_ERROR("Different number of reco and truth track collections (" << rec_track_collections.size() << "!=" << truth_track_collections.size() << ")" );
      }
    }
    if (m_SignalCounters.size() != rec_track_collections.size()) {
        ATH_MSG_ERROR("Number expected reco track collections does not match the actual number of such collections ("
                      << m_SignalCounters.size() << "!=" << rec_track_collections.size() << ")" );
    }

    std::vector< SG::ReadHandle<TrackCollection> >::iterator rec_track_collections_iter = rec_track_collections.begin();
    std::vector< SG::ReadHandle<TrackTruthCollection> >::iterator truth_track_collections_iter = truth_track_collections.begin();
    for (std::vector <class TrackStatHelper *>::const_iterator statHelper
	 =  m_SignalCounters.begin();
	 statHelper !=  m_SignalCounters.end();
         ++statHelper, ++rec_track_collections_iter) {
      assert( rec_track_collections_iter != rec_track_collections.end());

      ATH_MSG_DEBUG("Acessing TrackCollection " <<  m_RecTrackCollection_keys.at(rec_track_collections_iter - rec_track_collections.begin()).key());
      const TrackCollection       * RecCollection = &(**rec_track_collections_iter);
      const TrackTruthCollection  * TruthMap  = nullptr;

      if (RecCollection)  ATH_MSG_DEBUG("Retrieved " << RecCollection->size() << " reconstructed tracks from storegate");

      if (m_doTruth) {
        ATH_MSG_DEBUG("Acessing TrackTruthCollection " <<  m_TrackTruthCollection_keys.at(truth_track_collections_iter - truth_track_collections.begin()).key());
        assert( truth_track_collections_iter != truth_track_collections.end());
        TruthMap = &(**truth_track_collections_iter);
        if (TruthMap)   ATH_MSG_DEBUG("Retrieved " << TruthMap->size() << " TrackTruth elements from storegate");
        ++truth_track_collections_iter;
      }

      //start process of getting correct track summary

      std::vector <const Trk::Track *>          RecTracks, RecSignal;
      selectRecSignal                     (RecCollection, RecTracks,RecSignal,counter);

      ATH_MSG_DEBUG(
		    "  RecTracks.size()="          << RecTracks.size()
		    << ", GenSignal.size()="          << GenSignal.size());

      ATH_MSG_DEBUG("Accumulating Statistics...");
      (*statHelper)->addEvent    (RecCollection,
				  RecTracks,
				  GenSignal,
				  TruthMap,
				  m_idHelper,
				  m_pixelID,
				  m_sctID,
				  m_trkSummaryTool.operator->(),
				  m_UseTrackSummary,
				  &inTimeStart,
				  &inTimeEnd);

      counter.m_counter[kN_rec_tracks_processed] += RecCollection->size();

      for (  TrackCollection::const_iterator it = RecCollection->begin() ;
	     it < RecCollection->end(); ++ it){
	std::vector<const Trk::RIO_OnTrack*> rioOnTracks;
	Trk::RoT_Extractor::extract( rioOnTracks,
				     (*it)->measurementsOnTrack()->stdcont() );
	counter.m_counter[kN_spacepoints_processed] += rioOnTracks.size();
      }

    }
    m_counter += counter;

    ATH_MSG_DEBUG("leaving execute()");
    return StatusCode::SUCCESS;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

StatusCode InDet :: InDetRecStatisticsAlg :: finalize() {

  // Part 1: Get the messaging service, print where you are
  ATH_MSG_DEBUG("finalize()");

  printStatistics();

  for (std::vector <class TrackStatHelper *>::const_iterator collection =
       m_SignalCounters.begin(); collection !=  m_SignalCounters.end();
       ++collection) {
    ATH_MSG_DEBUG(s_linestr2);
    delete (*collection);
  }
  m_SignalCounters.clear();
  return StatusCode::SUCCESS;
}


StatusCode InDet :: InDetRecStatisticsAlg :: getServices ()
{
    // get the Particle Properties Service
    IPartPropSvc* partPropSvc = nullptr;
    StatusCode sc = evtStore()->service("PartPropSvc", partPropSvc, true);

    if (sc.isFailure()) {
        ATH_MSG_FATAL(" Could not initialize Particle Properties Service" );
        return StatusCode::FAILURE;
    }

    m_particleDataTable = partPropSvc->PDT();

    //Set up ATLAS ID helper to be able to identify the RIO's det-subsystem.

    // Get the dictionary manager from the detector store
    const IdDictManager*  idDictMgr = nullptr;
    sc = detStore()->retrieve(idDictMgr, "IdDict");
    if (sc.isFailure()) {
      ATH_MSG_FATAL("Could not get IdDictManager !");
      return StatusCode::FAILURE;
    }

    // Initialize the helper with the dictionary information.
    sc = detStore()->retrieve(m_idHelper, "AtlasID");
    if (sc.isFailure()) {
      ATH_MSG_FATAL("Could not get AtlasDetectorID helper.");
      return StatusCode::FAILURE;
    }

   //get Pixel, SCT, TRT managers and helpers

   if (detStore()->retrieve(m_pixelID, "PixelID").isFailure()) {
     msg(MSG::FATAL) << "Could not get Pixel ID helper" << endmsg;
     return StatusCode::FAILURE;
   }
   if (detStore()->retrieve(m_sctID, "SCT_ID").isFailure()) {
     msg(MSG::FATAL) << "Could not get SCT ID helper" << endmsg;
     return StatusCode::FAILURE;
   }

   //retrieve the TRT helper only if not-SLHC layout used
   sc = detStore()->retrieve(m_idDictMgr, "IdDict");
   if (sc.isFailure()) {
     ATH_MSG_FATAL("Could not get IdDictManager !");
     return StatusCode::FAILURE;
   }
   const IdDictDictionary* dict = m_idDictMgr->manager()->find_dictionary("InnerDetector");
   if(!dict) {
     ATH_MSG_FATAL(" Cannot access InnerDetector dictionary ");
     return StatusCode::FAILURE;
   }

   bool isSLHC = false;
   if (dict->file_name().find("SLHC")!=std::string::npos) isSLHC=true;

   if(!isSLHC){
     if (detStore()->retrieve(m_trtID, "TRT_ID").isFailure()) {
       msg(MSG::FATAL) << "Could not get TRT ID helper" << endmsg;
       return StatusCode::FAILURE;
     }
   }
   //

   if (m_UseTrackSummary) {
     if (m_trkSummaryTool.retrieve().isFailure() ) {
       ATH_MSG_FATAL("Failed to retrieve tool "
	   << m_trkSummaryTool);
       return StatusCode::FAILURE;
     } else {
       ATH_MSG_INFO("Retrieved tool " << m_trkSummaryTool);
     }
   } else {
     m_trkSummaryTool.disable();
   }

   // AG: init truthToTrack
   if (m_doTruth) {
     if (m_truthToTrack.retrieve().isFailure() ) {
       ATH_MSG_FATAL("Failed to retrieve tool " << m_truthToTrack);
       return StatusCode::FAILURE;
     } else {
       ATH_MSG_INFO("Retrieved tool " << m_truthToTrack);
     }
   } else {
     m_truthToTrack.disable();
   }

   //adding track selector tool
   if(m_useTrackSelection){
     if ( m_trackSelectorTool.retrieve().isFailure() ) {
       ATH_MSG_FATAL("Failed to retrieve tool " << m_trackSelectorTool);
       return StatusCode::FAILURE;
     } else {
       ATH_MSG_INFO("Retrieved tool " << m_trackSelectorTool);
     }
   } else {
     m_trackSelectorTool.disable();
   }
   return StatusCode :: SUCCESS;
}

StatusCode InDet :: InDetRecStatisticsAlg :: resetStatistics() {
    m_counter.reset();
    m_events_processed           = 0;

    for (std::vector<InDet::TrackStatHelper *>::const_iterator counter =
	   m_SignalCounters.begin();
	 counter != m_SignalCounters.end(); ++ counter) {
      (*counter)->reset();
    }
    return StatusCode :: SUCCESS;
}

void InDet::InDetRecStatisticsAlg::selectRecSignal(const TrackCollection* RecCollection,
						   std::vector <const Trk::Track *> & RecTracks ,
						   std::vector <const Trk::Track *> & RecSignal,
                                                   InDet::InDetRecStatisticsAlg::CounterLocal &counter) const {

  for (  TrackCollection::const_iterator it = RecCollection->begin() ;
	 it != RecCollection->end(); ++ it){
    RecTracks.push_back(*it);
    const DataVector<const Trk::TrackParameters>* trackpara =
      (*it)->trackParameters();

    if(!trackpara->empty()){
      const Trk::TrackParameters* para = trackpara->front();
      if (para){
	if (para->pT() >  m_minPt && std::abs(para->eta()) < m_maxEta) {
	  RecSignal.push_back(*it);
	}
      }
    }
    else {
      counter.m_counter[kN_rec_tracks_without_perigee] ++;
    }
  }
 }

// select charged, stable particles in allowed pt and eta range
void InDet :: InDetRecStatisticsAlg ::
selectGenSignal  (const McEventCollection* SimTracks,
		  std::vector <std::pair<HepMC::ConstGenParticlePtr,int> > & GenSignal,
		  unsigned int /*inTimeStart*/, unsigned int /*inTimeEnd*/,
                  InDet::InDetRecStatisticsAlg::CounterLocal &counter) const //'unused' compiler warning
{
  if (! SimTracks) return;

  unsigned int nb_mc_event = SimTracks->size();
  std::unique_ptr<PileUpType>  put = std::make_unique<PileUpType>(SimTracks);

  McEventCollection::const_iterator inTimeMBend;
  McEventCollection::const_iterator inTimeMBbegin;

  if (put)
    {
      inTimeMBbegin = put->in_time_minimum_bias_event_begin();
      inTimeMBend = put->in_time_minimum_bias_event_end();
    }

  for(unsigned int ievt=0; ievt<nb_mc_event; ++ievt)
    {
      const HepMC::GenEvent* genEvent = SimTracks->at(ievt);
      counter.m_counter[kN_gen_tracks_processed] += genEvent->particles_size();
      for (const auto& particle: *genEvent){
	  // require stable particle from generation or simulation
	  if (!MC::isStable(particle)) continue;
	  int   pdgCode = particle->pdg_id();
	  const HepPDT::ParticleData* pd = m_particleDataTable->particle(std::abs(pdgCode));
	  if (!pd) {
	    ATH_MSG_DEBUG("Could not get particle data for particle "<< particle);
	    ATH_MSG_DEBUG("GenParticle= " << particle);
	    continue;
	  }
	  float charge = pd->charge();
	  if (std::abs(charge)<0.5) continue;
	  if (std::abs(particle->momentum().perp()) >  m_minPt  &&
	      std::abs(particle->momentum().pseudoRapidity()) < m_maxEta ) {
	    std::pair<HepMC::ConstGenParticlePtr,int> thisPair(particle,ievt);
	    GenSignal.push_back(thisPair);
	  }
	} // End of a particle iteration
    } // End of one GenEvent iteration
  }

namespace {

   template <class T_Stream>
   class RestoreStream
   {
   public:
      RestoreStream(T_Stream &out) : m_stream(&out),m_precision(out.precision()) { }
      ~RestoreStream() { (*m_stream).precision(m_precision); }
   private:
      T_Stream  *m_stream;
      int        m_precision;
   };
}

void InDet :: InDetRecStatisticsAlg :: printStatistics() {
  if (!msgLvl(MSG::INFO)) return;

  ATH_MSG_INFO(" ********** Beginning InDetRecStatistics Statistics Table ***********");
  ATH_MSG_INFO("For documentation see https://twiki.cern.ch/twiki/bin/view/Atlas/InDetRecStatistics");
  ATH_MSG_INFO("(or for guaranteed latest version: http://atlas-sw.cern.ch/cgi-bin/viewcvs-atlas.cgi/offline/InnerDetector/InDetValidation/InDetRecStatistics/doc/mainpage.h?&view=markup )");
  ATH_MSG_INFO(" ********************************************************************");

  std::stringstream outstr;
  int def_precision(outstr.precision());
  outstr << "\n"
         << MSG::INFO
	 << std::setiosflags(std::ios::fixed | std::ios::showpoint)
	 << std::setw(7) << std::setprecision(2)
	 << s_linestr << "\n"
         << "Summary" << "\n"
         << "\tProcessed              : " << m_events_processed
	 << " events, "  << m_counter.m_counter[kN_rec_tracks_processed]
	 << " reconstructed tracks with " << m_counter.m_counter[kN_spacepoints_processed]
	 << " hits, and "  << m_counter.m_counter[kN_gen_tracks_processed]
	 << " truth particles" << "\n"
         << "\tProblem objects        : " <<  m_counter.m_counter[kN_rec_tracks_without_perigee]
	 << " tracks without perigee, "
	 << m_counter.m_counter[kN_unknown_hits] << " unknown hits" << "\n"
         << "\t" << "Reco  TrackCollections : ";
  bool first = true;
  for (std::vector <class TrackStatHelper *>::const_iterator collection =
	 m_SignalCounters.begin();
       collection !=  m_SignalCounters.end(); ++collection)
    {
      if (first) {
      	first = false;
      }
      else {
	outstr << ", ";
      }
      outstr << "\"" << (*collection)->key() << "\"";
    }
  ATH_MSG_INFO(outstr.str());
  outstr.str("");

  if (m_doTruth)
    {
      outstr.str("");
      outstr << "\n"
             << "\t" << "TrackTruthCollections  : ";
      first = true;
      for (std::vector <class TrackStatHelper *>::const_iterator collection =  m_SignalCounters.begin();
	   collection !=  m_SignalCounters.end(); ++collection)
	{
	  if (first) {
	    first = false;
	  }
	  else {
	    outstr << ", ";
	  }
	  outstr << "\"" << (*collection)->Truthkey() << "\"";
	}
      ATH_MSG_INFO(outstr.str());
      outstr.str("");
    }
   outstr.str("");
   outstr << "\n"
	  << s_linestr2 << "\n"
	  << "Cuts and Settings for Statistics Table" << "\n"
	  << "\t" << "TrackSummary Statistics" << "\t"
	  << (m_UseTrackSummary     ? "YES" : "NO") << "\n"
	  << "\t" << "Signal                \t" << "pT > "
	  << m_minPt/1000 << " GeV/c, |eta| < " << m_maxEta << "\t\t"
	  << "\t" << "Primary track start   \t" << "R < "
	  << m_maxRStartPrimary << "mm and |z| < "
	  << m_maxZStartPrimary << "mm" << "\n"
	  << "\t" << "Barrel                \t" << 0.0
	  << "< |eta| < " << m_maxEtaBarrel     << "\t\t\t"
	  << "\t" << "Primary track end     \t" << "R > "
	  << m_minREndPrimary   << "mm or |z| > " << m_minZEndPrimary
	  << "mm" << "\n"
	  << "\t" << "Transition Region     \t" << m_maxEtaBarrel
	  << "< |eta| < " << m_maxEtaTransition << "\t\t\t"
	  << "\t" << "Secondary (non-Primary) start \t"
	  << " R < "    << m_maxRStartSecondary << "mm and"
	  << " |z| < "  << m_maxZStartSecondary << " mm" << "\n"
	  << "\t" << "Endcap                \t" << m_maxEtaTransition
	  << "< |eta| < " << m_maxEtaEndcap     << "\t\t\t"
	  << "\t" << "Secondary (non-primary) end   \t"
	  << " R > "    << m_minREndSecondary   << "mm or"
	  << " |z| > "  << m_minREndSecondary   << "mm" << "\n"
          << "\t" << "Forward                \t"
          << "|eta| > " << m_minEtaFORWARD     << "\n"
	  << "\t" << "Low prob tracks #1    \t" << "< "
	  << m_fakeTrackCut  << " of hits from single Truth Track "
	  << "\n"
	  << "\t" << "Low prob tracks #2    \t" << "< "
	  << m_fakeTrackCut2 << " of hits from single Truth Track "
	  << "\n"
	  << "\t" << "No link tracks        \t  Track has no link associated to an HepMC Particle" << "\n"
	  << "\t" << "Good reco tracks      \t" << "> "
	  << m_matchTrackCut << " of hits from single Truth Track + a link !";
  ATH_MSG_INFO(outstr.str());
  outstr.str("");

  MsgStream &out = msg(MSG::INFO);
  {
  RestoreStream<MsgStream> restore(out);
  out << "\n" << s_linestr2 << "\n";
  m_SignalCounters.back()->print(out);

  if (m_UseTrackSummary) {
    std::string track_stummary_type_header = TrackStatHelper::getSummaryTypeHeader();
    out << "\n"
        << s_linestr2 << "\n"
        << "Detailed Statistics for Hits on Reconstructed tracks, using TrackSummary: (Preselection of tracks as described above.)" << "\n"
        << s_linestr2 << "\n"
        << "----------------------------------------------------------------------------------------------------------------------------------------------------" << "\n"
        << "  Reco Tracks                           .........................................hits/track.......................................................  " << "\n"
        << "----------------------------------------------------------------------------------------------------------------------------------------------------" << "\n"
        << "  in BARREL                tracks/event " << track_stummary_type_header << "\n"
        << "----------------------------------------------------------------------------------------------------------------------------------------------------" << "\n";
    printTrackSummary (out, ETA_BARREL);

    out<< "\n"
       << "----------------------------------------------------------------------------------------------------------------------------------------------------" << "\n"
       << "  in TRANSITION region     tracks/event  " << track_stummary_type_header << "\n"
       << "----------------------------------------------------------------------------------------------------------------------------------------------------"
       << "\n";
    printTrackSummary (out, ETA_TRANSITION);

    out << "\n"
        << "----------------------------------------------------------------------------------------------------------------------------------------------------" << "\n"
        << "  in ENDCAP                tracks/event " << track_stummary_type_header << "\n"
        << "----------------------------------------------------------------------------------------------------------------------------------------------------" << "\n";
    printTrackSummary (out, ETA_ENDCAP);

    out << "\n"
        << "----------------------------------------------------------------------------------------------------------------------------------------------------" << "\n"
        << "  in FORWARD region        tracks/event  " << track_stummary_type_header << "\n"
        << "----------------------------------------------------------------------------------------------------------------------------------------------------" << "\n";
    printTrackSummary (out, ETA_FORWARD);
  }

  if(m_printSecondary){
    outstr.str("");
    outstr << "\n" << std::setprecision(def_precision)
           <<s_linestr<<"\n"
           <<"Statistics for Secondaries (non-Primaries)"<<"\n"
           << "\t" << "Secondary track start \t"
	   << " R < "   << m_maxRStartSecondary << "mm and"
	   << " |z| < " << m_maxZStartSecondary << " mm" << "\n"
           << "\t" << "Secondary track end   \t"
	   << " R > "    << m_minREndSecondary << "mm or"
           << " |z| > "  << m_minZEndSecondary << "mm";
    ATH_MSG_INFO(outstr.str());
    outstr.str("");
    out << "\n" << s_linestr2 << "\n";
    m_SignalCounters.back()->printSecondary(out);

  }
  }
  out << endmsg;

  ATH_MSG_INFO(" ********** Ending InDetRecStatistics Statistics Table ***********");
  ATH_MSG_INFO( "\n"
               << s_linestr );
}


void InDet :: InDetRecStatisticsAlg ::printTrackSummary (MsgStream &out, enum eta_region eta_reg)
{
  bool printed = m_SignalCounters.back()->printTrackSummaryRegion(out, TRACK_ALL, eta_reg);

  if (printed) {
     out <<  "\n"
         << "----------------------------------------------------------------------------------------------------------------------------------------------" << "\n";
  }

  printed = m_SignalCounters.back()->printTrackSummaryRegion(out, TRACK_LOWTRUTHPROB, eta_reg);
  if (printed) {
     out << "\n"
         << "----------------------------------------------------------------------------------------------------------------------------------------------" << "\n";
  }

  m_SignalCounters.back()->printTrackSummaryRegion(out, TRACK_LOWTRUTHPROB2, eta_reg);

}

// =================================================================================================================
// calculatePull
// =================================================================================================================
float InDet :: InDetRecStatisticsAlg :: calculatePull(const float residual,
						      const float trkErr,
						      const float hitErr){
  double ErrorSum;
  ErrorSum = sqrt(pow(trkErr, 2) + pow(hitErr, 2));
  if (ErrorSum != 0) { return residual/ErrorSum; }
  else { return 0; }
}

const Trk::TrackParameters *  InDet::InDetRecStatisticsAlg::getUnbiasedTrackParameters(const Trk::TrackParameters* trkParameters, const Trk::MeasurementBase* measurement ){


  const Trk::TrackParameters *unbiasedTrkParameters = nullptr;

 // -----------------------------------------
  // use unbiased track states or normal ones?
  // unbiased track parameters are tried to retrieve if the updator tool
  //    is available and if unbiased track states could be produced before
  //    for the current track (ie. if one trial to get unbiased track states
  //    fail

  if (m_updator && (m_isUnbiased==1) ) {
    if ( trkParameters->covariance() ) {
      // Get unbiased state
      ATH_MSG_VERBOSE(" getting unbiased params");
      unbiasedTrkParameters =
	m_updator->removeFromState( *trkParameters,
				    measurement->localParameters(),
				    measurement->localCovariance()).release();

      if (!unbiasedTrkParameters) {
	ATH_MSG_WARNING("Could not get unbiased track parameters, "
	    <<"use normal parameters");
	m_isUnbiased = 0;
      }
    } else if(!m_UpdatorWarning) {
      // warn only once!
      ATH_MSG_WARNING("TrackParameters contain no covariance: "
	  <<"Unbiased track states can not be calculated "
	  <<"(ie. pulls and residuals will be too small)");
      m_UpdatorWarning = true;
      m_isUnbiased = 0;
    } else {
      m_isUnbiased = 0;
    }
  } // end if no measured track parameter
  return unbiasedTrkParameters;
}


Identifier  InDet::InDetRecStatisticsAlg::getIdentifier(const Trk::MeasurementBase* measurement ){
  Identifier id;
  const Trk::CompetingRIOsOnTrack *comprot = nullptr;
  // identify by ROT:
  const Trk::RIO_OnTrack *rot =
    dynamic_cast<const Trk::RIO_OnTrack*>(measurement);
  if (rot) {
    id = rot->identify();
  } else {
    // identify by CompetingROT:
    comprot = dynamic_cast<const Trk::CompetingRIOsOnTrack*>(measurement);
    if (comprot) {
      rot = &comprot->rioOnTrack(comprot->indexOfMaxAssignProb());
      id = rot->identify();
    } else {
      ATH_MSG_DEBUG("measurement is neither ROT nor competingROT:"
	  <<" can not determine detector type");
      id.clear();
    }
  }
  delete comprot;
  return id;
}
