/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKAEOTRACKSTATEONSURFACE_H
#define TRKAEOTRACKSTATEONSURFACE_H

#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkTrack/AlignmentEffectsOnTrack.h"
//
#include <memory>


namespace Trk {
/**
 * @brief Track State On Surface containing
 * AlignmentEffectsOnTrack
 * 
 * @author edward.moyse@cern.ch
 * @author Chistos Anastopoulos Athena MT migration
 */
class AEOTrackStateOnSurface : public TrackStateOnSurface
{

public:
  /**
   * Default ctor
   */
  AEOTrackStateOnSurface() = default;

  /**
   * Constructors
   */
  explicit AEOTrackStateOnSurface(
    const FitQualityOnSurface& fitQoS,
    std::unique_ptr<const MeasurementBase> meas,
    std::unique_ptr<const TrackParameters> trackParameters,
    std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack,
    std::unique_ptr<const MaterialEffectsBase> materialEffects = nullptr);

  explicit AEOTrackStateOnSurface(
    std::unique_ptr<const MeasurementBase> meas,
    std::unique_ptr<const TrackParameters> trackParameters,
    std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack,
    std::unique_ptr<const MaterialEffectsBase> materialEffects = nullptr);

  explicit AEOTrackStateOnSurface(
    const FitQualityOnSurface& fitQoS,
    std::unique_ptr<const MeasurementBase> meas,
    std::unique_ptr<const TrackParameters> trackParameters,
    std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack,
    std::unique_ptr<const MaterialEffectsBase> materialEffectsOnTrack,
    const std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>& typePattern);

  explicit AEOTrackStateOnSurface(
    std::unique_ptr<const MeasurementBase> meas,
    std::unique_ptr<const TrackParameters> trackParameters,
    std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack,
    std::unique_ptr<const MaterialEffectsBase> materialEffectsOnTrack,
    const std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>& typePattern);

  virtual AEOTrackStateOnSurface* clone() const override final;

  /** copy ctor*/
  AEOTrackStateOnSurface(const AEOTrackStateOnSurface& trackStateOnSurface);
  /** Move ctor*/
  AEOTrackStateOnSurface(AEOTrackStateOnSurface&& trackStateOnSurface) noexcept = default;

  /* Assignment */
  Trk::AEOTrackStateOnSurface& operator=(const Trk::AEOTrackStateOnSurface& rhs);
  Trk::AEOTrackStateOnSurface& operator=(Trk::AEOTrackStateOnSurface&& rhs) noexcept = default;
   
  /** destructor*/
  virtual ~AEOTrackStateOnSurface() = default;

  /** Use this method to find if this is a Single, Multi
   *  Align or AEOT TrackStateOnsurface
   */
  virtual Trk::TrackStateOnSurface::Variety variety() const override final
  {
    return Trk::TrackStateOnSurface::AEOT;
  }

  virtual const Trk::AlignmentEffectsOnTrack* alignmentEffectsOnTrack() const override final
  {
    return m_alignmentEffectsOnTrack.get();
  }

private:
  std::unique_ptr<const AlignmentEffectsOnTrack> m_alignmentEffectsOnTrack{};
};

}

#endif
