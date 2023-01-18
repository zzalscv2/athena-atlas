/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_ROBUSTTRACKINGGEOMETRYBUILDERIMPL_H
#define INDETTRACKINGGEOMETRY_ROBUSTTRACKINGGEOMETRYBUILDERIMPL_H

//Trk
#include "TrkDetDescrUtils/BinningType.h"
#include "TrkGeometry/TrackingVolumeManipulator.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkDetDescrInterfaces/ITrackingVolumeCreator.h"
#include "TrkDetDescrInterfaces/ILayerArrayCreator.h"
// EnvelopeDefinitionService
#include "SubDetectorEnvelopes/IEnvelopeDefSvc.h"
// Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/CachedUniquePtr.h"
#include "CxxUtils/checker_macros.h"
// Gaudi
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SystemOfUnits.h"
// STL
#include <vector>
#include <string>

#include "CxxUtils/checker_macros.h"

#ifndef TRKDETDESCR_TAKESMALLERBIGGER
#define TRKDETDESCR_TAKESMALLERBIGGER
#define takeSmaller(current,test) current = current < test ? current : test
#define takeBigger(current,test)  current = current > test ? current : test
#define takeSmallerBigger(cSmallest, cBiggest, test) takeSamller(cSmallest, cBiggest, test); takeBigger(cSmallest, cBiggest,test)
#endif


namespace Trk {
 class Layer;
 class Material;
}

namespace InDet {


  /** @class RobustTrackingGeometryBuilderImpl

      New Geometry builder that adapts to different layer setups

      Only a few parameters are not automated:
       - m_outwardsFraction: this defines how much you orient yourself on the next bigger layer
                             if you wrap an outer volume around an inner 0.5 would lead to a boundary fully in bewteen
                            1. at the outer boundary, 0. at the inner boundary

      @author Andreas.Salzburger@cern.ch

    */
  class ATLAS_NOT_THREAD_SAFE RobustTrackingGeometryBuilderImpl // not safe indexStaticLayers
    : public AthAlgTool
    , public Trk::TrackingVolumeManipulator
    {

    public:
      /** Constructor */
      RobustTrackingGeometryBuilderImpl(const std::string&,const std::string&,const IInterface*);

      /** Destructor */
      virtual ~RobustTrackingGeometryBuilderImpl() = default;

      /** AlgTool initialize method.*/
      virtual StatusCode initialize() override;

    protected:
      /** Private method, creates and packs a triple containing of
       * NegEndcap-Barrel-PosEndcap layers */
      Trk::TrackingVolume* packVolumeTriple(
        const std::vector<Trk::Layer*>& negLayers,
        const std::vector<Trk::Layer*>& centralLayers,
        const std::vector<Trk::Layer*>& posLayers,
        double rMin,
        double rMax,
        double zMin,
        double zPosCentral,
        const std::string& baseName = "UndefinedVolume",
        int colorCode = 21,
        Trk::BinningType bintype = Trk::arbitrary) const;

      /** Private method, creates and packs a triple containing of
       * NegEndcap-Barrel-PosEndcap volumes */
      Trk::TrackingVolume* packVolumeTriple(
        const std::vector<Trk::TrackingVolume*>& negVolumes,
        const std::vector<Trk::TrackingVolume*>& centralVolumes,
        const std::vector<Trk::TrackingVolume*>& posVolumes,
        const std::string& baseName = "UndefinedVolume") const;

      // material configuration
      CxxUtils::CachedUniquePtrT<Trk::Material> m_materialProperties;       //!< overal material properties of the ID

      // Configurable Properties
      // helper tools for the geometry building
      PublicToolHandle<Trk::ITrackingVolumeCreator> m_trackingVolumeCreator{this, "TrackingVolumeCreator", "Trk::CylinderVolumeCreator/CylinderVolumeCreator"}; //!< Helper Tool to create TrackingVolumes
      PublicToolHandle<Trk::ILayerArrayCreator> m_layerArrayCreator{this, "LayerArrayCreator", "Trk::LayerArrayCreator/LayerArrayCreator"}; //!< Helper Tool to create BinnedArrays

      // configurations for the layer builders
      IntegerArrayProperty m_layerBinningType{this, "LayerBinningType", {} };         //!< binning type for the provided layers
      IntegerArrayProperty m_colorCodesConfig{this, "ColorCodes", {} };         //!< Color codes

      // enclosing endcap/cylinder layer
      ServiceHandle<IEnvelopeDefSvc> m_enclosingEnvelopeSvc{this, "EnvelopeDefinitionSvc", "AtlasEnvelopeDefSvc"};                //!< the service to provide the ID envelope size
      DoubleArrayProperty m_enclosingCylinderRadius{this, "VolumeEnclosureCylinderRadii", {} };             //!< the cylinder layer inside the enclosing volume
      DoubleArrayProperty m_enclosingDiscPositionZ{this, "VolumeEnclosureDiscPositions",  {} };              //!< the disc position inside the enclosing volume

      DoubleProperty m_layerEnvelopeCover{this, "EnvelopeCover", 2*Gaudi::Units::mm};       //!< innermost - outermost
      BooleanProperty m_buildBoundaryLayers{this, "BuildBoundaryLayers", true};      //!< create boundary layers
      BooleanProperty m_replaceJointBoundaries{this, "ReplaceAllJointBoundaries", true};   //!< run with replacement of all joint boundaries

      // outer envelope
      DoubleProperty m_outwardsFraction{this, "OutwardsFraction", 0.75};         //!< defines how much you orient yourself in an outwards way (see above)
      // robust layer indexing
      BooleanProperty m_indexStaticLayers{this, "IndexStaticLayers", true};        //!< forces robust indexing for layers
      // naming schema
      StringProperty m_namespace{this, "VolumeNamespace", "InDet::"};                //!< identificaton namespace
      // ID container
      StringProperty m_exitVolume{this, "ExitVolumeName", "InDet::Containers::InnerDetector"};                //!< the final ID container
      BooleanProperty m_isITk{this, "isITk", false};                   //!< changes volume boundary calculation for ITk layouts
  };

} // end of namespace

#endif //INDETTRACKINGGEOMETRY_ROBUSTTRACKINGGEOMETRYBUILDERIMPL_H
