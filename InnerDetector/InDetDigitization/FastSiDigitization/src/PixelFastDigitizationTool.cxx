/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////////////
// PixelFastDigitizationTool.cxx
//   Implementation file for class PixelFastDigitizationTool
////////////////////////////////////////////////////////////////////////////

// Pixel digitization includes
#include "FastSiDigitization/PixelFastDigitizationTool.h"

// Det Descr
#include "Identifier/Identifier.h"

#include "ReadoutGeometryBase/SiReadoutCellId.h"
#include "InDetReadoutGeometry/SiDetectorDesign.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetSimData/InDetSimDataCollection.h"

// Random numbers
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandLandau.h"

// DataHandle
#include "StoreGate/DataHandle.h"

// Pile-up

#include "PixelReadoutGeometry/PixelModuleDesign.h"

// Fatras
#include "InDetPrepRawData/PixelCluster.h"
#include "TrkExUtils/LineIntersection2D.h"
#include "InDetPrepRawData/SiWidth.h"
#include "SiClusterizationTool/IPixelClusteringTool.h"
#include "SiClusterizationTool/PixelGangedAmbiguitiesFinder.h"
#include "InDetPrepRawData/PixelGangedClusterAmbiguities.h"
#include "InDetPrepRawData/SiClusterContainer.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"

#include "GeneratorObjects/HepMcParticleLink.h"
#include "AtlasHepMC/GenParticle.h"

//Package For New Tracking:
// Amg includes
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
// Trk includes

#include "TrkDigEvent/RectangularSegmentation.h"
#include "TrkDigEvent/TrapezoidSegmentation.h"

#include "TrkDigEvent/DigitizationCell.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"
#include "TrkSurfaces/SurfaceCollection.h"


using namespace InDetDD;
using namespace InDet;

// Constructor with parameters:
PixelFastDigitizationTool::PixelFastDigitizationTool(const std::string &type, const std::string &name,
                                                     const IInterface* parent):

  PileUpToolBase(type, name, parent)
{
}

PixelFastDigitizationTool::~PixelFastDigitizationTool() {
  if(m_pixelClusterMap) {
    delete m_pixelClusterMap;
  }
}

// Initialize method:
StatusCode PixelFastDigitizationTool::initialize()
{

  ATH_MSG_DEBUG ( "PixelDigitizationTool::initialize()" );

  ATH_CHECK(m_pixelReadout.retrieve());
  ATH_CHECK(m_chargeDataKey.initialize());
  ATH_CHECK(m_offlineCalibDataKey.initialize());
  ATH_CHECK(m_pixelDetEleCollKey.initialize());

  //locate the AtRndmGenSvc and initialize our local ptr
  if (!m_rndmSvc.retrieve().isSuccess())
    {
      ATH_MSG_ERROR ( "Could not find given RndmSvc" );
      return StatusCode::FAILURE;
    }

  if (detStore()->retrieve(m_pixel_ID, "PixelID").isFailure()) {
    ATH_MSG_ERROR ( "Could not get Pixel ID helper" );
    return StatusCode::FAILURE;
  }

  if (m_inputObjectName.empty())
    {
      ATH_MSG_FATAL ( "Property InputObjectName not set !" );
      return StatusCode::FAILURE;
    }
  else
    {
      ATH_MSG_DEBUG ( "Input objects: '" << m_inputObjectName << "'" );
    }

  // retrieve the offline cluster maker : for pixel and/or sct
  if ( m_pixUseClusterMaker) {
    if (m_clusterMaker.retrieve().isFailure()){
      ATH_MSG_WARNING( "Could not retrieve " << m_clusterMaker );
      ATH_MSG_WARNING( "-> Switching to simplified cluster creation!" );
      m_pixUseClusterMaker = false;
      m_clusterMaker.disable();
    }
  } else {
    m_clusterMaker.disable();
  }

  if (m_pixModuleDistortion) {
    ATH_CHECK(m_distortionKey.initialize());
  } 

  //locate the PileUpMergeSvc and initialize our local ptr
  if (!m_mergeSvc.retrieve().isSuccess()) {
    ATH_MSG_ERROR ( "Could not find PileUpMergeSvc" );
    return StatusCode::FAILURE;
  }


  // get the InDet::PixelGangedAmbiguitiesFinder
  if ( m_gangedAmbiguitiesFinder.retrieve().isFailure() ) {
    ATH_MSG_FATAL( m_gangedAmbiguitiesFinder.propertyName() << ": Failed to retrieve tool " << m_gangedAmbiguitiesFinder.type() );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG ( m_gangedAmbiguitiesFinder.propertyName() << ": Retrieved tool " << m_gangedAmbiguitiesFinder.type() );
  }

  ATH_CHECK(m_lorentzAngleTool.retrieve());

  return StatusCode::SUCCESS;
}



StatusCode PixelFastDigitizationTool::prepareEvent(const EventContext& /*ctx*/, unsigned int)
{

  m_siHitCollList.clear();
  m_thpcsi = new TimedHitCollection<SiHit>();
  m_HardScatterSplittingSkipper = false;
  return StatusCode::SUCCESS;
}


