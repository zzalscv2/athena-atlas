/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETTRACKFITTER_TRIGINDETROADPREDICTORTOOL_H
#define TRIGINDETTRACKFITTER_TRIGINDETROADPREDICTORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

#include "TrigInDetToolInterfaces/ITrigInDetRoadPredictorTool.h"

// MagField cache
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"

namespace Trk {	
  class SpacePoint;
}

class TrigInDetRoadPredictorTool: public AthAlgTool, virtual public ITrigInDetRoadPredictorTool
{
 public:
  TrigInDetRoadPredictorTool( const std::string&, const std::string&, const IInterface* );
  virtual StatusCode initialize() override;

  virtual int getRoad(const std::vector<const Trk::SpacePoint*>&, std::vector<const InDetDD::SiDetectorElement*>&, const EventContext&) const override;

private:
  
  Gaudi::Property<int> m_nClustersMin {this, "nClustersMin", 7, "Minimum number of clusters on track"};

  SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCondObjInputKey{
    this, "AtlasFieldCacheCondObj", "fieldCondObj",
    "Name of the Magnetic Field conditions object key"};
    
};

#endif
