/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>
#include <iostream>
#include <memory>
#include "GaudiKernel/SystemOfUnits.h"

#include "TrkParameters/TrackParameters.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkSpacePoint/SpacePoint.h"

#include "TrigInDetRoadPredictorTool.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaBaseComps/AthCheckMacros.h"

#include "TrkSurfaces/Surface.h"
#include "InDetPrepRawData/PixelCluster.h"

TrigInDetRoadPredictorTool::TrigInDetRoadPredictorTool(const std::string& t,
						       const std::string& n,
						       const IInterface*  p ): AthAlgTool(t,n,p)
{
  declareInterface< ITrigInDetRoadPredictorTool >( this );
}

StatusCode TrigInDetRoadPredictorTool::initialize() {

  ATH_CHECK( m_fieldCondObjInputKey.initialize());

  return StatusCode::SUCCESS;
}



int TrigInDetRoadPredictorTool::getRoad(const std::vector<const Trk::SpacePoint*>& seed, std::vector<const InDetDD::SiDetectorElement*>& road, const EventContext& ctx) const {

  //1. get magnetic field

  MagField::AtlasFieldCache fieldCache;

  SG::ReadCondHandle<AtlasFieldCacheCondObj> fieldCondObj{m_fieldCondObjInputKey, ctx};
  if (!fieldCondObj.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCondObjInputKey.key());
    return -1;
  }

  fieldCondObj->getInitializedCache (fieldCache);

  road.clear();

  unsigned int seedSize = seed.size();
 
  if(seedSize < 3) return -2;

  //2. adding spacepoints' DEs

  for(unsigned int spIdx=0;spIdx<seedSize;spIdx++) {
    const Trk::PrepRawData* prd  = seed.at(spIdx)->clusterList().first;
    const InDet::PixelCluster* pPixelHit = dynamic_cast<const InDet::PixelCluster*>(prd);
    road.push_back(pPixelHit->detectorElement());
  }

  return 0;
}