StatusCode PixelFastDigitizationTool::processBunchXing(int bunchXing,
                                                       SubEventIterator bSubEvents,
                                                       SubEventIterator eSubEvents)
{
  //decide if this event will be processed depending on HardScatterSplittingMode & bunchXing
  if (m_HardScatterSplittingMode == 2 && !m_HardScatterSplittingSkipper ) { m_HardScatterSplittingSkipper = true; return StatusCode::SUCCESS; }
  if (m_HardScatterSplittingMode == 1 && m_HardScatterSplittingSkipper )  { return StatusCode::SUCCESS; }
  if (m_HardScatterSplittingMode == 1 && !m_HardScatterSplittingSkipper ) { m_HardScatterSplittingSkipper = true; }

  using TimedHitCollList = PileUpMergeSvc::TimedList<SiHitCollection>::type;
  TimedHitCollList hitCollList;

  if (!(m_mergeSvc->retrieveSubSetEvtData(m_inputObjectName, hitCollList, bunchXing,
                                          bSubEvents, eSubEvents).isSuccess()) &&
      hitCollList.empty()) {
    ATH_MSG_ERROR("Could not fill TimedHitCollList");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_VERBOSE(hitCollList.size() << " SiHitCollections with key " <<
                    m_inputObjectName << " found");
  }

  TimedHitCollList::iterator iColl(hitCollList.begin());
  TimedHitCollList::iterator endColl(hitCollList.end());

  for( ; iColl != endColl; ++iColl) {
    SiHitCollection *siHitColl = new SiHitCollection(*iColl->second);
    PileUpTimeEventIndex timeIndex(iColl->first);
    ATH_MSG_DEBUG("SiHitCollection found with " << siHitColl->size() <<
                  " hits");
    ATH_MSG_VERBOSE("time index info. time: " << timeIndex.time()
                    << " index: " << timeIndex.index()
                    << " type: " << timeIndex.type());
    m_thpcsi->insert(timeIndex, siHitColl);
    m_siHitCollList.push_back(siHitColl);
  }

  return StatusCode::SUCCESS;
}


StatusCode PixelFastDigitizationTool::processAllSubEvents(const EventContext& ctx) {

  m_pixelClusterContainer = new InDet::PixelClusterContainer(m_pixel_ID->wafer_hash_max());

  if(!m_pixelClusterContainer) {
    ATH_MSG_FATAL( "[ --- ] Could not create PixelClusterContainer");
    return StatusCode::FAILURE;
  }

  InDet::SiClusterContainer* symSiContainer=nullptr;

  // --------------------------------------
  // Pixel Cluster container registration
  m_pixelClusterContainer->cleanup();
  if ((evtStore()->record(m_pixelClusterContainer, m_pixel_SiClustersName)).isFailure())   {
    ATH_MSG_FATAL("[ hitproc ] Error while registering PixelCluster container");
    return StatusCode::FAILURE;
  }

  // symlink the Pixel Cluster Container
  if ((evtStore()->symLink(m_pixelClusterContainer,symSiContainer)).isFailure()) {
    ATH_MSG_FATAL( "[ --- ] PixelClusterContainer could not be symlinked to SiClusterContainter in StoreGate !" );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG( "[ hitproc ] PixelClusterContainer symlinked to SiClusterContainer in StoreGate" );
  }

  // truth info

  m_pixPrdTruth = new PRD_MultiTruthCollection;

  if ((evtStore()->contains<PRD_MultiTruthCollection>(m_prdTruthNamePixel))){
    if((evtStore()->retrieve(m_pixPrdTruth, m_prdTruthNamePixel)).isFailure()){
      ATH_MSG_FATAL("Could not retrieve collection " << m_prdTruthNamePixel);
      return StatusCode::FAILURE;
    }
  }else{
    if((evtStore()->record(m_pixPrdTruth, m_prdTruthNamePixel)).isFailure()){
      ATH_MSG_FATAL("Could not record collection " << m_prdTruthNamePixel);
      return StatusCode::FAILURE;
    }
  }


  m_ambiguitiesMap =new PixelGangedClusterAmbiguities();


  //  get the container(s)
  using TimedHitCollList = PileUpMergeSvc::TimedList<SiHitCollection>::type;

  //this is a list<pair<time_t, DataLink<SCTUncompressedHitCollection> > >
  TimedHitCollList hitCollList;
  unsigned int numberOfSimHits(0);
  if ( !(m_mergeSvc->retrieveSubEvtsData(m_inputObjectName, hitCollList, numberOfSimHits).isSuccess()) && hitCollList.empty() ) {
    ATH_MSG_ERROR ( "Could not fill TimedHitCollList" );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG ( hitCollList.size() << " SiHitCollections with key " << m_inputObjectName << " found" );
  }

  // Define Hit Collection
  TimedHitCollection<SiHit> thpcsi(numberOfSimHits);

  //now merge all collections into one
  TimedHitCollList::iterator   iColl(hitCollList.begin());
  TimedHitCollList::iterator endColl(hitCollList.end()  );

  m_HardScatterSplittingSkipper = false;
  // loop on the hit collections
  while ( iColl != endColl ) {
    //decide if this event will be processed depending on HardScatterSplittingMode & bunchXing
    if (m_HardScatterSplittingMode == 2 && !m_HardScatterSplittingSkipper ) { m_HardScatterSplittingSkipper = true; ++iColl; continue; }
    if (m_HardScatterSplittingMode == 1 && m_HardScatterSplittingSkipper )  { ++iColl; continue; }
    if (m_HardScatterSplittingMode == 1 && !m_HardScatterSplittingSkipper ) { m_HardScatterSplittingSkipper = true; }
    const SiHitCollection* p_collection(iColl->second);
    thpcsi.insert(iColl->first, p_collection);
    ATH_MSG_DEBUG ( "SiHitCollection found with " << p_collection->size() << " hits" );
    ++iColl;
  }

  // Process the Hits straw by straw: get the iterator pairs for given straw
  if(this->digitize(ctx, thpcsi).isFailure()) {
    ATH_MSG_FATAL ( "digitize method failed!" );
    return StatusCode::FAILURE;
  }

  if (createAndStoreRIOs(ctx).isFailure()) {
    ATH_MSG_FATAL ( "createAndStoreRIOs() failed!" );
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG ( "createAndStoreRIOs() succeeded" );
  }


  if ((evtStore()->setConst(m_pixelClusterContainer)).isFailure()) {
    ATH_MSG_ERROR("[ ---- ] Could not set Pixel ROT container ");
  }
  ATH_MSG_DEBUG ("Ambiguities map has " << m_ambiguitiesMap->size() << " elements" );
  StatusCode sc = evtStore()->record(m_ambiguitiesMap,m_pixelClusterAmbiguitiesMapName,false);
  if (sc.isFailure()){
    ATH_MSG_FATAL ( "PixelClusterAmbiguitiesMap could not be recorded in StoreGate !" );
    return StatusCode::FAILURE;
  }else{
    ATH_MSG_DEBUG ( "PixelClusterAmbiguitiesMap recorded in StoreGate" );
  }




  return StatusCode::SUCCESS;

}



