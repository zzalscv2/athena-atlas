/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackFitInputPreparator, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRK_TRACKFITINPUTPREPARATOR_H
#define TRK_TRACKFITINPUTPREPARATOR_H

//Trk
#include "GaudiKernel/StatusCode.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkFitterUtils/FitterTypes.h"
#include "TrkEventUtils/TrkParametersComparisonFunction.h" // is a typedef
#include "GeoPrimitives/GeoPrimitives.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"

namespace Trk {

  class Track;
  typedef bool SortInputFlag;    //!< switch to toggle sorting
  typedef DataVector<const TrackStateOnSurface>::const_iterator TS_iterator;
  typedef std::vector<std::pair<const Trk::MeasurementBase*, int> > MB_IndexVector;

  /** @brief Helpers  to transform combinations of tracking input
      (for example, existing tracks plus additional measurements) into
      a digestable fitter input: either tracks, measurement-sets or
      proto-state vectors.

      @author Wolfgang Liebig <http://consult.cern.ch/xwho/people/54608>
    */

  namespace TrackFitInputPreparator {
      
    /** @brief fill a new track object from track+measurements using flags
        for sorting and outliers. This method is a factory, that is the
        client needs to take care of deleting the track. */
     Trk::Track *copyToTrack(const Track &, const MeasurementSet &,
                                   const SortInputFlag, const bool);

     /** @brief get the MeasurementSet out of a track+measurements combination.

         IMPORTANT: take this interface if you know there are no outliers on
         the track! Uses flags for sorting and outliers. */
     MeasurementSet stripMeasurements(const Track &, const MeasurementSet &);

     /** @brief create a vector of PrepRawData* from the mixed input of track
         and single PrepRawData. Switch for applying a sort function as well
         as re-integration of outliers onto the input track. */
     PrepRawDataSet stripPrepRawData(const Track &, const PrepRawDataSet &,
                                     const SortInputFlag, const bool);
     };  // namespace TrackFitInputPreparator
}


#endif // TRK_TRACKFITINPUTPREPARATOR_H
