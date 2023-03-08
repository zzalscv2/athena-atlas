/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDERCOND_H
#define INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDERCOND_H

//Trk
#include "TrkDetDescrInterfaces/IGeometryBuilderCond.h"
#include "TrkDetDescrInterfaces/ILayerProviderCond.h"
#include "TrkDetDescrUtils/BinningType.h"
#include "TrkGeometry/TrackingGeometry.h"
//InDet
#include "StagedTrackingGeometryBuilderImpl.h"
// Athena
#include "CxxUtils/checker_macros.h"
// Gaudi
#include "GaudiKernel/ToolHandle.h"
// STL
#include <vector>
#include <string>


namespace Trk {
 class Layer;
 class Material;
}

namespace InDet {
  /** @class StagedTrackingGeometryBuilderCond

      New Geometry builder that adapts to different layer setups

      Only a few parameters are not automated:
       - m_outwardsFraction: this defines how much you orient yourself on the next bigger layer
                             if you wrap an outer volume around an inner 0.5 would lead to a boundary fully in bewteen
                            1. at the outer boundary, 0. at the inner boundary

      @author Andreas.Salzburger@cern.ch

    */

  class ATLAS_NOT_THREAD_SAFE StagedTrackingGeometryBuilderCond : //const_cast
    public extends<StagedTrackingGeometryBuilderImpl, Trk::IGeometryBuilderCond> {

    public:
      /** Constructor */
      StagedTrackingGeometryBuilderCond(const std::string&,const std::string&,const IInterface*);

      /** Destructor */
      virtual ~StagedTrackingGeometryBuilderCond() = default;

      /** AlgTool initialize method.*/
      virtual StatusCode initialize() override final;

      /** TrackingGeometry Interface methods */
      virtual
      std::unique_ptr<Trk::TrackingGeometry> trackingGeometry(
                                                              const EventContext& ctx,
                                                              Trk::TrackingVolume* tVol,
                                                              SG::WriteCondHandle<Trk::TrackingGeometry>& whandle
                                                              ) const override final;
      /** The unique signature */
      virtual Trk::GeometrySignature geometrySignature() const override { return Trk::ID; }

    private:
      // helper tools for the geometry building
      ToolHandleArray<Trk::ILayerProviderCond>       m_layerProviders;          //!< Helper Tools for the Layer creation, includes beam pipe builder
  };

} // end of namespace

#endif //INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDERCOND_H
