/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackSurfaceIntersection.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk
#include "TrkExUtils/TrackSurfaceIntersection.h"
// Gaudi
#include "GaudiKernel/MsgStream.h"
// STD
#include <iomanip>
#include <iostream>

// constructor
Trk::TrackSurfaceIntersection::TrackSurfaceIntersection(
  const Amg::Vector3D& pos,
  const Amg::Vector3D& dir,
  double path)
  : m_position(pos)
  , m_direction(dir)
  , m_pathlength(path)
{
}

Trk::TrackSurfaceIntersection::TrackSurfaceIntersection(
  const TrackSurfaceIntersection& other)
  : m_position(other.m_position)
  , m_direction(other.m_direction)
  , m_pathlength(other.m_pathlength)
  , m_cache(other.m_cache ? other.m_cache->clone() : nullptr)
{
}

Trk::TrackSurfaceIntersection::TrackSurfaceIntersection(
  const TrackSurfaceIntersection& other,
  std::unique_ptr<IIntersectionCache> cache)
  : m_position(other.m_position)
  , m_direction(other.m_direction)
  , m_pathlength(other.m_pathlength)
  , m_cache(std::move(cache))
{
}

Trk::TrackSurfaceIntersection&
Trk::TrackSurfaceIntersection::operator=(const TrackSurfaceIntersection& other)
{
  if (this != &other) {
    m_position = other.m_position;
    m_direction = other.m_direction;
    m_pathlength = other.m_pathlength;
    m_cache = other.m_cache ? other.m_cache->clone() : nullptr;
  }
  return *this;
}

// Overload of << operator for both, MsgStream and std::ostream for debug output
MsgStream&
Trk::operator<<(MsgStream& sl, const Trk::TrackSurfaceIntersection& tsfi)
{
  const std::streamsize ss = sl.precision();
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << MSG::DEBUG << "Trk::TrackSurfaceIntersection  " << std::endl;
  sl << "    position  [mm]   =  (" << tsfi.position().x() << ", "
     << tsfi.position().y() << ", " << tsfi.position().z() << ")" << std::endl;
  sl << "    direction [mm]   =  (" << tsfi.direction().x() << ", "
     << tsfi.direction().y() << ", " << tsfi.direction().z() << ")"
     << std::endl;
  sl << "    delta pathlength =   " << tsfi.pathlength() << std::endl;
  sl.precision(ss);
  return sl;
}

std::ostream&
Trk::operator<<(std::ostream& sl, const Trk::TrackSurfaceIntersection& tsfi)
{
  const std::streamsize ss = sl.precision();
  sl << std::setiosflags(std::ios::fixed);
  sl << std::setprecision(7);
  sl << "Trk::TrackSurfaceIntersection  " << std::endl;
  sl << "    position  [mm]   =  (" << tsfi.position().x() << ", "
     << tsfi.position().y() << ", " << tsfi.position().z() << ")" << std::endl;
  sl << "    direction [mm]   =  (" << tsfi.direction().x() << ", "
     << tsfi.direction().y() << ", " << tsfi.direction().z() << ")"
     << std::endl;
  sl << "    delta pathlength =   " << tsfi.pathlength() << std::endl;
  sl.precision(ss);
  return sl;
}
