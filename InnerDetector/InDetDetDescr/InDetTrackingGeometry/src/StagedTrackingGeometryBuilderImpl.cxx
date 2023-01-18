/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// InDet
#include "InDetTrackingGeometry/StagedTrackingGeometryBuilderImpl.h"
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
//Athena
#include "CxxUtils/checker_macros.h"
#include "AthenaKernel/IOVInfiniteRange.h"
//Gaudi
#include "GaudiKernel/SystemOfUnits.h"
#include <iterator> //std::advance

// constructor
InDet::StagedTrackingGeometryBuilderImpl::StagedTrackingGeometryBuilderImpl(const std::string& t, const std::string& n, const IInterface* p) :
  AthAlgTool(t,n,p),
  Trk::TrackingVolumeManipulator()
{
}


// Athena standard methods
// initialize
StatusCode InDet::StagedTrackingGeometryBuilderImpl::initialize()
{

  // retrieve envelope definition service --------------------------------------------------
  ATH_CHECK(m_enclosingEnvelopeSvc.retrieve());

  // Retrieve the tracking volume creator  -------------------------------------------------
  ATH_CHECK(m_trackingVolumeCreator.retrieve());
  ATH_MSG_DEBUG( "Retrieved tool " << m_trackingVolumeCreator );

  // Retrieve the layer array creator  ----------------------------------------------------
  ATH_CHECK(m_layerArrayCreator.retrieve());
  ATH_MSG_INFO( "Retrieved tool " << m_layerArrayCreator );

  // Dummy MaterialProerties
  m_materialProperties.set(std::make_unique<Trk::Material>());

  ATH_MSG_INFO( "initialize() succesful" );

  return StatusCode::SUCCESS;
}


// finalize
StatusCode InDet::StagedTrackingGeometryBuilderImpl::finalize()
{
    ATH_MSG_INFO( "finalize() successful" );
    return StatusCode::SUCCESS;
}


/** Private helper method to estimate the layer dimensions */
void
InDet::StagedTrackingGeometryBuilderImpl::estimateLayerDimensions(
  const std::vector<Trk::Layer*>& layers,
  double& rMin,
  double& rMax,
  double& zMin,
  double& zMax) const
{
    // parse through the layers and estimate
      for (auto *const layer : layers){
          // the thickness of the layer needs to be taken into account
          double thickness = layer->thickness();
          // get the center
          const Amg::Vector3D& center = layer->surfaceRepresentation().center();
          // check if it is a cylinder layer
          const Trk::CylinderLayer* cLayer = dynamic_cast<const Trk::CylinderLayer*>(layer);
          if (cLayer){
              // now we have access to all the information
              double rMinC  = cLayer->surfaceRepresentation().bounds().r()-0.5*thickness-m_layerEnvelopeCover;
              double rMaxC  = cLayer->surfaceRepresentation().bounds().r()+0.5*thickness+m_layerEnvelopeCover;
              double hZ = cLayer->surfaceRepresentation().bounds().halflengthZ();
              takeSmaller(rMin,rMinC);
              takeBigger(rMax,rMaxC);
              takeSmaller(zMin,center.z()-hZ);
              takeBigger(zMax,center.z()+hZ);
          }
          // proceed further if it is a Disc layer
          const Trk::DiscBounds* dBounds = dynamic_cast<const Trk::DiscBounds*>(&(layer->surfaceRepresentation().bounds()));
          if (dBounds){
              // now we have access to all the information
              double rMinD =dBounds->rMin();
              double rMaxD =dBounds->rMax();
              double zMinD =  center.z()-0.5*thickness-m_layerEnvelopeCover;
              double zMaxD =  center.z()+0.5*thickness+m_layerEnvelopeCover;
              takeSmaller(rMin,rMinD);
              takeBigger(rMax,rMaxD);
              takeSmaller(zMin,zMinD);
              takeBigger(zMax,zMaxD);
          }
      }
}


