/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
//   Implementation file for class InDet::SiCombinatorialTrackFinder_xk
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
// Version 1.0 12/04/2007 I.Gavrilenko
///////////////////////////////////////////////////////////////////

#include "SiCombinatorialTrackFinderTool_xk/SiCombinatorialTrackFinder_xk.h"

#include "InDetPrepRawData/SiClusterContainer.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadHandle.h"
#include "TrkExInterfaces/IPatternParametersPropagator.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkToolInterfaces/IPatternParametersUpdator.h"
#include "TrkTrack/TrackInfo.h"

#include <iomanip>
#include <iostream>
#include <utility>
#include <stdexcept>

///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::SiCombinatorialTrackFinder_xk::SiCombinatorialTrackFinder_xk
(const std::string& t, const std::string& n, const IInterface* p)
  : base_class(t, n, p)
{
}

///////////////////////////////////////////////////////////////////
// Initialisation
///////////////////////////////////////////////////////////////////

StatusCode InDet::SiCombinatorialTrackFinder_xk::initialize()
{
  // Get RungeKutta propagator tool
  //
  if ( m_proptool.retrieve().isFailure() ) {
    ATH_MSG_FATAL("Failed to retrieve tool " << m_proptool);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Retrieved tool " << m_proptool);

  // Get updator tool
  //
  if ( m_updatortool.retrieve().isFailure() ) {
    ATH_MSG_FATAL("Failed to retrieve tool " << m_updatortool);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Retrieved tool " << m_updatortool);

  // Get RIO_OnTrack creator
  //
  if ( m_riocreator.retrieve().isFailure() ) {
    ATH_MSG_FATAL("Failed to retrieve tool " << m_riocreator);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Retrieved tool " << m_riocreator);

  // disable pixel/SCT conditions summary tool: pixel/SCT are not used, the status event data is used and not being validated
  ATH_CHECK( m_pixelCondSummaryTool.retrieve( DisableTool{!m_usePIX || (!m_pixelDetElStatus.empty() && !VALIDATE_STATUS_ARRAY_ACTIVATED)} ) );
  ATH_CHECK( m_sctCondSummaryTool.retrieve(   DisableTool{!m_useSCT || (!m_sctDetElStatus.empty()   && !VALIDATE_STATUS_ARRAY_ACTIVATED)} ) );
  //
  // Get InDetBoundaryCheckTool
  if ( m_boundaryCheckTool.retrieve().isFailure() ) {
    ATH_MSG_FATAL("Failed to retrieve tool " << m_boundaryCheckTool);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Retrieved tool " << m_boundaryCheckTool);



  // Setup callback for magnetic field
  //
  magneticFieldInit();

  // Get output print level
  //
  m_outputlevel = msg().level()-MSG::DEBUG;

  ATH_CHECK( m_pixcontainerkey.initialize (m_usePIX) );
  ATH_CHECK( m_boundaryPixelKey.initialize (m_usePIX) );

  ATH_CHECK( m_sctcontainerkey.initialize (m_useSCT) );
  ATH_CHECK( m_boundarySCTKey.initialize (m_useSCT) );

  // initialize conditions object key for field cache
  //
  ATH_CHECK( m_fieldCondObjInputKey.initialize() );
  ATH_CHECK( m_pixelDetElStatus.initialize( !m_pixelDetElStatus.empty() && m_usePIX) );
  ATH_CHECK( m_sctDetElStatus.initialize( !m_sctDetElStatus.empty() && m_useSCT) );
  m_minPt2Cut = std::pow(m_minPtCut.value(),2);
  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Finalize
///////////////////////////////////////////////////////////////////

StatusCode InDet::SiCombinatorialTrackFinder_xk::finalize()
{
  return AlgTool::finalize();
}

///////////////////////////////////////////////////////////////////
// Dumps relevant information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream&  InDet::SiCombinatorialTrackFinder_xk::dump(SiCombinatorialTrackFinderData_xk& data, MsgStream& out) const
{
  if (not data.isInitialized()) initializeCombinatorialData(Gaudi::Hive::currentContext(), data);

  out<<std::endl;
  if (data.nprint()) return dumpevent(data, out);
  return dumpconditions(out);
}

///////////////////////////////////////////////////////////////////
// Dumps conditions information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiCombinatorialTrackFinder_xk::dumpconditions(MsgStream& out) const
{
  int n = 62-m_proptool.type().size();
  std::string s1;
  for (int i=0; i<n; ++i) s1.append(" ");
  s1.append("|");

  std::string fieldmode[9] ={"NoField"       , "ConstantField", "SolenoidalField",
			     "ToroidalField" , "Grid3DField"  , "RealisticField" ,
			     "UndefinedField", "AthenaField"  , "?????"         };

  int mode = m_fieldprop.magneticFieldMode();
  if (mode<0 || mode>8 ) mode = 8;

  n     = 62-fieldmode[mode].size();
  std::string s3;
  for (int i=0; i<n; ++i) s3.append(" ");
  s3.append("|");

  n     = 62-m_updatortool.type().size();
  std::string s4;
  for (int i=0; i<n; ++i) s4.append(" ");
  s4.append("|");

  n     = 62-m_riocreator.type().size();
  std::string s5;
  for (int i=0; i<n; ++i) s5.append(" ");
  s5.append("|");

  n     = 62-m_pixcontainerkey.key().size();
  std::string s7;
  for (int i=0; i<n; ++i) s7.append(" ");
  s7.append("|");

  n     = 62-m_sctcontainerkey.key().size();
  std::string s8;
  for (int i=0; i<n; ++i) s8.append(" ");
  s8.append("|");

  out<<"|----------------------------------------------------------------------"
     <<"-------------------|"
     <<std::endl;
  if (m_usePIX) {
    out<<"| Pixel clusters location | "<<m_pixcontainerkey.key() <<s7<<std::endl;
  }
  if (m_useSCT) {
    out<<"| SCT   clusters location | "<<m_sctcontainerkey.key() <<s8<<std::endl;
  }
  out<<"| Tool for propagation    | "<<m_proptool   .type()<<s1<<std::endl;
  out<<"| Tool for updator        | "<<m_updatortool.type()<<s4<<std::endl;
  out<<"| Tool for rio  on track  | "<<m_riocreator .type()<<s5<<std::endl;
  out<<"| Magnetic field mode     | "<<fieldmode[mode]     <<s3<<std::endl;
  out<<"|----------------------------------------------------------------------"
     <<"-------------------|"
     <<std::endl;
  return out;
}

///////////////////////////////////////////////////////////////////
// Dumps event information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiCombinatorialTrackFinder_xk::dumpevent(SiCombinatorialTrackFinderData_xk& data, MsgStream& out)
{
  out<<"|---------------------------------------------------------------------|"
     <<std::endl;
  out<<"| Min pT of track (MeV)   | "<<std::setw(12)<<std::setprecision(5)<<data.pTmin()
     <<"                              |"<<std::endl;
  out<<"| Max Xi2 for cluster     | "<<std::setw(12)<<std::setprecision(5)<<data.xi2max()
     <<"                              |"<<std::endl;
  out<<"| Max Xi2 for outlayer    | "<<std::setw(12)<<std::setprecision(5)<<data.xi2maxNoAdd()
     <<"                              |"<<std::endl;
  out<<"| Max Xi2 for link        | "<<std::setw(12)<<std::setprecision(5)<<data.xi2maxlink()
     <<"                              |"<<std::endl;
  out<<"| Min number of clusters  | "<<std::setw(12)<<data.nclusmin()
     <<"                              |"<<std::endl;
  out<<"| Min number of wclusters | "<<std::setw(12)<<data.nwclusmin()
     <<"                              |"<<std::endl;
  out<<"| Max number holes        | "<<std::setw(12)<<data.nholesmax()
     <<"                              |"<<std::endl;
  out<<"| Max holes  gap          | "<<std::setw(12)<<data.dholesmax()
     <<"                              |"<<std::endl;
  out<<"| Use PRD to track assoc.?| "<<std::setw(12)<<(data.PRDtoTrackMap() ? "yes" : "no ")
     <<"                              |"<<std::endl;
  out<<"|---------------------------------------------------------------------|"
     <<std::endl;
  out<<"| Number input     seeds  | "<<std::setw(12)<<data.inputseeds()
     <<"                              |"<<std::endl;
  out<<"| Number accepted  seeds  | "<<std::setw(12)<<data.goodseeds()
     <<"                              |"<<std::endl;
  out<<"| Number initial  tracks  | "<<std::setw(12)<<data.inittracks()
     <<"                              |"<<std::endl;
  out<<"| Number wrong DE  roads  | "<<std::setw(12)<<data.roadbug()
     <<"                              |"<<std::endl;
  out<<"| Number output   tracks  | "<<std::setw(12)<<data.findtracks()
     <<"                              |"<<std::endl;
  out<<"|---------------------------------------------------------------------|"
     <<std::endl;
  return out;
}

///////////////////////////////////////////////////////////////////
// Initiate track finding tool
///////////////////////////////////////////////////////////////////

void InDet::SiCombinatorialTrackFinder_xk::newEvent(const EventContext& ctx, SiCombinatorialTrackFinderData_xk& data) const
{
  if (not data.isInitialized()) initializeCombinatorialData(ctx, data);

  // Erase statistic information
  //
  data.inputseeds() = 0;
  data.goodseeds()  = 0;
  data.inittracks() = 0;
  data.findtracks() = 0;
  data.roadbug()    = 0;

  // Set track info
  //
  data.trackinfo().setPatternRecognitionInfo(Trk::TrackInfo::SiSPSeededFinder);
  data.setCosmicTrack(0);

  // Add conditions object to SiCombinatorialTrackFinderData to be able to access the field cache for each new event
  // Get conditions object for field cache
  SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCondObjInputKey, ctx};
  const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};
  if (fieldCondObj == nullptr) {
      std::string msg = "InDet::SiCombinatorialTrackFinder_xk::newEvent: Failed to retrieve AtlasFieldCacheCondObj with key " + m_fieldCondObjInputKey.key();
      throw(std::runtime_error(msg));
  }
  data.setFieldCondObj(fieldCondObj);
}

///////////////////////////////////////////////////////////////////
// Initiate track finding tool
///////////////////////////////////////////////////////////////////

void InDet::SiCombinatorialTrackFinder_xk::newEvent
(const EventContext& ctx, SiCombinatorialTrackFinderData_xk& data, Trk::TrackInfo info, const TrackQualityCuts& Cuts) const
{

  if (not data.isInitialized()) {
    //Check if to use PRDAssociation before initializing all the tools
    int useasso;
    if(!Cuts.getIntCut   ("UseAssociationTool"  ,useasso      )) useasso = 0;
    data.tools().setAssociation(useasso);

    initializeCombinatorialData(ctx, data);
  }

  newEvent(ctx, data);
  data.trackinfo() = info;

  /// Get track quality cuts information from argument and write it into
  /// the event data
  getTrackQualityCuts(data, Cuts);

  data.setHeavyIon(false);
  data.setCosmicTrack(0);
  /// update pattern recognition flags in the event data based on track info arg
  if (info.patternRecoInfo(Trk::TrackInfo::SiSpacePointsSeedMaker_Cosmic))
    data.setCosmicTrack(1);
  else if (info.patternRecoInfo(Trk::TrackInfo::SiSpacePointsSeedMaker_HeavyIon))
    data.setHeavyIon(true);

}

///////////////////////////////////////////////////////////////////
// Finalize track finding tool for given event
///////////////////////////////////////////////////////////////////

void InDet::SiCombinatorialTrackFinder_xk::endEvent(SiCombinatorialTrackFinderData_xk& data) const
{
  if (not data.isInitialized()) initializeCombinatorialData(Gaudi::Hive::currentContext(), data);

  // Print event information
  //
  if (m_outputlevel<=0) {
    data.nprint() = 1;
    dump(data, msg(MSG::DEBUG));
  }
}

///////////////////////////////////////////////////////////////////
// Main method for track finding using space points
///////////////////////////////////////////////////////////////////

const std::list<Trk::Track*>&  InDet::SiCombinatorialTrackFinder_xk::getTracks
(SiCombinatorialTrackFinderData_xk& data,
 const Trk::TrackParameters& Tp,
 const std::vector<const Trk::SpacePoint*>& Sp,
 const std::vector<Amg::Vector3D>& Gp,
 std::vector<const InDetDD::SiDetectorElement*>& DE,
 const TrackQualityCuts& Cuts,
 const EventContext& ctx) const
{

  if (not data.isInitialized()) initializeCombinatorialData(ctx, data);

  data.statistic().fill(false);

  /// turn off brem fit & electron flags
  data.tools().setBremNoise(false, false);
  /// remove existing tracks
  data.tracks().erase(data.tracks().begin(), data.tracks().end());

  ++data.inputseeds();
  if (!m_usePIX && !m_useSCT) {
    return data.tracks();
  }

  // Get track quality cuts information and write them into the event data... again?
  getTrackQualityCuts(data, Cuts);
  std::multimap<const Trk::PrepRawData*, const Trk::Track*> PT;

  ///try to find the tracks
  EStat_t FT = findTrack(data, Tp, Sp, Gp, DE, PT, ctx);

  /// if we didn't find anything, bail out
  if(FT!=Success) {
    data.statistic()[FT] = true;
    if( ! data.flagToReturnFailedTrack() ) return data.tracks();
  }

  /// sort in step order
  data.trajectory().sortStep();


  Trk::Track* t = convertToTrack(data,ctx);
  if (t) {
     ++data.findtracks();
     if (m_writeHolesFromPattern) data.addPatternHoleSearchOutcome(t,data.trajectory().getHoleSearchResult());
     data.tracks().push_back(t);
  }

  if (!data.tools().multiTrack() ||
      data.simpleTrack() ||
      Sp.size()<=2 ||
      data.cosmicTrack() ||
      data.trajectory().pTfirst() < data.tools().pTmin()) return data.tracks();

  while ((t=convertToNextTrack(data))) {
    ++data.findtracks();
    if (m_writeHolesFromPattern) data.addPatternHoleSearchOutcome(t,data.trajectory().getHoleSearchResult());
    data.tracks().push_back(t);
  }
  return data.tracks();
}

///////////////////////////////////////////////////////////////////
// Main method for track finding using space points
///////////////////////////////////////////////////////////////////

const std::list<Trk::Track*>& InDet::SiCombinatorialTrackFinder_xk::getTracks
(SiCombinatorialTrackFinderData_xk& data,
 const Trk::TrackParameters& Tp,
 const std::vector<const Trk::SpacePoint*>& Sp,
 const std::vector<Amg::Vector3D>& Gp,
 std::vector<const InDetDD::SiDetectorElement*>& DE,
 std::multimap<const Trk::PrepRawData*, const Trk::Track*>& PT,
 const EventContext& ctx) const
{

  if (not data.isInitialized()) initializeCombinatorialData(ctx, data);

  data.tools().setBremNoise(false, false);
  data.tracks().erase(data.tracks().begin(), data.tracks().end());

  data.statistic().fill(false);
  ++data.inputseeds();
  if (!m_usePIX && !m_useSCT) {
    return data.tracks();
  }

  EStat_t FT = findTrack(data, Tp, Sp, Gp, DE, PT,ctx);
  if(FT!=Success) {
    data.statistic()[FT] = true;
    if( ! data.flagToReturnFailedTrack() || std::abs(data.resultCode()) < SiCombinatorialTrackFinderData_xk::ResultCodeThreshold::RecoverableForDisTrk ) return data.tracks();
  }
  if (!data.trajectory().isNewTrack(PT))
  {
     data.statistic()[NotNewTrk] = true;
     return data.tracks();
  }
  data.trajectory().sortStep();

  if(data.useFastTracking()) {
    if(!data.trajectory().filterWithPreciseClustersError(ctx)) {
      data.statistic()[CantFindTrk] = true;
      return data.tracks();
    }
  }

  // Trk::Track production
  //
  Trk::Track* t = convertToTrack(data,ctx);
  if (t==nullptr) return data.tracks(); // @TODO should one check if convertToNextTrack would yield anything ?
  if (m_writeHolesFromPattern) data.addPatternHoleSearchOutcome(t,data.trajectory().getHoleSearchResult());

  ++data.findtracks();
  data.tracks().push_back(t);

  if (!data.tools().multiTrack() ||
      data.simpleTrack() ||
      Sp.size()<=2 ||
      data.cosmicTrack() ||
      data.trajectory().pTfirst() < data.tools().pTmin()) return data.tracks();

  while ((t=convertToNextTrack(data))) {
    if (m_writeHolesFromPattern) data.addPatternHoleSearchOutcome(t,data.trajectory().getHoleSearchResult());
    ++data.findtracks();
    data.tracks().push_back(t);
  }
  return data.tracks();
}

///////////////////////////////////////////////////////////////////
// Main method for track finding using space points and
// using electron noise model
///////////////////////////////////////////////////////////////////

const std::list<Trk::Track*>&  InDet::SiCombinatorialTrackFinder_xk::getTracksWithBrem
(SiCombinatorialTrackFinderData_xk& data,
 const Trk::TrackParameters& Tp,
 const std::vector<const Trk::SpacePoint*>& Sp,
 const std::vector<Amg::Vector3D>& Gp,
 std::vector<const InDetDD::SiDetectorElement*>& DE,
 std::multimap<const Trk::PrepRawData*, const Trk::Track*>& PT,
 bool isCaloCompatible,
 const EventContext& ctx) const
{

  if (not data.isInitialized()) initializeCombinatorialData(ctx, data);

  data.statistic().fill(false);

  // Old information
  //
  int mult = 0;
  if (data.tools().multiTrack()) mult = 1;
  double Xi2m = data.tools().xi2multi();

  data.tools().setBremNoise(false, true);
  data.tracks().erase(data.tracks().begin(), data.tracks().end());

  ++data.inputseeds();
  if (!m_usePIX && !m_useSCT) {
    return data.tracks();
  }

  EStat_t FT = findTrack(data,Tp,Sp,Gp,DE,PT,ctx);

  bool Q = (FT==Success);
  if (Q){
    Q = data.trajectory().isNewTrack(PT);
  }

  int na = 0;
  if (Q) {
    data.trajectory().sortStep();

    // Trk::Track production
    //
    Trk::TrackInfo oldinfo = data.trackinfo();
    if (isCaloCompatible) data.trackinfo().setPatternRecognitionInfo(Trk::TrackInfo::TrackInCaloROI);

    data.tools().setMultiTracks(0, Xi2m);
    Trk::Track* t = convertToTrack(data,ctx);
    data.trackinfo() = oldinfo;
    data.tools().setMultiTracks(mult,Xi2m);

    if (t==nullptr) return data.tracks(); // @TODO should one check whether the next findTrack call would yield something ?
    if (m_writeHolesFromPattern) data.addPatternHoleSearchOutcome(t,data.trajectory().getHoleSearchResult());
    ++data.findtracks();
    data.tracks().push_back(t);
    na = data.trajectory().nclusters();
    if (na >=12 && !data.trajectory().nclustersNoAdd()) return data.tracks();

    if (data.trajectory().pTfirst() < data.pTminBrem()) return data.tracks();
  }
  if ((*Sp.begin())->clusterList().second) return data.tracks();

  // Repeat track finding using electron noise model
  //
  data.statistic()[BremAttempt] = true;

  data.tools().setBremNoise(true,true);
  FT = findTrack(data, Tp, Sp, Gp, DE, PT,ctx);

  if (FT!=Success) {
    data.statistic()[FT] = true;
    return data.tracks();
  }

  if (!data.trajectory().isNewTrack(PT)) { data.statistic()[NotNewTrk] = true; return data.tracks(); }

  int nb = data.trajectory().nclusters();
  if (nb <= na) return data.tracks();

  data.trajectory().sortStep();

  if(data.useFastTracking()) {
    if(!data.trajectory().filterWithPreciseClustersError(ctx)) {
      data.statistic()[CantFindTrk] = true;
      return data.tracks();
    }
  }

  // Trk::Track production
  //
  Trk::TrackInfo oldinfo = data.trackinfo();
  data.trackinfo().setTrackProperties(Trk::TrackInfo::BremFit          );
  data.trackinfo().setTrackProperties(Trk::TrackInfo::BremFitSuccessful);
  if (isCaloCompatible) data.trackinfo().setPatternRecognitionInfo(Trk::TrackInfo::TrackInCaloROI);

  data.tools().setMultiTracks(0, Xi2m);
  Trk::Track* t = convertToTrack(data, ctx);
  data.trackinfo() = oldinfo;
  data.tools().setMultiTracks(mult, Xi2m);

  if (t==nullptr) return data.tracks();
  if (m_writeHolesFromPattern) data.addPatternHoleSearchOutcome(t,data.trajectory().getHoleSearchResult());

  ++data.findtracks();
  data.tracks().push_back(t);
  return data.tracks();
}

///////////////////////////////////////////////////////////////////
// Initial pT of 3 space points seed estimation
///////////////////////////////////////////////////////////////////

double InDet::SiCombinatorialTrackFinder_xk::pTseed
(SiCombinatorialTrackFinderData_xk& data,
 const Trk::TrackParameters& Tp,
 const std::vector<const Trk::SpacePoint*>& Sp,
 const EventContext& ctx) const
{
  std::vector<const InDet::SiCluster*>           Cl;
  std::vector<const InDetDD::SiDetectorElement*> DE;
  if(!spacePointsToClusters(Sp,Cl,DE)) return 0.;

  std::vector<const InDet::SiDetElementBoundaryLink_xk*> DEL;
  detectorElementLinks(DE,DEL,ctx);
  return data.trajectory().pTseed(Tp,Cl,DEL,ctx);
}

///////////////////////////////////////////////////////////////////
// Main method for track finding using space points
///////////////////////////////////////////////////////////////////

InDet::SiCombinatorialTrackFinder_xk::EStat_t InDet::SiCombinatorialTrackFinder_xk::findTrack
(SiCombinatorialTrackFinderData_xk& data,
 const Trk::TrackParameters& Tp,
 const std::vector<const Trk::SpacePoint*>& Sp,
 const std::vector<Amg::Vector3D>& Gp,
 std::vector<const InDetDD::SiDetectorElement*>& DE,
 std::multimap<const Trk::PrepRawData*,const Trk::Track*>& PT,
 const EventContext& ctx) const
{
  /// init event data
  if (not data.isInitialized()) initializeCombinatorialData(ctx, data);

  /// populate a list of boundary links for the detector elements on our search road
  std::vector<const InDet::SiDetElementBoundaryLink_xk*> DEL;
  detectorElementLinks(DE, DEL,ctx);

  /// Retrieve cached pointers to SG collections, or create the cache
  const InDet::PixelClusterContainer* p_pixcontainer = data.pixContainer();
  if (m_usePIX && !p_pixcontainer) {
    SG::ReadHandle<InDet::PixelClusterContainer> pixcontainer(m_pixcontainerkey,ctx);
    p_pixcontainer = pixcontainer.ptr();
    data.setPixContainer(p_pixcontainer);
  }
  const InDet::SCT_ClusterContainer* p_sctcontainer = data.sctContainer();
  if (m_useSCT && !p_sctcontainer) {
    SG::ReadHandle<InDet::SCT_ClusterContainer> sctcontainer(m_sctcontainerkey,ctx);
    p_sctcontainer = sctcontainer.ptr();
    data.setSctContainer(p_sctcontainer);
  }

  /// Cluster list preparationn
  std::vector<const InDet::SiCluster*> Cl;
  bool isTwoPointSeed = false;

  /// in inside-out track finding, Sp.size() is typically 3
  /// for TRT-seeded backtracking, it is 2
  /// both applications go into this branch
  if (Sp.size() > 1) {

    /// returns false if two clusters are on the same detector element
    if (!spacePointsToClusters(Sp,Cl)) {
      return TwoCluster;
    }
    if (Sp.size()==2) isTwoPointSeed = true;
  }
  /// use case if we have a set of global positions rather than space points to start from
  else if (Gp.size() > 2) {
    if (!data.trajectory().globalPositionsToClusters(p_pixcontainer, p_sctcontainer, Gp, DEL, PT, Cl)) return TwoCluster;
  } else {
    /// use case if we have neither space-points nor global posittions, but track parameters to start from
    if (!data.trajectory().trackParametersToClusters(p_pixcontainer, p_sctcontainer, Tp, DEL, PT, Cl, ctx)) return TwoCluster;
  }
  ++data.goodseeds();

  /// Build initial trajectory
  bool Qr;
  /// This will initialize the trajectory using the clusters we have and the parameter estimate
  bool Q = data.trajectory().initialize(m_usePIX, m_useSCT, p_pixcontainer, p_sctcontainer, Tp, Cl, DEL, Qr,ctx);

  /// if the initialisation fails (indicating this is probably a bad attempt) and we are running with
  /// global positions instead of seeding
  if (!Q && Sp.size() < 2 && Gp.size() > 3) {
    /// reset our cluster list
    Cl.clear();
    /// try again using the clusters from the track parameters only
    if (!data.trajectory().trackParametersToClusters(p_pixcontainer, p_sctcontainer, Tp, DEL, PT, Cl,ctx)) return TwoCluster;

    if (!data.trajectory().initialize(m_usePIX, m_useSCT, p_pixcontainer, p_sctcontainer, Tp, Cl, DEL, Qr,ctx)) return TwoCluster;
    /// if it worked now, set the quality flag to true

    Q = Qr = true;
  }

  /// this can never happen?!
  if (!Qr){
    ++data.roadbug();
    return WrongRoad;
  }
  /// this can never happen either?!
  if (!Q) return WrongInit;

  ++data.inittracks();
  /// if the last cluster on track is in the pixels, this is assumed to come from a pixel seed
  bool pixseed = data.trajectory().isLastPixel();
  /// max #iterations
  int itmax    = 30;
  if (!data.useFastTracking() and data.simpleTrack()) itmax = 10;
  if (data.heavyIon()) itmax = 50;

  //
  bool toReturnFailedTrack = data.flagToReturnFailedTrack();
  if( toReturnFailedTrack ) data.setResultCode(SiCombinatorialTrackFinderData_xk::ResultCode::Unrecoverable);

  /// Track finding
  if (pixseed) {      /// Strategy for pixel seeds
    if (!data.trajectory().forwardExtension (false,itmax,ctx)) return CantFindTrk;
    if (!data.trajectory().backwardSmoother (false,ctx)      ) return CantFindTrk;
    if (!data.trajectory().backwardExtension(itmax,ctx)      ) return CantFindTrk;
    if (data.isITkGeometry() &&
	(data.trajectory().nclusters() < data.nclusmin() ||
	 data.trajectory().ndf() < data.nwclusmin()) ) return CantFindTrk;

    /// refine if needed
    if(!data.useFastTracking() && data.trajectory().difference() > 0){
      if (!data.trajectory().forwardFilter(ctx)) {
	if( toReturnFailedTrack ) {
	  data.setResultCode(SiCombinatorialTrackFinderData_xk::ResultCode::PixSeedDiffKFFwd);
	}
	else {
	  return CantFindTrk;
	}
      }

      if (!data.trajectory().backwardSmoother (false,ctx) ) {
	if( toReturnFailedTrack ) {
	  data.setResultCode(SiCombinatorialTrackFinderData_xk::ResultCode::PixSeedDiffKFBwd);
	}
	else {
	  return CantFindTrk;
	}
      }
    }

    int na = data.trajectory().nclustersNoAdd();
    /// check if we found enough clusters
    if (data.trajectory().nclusters()+na < data.nclusmin() ||
	data.trajectory().ndf() < data.nwclusmin()) {
       if( toReturnFailedTrack ) {
	  data.setResultCode(SiCombinatorialTrackFinderData_xk::ResultCode::PixSeedNCluster);
       }
       else {
	  return CantFindTrk;
       }
    }
  }

  /// case of a strip seed or mixed PPS
  else {      // Strategy for mixed seeds
    if (!data.trajectory().backwardSmoother(isTwoPointSeed,ctx)       ) return CantFindTrk;
    if (!data.trajectory().backwardExtension(itmax,ctx)    ) return CantFindTrk;
    if (!data.trajectory().forwardExtension(true,itmax,ctx)) return CantFindTrk;

    /// first application of hit cut
    int na = data.trajectory().nclustersNoAdd();
    if (data.trajectory().nclusters()+na < data.nclusmin() ||
	data.trajectory().ndf() < data.nwclusmin()) return CantFindTrk;
    /// backward smooting
    if (!data.trajectory().backwardSmoother(false,ctx)    ) return CantFindTrk;

    /// apply hit cut again following smoothing step
    na     = data.trajectory().nclustersNoAdd();
    if (data.trajectory().nclusters()+na < data.nclusmin() ||
	data.trajectory().ndf() < data.nwclusmin()) {
       if( toReturnFailedTrack ) {
	  data.setResultCode(SiCombinatorialTrackFinderData_xk::ResultCode::MixSeedNCluster);
       }
       else {
	  return CantFindTrk;
       }
    }

    /// refine if needed
    if (data.trajectory().difference() > 0) {
      if (!data.trajectory().forwardFilter(ctx)) {
	 if( toReturnFailedTrack ) {
	    data.setResultCode(SiCombinatorialTrackFinderData_xk::ResultCode::MixSeedDiffKFFwd);
	 }
	 else {
	    return CantFindTrk;
	 }
      }
      if (!data.trajectory().backwardSmoother (false, ctx)) {
	 if( toReturnFailedTrack ) {
	    data.setResultCode(SiCombinatorialTrackFinderData_xk::ResultCode::MixSeedDiffKFBwd);
	 }
	 else {
	    return CantFindTrk;
	 }
      }
    }
  }

  /// quality cut
  if (data.trajectory().qualityOptimization() < (m_qualityCut*data.nclusmin())) {
     if( toReturnFailedTrack ) {
	data.setResultCode(SiCombinatorialTrackFinderData_xk::ResultCode::Quality);
     }
     else {
	return CantFindTrk;
     }
  }

  if (data.trajectory().pTfirst  () < data.pTmin   () &&
      data.trajectory().nclusters() < data.nclusmin()) {
     if( toReturnFailedTrack ) {
	data.setResultCode(SiCombinatorialTrackFinderData_xk::ResultCode::Pt);
     }
     else {
	return CantFindTrk;
     }
  }

  if (data.trajectory().nclusters() < data.nclusminb() ||
      data.trajectory().ndf      () < data.nwclusmin()) {
     if( toReturnFailedTrack ) {
	data.setResultCode(SiCombinatorialTrackFinderData_xk::ResultCode::NCluster);
     }
     else {
	return CantFindTrk;
     }
  }

  /// refine the hole cut
  if (m_writeHolesFromPattern){
    data.trajectory().updateHoleSearchResult();
    if (!data.trajectory().getHoleSearchResult().passPatternHoleCut) {
       if( toReturnFailedTrack ) {
	  data.setResultCode(SiCombinatorialTrackFinderData_xk::ResultCode::HoleCut);
       }
       else {
	  return CantFindTrk;
       }
    }
  }

  if( toReturnFailedTrack ) {
     if(  data.resultCode() != SiCombinatorialTrackFinderData_xk::ResultCode::Unrecoverable ) {
	return CantFindTrk;
     }
     else {
	data.setResultCode(SiCombinatorialTrackFinderData_xk::ResultCode::Success);
     }
  }

  return Success;
}

///////////////////////////////////////////////////////////////////
// Trk::Track production
///////////////////////////////////////////////////////////////////

Trk::Track* InDet::SiCombinatorialTrackFinder_xk::convertToTrack(SiCombinatorialTrackFinderData_xk& data, const EventContext& ctx) const
{
  const Trk::PatternTrackParameters *param = data.trajectory().firstParameters();
  if (param) {
     double pt = param->transverseMomentum();
     // reject tracks with small pT
     // The cut should be large enough otherwise eta computation of such tracks may yield NANs.
     if (pt < m_minPtCut.value()) {
        ATH_MSG_DEBUG( "Reject low pT track (pT = " << pt << " < " << m_minPtCut.value() << ")");
        return nullptr;
     }
  }
  if (!data.simpleTrack()) {
     return new Trk::Track(
         data.trackinfo(),
         std::make_unique<Trk::TrackStates>(
             data.trajectory().convertToTrackStateOnSurface(
                 data.cosmicTrack())),
         data.trajectory().convertToFitQuality());
  }

  Trk::TrackInfo info = data.trackinfo();
  info.setPatternRecognitionInfo(Trk::TrackInfo::SiSPSeededFinderSimple);
  info.setParticleHypothesis(Trk::pion);
  if (!data.flagToReturnFailedTrack()) {
     return new Trk::Track(
         info,
         std::make_unique<Trk::TrackStates>(
             data.trajectory().convertToSimpleTrackStateOnSurface(
                 data.cosmicTrack(), ctx)),
         data.trajectory().convertToFitQuality());
  } else {
     return new Trk::Track(
         info,
         std::make_unique<Trk::TrackStates>(
             data.trajectory()
                 .convertToSimpleTrackStateOnSurfaceForDisTrackTrigger(
                     data.cosmicTrack(), ctx)),
         data.trajectory().convertToFitQuality());
  }
}

///////////////////////////////////////////////////////////////////
// Next Trk::Track production
///////////////////////////////////////////////////////////////////

Trk::Track* InDet::SiCombinatorialTrackFinder_xk::convertToNextTrack(SiCombinatorialTrackFinderData_xk& data) const
{
  auto tsos = std::make_unique<Trk::TrackStates>(
      data.trajectory().convertToNextTrackStateOnSurface());
  if (tsos->empty()){
     return nullptr;
  }

  // verify first track parameters
  const Trk::TrackParameters *param = nullptr;
  for (const Trk::TrackStateOnSurface *a_tsos : *tsos) {
     const Trk::TrackParameters *param = a_tsos->trackParameters();
     if (param) {
        break;
     }
  }

  if (param) {
     auto momentum = param->momentum();
     const auto  pt2 = momentum.perp2();
     // reject tracks with small pT
     // The cut should be large enough otherwise eta computation of such tracks may yield NANs.
     if (pt2 < m_minPt2Cut) {
        ATH_MSG_WARNING( "Reject low pT track (pT = " << sqrt(pt2) << " < " << m_minPtCut.value() << ")");
        return nullptr;
     }
  }
  return new Trk::Track(data.trackinfo(),
                        std::move(tsos),
                        data.trajectory().convertToFitQuality());
}

///////////////////////////////////////////////////////////////////
// Callback function - get the magnetic field /
///////////////////////////////////////////////////////////////////

void InDet::SiCombinatorialTrackFinder_xk::magneticFieldInit()
{
  // Build MagneticFieldProperties
  //
  if      (m_fieldmode == "NoField"    ) m_fieldprop = Trk::MagneticFieldProperties(Trk::NoField  );
  else if (m_fieldmode == "MapSolenoid") m_fieldprop = Trk::MagneticFieldProperties(Trk::FastField);
  else                                   m_fieldprop = Trk::MagneticFieldProperties(Trk::FullField);
}

///////////////////////////////////////////////////////////////////
// Convert space points to clusters and (for Run 4) detector elements
///////////////////////////////////////////////////////////////////

bool InDet::SiCombinatorialTrackFinder_xk::spacePointsToClusters
(const std::vector<const Trk::SpacePoint*>& Sp, std::vector<const InDet::SiCluster*>& Sc, std::optional<std::reference_wrapper<std::vector<const InDetDD::SiDetectorElement*>>> DE)
{
  Sc.reserve(Sp.size());
  /// loop over all SP
  for (const Trk::SpacePoint* s : Sp) {
     /// get the first cluster on an SP
     const Trk::PrepRawData* p = s->clusterList().first;
     if (p) {
        /// add to list
        const InDet::SiCluster* c = static_cast<const InDet::SiCluster*>(p);
        if (c){
          Sc.push_back(c);
        }
     }
     /// for strips, also make sure to pick up the second one!
     p = s->clusterList().second;
     if (p) {
        const InDet::SiCluster* c = static_cast<const InDet::SiCluster*>(p);
        if (c){
          Sc.push_back(c);
        }
     }
  }

  ///  Detector elments test
  std::vector<const InDet::SiCluster*>::iterator cluster = Sc.begin(), nextCluster, endClusters = Sc.end();

  /// here we reject cases where two subsequent clusters are on the same
  /// detector element
  if (DE) {
    DE->get().reserve(Sc.size());
  }
  for (; cluster != endClusters; ++cluster) {

     const InDetDD::SiDetectorElement* de = (*cluster)->detectorElement();

     nextCluster = cluster;
     ++nextCluster;
     for (; nextCluster != endClusters; ++nextCluster) {
        if (de == (*nextCluster)->detectorElement()){
          return false;
        }
     }
     if (DE) {
       DE->get().push_back(de);
     }
  }
  return true;
}

///////////////////////////////////////////////////////////////////
// Convert detector elements to detector element links
///////////////////////////////////////////////////////////////////

void InDet::SiCombinatorialTrackFinder_xk::detectorElementLinks
(std::vector<const InDetDD::SiDetectorElement*>        & DE,
 std::vector<const InDet::SiDetElementBoundaryLink_xk*>& DEL,
 const EventContext& ctx) const
{
  const InDet::SiDetElementBoundaryLinks_xk* boundaryPixel{nullptr};
  const InDet::SiDetElementBoundaryLinks_xk* boundarySCT{nullptr};
  if (m_usePIX) {
    SG::ReadCondHandle<InDet::SiDetElementBoundaryLinks_xk> boundaryPixelHandle(m_boundaryPixelKey,ctx);
    boundaryPixel = *boundaryPixelHandle;
    if (boundaryPixel==nullptr) {
      ATH_MSG_FATAL(m_boundaryPixelKey.fullKey() << " returns null pointer");
    }
  }
  if (m_useSCT) {
    SG::ReadCondHandle<InDet::SiDetElementBoundaryLinks_xk> boundarySCTHandle(m_boundarySCTKey,ctx);
    boundarySCT = *boundarySCTHandle;
    if (boundarySCT==nullptr) {
      ATH_MSG_FATAL(m_boundarySCTKey.fullKey() << " returns null pointer");
    }
  }

  DEL.reserve(DE.size());
  for (const InDetDD::SiDetectorElement* d: DE) {
    IdentifierHash id = d->identifyHash();
    if (d->isPixel() && boundaryPixel && id < boundaryPixel->size()) DEL.push_back(&(*boundaryPixel)[id]);
    else if (d->isSCT() && boundarySCT && id < boundarySCT->size()) DEL.push_back(&(*boundarySCT)[id]);
  }
}

///////////////////////////////////////////////////////////////////
// Get track quality cuts
///////////////////////////////////////////////////////////////////

void  InDet::SiCombinatorialTrackFinder_xk::getTrackQualityCuts
(SiCombinatorialTrackFinderData_xk& data, const TrackQualityCuts& Cuts)
{
  // Integer cuts
  //
  int intCut = 0;

  if (!Cuts.getIntCut("CosmicTrack"         , intCut)) intCut = 0;
  data.setCosmicTrack(intCut);

  if (!Cuts.getIntCut("MinNumberOfClusters" , intCut)) intCut = 7;
  data.setNclusmin(intCut);

  data.setNclusminb(std::max(3, data.nclusmin()-1));

  if (!Cuts.getIntCut("MinNumberOfWClusters", intCut)) intCut = 7;
  data.setNwclusmin(intCut);

  if (!Cuts.getIntCut("MaxNumberOfHoles"    , intCut)) intCut = 2;
  if(!data.cosmicTrack() && intCut>2) intCut = 2;
  data.setNholesmax(intCut);

  if (!Cuts.getIntCut("MaxHolesGap"         , intCut)) intCut = 2;
  if(!data.cosmicTrack() && intCut>2) intCut = 2;
  if(intCut > data.nholesmax())       intCut = data.nholesmax();
  data.setDholesmax(intCut);

  data.tools().setHolesClusters(data.nholesmax(), data.dholesmax(),
				data.nclusmin());

  if (!Cuts.getIntCut("UseAssociationTool"  , intCut)) intCut = 0;
  data.tools().setAssociation(intCut);

  if (!Cuts.getIntCut("SimpleTrack"         , intCut)) intCut = 0;
  data.setSimpleTrack(bool(intCut));

  // Double cuts
  //
  double doubleCut = 0.;

  if (!Cuts.getDoubleCut("pTmin"             , doubleCut)) doubleCut = 500.;
  data.setPTmin(doubleCut);

  if (!Cuts.getDoubleCut("pTminBrem"         , doubleCut)) doubleCut = 1000.;
  data.setPTminBrem(doubleCut);

  if (!Cuts.getDoubleCut("MaxXi2forCluster"  , doubleCut)) doubleCut = 9.;
  data.setXi2max(doubleCut);

  if (!Cuts.getDoubleCut("MaxXi2forOutlier"  , doubleCut)) doubleCut = 25.;
  if (!data.cosmicTrack() && doubleCut > 25.) doubleCut = 25.;
  if (doubleCut <= data.xi2max()) doubleCut = data.xi2max()+5.;
  data.setXi2maxNoAdd(doubleCut);

  if (!Cuts.getDoubleCut("MaxXi2forSearch"    , doubleCut)) doubleCut = 100.;
  data.setXi2maxlink(doubleCut);

  data.tools().setXi2pTmin(data.xi2max(), data.xi2maxNoAdd(),
			   data.xi2maxlink(), data.pTmin());

  if (!Cuts.getIntCut   ("doMultiTracksProd", intCut)) intCut = 0;
  if (!Cuts.getDoubleCut("MaxXi2MultiTracks", doubleCut)) doubleCut = 7.;
  if (!data.cosmicTrack() && doubleCut > 7.) doubleCut = 7.;
  data.tools().setMultiTracks(intCut, doubleCut);

  data.trajectory().setParameters();
}

void InDet::SiCombinatorialTrackFinder_xk::initializeCombinatorialData(const EventContext& ctx, SiCombinatorialTrackFinderData_xk& data) const {

  /// Add conditions object to SiCombinatorialTrackFinderData to be able to access the field cache for each new event
  /// Get conditions object for field cache
  SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCondObjInputKey, ctx};
  const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};
  if (fieldCondObj == nullptr) {
      std::string msg = "InDet::SiCombinatorialTrackFinder_xk::initializeCombinatorialData: Failed to retrieve AtlasFieldCacheCondObj with key " + m_fieldCondObjInputKey.key();
      throw(std::runtime_error(msg));
  }
  data.setFieldCondObj(fieldCondObj);

  /// Must have set fieldCondObj BEFORE calling setTools because fieldCondObj is used there
  data.setTools(&*m_proptool,
                &*m_updatortool,
                &*m_riocreator,
                (m_usePIX && (m_pixelDetElStatus.empty() || VALIDATE_STATUS_ARRAY_ACTIVATED)? &*m_pixelCondSummaryTool : nullptr),
                (m_useSCT && (m_sctDetElStatus.empty()   || VALIDATE_STATUS_ARRAY_ACTIVATED)? &*m_sctCondSummaryTool   : nullptr),
                &m_fieldprop,
                &*m_boundaryCheckTool);
  if (!m_pixelDetElStatus.empty()) {
     SG::ReadHandle<InDet::SiDetectorElementStatus> pixelDetElStatus( m_pixelDetElStatus, ctx);
     data.setPixelDetectorElementStatus( pixelDetElStatus.cptr() );
  }
  if (!m_sctDetElStatus.empty()) {
     SG::ReadHandle<InDet::SiDetectorElementStatus> sctDetElStatus( m_sctDetElStatus, ctx);
     data.setSCTDetectorElementStatus( sctDetElStatus.cptr() );
  }

  // Set the ITk Geometry setup
  data.setITkGeometry(m_ITkGeometry);
  // Set the ITk Fast Tracking setup
  data.setFastTracking(m_doFastTracking);
}

void InDet::SiCombinatorialTrackFinder_xk::fillStatistic(SiCombinatorialTrackFinderData_xk& data, std::array<bool,6>& information) const
{
   for(int i=0; i!=NumberOfStats; ++i) information[i] = data.statistic()[i];
}
