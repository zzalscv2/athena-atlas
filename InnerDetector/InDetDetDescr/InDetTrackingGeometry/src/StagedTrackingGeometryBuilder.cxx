/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// InDet
#include "InDetTrackingGeometry/StagedTrackingGeometryBuilder.h"
#include "InDetTrackingGeometry/DiscOverlapDescriptor.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
// Trk Geometry stuff
#include "TrkDetDescrUtils/BinnedArray.h"
#include "TrkDetDescrUtils/BinnedArray1D1D.h"
#include "TrkVolumes/VolumeBounds.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkGeometry/TrackingVolume.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/Material.h"
#include "TrkGeometry/Layer.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkSurfaces/DiscBounds.h"
// Athena
#include "CxxUtils/checker_macros.h"
//Gaudi
#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/MsgStream.h"
#include <iterator> //std::advance

// constructor
InDet::StagedTrackingGeometryBuilder::StagedTrackingGeometryBuilder(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p)
{
  // layer builders and their configurations
  declareProperty("LayerBuilders",                    m_layerProviders);
}

// Athena standard methods
// initialize
StatusCode InDet::StagedTrackingGeometryBuilder::initialize()
{

   // Retrieve the layer builders -----------------------------------------------------------
   if (m_layerProviders.retrieve().isFailure())
   {
      ATH_MSG_FATAL( "Failed to retrieve tool(s) " << m_layerProviders );
      return StatusCode::FAILURE;
   } else
      ATH_MSG_DEBUG( "Retrieved tool " << m_layerProviders );

   return StagedTrackingGeometryBuilderImpl::initialize();
}


