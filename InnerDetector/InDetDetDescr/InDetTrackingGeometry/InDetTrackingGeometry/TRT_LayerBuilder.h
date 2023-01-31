/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_TRT_LAYERBUILDER_H
#define INDETTRACKINGGEOMETRY_TRT_LAYERBUILDER_H

// Athena
#include "InDetTrackingGeometry/TRT_LayerBuilderImpl.h"
// Trk
#include "TrkDetDescrInterfaces/ILayerBuilder.h"
// STL
#include <vector>

namespace InDetDD {
  class TRT_DetectorManager;
}

namespace Trk {
  class CylinderLayer;
  class DiscLayer;
  class PlaneLayer;
}

namespace InDet {

  /** @class TRT_LayerBuilder

      @author Andreas.Salzburger@cern.ch
  */
  class ATLAS_NOT_THREAD_SAFE TRT_LayerBuilder :
    public extends<TRT_LayerBuilderImpl, Trk::ILayerBuilder> {

    /** Declare the TRT_VolumeBuilder as friend */
    friend class TRT_VolumeBuilder;

  public:

    /** AlgTool style constructor */
    TRT_LayerBuilder(const std::string&,const std::string&,const IInterface*);
    /** Destructor */
    virtual ~TRT_LayerBuilder() = default;

    /** AlgTool initialize method */
    virtual StatusCode initialize() override final;

    /** LayerBuilder interface method - returning Barrel-like layers */
    virtual std::unique_ptr<const std::vector<Trk::CylinderLayer*> > cylindricalLayers() const override final;

    /** LayerBuilder interface method - returning Endcap-like layers */
    virtual std::unique_ptr<const std::vector<Trk::DiscLayer*> > discLayers() const override final;

    /** LayerBuilder interface method - returning Planar-like layers */
    virtual std::unique_ptr<const std::vector<Trk::PlaneLayer*> > planarLayers() const override final;

    /** Name identification */
    virtual const std::string& identification() const override final;

  private:

    const InDetDD::TRT_DetectorManager* m_trtMgr{}; //!< the TRT Manager
    StringProperty m_trtMgrLocation{this, "TRT_DetManagerLocation", "TRT"}; //!< the location of the TRT Manager
  };

  inline std::unique_ptr<const std::vector<Trk::PlaneLayer* > > TRT_LayerBuilder::planarLayers() const
  { return nullptr; }

  inline const std::string& TRT_LayerBuilder::identification() const
  { return m_identification; }

} // end of namespace


#endif // INDETTRACKINGGEOMETRY_TRT_LAYERBUILDER_H
