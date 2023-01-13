/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKDETDESCRTOOLS_LAYERPROVIDERIMPL_H
#define TRKDETDESCRTOOLS_LAYERPROVIDERIMPL_H

// Gaudi & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"

namespace Trk {

  class Layer;
  class DiscLayer;
  class CylinderLayer;

/** @class LayerProviderImpl

  Wrapper around an ILayerBuilderImpl to feed into the StagedGeometryBuilderImpl

  @author Andreas.Salzburger@cern.ch
 */
class LayerProviderImpl
  : public AthAlgTool
{

public:
  /** Constructor */
  LayerProviderImpl(const std::string&, const std::string&, const IInterface*);

  /** Destructor */
  virtual ~LayerProviderImpl() = default;

 protected:
  std::pair<const std::vector<Trk::Layer*>, const std::vector<Trk::Layer*> >
  discLayersToEndcapLayers(std::unique_ptr<const std::vector<Trk::DiscLayer*> > discLayers) const;

  const std::vector<Trk::Layer*>
  cylindricalLayersToCentralLayers(std::unique_ptr<const std::vector<Trk::CylinderLayer*> > cylinderLayers) const;
};

} // end of namespace

#endif // TRKDETDESCRTOOLS_LAYERPROVIDERCOND_H

