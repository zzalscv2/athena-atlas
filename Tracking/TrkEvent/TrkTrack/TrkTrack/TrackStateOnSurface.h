/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/***************************************************************************
            TrackStateOnSurface.h  -  description
            -------------------
begin                : Wed Jan 21 2004
email                : edward.moyse@cern.ch
***************************************************************************/

#ifndef TRKTRACKSTATEONSURFACE_H
#define TRKTRACKSTATEONSURFACE_H

//
#include "TrkEventPrimitives/FitQualityOnSurface.h"
#include "TrkMaterialOnTrack/MaterialEffectsBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkEventPrimitives/FitQualityOnSurface.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkTrack/AlignmentEffectsOnTrack.h"
//
//
#include <atomic>
#include <cstdint>
#include <bitset>
#include <iostream>
#include <memory>

class MsgStream;
class TrackCollectionCnv;
class TrackStateOnSurfaceCnv_p3;

namespace Trk {
/**
 * @brief represents the track state (measurement, material, fit parameters
 *          and quality) at a surface.
 *
 * As the name suggests TrackStateOnSurface is intended to represent
 * the state of a track at a surface, and will be of particular
 * relevance for track fitting, extrapolating and following (for
 * example it provides a way to store scattering angle information).
 *
 * The principle is that these objects can contain pointers to other
 * objects. Not all are required to be passed, and so not all will
 * exist. As a rule of thumb, if an object is returned by pointer
 * then you cannot be sure it exists and should check.
 *
 * The code does not (for speed reasons) enforce contraints on what
 * is placed in a TrackStateOnSurface. What follows are simple guidelines
 * for the use of TrackStateOnSurface.
 *
 * - DO NOT create separate TrackStateOnSurface for the same surface
 *   i.e. put all the information for one surface into the same
 *   TrackStateOnSurface (to achieve what you want, have a look at
 *   Trk::CompetingRIO_OnTracks). Do not create one TSoS for the
 *   parameters, one for the MeasurementBase and one for the scatterer.
 *   They should all be together, in the same TSoS, if they exist on
 *   the same Trk::Surface.
 * - An MeasurementBase or MaterialEffectsBase *can* exist without a
 *   matching Trk::TrackParameters e.g. if the Track is meant to be
 *   used as input to track fitting. Otherwise and if possible add
 *   the matching Trk::TrackParameters and Trk::FitQualityOnSurface.
 * - A Trk::MaterialEffectsBase *must* have a matching
 *   Trk::TrackParameters. Otherwise information such as the changes
 *   in direction is meaningless without the coordinates provided
 *   by a Trk::TrackParameters
 *
 * @author edward.moyse@cern.ch
 * @author Chistos Anastopoulos Athena MT migration
 */
class TrackStateOnSurface
{
  friend class ::TrackCollectionCnv;
  friend class ::TrackStateOnSurfaceCnv_p3;

public:
  /*
   * Historically, we had a single bitset.
   *
   * for types and hints.
   * The relevant indices in the bitset were controlled
   * by a single enum.
   *
   * For MT we split the hints from the types.
   * So we have two enums controlling two
   * bitsets.
   *
   * The types can be queried as before
   * from user code.
   *
   * The Hints exist only for
   * interacting with persistification
   * In order to support existing code
   * They are atomic and are to be set
   * only once for each TSOS.
   * they had to remain part of TSOS.
   */
  enum TrackStateOnSurfaceType
  {
    /** This is a measurement, and will at least contain a
         Trk::MeasurementBase*/
    Measurement = 0,

    /** This represents inert material, and so will contain MaterialEffectsBase
     */
    InertMaterial = 1,

    /** This represents a brem point on the track,
     * and so will contain TrackParameters and MaterialEffectsBase */
    BremPoint = 2,

    /** This represents a scattering point on the track,
     * and so will contain TrackParameters and MaterialEffectsBase */
    Scatterer = 3,

    /** This represents a perigee, and so will contain a Perigee
     * object only*/
    Perigee = 4,

    /** This TSoS contains an outlier, that is, it contains a
     * MeasurementBase/RIO_OnTrack which was not used in the track
     * fit*/
    Outlier = 5,

    /** A hole on the track - this is defined in the following way.
     * A hole is a missing measurement BETWEEN the first and last
     * actual measurements. i.e. if your track starts in the SCT,
     * you should not consider a missing b-layer hit as a hole.*/
    Hole = 6,

