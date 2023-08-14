/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkTrack/MultiComponentStateOnSurface.h"

#include "GaudiKernel/MsgStream.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkMaterialOnTrack/MaterialEffectsBase.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include <iostream>
#include <string>

Trk::MultiComponentStateOnSurface::MultiComponentStateOnSurface()
  : TrackStateOnSurface(
      {},
      nullptr,
      nullptr,
      nullptr,
      std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>(
        1 << (int)TrackStateOnSurface::Measurement))
  , m_multiComponentState{}
{
}

Trk::MultiComponentStateOnSurface::MultiComponentStateOnSurface(
  const Trk::FitQualityOnSurface& fitQualityOnSurface,
  std::unique_ptr<const Trk::MeasurementBase> measurementBase,
  MultiComponentState&& multiComponentState,
  std::unique_ptr<const MaterialEffectsBase> materialEffectsOnTrack)
  : TrackStateOnSurface(
      fitQualityOnSurface,
      std::move(measurementBase),
      multiComponentState.front().first->uniqueClone(),
      std::move(materialEffectsOnTrack),
      std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>(
        1 << (int)TrackStateOnSurface::Measurement))
  , m_multiComponentState(std::move(multiComponentState))
{
}

Trk::MultiComponentStateOnSurface::MultiComponentStateOnSurface(
  const Trk::FitQualityOnSurface& fitQualityOnSurface,
  std::unique_ptr<const Trk::MeasurementBase> measurementBase,
  std::unique_ptr<const Trk::TrackParameters> trackParameters,
  MultiComponentState&& multiComponentState,
  std::unique_ptr<const MaterialEffectsBase> materialEffectsOnTrack)
  : TrackStateOnSurface(
      fitQualityOnSurface,
      std::move(measurementBase),
      std::move(trackParameters),
      std::move(materialEffectsOnTrack),
      std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>(
        1 << (int)TrackStateOnSurface::Measurement))
  , m_multiComponentState(std::move(multiComponentState))
{
}

Trk::MultiComponentStateOnSurface::MultiComponentStateOnSurface(
  const Trk::FitQualityOnSurface& fitQualityOnSurface,
  std::unique_ptr<const Trk::MeasurementBase> measurementBase,
  MultiComponentState&& multiComponentState,
  std::unique_ptr<const MaterialEffectsBase> materialEffectsOnTrack,
  const std::bitset<NumberOfTrackStateOnSurfaceTypes>& types)
  : TrackStateOnSurface(fitQualityOnSurface,
                        std::move(measurementBase),
                        multiComponentState.front().first->uniqueClone(),
                        std::move(materialEffectsOnTrack),
                        types)
  , m_multiComponentState(std::move(multiComponentState))
{
}

Trk::MultiComponentStateOnSurface::MultiComponentStateOnSurface(
  const Trk::FitQualityOnSurface& fitQualityOnSurface,
  std::unique_ptr<const Trk::MeasurementBase> measurementBase,
  std::unique_ptr<const Trk::TrackParameters> trackParameters,
  MultiComponentState&& multiComponentState,
  std::unique_ptr<const MaterialEffectsBase> materialEffectsOnTrack,
  const std::bitset<NumberOfTrackStateOnSurfaceTypes>& types)
  : TrackStateOnSurface(fitQualityOnSurface,
                        std::move(measurementBase),
                        std::move(trackParameters),
                        std::move(materialEffectsOnTrack),
                        types)
  , m_multiComponentState(std::move(multiComponentState))
{
}

Trk::MultiComponentStateOnSurface::MultiComponentStateOnSurface(
  std::unique_ptr<const Trk::MeasurementBase> measurementBase,
  MultiComponentState multiComponentState)
  : TrackStateOnSurface(
      {},
      std::move(measurementBase),
      multiComponentState.front().first->uniqueClone(),
      nullptr,
      std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>(
        1 << (int)TrackStateOnSurface::Measurement))
  , m_multiComponentState(std::move(multiComponentState))
{
}

Trk::MultiComponentStateOnSurface::MultiComponentStateOnSurface(
  const Trk::MultiComponentStateOnSurface& other)
  : TrackStateOnSurface(other)
  , m_multiComponentState(
      Trk::MultiComponentStateHelpers::clone(other.components()))
{
}

Trk::MultiComponentStateOnSurface&
Trk::MultiComponentStateOnSurface::operator=(
  const MultiComponentStateOnSurface& other)
{
  if (this != &other) {
    TrackStateOnSurface::operator=(other);
    m_multiComponentState =
      Trk::MultiComponentStateHelpers::clone(other.components());
  }
  return *this;
}

Trk::MultiComponentStateOnSurface*
Trk::MultiComponentStateOnSurface::clone() const
{
  return new MultiComponentStateOnSurface(*this);
}

MsgStream&
Trk::operator<<(MsgStream& log, const Trk::MultiComponentStateOnSurface&)
{
  return log;
}

std::ostream&
Trk::operator<<(std::ostream& log, const Trk::MultiComponentStateOnSurface&)
{
  return log;
}