bool InDet::StagedTrackingGeometryBuilderImpl::ringLayout(const std::vector<Trk::Layer*>& layers, std::vector<double>& rmins, std::vector<double>& rmaxs) const {
  // get the maximum extent in z
  std::vector<std::pair<double,double>> radii;
  ATH_MSG_DEBUG("Checking for Ring layout ... ");
  for (auto *const ring : layers) {
    // Surface
    const Trk::Surface&     ringSurface = ring->surfaceRepresentation();
    const Trk::DiscBounds*  ringBounds  = dynamic_cast<const Trk::DiscBounds*>(&(ringSurface.bounds()));
    if (ringBounds){
      // get the main parameters
      double zpos         = ringSurface.center().z();
      double rMin         = ringBounds->rMin();
      double rMax         = ringBounds->rMax();
      // take and check the couple rmin/rmax
      checkForInsert(rMin, rMax, radii);
      ATH_MSG_DEBUG(" -> Ring at z-position " << zpos << " - with rMin/rMax = " << rMin << "/" << rMax );
    }
  }

  // you need a post processing of the (rmin,rmax) in order to fit z-overlapping disks in the same ring
  std::vector<std::pair<double,double>> tmpradii;

  for (auto& rs: radii) {
    bool found = false;
    for (auto& tmprs: tmpradii) {
      if ((rs.first<tmprs.second and rs.second>tmprs.first) ) {
        tmprs.first  = std::min(tmprs.first ,rs.first );
        tmprs.second = std::max(tmprs.second,rs.second);
        found = true;
        break;
      }
    }
    if (found) continue;
    tmpradii.push_back(rs);
  }

  // now you fill rmin and rmax
  rmins.clear(); rmaxs.clear();
  for (auto& r: tmpradii) {
    rmins.push_back(r.first);
    rmaxs.push_back(r.second);
  }

  //add rmin and rmax
  return (rmins.size() > 1 );
}