std::unique_ptr<Trk::TrackingGeometry>
InDet::StagedTrackingGeometryBuilder::trackingGeometry
(Trk::TrackingVolume*) const
{
   // only one assumption:
   // layer builders are ordered in increasing r
   ATH_MSG_DEBUG( "[ Start building the ID TrackingGeometry. ]");
   ATH_MSG_DEBUG( "[ STEP 0 ] : Getting overal dimensions from DetectorEnvelope service." );

   ////////////////////////////////////////////////////////////////////////////////////////////////////////
   // The Overall Geometry

   // get the dimensions from the envelope service
   const RZPairVector& envelopeDefs = m_enclosingEnvelopeSvc->getInDetRZBoundary();
   ATH_MSG_VERBOSE("       -> retrieved Inner Detector envelope definitions at size " << envelopeDefs.size());
   double envelopeVolumeRadius = envelopeDefs[1].first;
   double envelopeVolumeHalfZ  = fabs(envelopeDefs[1].second);
   ATH_MSG_VERBOSE("       -> envelope R/Z defined as : " << envelopeVolumeRadius << " / " << envelopeVolumeHalfZ );

   ATH_MSG_DEBUG( "[ STEP 1 ] : Getting overal dimensions from the different layer builders." );
   size_t ilS = 0;
   double maximumLayerExtendZ   = 0.;
   double maximumLayerRadius    = 0.;
   std::vector<InDet::LayerSetup> layerSetups;
   for ( const auto& lProvider : m_layerProviders){
       // screen output
       ATH_MSG_DEBUG( "[ LayerBuilder : '" << lProvider->identification() << "' ] being processed. " );
       // retrieve the layers
       std::vector<Trk::Layer*> centralLayers = lProvider->centralLayers();
       std::pair<const std::vector<Trk::Layer*>, const std::vector<Trk::Layer*> > endcapLayersPair = lProvider->endcapLayer();
       ATH_MSG_VERBOSE("       -> retrieved "  << centralLayers.size()  << " central layers.");
       ATH_MSG_VERBOSE("       -> retrieved "  << endcapLayersPair.second.size() << " layers on negative side.");
       ATH_MSG_VERBOSE("       -> retrieved "  << endcapLayersPair.first.size() << " layers on positive side.");
       // getting the Layer setup from parsing the builder output
       InDet::LayerSetup lSetup =
         estimateLayerSetup(lProvider->identification(),
                            ilS,
                            endcapLayersPair.second,
                            centralLayers,
                            endcapLayersPair.first,
                            envelopeVolumeRadius,
                            envelopeVolumeHalfZ);
       // get the maxima - for R and Z
       takeBigger(maximumLayerRadius, lSetup.rMax);
       takeBigger(maximumLayerExtendZ, lSetup.zMax);
       //layer setups for the second run
       layerSetups.push_back(lSetup);
       // increase counter
       ++ilS;
   }
   ATH_MSG_VERBOSE("       -> layer max R/Z defined as : " << maximumLayerRadius << " / " << maximumLayerExtendZ );

   // create a volume cache for:
   // - ID volume, i.e. those that can be stacked into the overall container
   std::vector<Trk::TrackingVolume*> idVolumes;

   // create a layer setup cache for flushing when necessary
   std::vector<InDet::LayerSetup> layerSetupCache;

   // we only need to take care of the last flush radius
   double lastFlushRadius = 0.;

   // (I) PARSE THE LAYERS FOR OVERALL DIMENSIONS -------------------------------------------------------------
   ATH_MSG_DEBUG( "[ STEP 2 ] : Looping through the layer setups and flush them into the ID detector volume vector." );
   for ( auto& lSetup : layerSetups){
        // screen output
        ATH_MSG_DEBUG( "[ Layer setup: '" << lSetup.identification << "' ] being processed, current cache size is " << layerSetupCache.size() );
        ATH_MSG_VERBOSE("       -> estimated dimensions for this layer setup are");
        ATH_MSG_VERBOSE("       -> central sector rMin / rMax / zMax : "
            << lSetup.minRadiusCenter << " / " << lSetup.maxRadiusCenter << " / " << lSetup.zExtendCenter);
        if (lSetup.buildEndcap)
           ATH_MSG_VERBOSE("       -> endcap  sector rMin / rMax / zMin / zMax : "
               << lSetup.minRadiusEndcap << " / " << lSetup.maxRadiusEndcap << " / " << lSetup.minZextendEndcap << " / " << lSetup.maxZextendEndcap);
        else
           ATH_MSG_VERBOSE("       -> endcap is not being built.");

        // now check what is in the cache
           // [a] nothing in the cache or new setup is compatible (in this case sectorZ are updated in all setups)
        if (layerSetupCache.empty() || setupFitsCache(lSetup,layerSetupCache) ){
            ATH_MSG_VERBOSE("       -> cache is empty or new sector fits cache setup - add this one to the cache.");
        } else {
            // [b] cache is not empty - let's see what is going on:
            ATH_MSG_VERBOSE("       -> new sector does not fit the current cache specs -> flushing the cache." );
            // create the outer boundary
            //TODO CHECK THIS (NOEMI)
//             double flushRadius = layerSetupCache[layerSetupCache.size()-1].rMax > lSetup.rMin ?
//            0.5*(layerSetupCache[layerSetupCache.size()-1].rMax + lSetup.rMax) :  0.5*(layerSetupCache[layerSetupCache.size()-1].rMax + lSetup.rMin)  ;
            double flushRadius = 0.5*(layerSetupCache[layerSetupCache.size()-1].rMax + lSetup.rMin);
            // create a flush volume - clears the cache
            Trk::TrackingVolume* fVolume = createFlushVolume(layerSetupCache,lastFlushRadius,flushRadius,maximumLayerExtendZ);
            // stuff it into the idVolume
            idVolumes.push_back(fVolume);
            // remember the last flush radius
            lastFlushRadius = flushRadius;
        }
        // in any case, this setup needs to go into the cache
        layerSetupCache.push_back(lSetup);
   }

   // check if the cache is empty
   if (!layerSetupCache.empty()){
       ATH_MSG_DEBUG( "[ STEP 3 ] : Flush the remaining cache into the ID detector volume vector." );
       // set the maximum radius to the last layer radius
       double flushRadius = 0.5*(maximumLayerRadius  + envelopeVolumeRadius);
       Trk::TrackingVolume* fVolume = createFlushVolume(layerSetupCache,lastFlushRadius,flushRadius,maximumLayerExtendZ);
       // push it into the vector
       idVolumes.push_back(fVolume);
       lastFlushRadius = flushRadius;
   }

   ATH_MSG_DEBUG( "[ STEP 4 ] : Create the ID detector volumes" );
   // build the central enclosure first
   Trk::TrackingVolume* centralEnclosure =
     m_trackingVolumeCreator->createGapTrackingVolume(
                                                      *m_materialProperties,
                                                      lastFlushRadius,
                                                      envelopeVolumeRadius,
                                                      -maximumLayerExtendZ,
                                                      maximumLayerExtendZ,
                                                      1,
                                                      true,
                                                      m_namespace + "Gaps::CentralEnclosure");
   idVolumes.push_back(centralEnclosure);
   // now lets create the container
   std::string volumeName = m_namespace+"Detectors::Container";
   Trk::TrackingVolume* idContainer =
     m_trackingVolumeCreator->createContainerTrackingVolume(
       idVolumes,
       *m_materialProperties,
       volumeName,
       m_buildBoundaryLayers,
       m_replaceJointBoundaries);
   // finally create the two endplates: negative
   Trk::TrackingVolume* negativeEnclosure =
     m_trackingVolumeCreator->createGapTrackingVolume(
       *m_materialProperties,
       0.,
       envelopeVolumeRadius,
       -envelopeVolumeHalfZ,
       -maximumLayerExtendZ,
       1,
       false,
       m_namespace + "Gaps::NegativeEnclosure");

   // finally create the two endplates: positive
   Trk::TrackingVolume* positiveEnclosure =
     m_trackingVolumeCreator->createGapTrackingVolume(
       *m_materialProperties,
       0.,
       envelopeVolumeRadius,
       maximumLayerExtendZ,
       envelopeVolumeHalfZ,
       1,
       false,
       m_namespace + "Gaps::PositiveEnclosure");
   // and the final tracking volume
   std::vector<Trk::TrackingVolume*> enclosedVolumes;
   enclosedVolumes.push_back(negativeEnclosure);
   enclosedVolumes.push_back(idContainer);
   enclosedVolumes.push_back(positiveEnclosure);

   Trk::TrackingVolume* enclosedDetector =
     m_trackingVolumeCreator->createContainerTrackingVolume(
       enclosedVolumes,
       *m_materialProperties,
       m_exitVolume,
       m_buildBoundaryLayers,
       m_replaceJointBoundaries);

   //  create the TrackingGeometry ------------------------------------------------------
   auto trackingGeometry = std::make_unique<Trk::TrackingGeometry>(enclosedDetector);

   if (m_indexStaticLayers) {
      ATH_MSG_VERBOSE("Re-index the static layers ...");
      trackingGeometry->indexStaticLayers(Trk::Global);
   }

   return trackingGeometry;
}


