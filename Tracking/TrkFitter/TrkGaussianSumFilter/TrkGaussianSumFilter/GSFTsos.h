/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrkGsfTsos_H
#define TrkGsfTsos_H

#include "TrkTrack/MultiComponentStateOnSurface.h"

struct GSFTsos {

  Trk::MultiComponentState multiComponentState{};
  std::unique_ptr<Trk::TrackParameters> trackParameters{};
  std::unique_ptr<Trk::MeasurementBase> measurementOnTrack{};
  Trk::FitQualityOnSurface fitQualityOnSurface{};
  std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>
      typeFlags{};

  // default move, move assignment and dtor
  GSFTsos() = default;
  GSFTsos& operator=(GSFTsos&&) = default;
  GSFTsos(GSFTsos&&) = default;
  ~GSFTsos() = default;

  // implement copy and copy assignment
  GSFTsos(const GSFTsos& rhs) = delete;
  GSFTsos& operator=(const GSFTsos& rhs) =delete;

  // Full constructor with passing of type flags
  GSFTsos(
      const Trk::FitQualityOnSurface& inFitQualityOnSurface,
      std::unique_ptr<Trk::MeasurementBase> inMeasurementBase,
      std::unique_ptr<Trk::TrackParameters> inTrackParameters,
      Trk::MultiComponentState&& inMultiComponentState,
      std::bitset<Trk::TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>
          inTypeFlags)
      : multiComponentState(std::move(inMultiComponentState)),
        trackParameters(std::move(inTrackParameters)),
        measurementOnTrack(std::move(inMeasurementBase)),
        fitQualityOnSurface(inFitQualityOnSurface),
        typeFlags(inTypeFlags) {}

  // constructor with automatic setting of type flags
  GSFTsos(const Trk::FitQualityOnSurface& inFitQualityOnSurface,
          std::unique_ptr<Trk::MeasurementBase> inMeasurementBase,
          std::unique_ptr<Trk::TrackParameters> inTrackParameters,
          Trk::MultiComponentState&& inMultiComponentState)
      : multiComponentState(std::move(inMultiComponentState)),
        trackParameters(std::move(inTrackParameters)),
        measurementOnTrack(std::move(inMeasurementBase)),
        fitQualityOnSurface(inFitQualityOnSurface) {
    if (measurementOnTrack) {
      typeFlags.set(Trk::TrackStateOnSurface::Measurement);
    }
    if (trackParameters) {
      typeFlags.set(Trk::TrackStateOnSurface::Parameter);
    }
    if (fitQualityOnSurface) {
      typeFlags.set(Trk::TrackStateOnSurface::FitQuality);
    }
  }
  // convert pass ownership to MTSOS
  std::unique_ptr<const Trk::MultiComponentStateOnSurface> convert(bool slim) {
    if (slim) {
      multiComponentState.clear();
    }
    return std::make_unique<const Trk::MultiComponentStateOnSurface>(
        fitQualityOnSurface, std::move(measurementOnTrack),
        std::move(trackParameters), std::move(multiComponentState), nullptr,
        typeFlags);
  }
};

#endif