    /** For some reason this does not fall into any of the other categories
     * PLEASE DO NOT USE THIS - DEPRECATED!*/
    Unknown = 7,

    /** This TSOS contains a CaloEnergy object*/
    CaloDeposit = 8,

    /**
     * This TSOS contains a Trk::ParameterBase
     */
    Parameter = 9,

    /**
     * This TSOS contains a Trk::FitQualityOnSurface
     */
    FitQuality = 10,

    /**
     * This TSOS contains a Trk::AlignmentEffectsOnTrack
     */
    Alignment = 11,

    NumberOfTrackStateOnSurfaceTypes = 12
  };

  enum PersistencyHint
  {

    PartialPersistification = 0,
    /**
     *  Mark the measuremenet for persistification
     */
    PersistifyMeasurement = 1,

    /**
     *  Mark track parameters for persisitification
     */
    PersistifyTrackParameters = 2,

    /**
     *  Mark track parameters for persisitification
     */
    PersistifySlimCaloDeposit = 3,

    NumberOfPersistencyHints = 4
  };

  /*
   * Helpers to join and split the bitsets
   * for the types and hints
   * Needed in order to keep the same T/P separation.
   * Note that in the Persistent side we keep a single ulong
   */
  static unsigned int long joinBitsets(
    const std::bitset<NumberOfTrackStateOnSurfaceTypes>& types,
    const std::bitset<NumberOfPersistencyHints>& hints)
  {
    //put hints in place
    unsigned int long res = hints.to_ulong();
    //shift them up
    res = (res << NumberOfTrackStateOnSurfaceTypes);
    //Add type
    res += types.to_ulong();
    return res;
  }

  static void splitToBitsets(
    const unsigned int long input,
    std::bitset<NumberOfTrackStateOnSurfaceTypes>& types,
    std::bitset<NumberOfPersistencyHints>& hints)
  {
    // mask with NumberOfTrackStateOnSurfaceTypes bits set to 1
    constexpr unsigned int long maskTypesSet =
      (1 << NumberOfTrackStateOnSurfaceTypes) - 1;
    //keep just the Type part of the input
    types = std::bitset<NumberOfTrackStateOnSurfaceTypes>(input & maskTypesSet);
    //keep the upper Hints part of the input
    hints = std::bitset<NumberOfPersistencyHints>(
      input >> NumberOfTrackStateOnSurfaceTypes);
  }

  enum Variety
  {
    SingleComponent = 0,
    MultiComponent = 1,
    Align = 2,
  };
  /**
   * Default ctor for POOL. Do not use!
   */
  TrackStateOnSurface();

  /**
   * Partial constructors
   *
   * The objects passed in belong to the this object, or to the Track
   * to which this FQOS will be assigned.
   *
   * These  ctors will set appropriate flags for all non-null objects passed
   */
  explicit TrackStateOnSurface(
    const FitQualityOnSurface& fitQoS,
    std::unique_ptr<const MeasurementBase> meas,
    std::unique_ptr<const TrackParameters> trackParameters,
    std::unique_ptr<const MaterialEffectsBase> materialEffects = nullptr,
    std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack = nullptr);

  explicit TrackStateOnSurface(
    std::unique_ptr<const MeasurementBase> meas,
    std::unique_ptr<const TrackParameters> trackParameters,
    std::unique_ptr<const MaterialEffectsBase> materialEffects = nullptr,
    std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack = nullptr);

  /**
   * Full constructors
   *
   * @param[in] FitQualityOnSurface (we provide one without it)
   * @param[in] unique pointer to a MeasurementBase, or 0 if no object is being
   *            passed.
   * @param[in] unigue trackParameter pointer to a TrackParameters, or 0 if no object
   *            is being passed.
   *            being passed.
   * @param[in] materialEffectsOnTrack pointer to a MaterialEffectsBase, or 0
   *            if no object is being passed.
   * @param[in] typePattern The pattern of 'types' which correspond to this
   *            TSoS. You create the bitset as follows:
   */
  explicit TrackStateOnSurface(
    const FitQualityOnSurface& fitQoS,
    std::unique_ptr<const MeasurementBase> meas,
    std::unique_ptr<const TrackParameters> trackParameters,
    std::unique_ptr<const MaterialEffectsBase> materialEffectsOnTrack,
    const std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>& typePattern,
    std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack = nullptr);