Trk::TrackingVolume*
InDet::StagedTrackingGeometryBuilder::packVolumeTriple
(InDet::LayerSetup& layerSetup,
 double rMin,
 double& rMax,
 double zMax,
 double zPosCentral) const
{

  ATH_MSG_VERBOSE( '\t' << '\t'<< "Pack provided Layers from '" << layerSetup.identification << "' triple into a container volume. " );

  // create the strings
  std::string volumeBase = m_namespace+"Detectors::"+layerSetup.identification;

  Trk::TrackingVolume* negativeVolume =
    createTrackingVolume(layerSetup.negativeLayers,
                         rMin,
                         rMax,
                         -zMax,
                         -zPosCentral,
                         volumeBase + "::NegativeEndcap",
                         (Trk::BinningType)layerSetup.binningEndcap,
                         false);

  Trk::TrackingVolume* centralVolume =
    m_trackingVolumeCreator->createTrackingVolume(
      layerSetup.centralLayers,
      *m_materialProperties,
      rMin,
      rMax,
      -zPosCentral,
      zPosCentral,
      volumeBase + "::Barrel",
      (Trk::BinningType)layerSetup.binningCenter);

  Trk::TrackingVolume* positiveVolume =
    createTrackingVolume(layerSetup.positiveLayers,
                         rMin,
                         rMax,
                         zPosCentral,
                         zMax,
                         volumeBase + "::PositiveEndcap",
                         (Trk::BinningType)layerSetup.binningEndcap,
                         false);

  // the base volumes have been created
  ATH_MSG_VERBOSE(
    '\t' << '\t' << "Volumes have been created, now pack them into a triple.");
  // registerColorCode
  negativeVolume->registerColorCode(layerSetup.colorCode);
  centralVolume->registerColorCode(layerSetup.colorCode);
  positiveVolume->registerColorCode(layerSetup.colorCode);

  // pack them together
  std::vector<Trk::TrackingVolume*> tripleVolumes;
  tripleVolumes.push_back(negativeVolume);
  tripleVolumes.push_back(centralVolume);
  tripleVolumes.push_back(positiveVolume);

  // create the tiple container
  Trk::TrackingVolume* tripleContainer =
    m_trackingVolumeCreator->createContainerTrackingVolume(
      tripleVolumes,
      *m_materialProperties,
      volumeBase,
      m_buildBoundaryLayers,
      m_replaceJointBoundaries);

  ATH_MSG_VERBOSE('\t' << '\t' << "Created container volume with bounds: "
                       << tripleContainer->volumeBounds());

  return tripleContainer;
}


