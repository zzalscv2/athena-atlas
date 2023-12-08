/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// FitterTypes.h
//   Header file to encapsulate some standard types
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Markus.Elsing at cern.ch
///////////////////////////////////////////////////////////////////

#ifndef TRK_FITTERTYPES_H
#define TRK_FITTERTYPES_H

#include <vector>

namespace Trk {

//! switch to toggle quality processing after fit
typedef bool RunOutlierRemoval;

class PrepRawData;
//! vector of clusters and drift circles
using PrepRawDataSet = std::vector<const PrepRawData*>;

class MeasurementBase;
//! vector of fittable measurements
using MeasurementSet = std::vector<const MeasurementBase*>;

class RIO_OnTrack;
//! vector of detector hits on a track
using RIO_OnTrackSet = std::vector<const RIO_OnTrack*>;

class SpacePoint;
//! vector of space points
using SpacePointSet = std::vector<const SpacePoint*>;

}  // namespace Trk

#endif  // TRK_FITTERTYPES_H
