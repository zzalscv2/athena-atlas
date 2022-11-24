/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CompetingPixelClustersOnTrack.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "InDetCompetingRIOsOnTrack/CompetingPixelClustersOnTrack.h"
#include "GaudiKernel/MsgStream.h"
#include "TrkSurfaces/Surface.h"
#include <cmath>
#include <ostream>

// default constructor
InDet::CompetingPixelClustersOnTrack::CompetingPixelClustersOnTrack()
  : Trk::CompetingRIOsOnTrack()
  , m_globalPosition{}
  , m_containedChildRots()
{
}

// copy constructor
InDet::CompetingPixelClustersOnTrack::CompetingPixelClustersOnTrack(
  const InDet::CompetingPixelClustersOnTrack& compROT)
  : Trk::CompetingRIOsOnTrack(compROT)
  , m_globalPosition{}
  , m_containedChildRots()
{
  std::vector<const InDet::PixelClusterOnTrack*>::const_iterator rotIter =
    compROT.m_containedChildRots.begin();
  for (; rotIter != compROT.m_containedChildRots.end(); ++rotIter) {
    m_containedChildRots.push_back((*rotIter)->clone());
  }
  if (compROT.m_globalPosition) {
    m_globalPosition.store(std::make_unique<const Amg::Vector3D>(*compROT.m_globalPosition));
  }
}

// explicit constructor
InDet::CompetingPixelClustersOnTrack::CompetingPixelClustersOnTrack(
  std::vector<const InDet::PixelClusterOnTrack*>&& childrots,
  std::vector<AssignmentProb>&& assgnProb)
  : Trk::CompetingRIOsOnTrack(std::move(assgnProb))
  , m_globalPosition{}
  , m_containedChildRots(std::move(childrots))
{
  // initialize local position and error matrix
  setLocalParametersAndErrorMatrix();
}

InDet::CompetingPixelClustersOnTrack&
InDet::CompetingPixelClustersOnTrack::operator=(
  const InDet::CompetingPixelClustersOnTrack& compROT)
{
  if (this != &compROT) {
    // assignment operator of base class
    Trk::CompetingRIOsOnTrack::operator=(compROT);
    // clear rots
    clearChildRotVector();
    m_containedChildRots.clear();
    if (compROT.m_globalPosition) {
      m_globalPosition.store(std::make_unique<const Amg::Vector3D>(*compROT.m_globalPosition));
    } else if (m_globalPosition) {
      m_globalPosition.release().reset();
    }
    std::vector<const InDet::PixelClusterOnTrack*>::const_iterator rotIter =
      compROT.m_containedChildRots.begin();
    for (; rotIter != compROT.m_containedChildRots.end(); ++rotIter)
      m_containedChildRots.push_back((*rotIter)->clone());
  }
  return (*this);
}

InDet::CompetingPixelClustersOnTrack&
InDet::CompetingPixelClustersOnTrack::operator=(
  InDet::CompetingPixelClustersOnTrack&& compROT) noexcept
{
  if (this != &compROT) {
    // move operator of base class
    Trk::CompetingRIOsOnTrack::operator=(compROT);
    // clear rots
    clearChildRotVector();
    m_containedChildRots = std::move(compROT.m_containedChildRots);
    m_globalPosition = std::move(compROT.m_globalPosition);
  }
  return (*this);
}

InDet::CompetingPixelClustersOnTrack::~CompetingPixelClustersOnTrack()
{
  clearChildRotVector();
}

void
InDet::CompetingPixelClustersOnTrack::clearChildRotVector()
{
  std::vector<const InDet::PixelClusterOnTrack*>::const_iterator rotIter =
    m_containedChildRots.begin();
  for (; rotIter != m_containedChildRots.end(); ++rotIter)
    delete (*rotIter);
}

MsgStream&
InDet::CompetingPixelClustersOnTrack::dump(MsgStream& out) const
{
  using std::ios;
  out << "Trk::CompetingPixelClustersOnTrack with [" << numberOfContainedROTs()
      << "] competing Pixel RIO_OnTrack objects" << std::endl;
  Trk::CompetingRIOsOnTrack::dump(out);
  out << "  - GlobalPosition        : ";
  if (not m_globalPosition)
    out << "null pointer" << std::endl;
  else
    out << *m_globalPosition << endmsg;
  return out;
}

std::ostream&
InDet::CompetingPixelClustersOnTrack::dump(std::ostream& out) const
{
  using std::ios;
  out << "Trk::CompetingPixelClustersOnTrack with [" << numberOfContainedROTs()
      << "] competing Pixel RIO_OnTrack objects" << std::endl;
  Trk::CompetingRIOsOnTrack::dump(out);
  out << "  - GlobalPosition        : ";
  if (not m_globalPosition)
    out << "null pointer" << std::endl;
  else
    out << *m_globalPosition << std::endl;
  return out;
}

// Have all the contained ROTs a common associated surface?
bool
InDet::CompetingPixelClustersOnTrack::ROTsHaveCommonSurface(const bool) const
{
  return true;
}

const Amg::Vector3D&
InDet::CompetingPixelClustersOnTrack::globalPosition() const
{
  if (not m_globalPosition) {
    m_globalPosition.set(std::make_unique<const Amg::Vector3D>(
      associatedSurface().localToGlobal(localParameters())));
  }
  return *m_globalPosition;
}
