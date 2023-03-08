/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////
// RobustTrackingGeometryBuilderImpl.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// InDet
#include "InDetTrackingGeometry/RobustTrackingGeometryBuilderImpl.h"
// Athena
#include "AthenaKernel/IOVInfiniteRange.h"
#include "CxxUtils/checker_macros.h"
//Gaudi
#include "GaudiKernel/MsgStream.h"

// constructor
InDet::RobustTrackingGeometryBuilderImpl::RobustTrackingGeometryBuilderImpl(const std::string& t, const std::string& n, const IInterface* p) :
  AthAlgTool(t,n,p),
  Trk::TrackingVolumeManipulator()
{
}

// Athena standard methods
// initialize
StatusCode InDet::RobustTrackingGeometryBuilderImpl::initialize()
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

  ATH_MSG_INFO( "initialize() successful" );

  return StatusCode::SUCCESS;
}


Trk::TrackingVolume* InDet::RobustTrackingGeometryBuilderImpl::packVolumeTriple(
                                     const std::vector<Trk::Layer*>& negLayers,
                                     const std::vector<Trk::Layer*>& centralLayers,
                                     const std::vector<Trk::Layer*>& posLayers,
                                     double rMin, double rMax,
                                     double zMax, double zPosCentral,
                                     const std::string& baseName,
                                     int colorCode,
                                     Trk::BinningType bintyp) const
{


  ATH_MSG_VERBOSE( '\t' << '\t'<< "Pack provided Layers from '" << baseName << "' triple into a container volume. " );

  // create the strings
  std::string volumeBase = m_namespace+"Detectors::"+baseName;

  Trk::TrackingVolume* negativeVolume =
    m_trackingVolumeCreator->createTrackingVolume(negLayers,
                                                  *m_materialProperties,
                                                  rMin,
                                                  rMax,
                                                  -zMax,
                                                  -zPosCentral,
                                                  volumeBase +
                                                    "::NegativeEndcap",
                                                  bintyp);

  Trk::TrackingVolume* centralVolume =
    m_trackingVolumeCreator->createTrackingVolume(centralLayers,
                                                  *m_materialProperties,
                                                  rMin,
                                                  rMax,
                                                  -zPosCentral,
                                                  zPosCentral,
                                                  volumeBase + "::Barrel",
                                                  bintyp);

  Trk::TrackingVolume* positiveVolume =
    m_trackingVolumeCreator->createTrackingVolume(posLayers,
                                                  *m_materialProperties,
                                                  rMin,
                                                  rMax,
                                                  zPosCentral,
                                                  zMax,
                                                  volumeBase +
                                                    "::PositiveEndcap",
                                                  bintyp);

  // the base volumes have been created
  ATH_MSG_VERBOSE(
    '\t' << '\t' << "Volumes have been created, now pack them into a triple.");
  // registerColorCode
  negativeVolume->registerColorCode(colorCode);
  centralVolume->registerColorCode(colorCode);
  positiveVolume->registerColorCode(colorCode);

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


Trk::TrackingVolume* InDet::RobustTrackingGeometryBuilderImpl::packVolumeTriple(
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
