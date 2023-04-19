/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALORIMETER_CALOTRACKINGGEOMETRYBUILDERIMPL_H
#define CALORIMETER_CALOTRACKINGGEOMETRYBUILDERIMPL_H

#include "CaloIdentifier/CaloCell_ID.h"
// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
// Trk
#include "TrkDetDescrInterfaces/IGeometryBuilderCond.h"
#include "TrkDetDescrUtils/GeometrySignature.h"
#include "TrkDetDescrUtils/LayerIndexSampleMap.h"
#include "TrkGeometry/Material.h"
#include "TrkGeometry/TrackingGeometry.h"
// EnvelopeDefinitionService
#include "SubDetectorEnvelopes/IEnvelopeDefSvc.h"
// CaloDDM
#include "CaloDetDescr/CaloDetDescrManager.h"
// STL
#include <vector>
//
#include "CxxUtils/checker_macros.h"

namespace Trk {
class Layer;
class MagneticFieldProperties;
class ICaloTrackingVolumeBuilder;
class ITrackingVolumeCreator;
class ITrackingVolumeHelper;
class ITrackingVolumeArrayCreator;
}  // namespace Trk

class IEnvelopeDefSvc;

namespace Calo {

/** @class CaloTrackingGeometryBuilderImpl

  Retrieves the volume builders from Tile and LAr
  and combines the given volumes to a calorimeter tracking
  geometry.

  This is the underlying implementation logic
  that is shared by the the concrete builders
  */

class CaloTrackingGeometryBuilderImpl : public AthAlgTool {

 public:
  /** Destructor */
  virtual ~CaloTrackingGeometryBuilderImpl();

  /** AlgTool initailize method.*/
  virtual StatusCode initialize() override;

  /** TrackingGeometry Interface method */
  std::unique_ptr<Trk::TrackingGeometry> createTrackingGeometry(
      Trk::TrackingVolume* innerVol, const CaloDetDescrManager* caloDDM) const;

  /** The unique signature */
  Trk::GeometrySignature signature() const {
    return Trk::Calo;
  }


 protected:
  /** Constructor */
  CaloTrackingGeometryBuilderImpl(const std::string&, const std::string&,
                                  const IInterface*);

  //!< Helper Tool to create TrackingVolume Arrays
  ToolHandle<Trk::ITrackingVolumeArrayCreator> m_trackingVolumeArrayCreator;
  //!< Helper Tool to create TrackingVolumes
  ToolHandle<Trk::ITrackingVolumeHelper> m_trackingVolumeHelper;
  //!< Second helper for volume creation
  ToolHandle<Trk::ITrackingVolumeCreator> m_trackingVolumeCreator;
  //!< Volume Builder for the Liquid Argon Calorimeter
  ToolHandle<Trk::ICaloTrackingVolumeBuilder> m_lArVolumeBuilder;
  //!< Volume Builder for the Tile Calorimeter
  ToolHandle<Trk::ICaloTrackingVolumeBuilder> m_tileVolumeBuilder;

  //!< Material properties
  Trk::Material m_caloMaterial; 
  Trk::Material m_Ar;  
  Trk::Material m_Al;  
  Trk::Material m_Scint;
  Trk::Material m_crackMaterial;

  double m_caloEnvelope;  //!< Envelope cover for Gap Layers
  // enclosing endcap/cylindervolume
  ServiceHandle<IEnvelopeDefSvc> m_enclosingEnvelopeSvc;

  double m_caloDefaultRadius;       //!< the radius if not built from GeoModel
  double m_caloDefaultHalflengthZ;  //!< the halflength in z if not built from
                                    //!< GeoModel

  bool m_indexStaticLayers;  //!< forces robust indexing for layers

  bool m_recordLayerIndexCaloSampleMap;       //!< for deposition methods
  std::string m_layerIndexCaloSampleMapName;  //!< name to record it

  bool m_buildMBTS;  //!< MBTS like detectors
  // //!< MBTS like detectors
  std::vector<double> m_mbtsRadiusGap;    //!< MBTS like detectors
  std::vector<int> m_mbtsPhiSegments;     //!< MBTS like detectors
  std::vector<double> m_mbtsPhiGap;       //!< MBTS like detectors
  std::vector<double> m_mbtsPositionZ;    //!< MBTS like detectors
  std::vector<double> m_mbtsStaggeringZ;  //!< MBTS like detectors

  std::string m_entryVolume;  //!< name of the Calo entrance
  std::string m_exitVolume;   //!< name of the Calo container

  /** method to establish a link between the LayerIndex and the CaloCell_ID in
   * an associative container */
  void registerInLayerIndexCaloSampleMap(
      Trk::LayerIndexSampleMap& licsMAp,
      std::vector<CaloCell_ID::CaloSample> ccid, const Trk::TrackingVolume& vol,
      int side = 1) const;

  /** method to build enclosed beam pipe volumes */
  std::pair<Trk::TrackingVolume*, Trk::TrackingVolume*> createBeamPipeVolumes(
      const RZPairVector& bpCutouts, float, float, const std::string&,
      float&) const;

};

}  // namespace Calo

#endif  // CALORIMETER_CALOTRACKINGGEOMETRYBUILDERIMPL_H