StatusCode PixelFastDigitizationTool::mergeEvent(const EventContext& ctx)
{

  m_pixelClusterContainer = new InDet::PixelClusterContainer(m_pixel_ID->wafer_hash_max());

  if(!m_pixelClusterContainer) {
    ATH_MSG_FATAL( "[ --- ] Could not create PixelClusterContainer");
    return StatusCode::FAILURE;
  }

  InDet::SiClusterContainer* symSiContainer=nullptr;

  // --------------------------------------
  // Pixel_Cluster container registration
  m_pixelClusterContainer->cleanup();
  if ((evtStore()->record(m_pixelClusterContainer, "PixelClusters")).isFailure())   {
    ATH_MSG_FATAL("[ hitproc ] Error while registering PixelCluster container");
    return StatusCode::FAILURE;
  }

  // symlink the SCT Container
  if ((evtStore()->symLink(m_pixelClusterContainer,symSiContainer)).isFailure()) {
    ATH_MSG_FATAL( "[ --- ] PixelClusterContainer could not be symlinked to SiClusterContainter in StoreGate !" );
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_DEBUG( "[ hitproc ] PixelClusterContainer symlinked to SiClusterContainer in StoreGate" );
  }

  // truth info
  m_pixPrdTruth = new PRD_MultiTruthCollection;

  if ((evtStore()->contains<PRD_MultiTruthCollection>(m_prdTruthNamePixel))){
    if((evtStore()->retrieve(m_pixPrdTruth, m_prdTruthNamePixel)).isFailure()){
      ATH_MSG_FATAL("Could not retrieve collection " << m_prdTruthNamePixel);
      return StatusCode::FAILURE;
    }
  }else{
    if((evtStore()->record(m_pixPrdTruth, m_prdTruthNamePixel)).isFailure()){
      ATH_MSG_FATAL("Could not record collection " << m_prdTruthNamePixel);
      return StatusCode::FAILURE;
    }
  }

  m_ambiguitiesMap =new PixelGangedClusterAmbiguities();

  if (m_thpcsi != nullptr) {
    if(digitize(ctx, *m_thpcsi).isFailure()) {
      ATH_MSG_FATAL ( "Pixel digitize method failed!" );
      return StatusCode::FAILURE;
    }
  }

  delete m_thpcsi;
  for(SiHitCollection* ptr : m_siHitCollList) delete ptr;
  m_siHitCollList.clear();


  if (createAndStoreRIOs(ctx).isFailure()) {
    ATH_MSG_FATAL ( "createAndStoreRIOs() failed!" );
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG ( "createAndStoreRIOs() succeeded" );
  }

  if ((evtStore()->setConst(m_pixelClusterContainer)).isFailure()) {
    ATH_MSG_ERROR("[ ---- ] Could not set Pixel ROT container ");
  }
  ATH_MSG_DEBUG ("Ambiguities map has " << m_ambiguitiesMap->size() << " elements" );
  StatusCode sc = evtStore()->record(m_ambiguitiesMap,m_pixelClusterAmbiguitiesMapName,false);
  if (sc.isFailure()){
    ATH_MSG_FATAL ( "PixelClusterAmbiguitiesMap could not be recorded in StoreGate !" );
    return StatusCode::FAILURE;
  }else{
    ATH_MSG_DEBUG ( "PixelClusterAmbiguitiesMap recorded in StoreGate" );
  }



  return StatusCode::SUCCESS;
}