/** Private helper method, estimates the overal dimensions */
InDet::LayerSetup
InDet::StagedTrackingGeometryBuilder::estimateLayerSetup(
                                                         const std::string& idName,
                                                         size_t ilS,
                                                         const std::vector<Trk::Layer*>& negLayers,
                                                         const std::vector<Trk::Layer*>& cenLayers,
                                                         const std::vector<Trk::Layer*>& posLayers,
                                                         double maxR,
                                                         double maxZ) const
{
  // prepare the dimensions
  double cenMinR  = 10e10;
  double cenMaxR  = 0.;
  double cenMinZ  = 10e10;
  double cenMaxZ  = 0.;
  double posMinR  = 10e10;
  double posMaxR  = 0.;
  double posMinZ  = 10e10;
  double posMaxZ  = 0.;
  // parse through the central layers first
  estimateLayerDimensions(cenLayers, cenMinR, cenMaxR, cenMinZ, cenMaxZ);
  // parse throught the positive layers - we assume a symmetric setup
  estimateLayerDimensions(posLayers, posMinR, posMaxR, posMinZ, posMaxZ);
  // reset to maxZ and and maxR if overs
  if (posMaxZ > maxZ) {
      ATH_MSG_WARNING("Estimated z extended of central sector bigger than maximal z extened. Resetting - may lose layers though.");
      cenMaxZ = maxZ;
  } else if (posMaxZ > maxZ) {
      ATH_MSG_WARNING("Estimated z extended of endcap sector bigger than maximal z extened. Resetting - may lose layers though.");
      posMaxZ = maxZ;
  }
  // reset the radial wones.
  if (cenMaxR > maxR) {
      ATH_MSG_WARNING("Estimated r extended of central sector bigger than maximal r extened. Resetting - may lose layers though.");
      cenMaxR = maxR;
  }
  if (posMaxR > maxR) {
      ATH_MSG_WARNING("Estimated r extended of endcap sector bigger than maximal r extened. Resetting - may lose layers though.");
      posMaxR = maxR;
  }
  // create the layer setup class
  return InDet::LayerSetup(idName,
                           m_colorCodesConfig[ilS],
                           negLayers,
                           cenLayers,
                           posLayers,
                           cenMinR,
                           cenMaxR,
                           cenMaxZ,
                           m_layerBinningTypeCenter[ilS],
                           !posLayers.empty(),
                           posMinR,
                           posMaxR,
                           posMinZ,
                           posMaxZ,
                           m_layerBinningTypeEndcap[ilS]);
}