  explicit TrackStateOnSurface(
    std::unique_ptr<const MeasurementBase> meas,
    std::unique_ptr<const TrackParameters> trackParameters,
    std::unique_ptr<const MaterialEffectsBase> materialEffectsOnTrack,
    const std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>& typePattern,
    std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack = nullptr);

 /**
   * Pseudo-constructor: needed to avoid excessive RTTI
   */
  virtual TrackStateOnSurface* clone() const;

  /** copy ctor*/
  TrackStateOnSurface(const TrackStateOnSurface& trackStateOnSurface);
  /** Move ctor*/
  TrackStateOnSurface(TrackStateOnSurface&& trackStateOnSurface) noexcept;

  /* Assignment */
  Trk::TrackStateOnSurface& operator=(const Trk::TrackStateOnSurface& rhs);
  Trk::TrackStateOnSurface& operator=(Trk::TrackStateOnSurface&& rhs) noexcept;
  /** destructor*/
  virtual ~TrackStateOnSurface() = default;

  /** returns 0 if there is no FQOS object assigned*/
  const FitQualityOnSurface& fitQualityOnSurface() const;

  /**
   * returns trackparameters of TrackStateOnSurface, or 0 if no
   * parameter set.
   */
  const TrackParameters* trackParameters() const;

  /** returns MeasurementBase, or 0 if no RiO_OnTrack set.*/
  const MeasurementBase* measurementOnTrack() const;

  /** returns 0 if there is no material effects, and the material effects
   * otherwise*/
  const MaterialEffectsBase* materialEffectsOnTrack() const;

  /** returns 0 if there is no alignment effects, and the alignment effects
   * otherwise*/
  const AlignmentEffectsOnTrack* alignmentEffectsOnTrack() const;

  /**
   * Use this method to find out if the TSoS is of a certain type:
   * i.e. if ( tsos->type(TrackStateOnSurface::Measurement) { //etc }
   *
   * @return true if the TrackStateOnSurface is of this type
   */
  bool type(const TrackStateOnSurfaceType type) const;

  /** Use this method to find if this is a Single, Multi or Align
   * TrackStateOnsurface
   */
  virtual Trk::TrackStateOnSurface::Variety variety() const;

  /** returns a string with the expanded type of the object (i.e. if it has
   * several type bits set, they all will be returned)*/
  std::string dumpType() const;

  /** returns a bitset with the types of this bitset.
  As an example of how this is used, see the type( const
  TrackStateOnSurfaceType& type ) method but a better example would be if you
  wanted to check several type flags at once.
  @code
  const bitset<NumberOfTrackStateOnSurfaceTypes> mask;
  mask.set(Measurement,true);
  mask.set(BremPoint,true);
  // loop over lots of TSoS and call:
  if (tsos.types() & mask ) { // do something}
  @endcode
  */
  const std::bitset<NumberOfTrackStateOnSurfaceTypes> types() const;

  /**
   * Use this method to set persistification hints.
   * This can be called only once per TSOS
   * It will set a cached Value which we can not reset
   * But this allows it to be const..
   */
  void setHints(const uint8_t hints) const;
  /**
   * Use this method to get the persistification hints
   */
  const std::bitset<NumberOfPersistencyHints> hints() const;

  /**
   * return associated surface
   */
  const Trk::Surface& surface() const;
  //!< Used to perform sanity checks on this object (i.e. all consistuents are
  //!< on the same surface). Returns 'true' if it seems okay.
  bool isSane() const;

private:
  /** set sensible default flags*/
  void setFlags();

  FitQualityOnSurface m_fitQualityOnSurface{};
  std::unique_ptr<const TrackParameters> m_trackParameters{};
  std::unique_ptr<const MeasurementBase> m_measurementOnTrack{};
  std::unique_ptr<const MaterialEffectsBase> m_materialEffectsOnTrack{};
  std::unique_ptr<const AlignmentEffectsOnTrack> m_alignmentEffectsOnTrack{};
protected:
  uint16_t m_typeFlags{};
  mutable std::atomic<uint8_t> m_hints{};
};

/**Overload of << operator for MsgStream for debug output*/
MsgStream&
operator<<(MsgStream& sl, const TrackStateOnSurface& tsos);

/**Overload of << operator for std::ostream for debug output*/
std::ostream&
operator<<(std::ostream& sl, const TrackStateOnSurface& tsos);
}

#include "TrkTrack/TrackStateOnSurface.icc"

#endif
