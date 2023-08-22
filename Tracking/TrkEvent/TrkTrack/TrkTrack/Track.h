/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKTRACK_H
#define TRKTRACK_H

#include <atomic>
#include <iostream>
#include <memory>
#include <vector>

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "CxxUtils/CachedValue.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkEventPrimitives/TrkObjectCounter.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/TrackInfo.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkTrackSummary/TrackSummary.h"
class MsgStream;
class TrackCnv_p1;
class TrackCnv_p2;
class TrackCnv_p3;
class TrackCnv_p4;
class TrackCnv_p12;

namespace Trk {
using TrackStates = DataVector<const TrackStateOnSurface>;
/**
 * @brief The ATLAS Track class.
 *
 * This class is designed to work as a common track class, usable in a
 * wide variety of applications, whilst remaining as simple as possible.
 *
 * A Track is typically constructed via
 *
 * - Trk::TrackInfo which holds information for the fitter employed.
 *
 * - The relevant quality of the fit (Trk::FitQuality)
 *
 * - A DataVector of const Trk::TrackStateOnSurface (TrackStates)
 *   representint the relevant measurements, parameters etc.
 *
 * The usage of DataVector allows to constuct a track from a
 * Datavector<DerivedFromTSOS> where DerivedFromTSOS is a type that derives from
 * Trk::TrackStateOnSurface.
 *
 * A Track will be valid if it holds a non empty DataVector and a non-nullpr
 * FitQuality.
 *
 * This class provides convenient helpers to retrieve and cache
 * - DataVectors (VIEW ELEMENTs) const TrackParameters
 * - DataVectors (VIEW ELEMENTs) const Measurements
 * - DataVectors (VIEW ELEMENTs) const Outliers
 * - The Track Parameter at perigee
 *
 * The above are implemented via lazy initialization of
 * see CxxUtils::CachedValue
 *
 * If the TrackStates get modified the caches can be reset via the
 * resetCaches method
 *
 *  Furthermore a Track can contain a Trk::TrackSummary (ptr so can be nullptr).
 *  This is typically is created via the  TrackSummaryTool.
 *
 * @author edward.moyse@cern.ch
 * @author Kirill.Prokofiev@cern.ch
 * @author Christos Anastopoulos (MT modifications)
 */

class Track : public Trk::ObjectCounter<Trk::Track> {
 public:
  friend class TrackSlimmingTool;

  /**
   * Default constructor
   * Here for POOL and simple tests.
   * The track will return isValid() == false.
   * It is not expected to be used in
   * production.
   */
  Track() = default;

  /**
   * Full constructors
   */
  Track(const TrackInfo& info,
        std::unique_ptr<TrackStates> trackStateOnSurfaces,
        std::unique_ptr<FitQuality> fitQuality);

  Track(const Track& rhs);  //!< copy constructor

  Track& operator=(const Track& rhs);  //!< assignment operator

  Track(Track&& rhs) = default;  //!< move constructor

  Track& operator=(Track&& rhs) = default;  //!< move assignment operator

  virtual ~Track() = default;  //!< destructor

  /**
   * returns true if the track has non-nullptr
   * fitQuality  and
   * non-empty DataVector<const TrackStateOnSurface>
   */
  bool isValid() const;

  /**
   * return a pointer to the fit quality const-overload
   */
  const FitQuality* fitQuality() const;
  /**
   * return a pointer to the fit quality non-const overload
   */
  FitQuality* fitQuality();
  /**
   * set FitQuality.
   */
  void setFitQuality(std::unique_ptr<FitQuality> quality);

  /**
   * return a pointer to a const DataVector of const TrackStateOnSurfaces.
   * const overload
   */
  const DataVector<const TrackStateOnSurface>* trackStateOnSurfaces() const;
  /**
   * return a pointer to a DataVector of const TrackStateOnSurfaces.
   * non-const overload
   */
  DataVector<const TrackStateOnSurface>* trackStateOnSurfaces();

  /**
   * Set the TrackStateOnSurfaces.
   */
  void setTrackStateOnSurfaces(
      std::unique_ptr<DataVector<const Trk::TrackStateOnSurface>> input);

  /**
   * Returns a const ref to info of a const tracks.
   */
  const TrackInfo& info() const;

  /**
   * returns a ref to the  info. non-const overload
   */
  TrackInfo& info();

  /**
   * set the info.
   */
  void setInfo(const TrackInfo& input);

  /**
   * Returns  a pointer to the const Trk::TrackSummary owned by this const track
   * (could be nullptr)
   */
  const Trk::TrackSummary* trackSummary() const;

  /**
   * Returns a  pointer to the Trk::TrackSummary owned by this track (could be
   * nullptr)
   */
  Trk::TrackSummary* trackSummary();

  /**
   * Set the track summary.
   */
  void setTrackSummary(std::unique_ptr<Trk::TrackSummary> input);