StatusCode PixelFastDigitizationTool::digitize(const EventContext& ctx,
                                               TimedHitCollection<SiHit>& thpcsi)
{
  // Set the RNG to use for this event.
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_randomEngineName);
  const std::string rngName = name()+m_randomEngineName;
  rngWrapper->setSeed( rngName, ctx );
  CLHEP::HepRandomEngine *rndmEngine = rngWrapper->getEngine(ctx);

  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(m_pixelDetEleCollKey, ctx);
  const InDetDD::SiDetectorElementCollection* elements(*pixelDetEleHandle);
  if (not pixelDetEleHandle.isValid() or elements==nullptr) {
    ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
    return StatusCode::FAILURE;
  }

  TimedHitCollection<SiHit>::const_iterator i, e;

  if(!m_pixelClusterMap) { m_pixelClusterMap = new Pixel_detElement_RIO_map; }
  else { m_pixelClusterMap->clear(); }

  SG::ReadCondHandle<PixelChargeCalibCondData> calibDataHandle(m_chargeDataKey, ctx);
  const PixelChargeCalibCondData *calibData = *calibDataHandle;
  SG::ReadCondHandle<PixelCalib::PixelOfflineCalibData> offlineCalibData(m_offlineCalibDataKey, ctx);
  std::vector<int> trkNo;
  std::vector<Identifier> detEl;

  while (thpcsi.nextDetectorElement(i, e)) {

    Pixel_detElement_RIO_map PixelDetElClusterMap;

    trkNo.clear();
    detEl.clear();

    while (i != e) {

      const TimedHitPtr<SiHit>& hit(*i++);


      const int barrelEC = hit->getBarrelEndcap();
      const int layerDisk = hit->getLayerDisk();
      const int phiModule = hit->getPhiModule();
      const int etaModule = hit->getEtaModule();

      const Identifier moduleID = m_pixel_ID->wafer_id(barrelEC, layerDisk, phiModule, etaModule);
      const IdentifierHash waferHash = m_pixel_ID->wafer_hash(moduleID);
      const InDetDD::SiDetectorElement* hitSiDetElement = elements->getDetectorElement(waferHash);
      if (!hitSiDetElement) {ATH_MSG_ERROR( " could not get detector element "); continue;}

      if (!(hitSiDetElement->isPixel())) {continue;}



      std::vector<HepMcParticleLink> hit_vector; //Store the hits in merged cluster

      const int trkn = hit->trackNumber();

      const Identifier hitId = hitSiDetElement->identify(); // Isn't this is identical to moduleID?
      //const IdentifierHash hitIdHash = hitSiDetElement->identifyHash();


      bool isRep = false;

      for (int j : trkNo) {
        for (auto & k : detEl) {
          if ((trkn > 0) && (trkn == j) && (hitId == k)) {isRep = true; break;}
        }
        if (isRep) break;
      }

      if (isRep) continue;

      trkNo.push_back(trkn);
      detEl.push_back(hitId);

      HepGeom::Point3D<double> localStartPosition = hit->localStartPosition();
      HepGeom::Point3D<double> localEndPosition = hit->localEndPosition();

      localStartPosition = hitSiDetElement->hitLocalToLocal3D(localStartPosition);
      localEndPosition = hitSiDetElement->hitLocalToLocal3D(localEndPosition);

      int isEndcap = (barrelEC==0) ? 0:1;

      double shiftX = isEndcap ? m_pixDiffShiftEndCX : m_pixDiffShiftBarrX;
      double shiftY = isEndcap ? m_pixDiffShiftEndCY : m_pixDiffShiftBarrY;

      //       std::cout<<"Thr "<<m_ThrConverted<<std::endl;

      //New function to tune the cluster size
      Diffuse(localStartPosition, localEndPosition, shiftX, shiftY);

      const Amg::Vector3D localDirection(localEndPosition.x()-localStartPosition.x(), localEndPosition.y()-localStartPosition.y(), localEndPosition.z()-localStartPosition.z());

      Amg::Vector3D entryPoint(localStartPosition.x(),localStartPosition.y(),localStartPosition.z());
      Amg::Vector3D exitPoint(localEndPosition.x(),localEndPosition.y(),localEndPosition.z());

      const InDetDD::PixelModuleDesign* design(dynamic_cast<const InDetDD::PixelModuleDesign*>(&hitSiDetElement->design()));
      if (not design){
        ATH_MSG_FATAL("Failed to cast the hitSiDetElement::design pointer to a PixelModuleDesign*; aborting due to null pointer");
        return StatusCode::FAILURE;
      }
      const Amg::Vector2D localEntry(localStartPosition.x(),localStartPosition.y());
      const Amg::Vector2D localExit(localEndPosition.x(),localEndPosition.y());

      Identifier entryId = hitSiDetElement->identifierOfPosition(localEntry);
      Identifier exitId  = hitSiDetElement->identifierOfPosition(localExit);

      InDetDD::SiCellId entryCellId = hitSiDetElement->cellIdFromIdentifier(entryId);
      InDetDD::SiCellId exitCellId = hitSiDetElement->cellIdFromIdentifier(exitId);

      double halfthickness = hitSiDetElement->thickness()*0.5;

      bool EntryValid(entryCellId.isValid());
      bool ExitValid(exitCellId.isValid());

      double pixMinimalPathCut= 1. / m_pixPathLengthTotConv;

      Identifier diodeID = hitId;
      unsigned int FE = m_pixelReadout->getFE(diodeID, moduleID);
      InDetDD::PixelDiodeType type = m_pixelReadout->getDiodeType(diodeID);

      double th0 = calibData->getAnalogThreshold(type, waferHash, FE) / m_ThrConverted;

      //        if (old_th != th0) std::cout<<"converted threshold "<<th0<<std::endl, old_th= th0;

      //Avoid to store pixels with 0 ToT
      //if (m_pixMinimalPathCut > pixMinimalPathCut) pixMinimalPathCut=m_pixMinimalPathCut;
      //if (th0 > pixMinimalPathCut) ;
      pixMinimalPathCut=th0;

      if (!EntryValid || !ExitValid)
        {
          //If entry or exit aren't valid search for the valid entry/exit of the hit as Intersection with the module

          if ( !EntryValid && !ExitValid) continue;

          Amg::Vector3D Point = EntryValid ? entryPoint : exitPoint ;

          Amg::Vector3D Intersection =  CalculateIntersection(Point, localDirection, Amg::Vector2D(design->width() * 0.5,design->length() * 0.5),halfthickness);

          if( Intersection == Amg::Vector3D(0.,0.,0.))
            {
              ATH_MSG_WARNING("ATTENTION THE INTERSECTION COORDINATE IS OUT OF THE MODULE");
              continue;
            }

          const Amg::Vector2D Intersection_2d(Intersection.x(),Intersection.y());
          Identifier Intersection_2dId = hitSiDetElement->identifierOfPosition(Intersection_2d);

          InDetDD::SiCellId Intersection_2dCellId = hitSiDetElement->cellIdFromIdentifier(Intersection_2dId);


          if(!Intersection_2dCellId.isValid()) continue;

          if(EntryValid)
            exitPoint = Intersection;
          else
            entryPoint = Intersection;


        }



      Trk::DigitizationModule * digitizationModule = buildDetectorModule(hitSiDetElement);
      if(!digitizationModule){
        ATH_MSG_FATAL( " could not get build detector module ");
        return StatusCode::FAILURE;
      }

      // Getting the steps in the sensor
      std::vector<Trk::DigitizationStep> digitizationSteps = m_digitizationStepper->cellSteps(*digitizationModule,entryPoint,exitPoint);



      // the pixel positions and other needed stuff for the geometrical clustering
      InDet::PixelCluster* pixelCluster = nullptr;
      Amg::Vector2D       clusterPosition(0.,0.);

      std::vector<Identifier>           rdoList;
      std::vector<int>                  totList;

      const bool   isGanged = false;
      int lvl1a = 0;

      double accumulatedPathLength=0.;

      //ATTENTION index max e min da rdo + manager
      int phiIndexMax = -999999;
      int phiIndexMin = 1000000;
      int etaIndexMax = -999999;
      int etaIndexMin = 1000000;


      for (auto& dStep : digitizationSteps){

        double pathlength = dStep.stepLength;
        // two options fro charge smearing: landau / gauss
        if ( m_pixSmearPathLength > 0. ) {
          // create the smdar parameter
          double sPar = m_pixSmearLandau ?
            m_pixSmearPathLength*CLHEP::RandLandau::shoot(rndmEngine) :
            m_pixSmearPathLength*CLHEP::RandGaussZiggurat::shoot(rndmEngine);
          pathlength *=  (1.+sPar);
        }


        if (pathlength < pixMinimalPathCut) continue;

        // position on the diode map
        Trk::DigitizationCell cell(dStep.stepCell.first,dStep.stepCell.second);
        Amg::Vector2D PositionOnModule = digitizationModule->segmentation().cellPosition(cell);
        InDetDD::SiCellId diode = hitSiDetElement->cellIdOfPosition(PositionOnModule);


        if (!diode.isValid())
          continue;

        Amg::Vector2D chargeCenterPosition = hitSiDetElement->rawLocalPositionOfCell(diode);

        const Identifier rdoId            =  hitSiDetElement->identifierOfPosition(chargeCenterPosition);
        clusterPosition += pathlength * chargeCenterPosition;

        int currentEtaIndex = diode.etaIndex();
        int currentPhiIndex = diode.phiIndex();
        if(currentEtaIndex > etaIndexMax) etaIndexMax = currentEtaIndex;
        if(currentEtaIndex < etaIndexMin) etaIndexMin = currentEtaIndex;
        if(currentPhiIndex > phiIndexMax) phiIndexMax = currentPhiIndex;
        if(currentPhiIndex < phiIndexMin) phiIndexMin = currentPhiIndex;


        // record - positions, rdoList and totList
        accumulatedPathLength += pathlength;
        //Fail
        rdoList.push_back(rdoId);
        totList.push_back(int(pathlength*m_pixPathLengthTotConv));

      }

      delete digitizationModule;

      // the col/row
      int siDeltaPhiCut = phiIndexMax-phiIndexMin+1;
      int siDeltaEtaCut = etaIndexMax-etaIndexMin+1;

      int totalToT=std::accumulate(totList.begin(), totList.end(), 0);;

      // bail out if 0 pixel or path length problem
      if (rdoList.empty() || accumulatedPathLength < pixMinimalPathCut || totalToT == 0) {
        if (totalToT == 0 && !rdoList.empty() ) ATH_MSG_WARNING("The total ToT of the cluster is 0, this should never happen");
        continue;
      }

      // weight the cluster position
      clusterPosition *= 1./accumulatedPathLength;
      Identifier clusterId = hitSiDetElement->identifierOfPosition(clusterPosition);


      // merging clusters

      bool merged = false;
      if(m_mergeCluster){ // merge to the current cluster "near" cluster in the cluster map, in the current detector element

        for(Pixel_detElement_RIO_map::iterator currentClusIter = PixelDetElClusterMap.begin(); currentClusIter != PixelDetElClusterMap.end();) {
          //make a temporary to use within the loop and possibly erase - increment the main interator at the same time.
          Pixel_detElement_RIO_map::iterator clusIter = currentClusIter++;
          InDet::PixelCluster* currentCluster = clusIter->second;
          const std::vector<Identifier> &currentRdoList = currentCluster->rdoList();
          bool areNb = false;
          for (auto rdoIter : rdoList) {
            areNb = PixelFastDigitizationTool::areNeighbours(currentRdoList, rdoIter, hitSiDetElement,*m_pixel_ID);
            if (areNb) { break; }
          }
          if (areNb) {
            const std::vector<int> &currentTotList = currentCluster->totList();
            rdoList.insert(rdoList.end(), currentRdoList.begin(), currentRdoList.end() );
            totList.insert(totList.end(), currentTotList.begin(), currentTotList.end() );
            Amg::Vector2D       currentClusterPosition(currentCluster->localPosition());
            float c1 = (float)currentRdoList.size();
            float c2 = (float)rdoList.size();
            clusterPosition = (clusterPosition*c2 + currentClusterPosition*c1)/((c1 + c2));
            clusterId = hitSiDetElement->identifierOfPosition(clusterPosition);
            merged = true;
            PixelDetElClusterMap.erase(clusIter);

            //Store HepMcParticleLink connected to the cluster removed from the collection
            std::pair<PRD_MultiTruthCollection::iterator,PRD_MultiTruthCollection::iterator> saved_hit = m_pixPrdTruth->equal_range(currentCluster->identify());
            for (PRD_MultiTruthCollection::iterator this_hit = saved_hit.first; this_hit != saved_hit.second; ++this_hit)
              {
                hit_vector.push_back(this_hit->second);
              }
            //Delete all the occurency of the currentCluster from the multi map
            if (saved_hit.first != saved_hit.second) m_pixPrdTruth->erase(currentCluster->identify());
            delete currentCluster;
            //break; //commenting out this break statement allows for multiple existing clusters to be merged.
          }
        }
      }

      bool not_valid = false;
      for (auto & entry : rdoList) {
        if (!(entry.is_valid())) { not_valid = true; break;}
      }

      if (not_valid) continue;

      if(merged) {
        //Hacks for merged clusters
        for (auto rdoIter : rdoList) {
          const InDetDD::SiCellId& chargeCellId =  hitSiDetElement->cellIdFromIdentifier(rdoIter);
          // phi/ eta index
          int chargePhiIndex = chargeCellId.phiIndex();
          int chargeEtaIndex = chargeCellId.etaIndex();
          // set max min
          phiIndexMin = chargePhiIndex < phiIndexMin ?  chargePhiIndex : phiIndexMin;
          phiIndexMax = chargePhiIndex > phiIndexMax ?  chargePhiIndex : phiIndexMax;
          etaIndexMin = chargeEtaIndex < etaIndexMin ?  chargeEtaIndex : etaIndexMin;
          etaIndexMax = chargeEtaIndex > etaIndexMax ?  chargeEtaIndex : etaIndexMax;
        }
        siDeltaPhiCut = (phiIndexMax-phiIndexMin)+1;
        siDeltaEtaCut = (etaIndexMax-etaIndexMin)+1;
      }

      // ---------------------------------------------------------------------------------------------
      //  PART 2: Cluster && ROT creation


      //ATTENTION
      //       // correct shift implied by the scaling of the Lorentz angle
      //       double newshift = 0.5*thickness*tanLorAng;
      //       double corr = ( shift - newshift );
      // 2a) Cluster creation ------------------------------------
      if (m_pixUseClusterMaker){


        //ATTENTION this can be enabled, take a look to localDirection
        //         if (m_pixModuleDistortion &&  hitSiDetElement->isBarrel() )
        //           clusterPosition = SG::ReadCondHandle<PixelDistortionData>(m_distortionKey)->correctSimulation(m_pixel_ID->wafer_hash(hitSiDetElement->identify()), clusterPosition, localDirection);

        // from InDetReadoutGeometry: width from eta
        const auto *pixModDesign = dynamic_cast<const InDetDD::PixelModuleDesign*>(&hitSiDetElement->design());
        if (!pixModDesign) {
          return StatusCode::FAILURE;
        }
        double etaWidth = pixModDesign->widthFromColumnRange(etaIndexMin, etaIndexMax);
        // from InDetReadoutGeometry : width from phi
        double phiWidth = pixModDesign->widthFromRowRange(phiIndexMin, phiIndexMax);

        InDet::SiWidth siWidth(Amg::Vector2D(siDeltaPhiCut,siDeltaEtaCut),
                               Amg::Vector2D(phiWidth,etaWidth));

        // use the cluster maker from the offline software
        pixelCluster = m_clusterMaker->pixelCluster(clusterId,
                                                    clusterPosition,
                                                    rdoList,
                                                    lvl1a,
                                                    totList,
                                                    siWidth,
                                                    hitSiDetElement,
                                                    isGanged,
                                                    m_pixErrorStrategy,
                                                    *m_pixel_ID,
                                                    false,
                                                    0.0,
                                                    0.0,
                                                    calibData,
                                                    *offlineCalibData);
        if (isGanged)  pixelCluster->setGangedPixel(isGanged);
      } else {
        ATH_MSG_WARNING("[ cluster - pix ] No pixels errors provided, but configured to use them.");
        ATH_MSG_WARNING("                  -> No pixels cluster will be created.");
        continue;
      }

      if(!(pixelCluster->identify().is_valid()))
        {
          delete pixelCluster;
          continue;
        }

      if (! (m_pixel_ID->is_pixel(pixelCluster->identify()))) {delete pixelCluster; continue;}

      (void) PixelDetElClusterMap.insert(Pixel_detElement_RIO_map::value_type(waferHash, pixelCluster));

      if (hit->particleLink().isValid()){
        const int barcode( hit->particleLink().barcode());
        if ( barcode !=0 && barcode != m_vetoThisBarcode ) {
          m_pixPrdTruth->insert(std::make_pair(pixelCluster->identify(), hit->particleLink()));
          ATH_MSG_DEBUG("Truth map filled with cluster" << pixelCluster << " and link = " << hit->particleLink());
        }
      }else{
        ATH_MSG_DEBUG("Particle link NOT valid!! Truth map NOT filled with cluster" << pixelCluster << " and link = " << hit->particleLink());
      }

      //Add all hit that was connected to the cluster
      for(const HepMcParticleLink& p: hit_vector){

        m_pixPrdTruth->insert(std::make_pair(pixelCluster->identify(), p ));
      }


      hit_vector.clear();
    } // end hit while


    (void) m_pixelClusterMap->insert(PixelDetElClusterMap.begin(), PixelDetElClusterMap.end());


  } // end nextDetectorElement while


  return StatusCode::SUCCESS;
}

