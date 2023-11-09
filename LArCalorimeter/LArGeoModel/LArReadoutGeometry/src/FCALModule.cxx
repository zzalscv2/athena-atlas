/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "LArReadoutGeometry/FCALModule.h"
#include "LArReadoutGeometry/FCAL_ChannelMap.h"
#include "LArReadoutGeometry/FCALDetectorManager.h"

#include "GeoModelKernel/GeoVFullPhysVol.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoTubs.h"
#include "GeoModelUtilities/GeoAlignmentStore.h"

#include <algorithm>

bool operator < (unsigned int i, const FCALTile &t) {
  return i < t.identify();
}

bool operator < (const FCALTile &t, unsigned int i) {
  return t.identify() < i;
}



// Class FCALModule 

FCALModule::FCALModule (const GeoVFullPhysVol *physVol, Module module, Endcap endcap, double projectivityDisplacement)
  : GeoVDetectorElement(physVol)
  , m_Mod(module)
  , m_EC(endcap)
  , m_manager(nullptr)
  , m_dz(((const GeoTubs *) physVol->getLogVol()->getShape())->getZHalfLength()*2.0)
  , m_projectivityDisplacement(projectivityDisplacement)
{

}


FCALModule::~FCALModule()
= default;



FCALModule::ConstIterator FCALModule::beginTiles () const
{
 return m_tileList.begin();
}

FCALModule::ConstIterator FCALModule::endTiles () const
{
 return m_tileList.end();
}

const FCALTile* FCALModule::getTile (int i, int j) const
{

  unsigned int id = (j<<16) | i;

  std::vector<FCALTile>::const_iterator it = lower_bound(m_tileList.begin(),m_tileList.end(), id);
  if ((*it).getIndexI()!=i || (*it).getIndexJ()!=j) {
    return nullptr;
  }
  return &*it;

}

FCALModule::Endcap FCALModule::getEndcapIndex () const
{
  return m_EC;
}

FCALModule::Module FCALModule::getModuleIndex () const
{
  return m_Mod;
}

double FCALModule::getFullWidthX (const FCALTile& tile) const
{
  return getFullWidths (tile.getNumTubes()).first;
}

double FCALModule::getFullWidthY (const FCALTile& tile) const
{
  return getFullWidths (tile.getNumTubes()).second;
}

double FCALModule::getFullDepthZ (const FCALTile& ) const
{
  return m_dz;
}

const Amg::Transform3D&  FCALModule::getAbsoluteTransform (const GeoAlignmentStore* alignStore) const
{
  const GeoVFullPhysVol *fullPhysVol = getMaterialGeom();
  return alignStore
    ? fullPhysVol->getCachedAbsoluteTransform(alignStore)
    : fullPhysVol->getAbsoluteTransform();
}

const Amg::Transform3D&  FCALModule::getDefAbsoluteTransform (const GeoAlignmentStore* alignStore) const
{
  const GeoVFullPhysVol *fullPhysVol = getMaterialGeom();
  return alignStore
    ? fullPhysVol->getCachedDefAbsoluteTransform(alignStore)
    : fullPhysVol->getDefAbsoluteTransform();
}

void FCALModule::setManager (FCALDetectorManager* fcalManager)
{
  m_manager = fcalManager;
  const FCAL_ChannelMap *cMap = m_manager->getChannelMap();
  FCAL_ChannelMap::tileMap_const_iterator t,begin=cMap->begin(m_Mod),end=cMap->end(m_Mod);
  for (t=begin;t!=end;++t) {
    FCALTile tile(this,t);
    m_tileList.push_back(tile);
  }
  std::sort(m_tileList.begin(),m_tileList.end());
}


const FCALModule::tubexy_t&
FCALModule::getFullWidths (unsigned int ntubes) const
{
  if (ntubes > MAXTUBES) std::abort();
  if (!m_tileSizes[ntubes-1].isValid()) {
    const FCAL_ChannelMap *cMap=m_manager->getChannelMap();
    float dx,dy;
    cMap->tileSize(m_Mod,ntubes,dx,dy);
    m_tileSizes[ntubes-1].set (std::make_pair (dx, dy));
  }
  return *m_tileSizes[ntubes-1].ptr();
}