Trk::TrackingVolume*
InDet::StagedTrackingGeometryBuilderImpl::createTrackingVolume(
  const std::vector<Trk::Layer*>& layers,
  double innerRadius,
  double& outerRadius,
  double zMin,
  double zMax,
  const std::string& volumeName,
  Trk::BinningType binningType,
  bool doAdjustOuterRadius) const
{

    // first loop - this is for diagnostics for the radii
    std::vector<double> ringRmins;
    std::vector<double> ringRmaxa;
    if (m_checkForRingLayout && ringLayout(layers,ringRmins, ringRmaxa)){
        ATH_MSG_INFO("Ring layout is present for volume '" << volumeName << "' dealing with it.");
        // create the vector for the sub volumes
        std::vector<Trk::TrackingVolume* > ringVolumes;
        std::vector<Trk::TrackingVolume* > const_ringVolumes;

        // now sort the necessary layers --- for the sub volumes
        std::vector< std::vector<Trk::Layer*> > groupedDiscs(ringRmins.size(), std::vector<Trk::Layer*>() );
        // second loop over the rings
        for (auto *ring : layers){
            // Surface
            const Trk::Surface&     ringSurface = ring->surfaceRepresentation();
            const Trk::DiscBounds*  ringBounds  = dynamic_cast<const Trk::DiscBounds*>(&(ringSurface.bounds()));
            if (ringBounds){
                // get the main parameters
                double rMax         = ringBounds->rMax();
                size_t rPos         = 0;
                // fill into the right group
                for (auto& rm : ringRmaxa){
                    if (rMax < rm+m_ringTolerance) break;
                    ++rPos;
                }
                // fill it it
                Trk::DiscLayer* dring = dynamic_cast<Trk::DiscLayer*>(ring);
                if (dring) groupedDiscs[rPos].push_back(dring);
            }
        }
      // layer merging may be needed
      std::vector< std::vector<Trk::Layer*> > mergedLayers;
      std::vector< float > mergedRmax;
      std::vector< std::vector< int > > merge;
      std::vector<int> laySet(1,0); merge.push_back(laySet);
      double rCurr = ringRmaxa[0];
      mergedRmax.push_back(rCurr);
      for (int idset = 1; idset < int(groupedDiscs.size()); idset++){
        if (ringRmins[idset]<=rCurr + m_ringTolerance) {
          merge.back().push_back(idset);
          if (ringRmaxa[idset]>mergedRmax.back()) mergedRmax.back()=ringRmaxa[idset];
        } else {
          merge.emplace_back(1,idset);
          mergedRmax.push_back(ringRmaxa[idset]);
        }
        rCurr = ringRmaxa[idset];
      }
      for ( const auto& layset : merge ) {
        std::vector<Trk::Layer*> ringSet;
        for ( const auto& lay : layset ) {
          for ( auto *ring : groupedDiscs[lay]) {
            float zPos = ring->surfaceRepresentation().center().z();
            if (ringSet.empty() || zPos>ringSet.back()->surfaceRepresentation().center().z()) ringSet.push_back(ring);
            else {
              std::vector<Trk::Layer*>::iterator lit = ringSet.begin();
              while (lit!=ringSet.end() && zPos>(*lit)->surfaceRepresentation().center().z()) ++lit;
              ringSet.insert(lit,ring);
            }
          }
        }
        // rings ordered in z : resolve overlap
        mergedLayers.push_back(checkZoverlap(ringSet));
      }
      // we are now grouped in cylinder rings per volume
      for (int idset = 0; idset < int(mergedLayers.size()); idset++){
        // always keep the boundaries in mind for the radial extend
        double crmin = idset ? mergedRmax[idset-1]+m_layerEnvelopeCover : innerRadius;
        double crmax = mergedRmax[idset]+m_layerEnvelopeCover;
        if(idset==int(mergedLayers.size())-1 && !doAdjustOuterRadius) crmax = outerRadius;
        // now create the sub volume
        std::string ringVolumeName = volumeName+"Ring"+std::to_string(idset);
        Trk::TrackingVolume* ringVolume =
          m_trackingVolumeCreator->createTrackingVolume(mergedLayers[idset],
                                                        *m_materialProperties,
                                                        crmin,
                                                        crmax,
                                                        zMin,
                                                        zMax,
                                                        ringVolumeName,
                                                        binningType);
        // push back into the vectors
        ringVolumes.push_back(ringVolume);
        const_ringVolumes.push_back(ringVolume);
      }
      // set the outer radius
      if(doAdjustOuterRadius) outerRadius = ringRmaxa[ringRmaxa.size()-1]+m_layerEnvelopeCover;
      //
      ATH_MSG_INFO("      -> adjusting the outer radius to the last ring at " << outerRadius );
      ATH_MSG_INFO("      -> created " << ringVolumes.size() << " ring volumes for Volume '" << volumeName << "'.");
      // create the tiple container
      if (ringVolumes.size()==1)
        return ringVolumes.at(0);
      else
        return m_trackingVolumeCreator->createContainerTrackingVolume(const_ringVolumes,
                                                                      *m_materialProperties,
                                                                      volumeName,
                                                                      m_buildBoundaryLayers,
                                                                      m_replaceJointBoundaries);
    } else
        return m_trackingVolumeCreator->createTrackingVolume(layers,
                                                             *m_materialProperties,
                                                             innerRadius,outerRadius,
                                                             zMin,zMax,
                                                             volumeName,
                                                             binningType);
}


