/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackFitInputPreparator, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRK_TRACKFITINPUTPREPARATOR_H
#define TRK_TRACKFITINPUTPREPARATOR_H

#include "AthContainers/DataVector.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkFitterUtils/FitterTypes.h"
#include "TrkTrack/TrackStateOnSurface.h"

namespace Trk {

class Track;
typedef bool SortInputFlag;  //!< switch to toggle sorting
typedef DataVector<const TrackStateOnSurface>::const_iterator TS_iterator;
typedef std::vector<std::pair<const Trk::MeasurementBase *, int> >
    MB_IndexVector;

/** @brief Helpers  to transform combinations of tracking input
    (for example, existing tracks plus additional measurements) into
    a digestable fitter input: either tracks, measurement-sets or
    proto-state vectors.

    @author Wolfgang Liebig <http://consult.cern.ch/xwho/people/54608>
  */

namespace TrackFitInputPreparator {

/** @brief get the MeasurementSet out of a track+measurements combination.*/
MeasurementSet stripMeasurements(const Track &, const MeasurementSet &);

/** @brief create a vector of PrepRawData* from the mixed input of track
    and single PrepRawData. Switch for applying a sort function as well
    as re-integration of outliers onto the input track. */
PrepRawDataSet stripPrepRawData(const Track &, const PrepRawDataSet &,
                                const SortInputFlag, const bool);
};  // namespace TrackFitInputPreparator
}  // namespace Trk

#endif  // TRK_TRACKFITINPUTPREPARATOR_H