StatusCode PixelFastDigitizationTool::createAndStoreRIOs(const EventContext& ctx)
{
  SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(m_pixelDetEleCollKey, ctx);
  const InDetDD::SiDetectorElementCollection* elements(*pixelDetEleHandle);
  if (not pixelDetEleHandle.isValid() or elements==nullptr) {
    ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
    return StatusCode::FAILURE;
  }

  Pixel_detElement_RIO_map::iterator i = m_pixelClusterMap->begin();
  Pixel_detElement_RIO_map::iterator e = m_pixelClusterMap->end();

  InDet::PixelClusterCollection* clusterCollection = nullptr;
  IdentifierHash waferHash;

  for (; i != e; i = m_pixelClusterMap->upper_bound(i->first)){

    std::pair <Pixel_detElement_RIO_map::iterator, Pixel_detElement_RIO_map::iterator> range;
    range = m_pixelClusterMap->equal_range(i->first);

    Pixel_detElement_RIO_map::iterator firstDetElem;
    firstDetElem = range.first;

    waferHash = firstDetElem->first;

    const InDetDD::SiDetectorElement* detElement = elements->getDetectorElement(waferHash);

    clusterCollection = new InDet::PixelClusterCollection(waferHash);
    clusterCollection->setIdentifier(detElement->identify());


    for ( Pixel_detElement_RIO_map::iterator iter = range.first; iter != range.second; ++iter){

      InDet::PixelCluster* pixelCluster = (*iter).second;
      pixelCluster->setHashAndIndex(clusterCollection->identifyHash(),clusterCollection->size());
      clusterCollection->push_back(pixelCluster);

    }


    if (clusterCollection) {
      if (!clusterCollection->empty()) {
        ATH_MSG_DEBUG ( "Filling ambiguities map" );
        m_gangedAmbiguitiesFinder->execute(clusterCollection,*m_ambiguitiesMap);
        ATH_MSG_DEBUG ( "Ambiguities map: " << m_ambiguitiesMap->size() << " elements" );
        if ((m_pixelClusterContainer->addCollection(clusterCollection, waferHash)).isFailure()){
          ATH_MSG_WARNING( "Could not add collection to Identifyable container !" );
        }
      }
      else {delete clusterCollection;}
    }


  } // end for

  m_pixelClusterMap->clear();

  return StatusCode::SUCCESS;
}
// copied from PixelClusteringToolBase
bool PixelFastDigitizationTool::areNeighbours
(const std::vector<Identifier>& group,
 const Identifier& rdoID,
 const InDetDD::SiDetectorElement* /*element*/,
 const PixelID& pixelID) 
{
  // note: in the PixelClusteringToolBase, m_splitClusters is a variable; here
  // splitClusters was explicitly set to zero, acceptDiagonalClusters = 1
  // so much of the original code is redundant and only one path through the code
  // is possible.

  std::vector<Identifier>::const_iterator groupBegin = group.begin();
  std::vector<Identifier>::const_iterator groupEnd = group.end();

  int row2 = pixelID.phi_index(rdoID);
  int col2 = pixelID.eta_index(rdoID);

  int rowmax = row2;
  bool match=false;
  while (groupBegin!=groupEnd)
    {
      Identifier id = *groupBegin;
      int row1 = pixelID.phi_index(id);
      int col1 = pixelID.eta_index(id);
      if(row1 > rowmax) rowmax = row1;
      int deltarow = abs(row2-row1);
      int deltacol = abs(col2-col1);

      // a side in common
      if(deltacol+deltarow < 2) match = true;
      //condition "if (acceptDiagonalClusters !=0") is redundant
      if(deltacol == 1 && deltarow == 1) match = true;

      ++groupBegin;
    }


  return match;
}