  /**
   * return Perigee. Can be nullptr if no perigee parameters
   * were assigned to the Track.
   *
   * This method performs lazy initialization and caches the result.
   *
   * <b>PLEASE NOTE!</b>
   * if there is more than one Perigee in trackStateOnSurfaces
   * (which there shouldn't be!!), only the first one will be
   * returned by Trk::Track::perigeeParameters
   * Although the Perigee is just a type of TrackParameter, it has a
   * dedicated method because of the specific physics interest
   */
  const Perigee* perigeeParameters() const;

  /**
   * Return a pointer to a vector of TrackParameters.
   * It is created Lazily by this method	and then cached.
   *
   * @return Pointer to a DV of TrackParameters, or 0. The TrackParameters
   *	are not owned by the DV (it is a view)
   */
  const DataVector<const TrackParameters>* trackParameters() const;

  /**
   * return a pointer to a vector of MeasurementBase (*NOT* including
   * any that come from outliers). This DataVector is lazily created
   * by this method and cached.
   *
   * @return Pointer to a DV of MeasurementBase. The MeasurementBases
   *	 are not owned by the DV (it is a view)
   */
  const DataVector<const MeasurementBase>* measurementsOnTrack() const;

  /**
   * return a pointer to a vector of MeasurementBase, which represent
   * outliers (i.e. measurements  not used in the track fit). This
   * DataVector is created lazily by this method and then cached.
   *
   * @return Pointer to a DV of MeasurementBase, representing outliers.
   *	  The MeasurementBases are not owned by the DV (it is a view)
   */
  const DataVector<const MeasurementBase>* outliersOnTrack() const;

  /**
   * reset all caches
   */
  void resetCaches();

 protected:
  friend class ::TrackCnv_p1;
  friend class ::TrackCnv_p2;
  friend class ::TrackCnv_p3;
  friend class ::TrackCnv_p4;
  friend class ::TrackCnv_p12;

  typedef DataVector<const TrackStateOnSurface>::const_iterator TSoS_iterator;
  typedef DataVector<const TrackParameters>::const_iterator TP_iterator;

  /**
   * Find perigee in the vector of track parameters.
   * It can be used to lazy-init the m_perigeeParameters
   */
  void findPerigee() const;

  /**
   * Helper method to factor common
   * part of copy ctor and copy assignment
   */
  void copyHelper(const Track& rhs);

  /**
   * TrackStateOnSurface
   *
   * These objects link the various parameters related to a surface,
   * for example, TrackParameter, RIO_OnTrack and FitQualityOnSurface
   */
  std::unique_ptr<TrackStates> m_trackStateVector = nullptr;

  /**
   * A vector of TrackParameters: these can be any of the classes that
   * derive from Trk::TrackParameters, for example, Perigee,
   * MeasuredPerigee, AtaCylinder etc.
   *
   * It is created in the return method by looping over all
   * Trk::TrackStateOnSurface adding their pointers to the payload
   * of m_cachedParameterVector
   */
  CxxUtils::CachedValue<DataVector<const TrackParameters>>
      m_cachedParameterVector{};

  /**
   * A vector of MeasurementBase: these objects represent the "hits" on
   * the track (but not outliers - see m_cachedOutlierVector)
   *
   * It is created in the return method by looping over all
   * Trk::TrackStateOnSurface adding their pointers to the payload of
   * m_cachedMeasurementVector
   */
  CxxUtils::CachedValue<DataVector<const MeasurementBase>>
      m_cachedMeasurementVector{};

  /**
   * These objects represent the "outliers" on the track.
   * It is created in the return method by looping over all
   * Trk::TrackStateOnSurface adding their pointers to  the payload of
   * m_cachedRioVector
   */
  CxxUtils::CachedValue<DataVector<const MeasurementBase>>
      m_cachedOutlierVector{};

  /**
   * A pointer to the Track's Perigee parameters.
   *
   * This will be null if the track does not contain a Perigee
   * or MeasuredPerigee parameter
   */
  CxxUtils::CachedValue<const Perigee*> m_perigeeParameters{};

  /**
   * A pointer to the Track's FitQuality.
   */
  std::unique_ptr<FitQuality> m_fitQuality{nullptr};

  /**
   * Datamember to cache the TrackSummary
   */
  std::unique_ptr<Trk::TrackSummary> m_trackSummary{nullptr};

  /**
   * This is a class which stores the identity of where the track
   * was created, fitted, which properties the reconstruction had
   */
  Trk::TrackInfo m_trackInfo{};

 private:
  /**
   * find PerigeeImpl.
   * Assumes that Perigee parameters are currently inValid.
   */
  void findPerigeeImpl() const;

};  // end of class definitions

/**
 * Overload of << operator for MsgStream for debug output
 */
MsgStream& operator<<(MsgStream& sl, const Track& track);

/**
 * Overload of << operator for std::ostream for debug output
 */
std::ostream& operator<<(std::ostream& sl, const Track& track);

}  // namespace Trk

#include "Track.icc"
#endif

