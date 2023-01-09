/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_TRT_LAYERBUILDERCOND_H
#define INDETTRACKINGGEOMETRY_TRT_LAYERBUILDERCOND_H

// Athena
#include "InDetTrackingGeometry/TRT_LayerBuilderImpl.h"
// Trk
#include "TrkDetDescrInterfaces/ILayerBuilderCond.h"
#include "TrkGeometry/TrackingGeometry.h"
// InDet
#include "TRT_ReadoutGeometry/TRT_DetElementContainer.h"
// StoreGate
#include "StoreGate/ReadCondHandleKey.h"
// STL
#include <vector>

namespace Trk {
  class CylinderLayer;
  class DiscLayer;
  class PlaneLayer;
}

namespace InDet {

  /** @class TRT_LayerBuilderCond

      @author Andreas.Salzburger@cern.ch
  */
  class ATLAS_NOT_THREAD_SAFE TRT_LayerBuilderCond :  //const_cast
    public extends<TRT_LayerBuilderImpl, Trk::ILayerBuilderCond> {

    /** Declare the TRT_VolumeBuilder as friend */
    friend class TRT_VolumeBuilder;

  public:

    /** AlgTool style constructor */
    TRT_LayerBuilderCond(const std::string&,const std::string&,const IInterface*);

    /** Destructor */
    virtual ~TRT_LayerBuilderCond() = default;

    /** AlgTool initialize method */
    virtual StatusCode initialize() override final;

    /** LayerBuilderCond interface method - returning Barrel-like layers */
    virtual std::unique_ptr<const std::vector<Trk::CylinderLayer*> >
      cylindricalLayers(const EventContext& ctx,
                        SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const override final;

    /** LayerBuilderCond interface method - returning Endcap-like layers */
    virtual std::unique_ptr<const std::vector<Trk::DiscLayer*> >
      discLayers(const EventContext& ctx,
                 SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const override final;

    /** LayerBuilderCond interface method - returning Planar-like layers */
    virtual std::unique_ptr<const std::vector<Trk::PlaneLayer*> >
      planarLayers(const EventContext& ctx,
                   SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const override final;

    /** Name identification */
    virtual const std::string& identification() const override final;

  private:

    SG::ReadCondHandleKey<InDetDD::TRT_DetElementContainer>
      m_readKeyTRTContainer{
      this,
        "ReadKeyTRTDetectorElements",
        "TRT_DetElementContainer",
        "Key for input TRT detector element container read from cond store"
        };
  };

  inline std::unique_ptr<const std::vector<Trk::PlaneLayer*> >
  TRT_LayerBuilderCond::planarLayers(const EventContext&,
                                     SG::WriteCondHandle<Trk::TrackingGeometry>& /*whandle*/) const
  { return nullptr; }

  inline const std::string& TRT_LayerBuilderCond::identification() const
  { return m_identification; }

} // end of namespace


#endif // INDETTRACKINGGEOMETRY_TRT_LAYERBUILDERCOND_H
