/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_ROBUSTTRACKINGGEOMETRYBUILDERCOND_H
#define INDETTRACKINGGEOMETRY_ROBUSTTRACKINGGEOMETRYBUILDERCOND_H

// InDet
#include "InDetTrackingGeometry/RobustTrackingGeometryBuilderImpl.h"
//Trk
#include "TrkDetDescrInterfaces/IGeometryBuilderCond.h"
#include "TrkDetDescrInterfaces/ILayerBuilderCond.h"
#include "TrkDetDescrUtils/BinningType.h"
#include "TrkGeometry/TrackingVolumeManipulator.h"
#include "TrkGeometry/TrackingGeometry.h"
// Gaudi
#include "GaudiKernel/ToolHandle.h"
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


  /** @class RobustTrackingGeometryBuilderCond

      New Geometry builder that adapts to different layer setups

      Only a few parameters are not automated:
       - m_outwardsFraction: this defines how much you orient yourself on the next bigger layer
                             if you wrap an outer volume around an inner 0.5 would lead to a boundary fully in bewteen
                            1. at the outer boundary, 0. at the inner boundary

      @author Andreas.Salzburger@cern.ch

    */
  class ATLAS_NOT_THREAD_SAFE RobustTrackingGeometryBuilderCond // not safe indexStaticLayers
    : public extends<InDet::RobustTrackingGeometryBuilderImpl, Trk::IGeometryBuilderCond>
    {

    public:
      /** Constructor */
      RobustTrackingGeometryBuilderCond(const std::string&,const std::string&,const IInterface*);

      /** Destructor */
      virtual ~RobustTrackingGeometryBuilderCond() = default;

      /** AlgTool initialize method.*/
      virtual StatusCode initialize() override final;

      /** TrackingGeometry Interface methods */
      virtual std::unique_ptr<Trk::TrackingGeometry> trackingGeometry(
        const EventContext& ctx,
        Trk::TrackingVolume* tVolPair,
        SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const override final;
      /** The unique signature */
      virtual Trk::GeometrySignature geometrySignature() const override final { return Trk::ID; }

    private:
      // Configurable Properties

      // helper tools for the geometry building
      PublicToolHandle<Trk::ILayerBuilderCond> m_beamPipeBuilder{this, "BeamPipeBuilder", "InDet::BeamPipeBuilder/AtlasBeamPipeBuilder"}; //!< BeamPipe builder (is different from layers)
      PublicToolHandleArray<Trk::ILayerBuilderCond> m_layerBuilders{this, "LayerBuilders",  {} };            //!< Helper Tools for the Layer creation
  };

} // end of namespace

#endif //INDETTRACKINGGEOMETRY_ROBUSTTRACKINGGEOMETRYBUILDERCOND_H
