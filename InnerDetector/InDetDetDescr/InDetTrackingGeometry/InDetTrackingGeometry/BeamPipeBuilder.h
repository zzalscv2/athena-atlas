/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_BEAMPIPEBUILDER_H
#define INDETTRACKINGGEOMETRY_BEAMPIPEBUILDER_H

// Athena
#include "InDetTrackingGeometry/BeamPipeBuilderImpl.h"
// Trk
#include "TrkDetDescrInterfaces/ILayerBuilder.h"

// STL
#include <vector>

namespace Trk {
  class CylinderLayer;
  class DiscLayer;
  class PlaneLayer;
}

namespace InDet {

  /** @class BeamPipeBuilder
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
  class BeamPipeBuilder : public extends <BeamPipeBuilderImpl, Trk::ILayerBuilder> {


  public:
    /** AlgTool style constructor */
    BeamPipeBuilder(const std::string&,const std::string&,const IInterface*);
    /** Destructor */
    virtual ~BeamPipeBuilder() = default;

    /** LayerBuilder interface method - returning Barrel-like layers */
    virtual std::unique_ptr<const std::vector<Trk::CylinderLayer*> >
    cylindricalLayers() const override final;

    /** LayerBuilder interface method - returning Endcap-like layers */
    virtual std::unique_ptr<const std::vector<Trk::DiscLayer*> >
    discLayers() const override final;

    /** LayerBuilder interface method - returning Planar-like layers */
    virtual std::unique_ptr<const std::vector<Trk::PlaneLayer*> >
    planarLayers() const override final;

    /** Name identification */
    virtual const std::string& identification() const override;
  };

  inline std::unique_ptr<const std::vector<Trk::DiscLayer*> > BeamPipeBuilder::discLayers() const
  { return 0; }

  inline std::unique_ptr<const std::vector<Trk::PlaneLayer*> > BeamPipeBuilder::planarLayers() const
  { return 0; }

  inline const std::string& BeamPipeBuilder::identification() const
  { return m_identification; }

} // end of namespace


#endif // INDETTRACKINGGEOMETRY_BEAMPIPEBUILDER_H