/** Private helper method to check if a sector is compatible with the cache */
bool
InDet::StagedTrackingGeometryBuilder::setupFitsCache(
                                                     LayerSetup& layerSetup,
                                                     std::vector<InDet::LayerSetup>& layerSetupCache) const
{
    // the maximum center and overall extend of the cache
    double maxCenterCacheZ = 0.;
    // the maximum endcap extend of the cache
    double minEndcapCacheZ = 10e10;
    // chech if he have a fullSectorSetup
    bool fullSectorSetup   = false;
    //
    for (const auto& lCacheSetup : layerSetupCache){
        takeBigger(maxCenterCacheZ, lCacheSetup.zExtendCenter);
        takeSmaller(minEndcapCacheZ, lCacheSetup.minZextendEndcap);
        // once true always true - otherwise it would have been flushed
        fullSectorSetup = lCacheSetup.buildEndcap ? true : fullSectorSetup;

    }
    // if we do not have a full sector setup - > flush directly
    if (!fullSectorSetup) {
        ATH_MSG_VERBOSE("       -> only central sector being built, flush the cache ... ");
        return false;
    }
    // if the cached minimum z endcap z extend cuts within the new barrel -> flush it
    if (minEndcapCacheZ < layerSetup.zExtendCenter){
        ATH_MSG_VERBOSE("       -> cache endcap extend reaches into new central sector, flush the cache ... ");
        return false;
    }
    // the cache center maximumg exceeds the new endcap minimum -> does not fit
    if (maxCenterCacheZ < layerSetup.minZextendEndcap ) {
        ATH_MSG_VERBOSE("       -> sector fully fits into cache! Add it and start synchronising ...");
        // calculate the new sector gap and synchronise
        double newCenterMaxZ = maxCenterCacheZ > layerSetup.zExtendCenter ? maxCenterCacheZ : layerSetup.zExtendCenter;
        double newEndcapMinZ = minEndcapCacheZ < layerSetup.minZextendEndcap ? minEndcapCacheZ : layerSetup.minZextendEndcap;
        double newSectorZ    = 0.5*(newCenterMaxZ+newEndcapMinZ);
        // and syncrhonise the boundaries
        for (auto& lCacheSetup : layerSetupCache)
            lCacheSetup.zSector = newSectorZ;
        layerSetup.zSector = newSectorZ;
        return true;
    }
    // it simply does not fit so return false
    return false;
}


