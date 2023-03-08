/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDER_H
#define INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDER_H

//Trk
#include "TrkDetDescrInterfaces/IGeometryBuilder.h"
#include "TrkDetDescrInterfaces/ILayerProvider.h"
#include "TrkDetDescrUtils/BinningType.h"
// Athena
#include "StagedTrackingGeometryBuilderImpl.h"
// Gaudi
#include "GaudiKernel/ToolHandle.h"
// STL
#include <vector>
#include <string>

#include "CxxUtils/checker_macros.h"


namespace Trk {
 class TrackingGeometry;
 class Layer;
 class Material;
}

namespace InDet {

  /** @class StagedTrackingGeometryBuilder

      New Geometry builder that adapts to different layer setups

      Only a few parameters are not automated:
       - m_outwardsFraction: this defines how much you orient yourself on the next bigger layer
                             if you wrap an outer volume around an inner 0.5 would lead to a boundary fully in bewteen
                            1. at the outer boundary, 0. at the inner boundary

      @author Andreas.Salzburger@cern.ch

    */

  class ATLAS_NOT_THREAD_SAFE StagedTrackingGeometryBuilder : // const_cast
    public extends <StagedTrackingGeometryBuilderImpl, Trk::IGeometryBuilder> {


    public:
      /** Constructor */
      StagedTrackingGeometryBuilder(const std::string&,const std::string&,const IInterface*);

      /** Destructor */
      virtual ~StagedTrackingGeometryBuilder() = default;

      /** AlgTool initialize method.*/
      virtual StatusCode initialize() override final;

      /** TrackingGeometry Interface methods */
      virtual
        std::unique_ptr<Trk::TrackingGeometry> trackingGeometry(
                                                                Trk::TrackingVolume* tvol = 0
                                                                ) const override final;
      /** The unique signature */
      virtual Trk::GeometrySignature geometrySignature() const override final { return Trk::ID; }

    private:
      // helper tools for the geometry building
      ToolHandleArray<Trk::ILayerProvider>           m_layerProviders;          //!< Helper Tools for the Layer creation, includes beam pipe builder
  };

} // end of namespace

#endif //INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDER_H