Trk::DigitizationModule* PixelFastDigitizationTool::buildDetectorModule(const InDetDD::SiDetectorElement* hitSiDetElement ) const {

  const InDetDD::PixelModuleDesign* design(dynamic_cast<const InDetDD::PixelModuleDesign*>(&hitSiDetElement->design()));

  if (!design) {
    ATH_MSG_DEBUG ( "Could not get design"<< design) ;
    return nullptr;
  }

  //Read from the SiDetectorElement information to build the digitization module
  const double halfThickness = hitSiDetElement->thickness() * 0.5;
  const double halfWidth     = design->width() * 0.5;
  const double halfLength    = design->length() * 0.5;

  int binsX = design->rows();
  int binsY = design->columns();
  double numberOfChip = design->numberOfCircuits();

  InDetDD::SiCellId cell(0,binsY/2);
  float LongPitch  =design->parameters(cell).width().xEta();
  //std::cout<<"numberOfChip "<<numberOfChip<<" LongPitch "<<LongPitch<<std::endl;

  ATH_MSG_VERBOSE("Retrieving infos: halfThickness = " << halfThickness << " --- halfWidth = " << halfWidth << " --- halfLength = " << halfLength );
  ATH_MSG_VERBOSE("Retrieving infos: binsX = " << binsX << " --- binsY = " << binsY << " --- numberOfChip = " << numberOfChip);

  int readoutDirection = design->readoutSide();

  const bool useLorentzAngle{true};
  const IdentifierHash detElHash = hitSiDetElement->identifyHash();
  float lorentzAngle   = useLorentzAngle ? hitSiDetElement->hitDepthDirection()*hitSiDetElement->hitPhiDirection()*std::atan(m_lorentzAngleTool->getTanLorentzAngle(detElHash)) : 0.;

  // added for degugging

  // std::cout << "barrel_ec  = " << m_detID->barrel_ec(hitSiDetElement->identify())
  //           << " --  layer_disk = " << m_detID->layer_disk(hitSiDetElement->identify())
  //           << " --  eta_module = " << m_detID->eta_module(hitSiDetElement->identify())
  //           << " --  lorentzAngle = " << lorentzAngle
  //        << " --  lorentzCorrection = " << shift
  //        << " --  readoutDirection = " << readoutDirection << std::endl;
  // std::cout << "element->hitDepthDirection() = " << hitSiDetElement->hitDepthDirection()
  //        << " --  element->hitPhiDirection() = " << hitSiDetElement->hitPhiDirection() << std::endl;

  // rectangle bounds
  auto rectangleBounds = std::make_shared<const Trk::RectangleBounds>(halfWidth,halfLength);
  ATH_MSG_VERBOSE("Initialized rectangle Bounds");
  // create the segmentation
  std::shared_ptr<const Trk::Segmentation> rectangleSegmentation(new Trk::RectangularSegmentation(std::move(rectangleBounds),(size_t)binsX,LongPitch,(size_t)binsY, numberOfChip));
  // build the module
  ATH_MSG_VERBOSE("Initialized rectangleSegmentation");
  Trk::DigitizationModule * digitizationModule = new Trk::DigitizationModule(std::move(rectangleSegmentation),
                                                                               halfThickness,
                                                                               readoutDirection,
                                                                               lorentzAngle);
  ATH_MSG_VERBOSE("Building Rectangle Segmentation with dimensions (halfX, halfY) = (" << halfWidth << ", " << halfLength << ")");

  // success return
  return digitizationModule;
}


