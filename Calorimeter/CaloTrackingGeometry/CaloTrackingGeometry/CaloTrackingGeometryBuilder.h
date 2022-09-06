/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CaloTrackingGeometryBuilder.hm (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef CALORIMETER_CALOTRACKINGGEOMETRYBUILDER_H
#define CALORIMETER_CALOTRACKINGGEOMETRYBUILDER_H

#include "CaloIdentifier/CaloCell_ID.h"
// Gaudi/Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
// Trk
#include "TrkDetDescrInterfaces/IGeometryBuilder.h"
#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkDetDescrUtils/LayerIndexSampleMap.h"
// EnvelopeDefinitionService
#include "SubDetectorEnvelopes/IEnvelopeDefSvc.h"

// STL
#include <vector>

#include <CxxUtils/checker_macros.h>
namespace Trk {
  class Material;
  class Layer;
  class MagneticFieldProperties;
  class TrackingGeometry;
  class ICaloTrackingVolumeBuilder;
  class ITrackingVolumeCreator;
  class ITrackingVolumeHelper;
  class ITrackingVolumeArrayCreator;
  class IMagneticFieldTool;
}

class IEnvelopeDefSvc;

namespace Calo {

  /** @class CaloTrackingGeometryBuilder
     
    Retrieves the volume builders from Tile and LAr
    and combines the given volumes to a calorimeter tracking
    geometry.
    
    It also wraps an inner volume if provided though
    the mehtod interface.
  
    @author Andreas.Salzburger@cern.ch
    */
class ATLAS_NOT_THREAD_SAFE CaloTrackingGeometryBuilder // Thread unsafe TrackingGeometry::indexStaticLayers
  : public AthAlgTool
  , virtual public Trk::IGeometryBuilder
{

public:
  /** Constructor */
  CaloTrackingGeometryBuilder(const std::string&, const std::string&, const IInterface*);

  /** Destructor */
  virtual ~CaloTrackingGeometryBuilder();

  /** AlgTool initailize method.*/
  virtual StatusCode initialize() override;

  /** AlgTool finalize method */
  virtual StatusCode finalize() override;

  /** TrackingGeometry Interface methode */
  virtual Trk::TrackingGeometry* trackingGeometry(Trk::TrackingVolume* tvol = 0) const override;

  /** The unique signature */
  virtual Trk::GeometrySignature geometrySignature() const override { return Trk::Calo; }

private:
  ToolHandle<Trk::ITrackingVolumeArrayCreator>
    m_trackingVolumeArrayCreator; //!< Helper Tool to create TrackingVolume Arrays

  ToolHandle<Trk::ITrackingVolumeHelper> m_trackingVolumeHelper; //!< Helper Tool to create TrackingVolumes

  ToolHandle<Trk::ITrackingVolumeCreator> m_trackingVolumeCreator; //!< Second helper for volume creation

  ToolHandle<Trk::ICaloTrackingVolumeBuilder> m_lArVolumeBuilder; //!< Volume Builder for the Liquid Argon Calorimeter

  ToolHandle<Trk::ICaloTrackingVolumeBuilder> m_tileVolumeBuilder; //!< Volume Builder for the Tile Calorimeter

  Trk::Material* m_caloMaterial; //!< Material properties

  double m_caloEnvelope; //!< Envelope cover for Gap Layers
  // enclosing endcap/cylindervolume
  ServiceHandle<IEnvelopeDefSvc> m_enclosingEnvelopeSvc;

  double m_caloDefaultRadius;      //!< the radius if not built from GeoModel
  double m_caloDefaultHalflengthZ; //!< the halflength in z if not built from GeoModel

  bool m_indexStaticLayers; //!< forces robust indexing for layers

  bool m_recordLayerIndexCaloSampleMap;      //!< for deposition methods
  std::string m_layerIndexCaloSampleMapName; //!< name to record it

  bool m_buildMBTS; //!< MBTS like detectors
  // int				                            m_mbstSurfaceShape;		          //!< MBTS like
  // detectors
  std::vector<double> m_mbtsRadiusGap;     //!< MBTS like detectors
  std::vector<int> m_mbtsPhiSegments;      //!< MBTS like detectors
  std::vector<double> m_mbtsPhiGap;        //!< MBTS like detectors
  std::vector<double> m_mbtsPositionZ;     //!< MBTS like detectors
  std::vector<double> m_mbtsStaggeringZ;   //!< MBTS like detectors

  std::string m_entryVolume; //!< name of the Calo entrance
  std::string m_exitVolume;  //!< name of the Calo container

  /** method to establish a link between the LayerIndex and the CaloCell_ID in an associative container */
  void registerInLayerIndexCaloSampleMap(Trk::LayerIndexSampleMap& licsMAp,
                                         std::vector<CaloCell_ID::CaloSample> ccid,
                                         const Trk::TrackingVolume& vol,
                                         int side = 1) const;

  /** method to build enclosed beam pipe volumes */
  std::pair<Trk::TrackingVolume*, Trk::TrackingVolume*>
  createBeamPipeVolumes(const RZPairVector& bpCutouts,
                        float zmin, float zmax,
                        const std::string& name, float& outerRadius) const;

  /** cleanup of material */
  mutable std::mutex m_garbageMutex;
  mutable std::vector<const Trk::Material*> m_materialGarbage ATLAS_THREAD_SAFE;
};

} // end of namespace

#endif // CALORIMETER_CALOTRACKINGGEOMETRYBUILDER_H


