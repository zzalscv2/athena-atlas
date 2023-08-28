/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackStateOnSurfaceDecorator.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author:Anthony Morley
//

#include "DerivationFrameworkInDet/TrackStateOnSurfaceDecorator.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

#include "xAODTracking/TrackStateValidationContainer.h"
#include "xAODTracking/TrackStateValidationAuxContainer.h"


#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkEventPrimitives/TrackStateDefs.h"
#include "TrkParameters/TrackParameters.h"

#include "AtlasDetDescr/AtlasDetectorID.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/TRT_ID.h"
#include "TrkPrepRawData/PrepRawData.h"

#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "InDetRIO_OnTrack/TRT_DriftCircleOnTrack.h"

#include "TrkToolInterfaces/IUpdator.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"
#include "TrkToolInterfaces/ITrackHoleSearchTool.h"
#include "TrkEventPrimitives/ResidualPull.h"
#include "TrkEventUtils/TrackStateOnSurfaceComparisonFunction.h"

#include "TRT_ConditionsServices/ITRT_CalDbTool.h"


#include "InDetReadoutGeometry/SiDetectorElement.h"

#include "xAODTracking/VertexContainer.h"   
#include "TrkTrack/Track.h"

#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkEventPrimitives/PropDirection.h"

#include "TRT_ElectronPidTools/ITRT_ToT_dEdx.h"  
#include "TrkToolInterfaces/IPRD_AssociationTool.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "DerivationFrameworkInDet/DecoratorUtils.h"

#include <vector>
#include <string>

namespace DerivationFramework {

  TrackStateOnSurfaceDecorator::TrackStateOnSurfaceDecorator(const std::string& t,
      const std::string& n,
      const IInterface* p) : 
    AthAlgTool(t,n,p),
    m_idHelper(nullptr),
    m_pixId(nullptr),
    m_sctId(nullptr),
    m_trtId(nullptr),
    m_updator("Trk::KalmanUpdator"),
    m_residualPullCalculator("Trk::ResidualPullCalculator/ResidualPullCalculator"),
    m_holeSearchTool("InDet::InDetTrackHoleSearchTool/InDetHoleSearchTool"),
    m_extrapolator("Trk::Extrapolator/AtlasExtrapolator"),
    m_trtcaldbTool("TRT_CalDbTool",this),
    m_TRTdEdxTool("InDet::TRT_ElectronPidTools/TRT_ToT_dEdx")
  {
    declareInterface<DerivationFramework::IAugmentationTool>(this);
    // --- Steering and configuration flags
    declareProperty("IsSimulation",           m_isSimulation=true);

    declareProperty("StoreHoles",             m_storeHoles =true);
    declareProperty("StoreOutliers",          m_storeOutliers = true);
    declareProperty("StoreTRT",               m_storeTRT =false);
    declareProperty("StoreSCT",               m_storeSCT = true);
    declareProperty("StorePixel",             m_storePixel =true);
    declareProperty("AddPulls",               m_addPulls =true);
    declareProperty("AddSurfaceInfo",         m_addSurfaceInfo =true);
    declareProperty("AddPRD",                 m_addPRD =true);
    declareProperty("AddExtraEventInfo",      m_addExtraEventInfo=true);

    // -- Tools 
    declareProperty("Updator",                m_updator);   
    declareProperty("ResidualPullCalculator", m_residualPullCalculator);
    declareProperty("HoleSearch",             m_holeSearchTool);
    declareProperty("TRT_CalDbTool",           m_trtcaldbTool);
    declareProperty("TRT_ToT_dEdx",           m_TRTdEdxTool);
    declareProperty("TrackExtrapolator",      m_extrapolator);
  }