Trk::TrackingVolume* InDet::StagedTrackingGeometryBuilderImpl::packVolumeTriple(
                                     const std::vector<Trk::TrackingVolume*>& negVolumes,
                                     const std::vector<Trk::TrackingVolume*>& centralVolumes,
                                     const std::vector<Trk::TrackingVolume*>& posVolumes,
                                     const std::string& baseName) const
{
  ATH_MSG_VERBOSE( '\t' << '\t'<< "Pack provided Volumes from '" << baseName << "' triple into a container volume. " );

  unsigned int negVolSize = negVolumes.size();
  unsigned int cenVolSize = centralVolumes.size();
  unsigned int posVolSize = posVolumes.size();



    // create the strings
  std::string volumeBase = m_namespace+"Containers::"+baseName;

  Trk::TrackingVolume* negativeVolume = (negVolSize > 1) ?
       m_trackingVolumeCreator->createContainerTrackingVolume(negVolumes,
                                                       *m_materialProperties,
                                                       volumeBase+"::NegativeSector",
                                                       m_buildBoundaryLayers,
                                                       m_replaceJointBoundaries) :
                                             (negVolSize ? negVolumes[0] : nullptr);
  Trk::TrackingVolume* centralVolume = (cenVolSize > 1) ?
         m_trackingVolumeCreator->createContainerTrackingVolume(centralVolumes,
                                                       *m_materialProperties,
                                                       volumeBase+"::CentralSector",
                                                       m_buildBoundaryLayers,
                                                       m_replaceJointBoundaries) :
                                              (cenVolSize ? centralVolumes[0] : nullptr) ;

   Trk::TrackingVolume* positiveVolume = ( posVolSize > 1) ?
         m_trackingVolumeCreator->createContainerTrackingVolume(posVolumes,
                                                       *m_materialProperties,
                                                       volumeBase+"::PositiveSector",
                                                       m_buildBoundaryLayers,
                                                       m_replaceJointBoundaries) :
                                               (posVolSize ? posVolumes[0] : nullptr);

   if (!negativeVolume && !positiveVolume){
       ATH_MSG_DEBUG( "No negative/positive sector given - no packing needed, returning central container!" );
       return centralVolume;
   }
   // pack them together
   std::vector<Trk::TrackingVolume*> tripleVolumes;
   if (negativeVolume) tripleVolumes.push_back(negativeVolume);
   if (centralVolume) tripleVolumes.push_back(centralVolume);
   if (positiveVolume) tripleVolumes.push_back(positiveVolume);
   // create the tiple container
   Trk::TrackingVolume* tripleContainer =
         m_trackingVolumeCreator->createContainerTrackingVolume(tripleVolumes,
                                                                *m_materialProperties,
                                                                volumeBase,
                                                                m_buildBoundaryLayers,
                                                                m_replaceJointBoundaries);
   return tripleContainer;
}


std::vector<Trk::Layer*> InDet::StagedTrackingGeometryBuilderImpl::checkZoverlap(std::vector<Trk::Layer*>& lays) const
{
  // look for layers to merge if they overlap in z

  // caching the layers with locations in z
  std::map < float , std::vector<Trk::Layer*> > locationAndLayers;

  // loop on the layers and save the location:
  // if one layer location is compatible with
  // another one (considering the layer thickness)
  // then the two layers have to be merged
  for (auto *lay : lays) {
    float zpos= lay->surfaceRepresentation().center().z();
    float thick = 0.5*lay->thickness();

    bool foundZoverlap = false;
    for (auto& singlePosLayer : locationAndLayers) {
      if (abs(zpos - singlePosLayer.first) < thick) {
        singlePosLayer.second.push_back(lay);
        foundZoverlap = true;
        break;
      }
    }

    // if no overlap is found, a new location (with corresponding layer)
    // has to be added to the map
    if (not foundZoverlap) {
      locationAndLayers[zpos] = std::vector<Trk::Layer*>();
      locationAndLayers[zpos].push_back(lay);
    }
  }

  // If the number of final layers decreases,
  // merging is detected and discs need to be merged.
  // The new merged layers are returned instead of the initial ones.
  if (lays.size()>locationAndLayers.size()) {
    std::vector<Trk::Layer*> mergedDiscLayers;
    for (auto& singlePosLayer : locationAndLayers) {
      Trk::Layer* nd = mergeDiscLayers(singlePosLayer.second);
      if (nd) mergedDiscLayers.push_back(nd);
      else {
        ATH_MSG_WARNING("radial merge of rings failed, return the input layer set");
        return lays;
      }
    }
    return mergedDiscLayers;
  }

  return lays;

}

Trk::Layer* InDet::StagedTrackingGeometryBuilderImpl::mergeDiscLayers (std::vector<Trk::Layer*>&) const {
  // Dummy implementation to be overridden
  return nullptr;
}
