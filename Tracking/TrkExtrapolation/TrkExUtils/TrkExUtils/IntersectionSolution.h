/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IntersectionSolution.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXUTILS_INTERSECTIONSOLUTION_H
#define TRKEXUTILS_INTERSECTIONSOLUTION_H

// Trk

#include <vector>
#include <iosfwd>
#include <memory>
#include <optional>


class MsgStream;

namespace Trk {
class TrackSurfaceIntersection;
  
using IntersectionSolution = std::vector<std::optional<Trk::TrackSurfaceIntersection>> ;
using IntersectionSolutionIter = std::vector<std::optional<Trk::TrackSurfaceIntersection>>::const_iterator;


/**Overload of << operator for both, MsgStream and std::ostream for debug
 * output*/
MsgStream&
operator<<(MsgStream& sl, const IntersectionSolution& sf);
std::ostream&
operator<<(std::ostream& sl, const IntersectionSolution& sf);

} // end of namespace

#endif // TRKEXUTILS_INTERSECTIONSOLUTION_H