/** Private helper method to flush the cache into the id volumes - return volume is the one to be provided */
Trk::TrackingVolume*
InDet::StagedTrackingGeometryBuilder::createFlushVolume
(std::vector<InDet::LayerSetup>& layerSetupCache,
 double innerRadius,
 double& outerRadius,
 double extendZ) const
{
  // the return volume
  Trk::TrackingVolume* flushVolume = nullptr;
  //
  if (layerSetupCache.size() == 1 ){
    ATH_MSG_VERBOSE("       -> single sector setup - synchronising from inner (" << innerRadius << ") to outer (" << outerRadius << ") radius.");
    ATH_MSG_VERBOSE("       -> setup identification : " << layerSetupCache[0].identification );
    // create the new tracking volume - either as a triple or as a single
        flushVolume = layerSetupCache[0].buildEndcap ?
        packVolumeTriple(layerSetupCache[0],
                         innerRadius, outerRadius,
                         extendZ,layerSetupCache[0].zSector) :
        m_trackingVolumeCreator->createTrackingVolume(layerSetupCache[0].centralLayers,
                                                      *m_materialProperties,
                                                      innerRadius,outerRadius,
                                                      -extendZ,extendZ,
                                                      layerSetupCache[0].identification,
                                                      (Trk::BinningType)layerSetupCache[0].binningCenter);

   } else {
       ATH_MSG_VERBOSE("       -> setup with " << layerSetupCache.size() << " entries - synchronising from inner (" << innerRadius << ") to outer (" << outerRadius << ") radius.");
       // prepare the volume vectors & name identification
       std::vector<Trk::TrackingVolume*> negVolumes;
       std::vector<Trk::TrackingVolume*> centralVolumes;
       std::vector<Trk::TrackingVolume*> posVolumes;
       std::string combinedName;
       for (size_t ilS = 0; ilS < layerSetupCache.size(); ++ilS){
           // take the given inner radius for the first one - median otherwise
           double irE = ilS ? 0.5*(layerSetupCache[ilS].minRadiusEndcap+layerSetupCache[ilS-1].maxRadiusEndcap) : innerRadius;
           double irC = ilS ? 0.5*(layerSetupCache[ilS].minRadiusCenter+layerSetupCache[ilS-1].maxRadiusCenter) : innerRadius;
           // take the given outer radius for the last one - median otherwise
           double orE = ((ilS+1)==layerSetupCache.size()) ? outerRadius : 0.5*(layerSetupCache[ilS+1].minRadiusEndcap+layerSetupCache[ilS].maxRadiusEndcap);
           double orC = ((ilS+1)==layerSetupCache.size()) ? outerRadius : 0.5*(layerSetupCache[ilS+1].minRadiusCenter+layerSetupCache[ilS].maxRadiusCenter);
           // Adjust last volumes in R to the same maximal radial extends!
           if(ilS==layerSetupCache.size()-1) {
             ATH_MSG_VERBOSE("Processing last volume");
             ATH_MSG_VERBOSE("  --> adjust volumes to same extends: orE=" << orE << " orC=" << orC);
             if(orE>orC) orC=orE; else orE=orC;
           }
           // create the three volumes
           Trk::TrackingVolume* nVolume = createTrackingVolume(
             layerSetupCache[ilS].negativeLayers,
             irE,
             orE,
             -extendZ,
             -layerSetupCache[ilS].zSector,
             layerSetupCache[ilS].identification + "::NegativeEndcap",
             (Trk::BinningType)layerSetupCache[ilS].binningEndcap,
             false);
           Trk::TrackingVolume* cVolume =
             m_trackingVolumeCreator->createTrackingVolume(
               layerSetupCache[ilS].centralLayers,
               *m_materialProperties,
               irC,
               orC,
               -layerSetupCache[ilS].zSector,
               layerSetupCache[ilS].zSector,
               layerSetupCache[ilS].identification + "::Barrel",
               (Trk::BinningType)layerSetupCache[ilS].binningCenter);
           Trk::TrackingVolume* pVolume = createTrackingVolume(
             layerSetupCache[ilS].positiveLayers,
             irE,
             orE,
             layerSetupCache[ilS].zSector,
             extendZ,
             layerSetupCache[ilS].identification + "::PositiveEndcap",
             (Trk::BinningType)layerSetupCache[ilS].binningEndcap,
             false);
           // register the right color code
           nVolume->registerColorCode(layerSetupCache[ilS].colorCode);
           cVolume->registerColorCode(layerSetupCache[ilS].colorCode);
           pVolume->registerColorCode(layerSetupCache[ilS].colorCode);
           // push them into the volume containers
           negVolumes.push_back(nVolume);
           centralVolumes.push_back(cVolume);
           posVolumes.push_back(pVolume);
           // combined name
           combinedName += "_"+layerSetupCache[ilS].identification;
       }
       ATH_MSG_VERBOSE("       -> setup identification : " << combinedName );
       flushVolume  = packVolumeTriple(negVolumes,centralVolumes,posVolumes,combinedName);
   }
   // clear the cache
   layerSetupCache.clear();
   // return the volume
   return flushVolume;

}

