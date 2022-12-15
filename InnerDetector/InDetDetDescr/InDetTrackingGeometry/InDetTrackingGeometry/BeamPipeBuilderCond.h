/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_BEAMPIPEBUILDERCOND_H
#define INDETTRACKINGGEOMETRY_BEAMPIPEBUILDERCOND_H

// Athena
#include "InDetTrackingGeometry/BeamPipeBuilderImpl.h"
// Trk
#include "TrkDetDescrInterfaces/ILayerBuilderCond.h"

#include "TrkGeometry/TrackingGeometry.h"
// STL
#include <vector>
#include <utility>

namespace Trk {
  class CylinderLayer;
  class DiscLayer;
  class PlaneLayer;
}

namespace InDet {

  /** @class BeamPipeBuilderCond
      Simple LayerBuilder for the BeamPipe,
      can be configured through jobOptions:
      - radius
      - halflength
      - thickness
      - MaterialProperties

      later on the slight shift/rotation of the BeamPipe can be implemented
      - make a binding to the database afterwards

      @author Andreas.Salzburger@cern.ch
  */
  class BeamPipeBuilderCond : public extends <BeamPipeBuilderImpl, Trk::ILayerBuilderCond> {


  public:
    /** AlgTool style constructor */
    BeamPipeBuilderCond(const std::string&,const std::string&,const IInterface*);
    /** Destructor */
    virtual ~BeamPipeBuilderCond() = default;

    /** LayerBuilder interface method - returning Barrel-like layers */
    virtual std::unique_ptr<const std::vector<Trk::CylinderLayer*> >
    cylindricalLayers(const EventContext& ctx,
                      SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const override final;

    /** LayerBuilder interface method - returning Endcap-like layers */
    virtual std::unique_ptr<const std::vector<Trk::DiscLayer*> >
    discLayers(const EventContext& ctx,
               SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const override final;

    /** LayerBuilder interface method - returning Planar-like layers */
    virtual std::unique_ptr<const std::vector<Trk::PlaneLayer*> >
    planarLayers(const EventContext& ctx,
                 SG::WriteCondHandle<Trk::TrackingGeometry>& whandle) const override final;

    /** Name identification */
    virtual const std::string& identification() const override;
  };

  inline const std::string& BeamPipeBuilderCond::identification() const
  { return m_identification; }

  inline std::unique_ptr<const std::vector<Trk::DiscLayer*> >
  BeamPipeBuilderCond::discLayers(const EventContext&,
                                  SG::WriteCondHandle<Trk::TrackingGeometry>& /*whandle*/) const
  {
    return nullptr;
  }

  inline std::unique_ptr<const std::vector<Trk::PlaneLayer*> >
  BeamPipeBuilderCond::planarLayers(const EventContext&,
                                    SG::WriteCondHandle<Trk::TrackingGeometry>& /*whandle*/) const
  {
    return nullptr;
  }

 } // end of namespace


#endif // INDETTRACKINGGEOMETRY_BEAMPIPEBUILDERCOND_H