Amg::Vector3D PixelFastDigitizationTool::CalculateIntersection(const Amg::Vector3D & Point, const Amg::Vector3D & Direction, Amg::Vector2D PlaneBorder, double halfthickness) 
{
  Amg::Vector3D Intersection(0.,0.,0.);

  //Using parameter definition of a line in 3d z=z_point + direction_z t
  std::vector<double> parameters;
  parameters.push_back((PlaneBorder.x() - Point.x())/Direction.x());
  parameters.push_back((-PlaneBorder.x() - Point.x())/Direction.x());
  parameters.push_back((PlaneBorder.y() - Point.y())/Direction.y());
  parameters.push_back((-PlaneBorder.y() - Point.y())/Direction.y());

  for(double parameter: parameters)
    {
      double z =  Point.z() + Direction.z() * parameter;
      if( std::abs(z) > halfthickness )
        continue;


      double x = Point.x() + Direction.x() * parameter;
      double y = Point.y() + Direction.y() * parameter;

      if(std::abs(x) > PlaneBorder.x() || std::abs(y) > PlaneBorder.y())
        continue;


      Intersection = Amg::Vector3D(x,y,z);
      break;

    }

  return Intersection;
}

void PixelFastDigitizationTool::Diffuse(HepGeom::Point3D<double>& localEntry, HepGeom::Point3D<double>& localExit, double shiftX, double shiftY) {

  double localEntryX = localEntry.x();
  double localEntryY = localEntry.y();
  double localExitX = localExit.x();
  double localExitY = localExit.y();

  double signX = localExitX>localEntryX ? 1:-1 ;
  double signY = localExitY>localEntryY ? 1:-1 ;

  localEntryX += shiftX*signX*(-1);
  localExitX += shiftX*signX;
  localEntryY += shiftY*signY*(-1);
  localExitY += shiftY*signY;

  localEntry.setX(localEntryX);
  localEntry.setY(localEntryY);
  localExit.setX(localExitX);
  localExit.setY(localExitY);

}
