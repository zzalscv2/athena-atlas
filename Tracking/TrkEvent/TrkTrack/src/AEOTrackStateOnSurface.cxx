/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkTrack/AEOTrackStateOnSurface.h"
#include "GaudiKernel/MsgStream.h"
#include <stdexcept>
#include <string>

namespace Trk {

// partial
AEOTrackStateOnSurface::AEOTrackStateOnSurface(
  const FitQualityOnSurface& fitQoS,
  std::unique_ptr<const MeasurementBase> meas,
  std::unique_ptr<const TrackParameters> trackParameters,
  std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack,
  std::unique_ptr<const MaterialEffectsBase> materialEffects)
  : TrackStateOnSurface(fitQoS,
                        std::move(meas),
                        std::move(trackParameters),
                        std::move(materialEffects))
  , m_alignmentEffectsOnTrack(std::move(alignmentEffectsOnTrack))
{

  if (m_alignmentEffectsOnTrack) {
    m_typeFlags |= 1 << Alignment;
  }
}

AEOTrackStateOnSurface::AEOTrackStateOnSurface(
  std::unique_ptr<const MeasurementBase> meas,
  std::unique_ptr<const TrackParameters> trackParameters,
  std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack,
  std::unique_ptr<const MaterialEffectsBase> materialEffects)
  : TrackStateOnSurface(std::move(meas),
                        std::move(trackParameters),
                        std::move(materialEffects))
  , m_alignmentEffectsOnTrack(std::move(alignmentEffectsOnTrack))
{
  if (m_alignmentEffectsOnTrack) {
    m_typeFlags |= 1 << Alignment;
  }
}

// full
AEOTrackStateOnSurface::AEOTrackStateOnSurface(
  const FitQualityOnSurface& fitQoS,
  std::unique_ptr<const MeasurementBase> meas,
  std::unique_ptr<const TrackParameters> trackParameters,
  std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack,
  std::unique_ptr<const MaterialEffectsBase> materialEffects,
  const std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>& typePattern)
  : TrackStateOnSurface(fitQoS,
                        std::move(meas),
                        std::move(trackParameters),
                        std::move(materialEffects),
                        typePattern)
  , m_alignmentEffectsOnTrack(std::move(alignmentEffectsOnTrack))
{
}

AEOTrackStateOnSurface::AEOTrackStateOnSurface(
  std::unique_ptr<const MeasurementBase> meas,
  std::unique_ptr<const TrackParameters> trackParameters,
  std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack,
  std::unique_ptr<const MaterialEffectsBase> materialEffects,
  const std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>& typePattern)
  : TrackStateOnSurface(std::move(meas),
                        std::move(trackParameters),
                        std::move(materialEffects),
                        typePattern)
  , m_alignmentEffectsOnTrack(std::move(alignmentEffectsOnTrack))
{
}

// copy
AEOTrackStateOnSurface::AEOTrackStateOnSurface(
  const AEOTrackStateOnSurface& rhs)
  : TrackStateOnSurface(rhs)
  , m_alignmentEffectsOnTrack(
      rhs.m_alignmentEffectsOnTrack
        ? std::make_unique<const AlignmentEffectsOnTrack>(
            *rhs.m_alignmentEffectsOnTrack)
        : nullptr)
{
}

// copy assignment
AEOTrackStateOnSurface&
AEOTrackStateOnSurface::operator=(const AEOTrackStateOnSurface& rhs)
{
  if (this != &rhs) {
    TrackStateOnSurface::operator=(rhs);
    m_alignmentEffectsOnTrack =
      rhs.m_alignmentEffectsOnTrack
        ? std::make_unique<const AlignmentEffectsOnTrack>(
            *rhs.m_alignmentEffectsOnTrack)
        : nullptr;
  }
  return *this;
}

AEOTrackStateOnSurface*
AEOTrackStateOnSurface::clone() const
{
  return new AEOTrackStateOnSurface(*this);
}

} // namespace Trk