  StatusCode TrackStateOnSurfaceDecorator::initialize()
  {
    ATH_MSG_DEBUG("Initialize");
    
    if (m_sgName.value() == "notSet") {
      ATH_MSG_ERROR("No decoration prefix name provided for the output of TrackStateOnSurfaceDecorator! Use the variable DecorationPrefix to properly set a prefix.");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Prefix for decoration: " << m_sgName);

    ATH_CHECK(m_eventInfoKey.initialize( m_addExtraEventInfo ));
    if (m_containerName.key().empty()) {
      ATH_MSG_ERROR("No TrackParticle collection provided for TrackStateOnSurfaceDecorator!");
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Input TrackParticle container: " << m_containerName.key());
    ATH_CHECK( m_containerName.initialize() );

    // need Atlas id-helpers to identify sub-detectors, take them from detStore
    if (detStore()->retrieve(m_idHelper, "AtlasID").isFailure()) {
      ATH_MSG_ERROR("Could not get AtlasDetectorID helper");
      return StatusCode::FAILURE;
    }   

    if( m_storePixel && detStore()->retrieve(m_pixId,"PixelID").isFailure() ){
      ATH_MSG_ERROR("Unable to retrieve pixel ID helper");
      return StatusCode::FAILURE;
    }

    if( m_storeSCT && detStore()->retrieve(m_sctId,"SCT_ID").isFailure() ){
      ATH_MSG_ERROR("Could not retrieve SCT helper");
      return StatusCode::FAILURE; 
    } 

    if( m_storeTRT && detStore()->retrieve(m_trtId,"TRT_ID").isFailure() ){
      ATH_MSG_ERROR("Could not retrieve TRT helper");
      return StatusCode::FAILURE; 
    }

    ATH_CHECK( m_trtcaldbTool.retrieve(DisableTool{ !m_storeTRT }));
    ATH_CHECK( m_prdToTrackMap.initialize( !m_prdToTrackMap.key().empty() && m_storeTRT) );

    ATH_CHECK( m_updator.retrieve(DisableTool{ !m_addPulls }));
    ATH_CHECK( m_residualPullCalculator.retrieve(DisableTool{ !m_addPulls }));

    ATH_CHECK( m_holeSearchTool.retrieve( DisableTool{ !m_storeHoles}) );

    ATH_CHECK( m_TRTdEdxTool.retrieve( DisableTool{!m_storeTRT || m_TRTdEdxTool.empty()}) );

    ATH_CHECK(m_extrapolator.retrieve());


    ATH_CHECK( m_SCTDetEleCollKey.initialize( m_storeSCT ));

    if (m_addExtraEventInfo) {
       std::vector<std::string> decor_names{"TrtPhaseTime"};
       std::vector<SG::WriteDecorHandleKey<xAOD::EventInfo> > decor_key_out;
       createDecoratorKeys(*this,m_eventInfoKey, m_sgName, decor_names,m_trtPhaseDecorKey);
       assert(m_trtPhaseDecorKey.size() == 1);
    }
    if (m_storeTRT && m_TRTdEdxTool.isEnabled()) {
       std::vector<std::string> names;
       names.resize(kNTRTFloatDecor);
       names[kTRTdEdxDecor]="ToT_dEdx";
       names[kTRTusedHitsDecor]="ToT_usedHits";
       names[kTRTdEdx_noHT_divByLDecor]="ToT_dEdx_noHT_divByL";
       names[kTRTusedHits_noHT_divByLDecor]="ToT_usedHits_noHT_divByL";
       createDecoratorKeys(*this,m_containerName, m_sgName, names, m_trackTRTFloatDecorKeys);
    }
    ATH_CHECK( m_trtPhaseKey.initialize(m_addExtraEventInfo) );
    ATH_CHECK( m_pixelMapName.initialize(m_storePixel && m_addPRD) );
    ATH_CHECK( m_sctMapName.initialize(m_storeSCT && m_addPRD) );
    ATH_CHECK( m_trtMapName.initialize(m_storeTRT && m_addPRD) );

    ATH_CHECK( m_pixelClustersName.initialize(m_storePixel && m_addPRD) );
    ATH_CHECK( m_sctClustersName.initialize(m_storeSCT && m_addPRD) );
    ATH_CHECK( m_trtDCName.initialize(m_storeTRT && m_addPRD) );

    ATH_CHECK( m_pixelMsosName.initialize(m_storePixel && m_addPRD) );
    ATH_CHECK( m_sctMsosName.initialize(m_storeSCT && m_addPRD) );
    ATH_CHECK( m_trtMsosName.initialize(m_storeTRT && m_addPRD) );

    {
       std::vector<std::string> names;
       names.resize(kNPixFloatDecor);
       names[kTrkIBLXDecor]="TrkIBLX";
       names[kTrkIBLYDecor]="TrkIBLY";
       names[kTrkIBLZDecor]="TrkIBLZ";
       names[kTrkBLXDecor]="TrkBLX";
       names[kTrkBLYDecor]="TrkBLY";
       names[kTrkBLZDecor]="TrkBLZ";
       names[kTrkL1XDecor]="TrkL1X";
       names[kTrkL1YDecor]="TrkL1Y";
       names[kTrkL1ZDecor]="TrkL1Z";
       names[kTrkL2XDecor]="TrkL2X";
       names[kTrkL2YDecor]="TrkL2Y";
       names[kTrkL2ZDecor]="TrkL2Z";
       createDecoratorKeys(*this,m_containerName, m_sgName, names, m_trackPixFloatDecorKeys);
    }
    m_trackTSOSMOSLinkDecorKey = m_containerName.key() + "." + m_sgName + "msosLink";
    ATH_CHECK( m_trackTSOSMOSLinkDecorKey.initialize() );

    ATH_MSG_DEBUG("Initialization finished.");

    return StatusCode::SUCCESS;
  }

  StatusCode TrackStateOnSurfaceDecorator::finalize()
  {
    ATH_MSG_DEBUG("Finalize");
    return StatusCode::SUCCESS;
  }

  StatusCode TrackStateOnSurfaceDecorator::addBranches() const
  {
    const EventContext& ctx = Gaudi::Hive::currentContext();
    ATH_MSG_DEBUG("Adding TSOS decorations the track particles");

    SG::WriteDecorHandle<xAOD::TrackParticleContainer,std::vector< ElementLink< xAOD::TrackStateValidationContainer >  > > dectsos_msosLink(m_trackTSOSMOSLinkDecorKey);

    // --- Retrieve track container (absolutely needed for decoration)
    SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_containerName,ctx);
    if( ! tracks.isValid() ) {
        ATH_MSG_ERROR ("Couldn't retrieve TrackParticles with key: " << m_containerName.key() );
        return StatusCode::FAILURE;
    }
    
    
    SG::ReadHandle<std::vector<unsigned int> > pixelClusterOffsets;
    SG::ReadHandle<std::vector<unsigned int> > sctClusterOffsets;
    SG::ReadHandle<std::vector<unsigned int> > trtDCOffsets;
    
    SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> pixelClusters;
    SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> sctClusters;
    SG::ReadHandle<xAOD::TrackMeasurementValidationContainer> trtDCs;
 
    
    // Create the xAOD container and its auxiliary store
    SG::WriteHandle<xAOD::TrackStateValidationContainer>    msosPixel;
    SG::WriteHandle<xAOD::TrackStateValidationContainer>    msosSCT;
    SG::WriteHandle<xAOD::TrackStateValidationContainer>    msosTRT;

    int nPixelMSOS(0);
    int nSCT_MSOS(0);
    int nTRT_MSOS(0);

    // --- Add event-level information
    if (m_addExtraEventInfo) {
      ATH_MSG_DEBUG("Adding EventInfo decorations");
      SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey,ctx);
      if (!eventInfo.isValid()) {
        ATH_MSG_ERROR(" Cannot access to event info.");
        return StatusCode::FAILURE;
      }

      //Add TRT event phase
      SG::ReadHandle<ComTime> trtPhase(m_trtPhaseKey, ctx);
      float trtPhase_time=0.;
      if (!trtPhase.isValid()) {
         ATH_MSG_DEBUG("Failed to retrieve TRT phase information.");
      } else {
         trtPhase_time = trtPhase->getTime();
      } //TRT phase
      SG::WriteDecorHandle<xAOD::EventInfo,float> decorTRTPhase(*(m_trtPhaseDecorKey.begin()),ctx);
      decorTRTPhase(*eventInfo) = trtPhase_time;
    } //extra event info


    // --- Add track states containers
    if(m_addPRD){
      // Get clusters and the mapping between xAOD::PRD and Trk::PRD
      // Store the MSOS's in a conatiner based on the type of the detector 
      if(m_storePixel){
        ATH_MSG_DEBUG("Creating Pixel track state container");
        pixelClusterOffsets=SG::ReadHandle<std::vector<unsigned int> >(m_pixelMapName,ctx);
        pixelClusters=SG::ReadHandle<xAOD::TrackMeasurementValidationContainer >(m_pixelClustersName,ctx);

        msosPixel = SG::WriteHandle<xAOD::TrackStateValidationContainer>(m_pixelMsosName,ctx);
        if (msosPixel.record(std::make_unique<xAOD::TrackStateValidationContainer>(),
                             std::make_unique<xAOD::TrackStateValidationAuxContainer>()).isFailure()) {
           ATH_MSG_ERROR("Failed to record " << m_pixelMsosName.key() );
           return StatusCode::FAILURE;
        }
      }
      if(m_storeSCT){
        ATH_MSG_DEBUG("Creating SCT track state container");
        sctClusterOffsets=SG::ReadHandle<std::vector<unsigned int> >(m_sctMapName,ctx);
        sctClusters=SG::ReadHandle<xAOD::TrackMeasurementValidationContainer >(m_sctClustersName,ctx);

        msosSCT = SG::WriteHandle<xAOD::TrackStateValidationContainer>(m_sctMsosName,ctx);
        if (msosSCT.record(std::make_unique<xAOD::TrackStateValidationContainer>(),
                             std::make_unique<xAOD::TrackStateValidationAuxContainer>()).isFailure()) {
           ATH_MSG_ERROR("Failed to record " << m_sctMsosName.key() );
           return StatusCode::FAILURE;
        }
      }
      if(m_storeTRT){
        ATH_MSG_DEBUG("Creating TRT track state container");
        trtDCOffsets=SG::ReadHandle<std::vector<unsigned int> >(m_trtMapName,ctx);
        trtDCs=SG::ReadHandle<xAOD::TrackMeasurementValidationContainer >(m_trtDCName,ctx);

        msosTRT = SG::WriteHandle<xAOD::TrackStateValidationContainer>(m_trtMsosName,ctx);
        if (msosTRT.record(std::make_unique<xAOD::TrackStateValidationContainer>(),
                             std::make_unique<xAOD::TrackStateValidationAuxContainer>()).isFailure()) {
           ATH_MSG_ERROR("Failed to record " << m_trtMsosName.key() );
           return StatusCode::FAILURE;
        }
      }
    }

    SG::ReadHandle<Trk::PRDtoTrackMap>  prd_to_track_map;
    const Trk::PRDtoTrackMap *prd_to_track_map_cptr = nullptr;
    if (!m_prdToTrackMap.key().empty()) {
       prd_to_track_map=SG::ReadHandle<Trk::PRDtoTrackMap>(m_prdToTrackMap);
       if (!prd_to_track_map.isValid()) {
          ATH_MSG_ERROR("Failed to read PRD to track association map: " << m_prdToTrackMap.key());
       }
       prd_to_track_map_cptr = prd_to_track_map.cptr();
    }

    std::vector<SG::WriteDecorHandle<xAOD::TrackParticleContainer,float> > trackTRTFloatDecorators;
    if (m_storeTRT && m_TRTdEdxTool.isEnabled()) {
       trackTRTFloatDecorators = createDecorators<xAOD::TrackParticleContainer,float>(m_trackTRTFloatDecorKeys,ctx);
    }
    std::vector<SG::WriteDecorHandle<xAOD::TrackParticleContainer,float> >
       trackPixFloatDecorators = createDecorators<xAOD::TrackParticleContainer,float>(m_trackPixFloatDecorKeys,ctx);
    // -- Run over each track and decorate it
    for (const auto *const track : *tracks) {
      //-- Start with things that do not need a Trk::Track object

      // -- Now things that require a Trk::Track object
      if( !track->trackLink().isValid() || track->track() == nullptr ) {
        ATH_MSG_WARNING("Track particle without Trk::Track");
        continue;
      }
      ATH_MSG_DEBUG("We have a Trk::Track");

      // We now have a valid Trk::Track
      const Trk::Track* trkTrack = track->track();

      //  This is the vector in which we will store the element links to the MSOS's
      std::vector< ElementLink< xAOD::TrackStateValidationContainer > > msosLink;

      if ( m_storeTRT && m_TRTdEdxTool.isEnabled() ) {
	// for dEdx studies
        trackTRTFloatDecorators[kTRTdEdxDecor] (*track)                 = m_TRTdEdxTool->dEdx(trkTrack,true);
        trackTRTFloatDecorators[kTRTusedHitsDecor] (*track)             = m_TRTdEdxTool->usedHits(trkTrack);
        trackTRTFloatDecorators[kTRTdEdx_noHT_divByLDecor] (*track)     = m_TRTdEdxTool->dEdx(trkTrack, false);
        trackTRTFloatDecorators[kTRTusedHits_noHT_divByLDecor] (*track) = m_TRTdEdxTool->usedHits(trkTrack, false);
      }

      if ( trkTrack->perigeeParameters() ){

        if(m_pixelLayerRadii.size() < 4) ATH_MSG_WARNING("Too few layer radii set! Should be at least 4!");  

        Trk::CylinderSurface cylSurfIBL(m_pixelLayerRadii[0], 3000.0);
        Trk::CylinderSurface cylSurfBL(m_pixelLayerRadii[1], 3000.0);
        Trk::CylinderSurface cylSurfL1(m_pixelLayerRadii[2], 3000.0);
        Trk::CylinderSurface cylSurfL2(m_pixelLayerRadii[3], 3000.0);

        bool allExtrapolationsSucceded = true;
        Trk::PropDirection whichDir = Trk::alongMomentum;
        Trk::MaterialUpdateMode whichMode = Trk::removeNoise;
        //check the radius of the start parameters, to see which direction we need to go to the target surface
        float startRadius = trkTrack->perigeeParameters()->associatedSurface().center().perp();
        ATH_MSG_VERBOSE("Start radius for extrapolating to layers: "<<startRadius);
        //see if we go along or opposite momentum
        if(startRadius>m_pixelLayerRadii[0]) {whichDir = Trk::oppositeMomentum; whichMode = Trk::addNoise;}
        std::unique_ptr<const Trk::TrackParameters> outputParamsIBL(
          m_extrapolator->extrapolate(ctx,
                                      *(trkTrack->perigeeParameters()),
                                      cylSurfIBL,
                                      whichDir,
                                      true,
                                      Trk::pion,
                                      whichMode));
        if(startRadius>m_pixelLayerRadii[1]) {whichDir = Trk::oppositeMomentum; whichMode = Trk::addNoise;}                              
        std::unique_ptr<const Trk::TrackParameters> outputParamsBL(
          m_extrapolator->extrapolate(ctx,
                                      *(trkTrack->perigeeParameters()),
                                      cylSurfBL,
                                      whichDir,
                                      true,
                                      Trk::pion,
                                      whichMode));
        if(startRadius>m_pixelLayerRadii[2]) {whichDir = Trk::oppositeMomentum; whichMode = Trk::addNoise;}                             
        std::unique_ptr<const Trk::TrackParameters> outputParamsL1(
          m_extrapolator->extrapolate(ctx,
                                      *(trkTrack->perigeeParameters()),
                                      cylSurfL1,
                                      whichDir,
                                      true,
                                      Trk::pion,
                                      whichMode));
        if(startRadius>m_pixelLayerRadii[2]) {whichDir = Trk::oppositeMomentum; whichMode = Trk::addNoise;}                             
        std::unique_ptr<const Trk::TrackParameters> outputParamsL2(
          m_extrapolator->extrapolate(ctx,
                                      *(trkTrack->perigeeParameters()),
                                      cylSurfL2,
                                      whichDir,
                                      true,
                                      Trk::pion,
                                      whichMode));

        if (outputParamsIBL.get()) {
          trackPixFloatDecorators[kTrkIBLXDecor](*track) = outputParamsIBL->position().x();
          trackPixFloatDecorators[kTrkIBLYDecor](*track) = outputParamsIBL->position().y();
          trackPixFloatDecorators[kTrkIBLZDecor](*track) = outputParamsIBL->position().z();
        }
        else {
          allExtrapolationsSucceded = false;
          ATH_MSG_VERBOSE("Extrapolation to IBL failed...");
          trackPixFloatDecorators[kTrkIBLXDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkIBLYDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkIBLZDecor](*track) = 0.0;
        }

        if (outputParamsBL.get()) {
          trackPixFloatDecorators[kTrkBLXDecor](*track) = outputParamsBL->position().x();
          trackPixFloatDecorators[kTrkBLYDecor](*track) = outputParamsBL->position().y();
          trackPixFloatDecorators[kTrkBLZDecor](*track) = outputParamsBL->position().z();
        }
        else {
          allExtrapolationsSucceded = false;
          ATH_MSG_VERBOSE("Extrapolation to BLayer failed...");
          trackPixFloatDecorators[kTrkBLXDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkBLYDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkBLZDecor](*track) = 0.0;
        }

        if (outputParamsL1.get()) {
          trackPixFloatDecorators[kTrkL1XDecor](*track) = outputParamsL1->position().x();
          trackPixFloatDecorators[kTrkL1YDecor](*track) = outputParamsL1->position().y();
          trackPixFloatDecorators[kTrkL1ZDecor](*track) = outputParamsL1->position().z();
        }
        else {
          allExtrapolationsSucceded = false;
          ATH_MSG_VERBOSE("Extrapolation to L1 failed...");
          trackPixFloatDecorators[kTrkL1XDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkL1YDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkL1ZDecor](*track) = 0.0;
        }

        if (outputParamsL2.get()) {
          trackPixFloatDecorators[kTrkL2XDecor](*track) = outputParamsL2->position().x();
          trackPixFloatDecorators[kTrkL2YDecor](*track) = outputParamsL2->position().y();
          trackPixFloatDecorators[kTrkL2ZDecor](*track) = outputParamsL2->position().z();
        }
        else {
          allExtrapolationsSucceded = false;
          ATH_MSG_VERBOSE("Extrapolation to L2 failed...");
          trackPixFloatDecorators[kTrkL2XDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkL2YDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkL2ZDecor](*track) = 0.0;
        }
      if(!allExtrapolationsSucceded) ATH_MSG_WARNING("At least one extrapolation to a Pixel layer failed!");
      }
      else{
          ATH_MSG_WARNING("No perigee TrackParameters found - filling positions on layers to (0,0,0)!");
          //should decorate nonetheless, to make sure decorations are consistent across events
          trackPixFloatDecorators[kTrkIBLXDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkIBLYDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkIBLZDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkBLXDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkBLYDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkBLZDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkL1XDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkL1YDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkL1ZDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkL2XDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkL2YDecor](*track) = 0.0;
          trackPixFloatDecorators[kTrkL2ZDecor](*track) = 0.0;
      }
      // -- Add Track states to the current track, filtering on their type
      std::vector<const Trk::TrackStateOnSurface*> tsoss;
      for (const auto *const trackState: *(trkTrack->trackStateOnSurfaces())){
        //Get rid of any holes that already exist  --  we are doing the search again
        if( trackState->types()[Trk::TrackStateOnSurface::Hole] )
          continue;
        tsoss.push_back(trackState);
      } 

      std::unique_ptr<const Trk::TrackStates>  holes; 
      if(m_storeHoles){
        holes =  std::unique_ptr<const Trk::TrackStates>( m_holeSearchTool->getHolesOnTrack(*trkTrack, trkTrack->info().particleHypothesis()) ); 
        for (const auto *hole: *holes){
          tsoss.push_back(hole);
        }
        if(trkTrack->perigeeParameters()){
          Trk::TrackStateOnSurfaceComparisonFunction CompFunc( trkTrack->perigeeParameters()->momentum() );
          stable_sort( tsoss.begin(), tsoss.end(), CompFunc );
        } else {
          ATH_MSG_ERROR("Track has no perigee parameters");
        }
      }

      //Loop over the TrkStateOnSurfaces
      for (const auto& trackState: tsoss){

        //Only store Holes, Measurement &  Outliers 
        if( !trackState->types()[Trk::TrackStateOnSurface::Hole] && 
            !trackState->types()[Trk::TrackStateOnSurface::Measurement] &&
            !trackState->types()[Trk::TrackStateOnSurface::Outlier] ) {
          continue;
        }

        // Check if we want to store this types of TSOS
        if(!m_storeOutliers && trackState->types()[Trk::TrackStateOnSurface::Outlier] )
          continue;


        if(!m_storeHoles && trackState->types()[Trk::TrackStateOnSurface::Hole] )
          continue;
                
        // Check that the surface has detector element
        if(!trackState->surface().associatedDetectorElement()){
          continue;
        }

        // Check that the surface ID is valid
        Identifier surfaceID = trackState->surface().associatedDetectorElement()->identify();
        if( !surfaceID.is_valid() ){
          ATH_MSG_WARNING("Invalid surface ID");
          continue;
        }

        //Determine what detector the hit is in
        bool isPixel(false); 
        bool isSCT(false);  
        bool isTRT(false);  
      
        if( m_idHelper->is_trt(surfaceID) ){
          isTRT = true;
          if(!m_storeTRT)
            continue;
        }else if( m_idHelper->is_sct(surfaceID) ){
          isSCT = true;
          if(!m_storeSCT)
            continue;
        }else if( m_idHelper->is_pixel(surfaceID) ){
          isPixel = true; 
          if(!m_storePixel)
            continue;
        } 
        
        if( !isPixel && !isSCT && !isTRT ){
          continue;
        } 
         
        //Create new MSOS to fill with information
        xAOD::TrackStateValidation*  msos =  new xAOD::TrackStateValidation();

        //Put it in the obeject in the correct conatiner -  one for each detector type.
        if(isTRT){        
          //Add the msos to the container 
          msosTRT->push_back( msos );
          //Set the det id
          msos->setDetType( Trk::TrackState::TRT );
          //Build the element link to the MSOS
          ElementLink< xAOD::TrackStateValidationContainer > elink( *msosTRT, nTRT_MSOS ); 
          elink.toPersistent();
          msosLink.push_back(elink);
          ++nTRT_MSOS;
        }else if(isSCT){
          //Add the msos to the container 
          msosSCT->push_back( msos );
          //Set the det id
          msos->setDetType( Trk::TrackState::SCT );
          //Build the element link to the MSOS
          ElementLink< xAOD::TrackStateValidationContainer > elink( *msosSCT, nSCT_MSOS ); 
          elink.toPersistent();
          msosLink.push_back(elink);
          ++nSCT_MSOS;
        }else if(isPixel){
          //Add the msos to the container 
          msosPixel->push_back( msos );
          //Set the det id
          msos->setDetType( Trk::TrackState::Pixel );
          //Build the element link to the MSOS
          ElementLink< xAOD::TrackStateValidationContainer > elink( *msosPixel, nPixelMSOS ); 
          elink.toPersistent();
          msosLink.push_back(elink);
          ++nPixelMSOS;
        }
        else {
           ATH_MSG_WARNING("NOT a pixel, SCT or TRT track state on surface.");
           delete msos;
           continue;
        }

        //fill type
        if( trackState->types()[Trk::TrackStateOnSurface::Hole] ){   
          msos->setType( Trk::TrackStateOnSurface::Hole );
        } else if (trackState->types()[Trk::TrackStateOnSurface::Measurement]){
          msos->setType( Trk::TrackStateOnSurface::Measurement );
        } else if ( trackState->types()[Trk::TrackStateOnSurface::Outlier] ) {
          msos->setType( Trk::TrackStateOnSurface::Outlier );
        }

        //Fill surface id
        msos->setDetElementId(  surfaceID.get_compact() );


	const Trk::TrackParameters* tp = trackState->trackParameters();       

	  // some more detailed hit info
	double lTheta=-1000., lPhi=-1000.;
        //Get the measurement base object
	const Trk::MeasurementBase* measurement=trackState->measurementOnTrack();

        msos->auxdata<float>("errDC") = -1 ;
        const Trk::RIO_OnTrack* rotp = dynamic_cast<const Trk::RIO_OnTrack*>(measurement) ;
        if (rotp) msos->auxdata<float>("errDC") = sqrt(rotp->localCovariance()(Trk::driftRadius, Trk::driftRadius)) ;

	if (m_storeTRT) {
	  const InDet::TRT_DriftCircleOnTrack *driftcircle = dynamic_cast<const InDet::TRT_DriftCircleOnTrack*>(measurement);
	  if (!measurement) {
	    msos->auxdata<float>("HitZ")=-3000;
	    msos->auxdata<float>("HitR")=-1;
	    msos->auxdata<float>("rTrkWire")=-1;
	  }
	  else { 
	    if (!driftcircle) {
	      msos->auxdata<float>("HitZ")=-3000;
	      msos->auxdata<float>("HitR")=-1;
	      msos->auxdata<float>("rTrkWire")=-1;
	    }
	    else {
	      if (tp) {
		const Amg::Vector3D& gp = driftcircle->globalPosition();
		msos->auxdata<float>("HitZ")=gp.z();
		msos->auxdata<float>("HitR")=gp.perp();
		msos->auxdata<float>("rTrkWire")= fabs(trackState->trackParameters()->parameters()[Trk::driftRadius]);
		lTheta = trackState->trackParameters()->parameters()[Trk::theta];
		lPhi = trackState->trackParameters()->parameters()[Trk::phi]; 
	      }
	      else {
		msos->auxdata<float>("HitZ") =driftcircle->associatedSurface().center().z();
		msos->auxdata<float>("HitR") =driftcircle->associatedSurface().center().perp();
		msos->auxdata<float>("rTrkWire")=0;
	      }
	    }
	  }
	  msos->setLocalAngles(lTheta, lPhi);

	  bool isShared=false;
          if (prd_to_track_map_cptr) {
             const Trk::RIO_OnTrack* hit_trt = measurement ? dynamic_cast<const Trk::RIO_OnTrack*>(measurement) : nullptr;
             if (hit_trt) {
                if (prd_to_track_map_cptr->isShared(*(hit_trt->prepRawData())) ) isShared=true;
                msos->auxdata<bool>("isShared") = isShared;
             }
          }
	}



        // Track extrapolation
        std::unique_ptr<const Trk::TrackParameters> extrap( m_extrapolator->extrapolateTrack(ctx,*trkTrack,trackState->surface()) );

        // Set local positions on the surface
        if (tp) { 
          msos->setLocalPosition( tp->parameters()[0], tp->parameters()[1] ); 

          if (extrap.get()) {
            ATH_MSG_DEBUG("    Original position " << tp->parameters()[0] << " " << tp->parameters()[1]);
            ATH_MSG_DEBUG("Extrapolated position " << extrap->parameters()[0] << " " << extrap->parameters()[1]);
          }

        }
        else {
          if (extrap.get()) {
            msos->setLocalPosition( extrap->parameters()[0], extrap->parameters()[1] ); 
          }
          else { 
            ATH_MSG_DEBUG("Track extrapolation failed."); 
          }
        }

        // Set calculate local incident angles
        const Trk::TrkDetElementBase *de = trackState->surface().associatedDetectorElement();
        const InDetDD::SiDetectorElement *side = dynamic_cast<const InDetDD::SiDetectorElement *>(de);
        if (side && (isSCT || isPixel)) {
          const Amg::Vector3D& mynormal = side->normal();
          const Amg::Vector3D& myphiax = side->phiAxis();
          const Amg::Vector3D& myetaax = side->etaAxis();
          if (tp) {
            Amg::Vector3D mytrack = tp->momentum();
            float trketacomp = mytrack.dot(myetaax);
            float trkphicomp = mytrack.dot(myphiax);
            float trknormcomp = mytrack.dot(mynormal);  

            ATH_MSG_DEBUG("     Original incident angle " << trketacomp << " " << trkphicomp << " " << trknormcomp);
            if (extrap.get()) {
              Amg::Vector3D metrack = extrap->momentum();
              float trketacompX = metrack.dot(myetaax);
              float trkphicompX = metrack.dot(myphiax);
              float trknormcompX = metrack.dot(mynormal);  
              ATH_MSG_DEBUG("Extrapolated incident angle " << trketacompX << " " << trkphicompX << " " << trknormcompX);
            }
            msos->setLocalAngles( atan2(trketacomp,trknormcomp), atan2(trkphicomp,trknormcomp) );
          }
          else {
            if (extrap.get()) {
              Amg::Vector3D metrack = extrap->momentum();
              float trketacompX = metrack.dot(myetaax);
              float trkphicompX = metrack.dot(myphiax);
              float trknormcompX = metrack.dot(mynormal);  
              msos->setLocalAngles( atan2(trketacompX,trknormcompX), atan2(trkphicompX,trknormcompX) );
            }
          }
        } 
 
        if(!measurement)
          continue;
      
        const Trk::RIO_OnTrack* hit = measurement ? dynamic_cast<const Trk::RIO_OnTrack*>(measurement) : nullptr;

        if(!hit){
          const Trk::CompetingRIOsOnTrack *crot = dynamic_cast<const Trk::CompetingRIOsOnTrack*>(measurement);
          if(crot){
            hit = &crot->rioOnTrack( crot->indexOfMaxAssignProb() );
          }
        }
        
        if(m_addPRD && hit){
          // Build an element link to the xAOD PRD
          const Trk::PrepRawData* prd = hit->prepRawData();
          if(prd && prd->getHashAndIndex().isValid() ){            
            if(isTRT){
              msos->setTrackMeasurementValidationLink( buildElementLink( prd, trtDCOffsets.cptr(), trtDCs.cptr()) );
            }else if(isSCT){
              msos->setTrackMeasurementValidationLink( buildElementLink( prd, sctClusterOffsets.cptr(), sctClusters.cptr()) );
            }else if(isPixel){
              msos->setTrackMeasurementValidationLink( buildElementLink( prd, pixelClusterOffsets.cptr(), pixelClusters.cptr()) );
            }
          } 
        }

        // Add the drift time for the tracks position -- note the position is biased 
        if (isTRT) {
          TRTCond::RtRelation const *rtr = m_trtcaldbTool->getRtRelation(surfaceID);
          if(rtr) {
            if (tp){
              msos->auxdata<float>("driftTime") = rtr->drifttime(fabs(tp->parameters()[0]));
            }
            else {
              if (extrap.get()) {
                msos->auxdata<float>("driftTime") = rtr->drifttime(fabs(extrap->parameters()[0]));
              }
            }
          }
	}

        if (m_addPulls) {

          std::unique_ptr<const Trk::ResidualPull> biased;
          std::unique_ptr<const Trk::ResidualPull> unbiased;
          if (tp) { 
            biased= m_residualPullCalculator->residualPull(measurement, tp, Trk::ResidualPull::Biased);
	    if (m_storeTRT) msos->auxdata<float>("TrackError_biased") = sqrt(fabs((*tp->covariance())(Trk::locX,Trk::locX)));

	    if (m_storeTRT) msos->auxdata<float>("TrackError_biased") = sqrt(fabs((*tp->covariance())(Trk::locX,Trk::locX)));
            std::unique_ptr<const Trk::TrackParameters> unbiasedTp( m_updator->removeFromState(*tp, measurement->localParameters(), measurement->localCovariance()) );
            if(unbiasedTp.get()) {
               if (m_storeTRT) msos->auxdata<float>("TrackError_unbiased") = sqrt(fabs((*unbiasedTp.get()->covariance())(Trk::locX,Trk::locX)));
               unbiased = m_residualPullCalculator->residualPull(measurement, unbiasedTp.get(), Trk::ResidualPull::Unbiased);
            }
          }
          else {
            if (extrap.get()) {
	      if (m_storeTRT) msos->auxdata<float>("TrackError_unbiased") = sqrt(fabs((*extrap.get()->covariance())(Trk::locX,Trk::locX)));
              biased = m_residualPullCalculator->residualPull(measurement, extrap.get(), Trk::ResidualPull::Biased);
              unbiased = m_residualPullCalculator->residualPull(measurement, extrap.get(), Trk::ResidualPull::Unbiased);
            }
          }

          if (biased) {
            if(biased->dimension()>Trk::locY){  
              msos->setBiasedResidual( biased->residual()[Trk::locX], biased->residual()[Trk::locY] );
              msos->setBiasedPull( biased->pull()[Trk::locX], biased->pull()[Trk::locY] );            
            } else {
              msos->setBiasedResidual( biased->residual()[Trk::locX], 0 );
              msos->setBiasedPull( biased->pull()[Trk::locX], 0 );            
            }
          } 

          if (unbiased) {
            if(unbiased->dimension()>Trk::locY){  
              msos->setUnbiasedResidual( unbiased->residual()[Trk::locX], unbiased->residual()[Trk::locY] );
              msos->setUnbiasedPull( unbiased->pull()[Trk::locX], unbiased->pull()[Trk::locY] );            
            } else {
              msos->setUnbiasedResidual( unbiased->residual()[Trk::locX], 0 );
              msos->setUnbiasedPull( unbiased->pull()[Trk::locX], 0 );            
            }
          }

        }

      } //end loop over TSOS's
        
      ATH_MSG_DEBUG("The number of TSOS's " << msosLink.size() );

      dectsos_msosLink( *track ) = msosLink;
      
      ATH_MSG_DEBUG("Finished dressing TrackParticle");
      

    } // end of loop over tracks              
    return StatusCode::SUCCESS;
    }  
  
  
  ElementLink< xAOD::TrackMeasurementValidationContainer >  TrackStateOnSurfaceDecorator::buildElementLink( const Trk::PrepRawData* prd, 
                                                                  const std::vector<unsigned int>* offsets, 
                                                                  const xAOD::TrackMeasurementValidationContainer* xaodPrdCont) const
  {
                                                                  
    const IdentContIndex& contIndex = prd->getHashAndIndex();
    if( contIndex.collHash() >= offsets->size() ){
      ATH_MSG_ERROR(" Offsets are incorrect " <<  contIndex.collHash() << " " <<  offsets->size() <<" "<< contIndex.objIndex());
      return {0,0};
    }
    
    unsigned int xaodIndex = offsets->at( contIndex.collHash() ) + contIndex.objIndex(); 
    ElementLink< xAOD::TrackMeasurementValidationContainer > el( *xaodPrdCont, xaodIndex ); 
    el.toPersistent();
    
    return el;
 
  }
  
  
}
