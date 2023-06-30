/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkTrack/TrackStateOnSurface.h"
#include "GaudiKernel/MsgStream.h"
#include "TrkEventPrimitives/SurfaceConsistencyCheck.h"
#include <stdexcept>
#include <string>

namespace Trk {
TrackStateOnSurface::TrackStateOnSurface() = default;

// partial
TrackStateOnSurface::TrackStateOnSurface(
  const FitQualityOnSurface& fitQoS,
  std::unique_ptr<const MeasurementBase> meas,
  std::unique_ptr<const TrackParameters> trackParameters,
  std::unique_ptr<const MaterialEffectsBase> materialEffects,
  std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack)
  : m_fitQualityOnSurface(fitQoS)
  , m_trackParameters(std::move(trackParameters))
  , m_measurementOnTrack(std::move(meas))
  , m_materialEffectsOnTrack(std::move(materialEffects))
  , m_alignmentEffectsOnTrack(std::move(alignmentEffectsOnTrack))
{
  assert(isSane());
  setFlags();
}

TrackStateOnSurface::TrackStateOnSurface(
  std::unique_ptr<const MeasurementBase> meas,
  std::unique_ptr<const TrackParameters> trackParameters,
  std::unique_ptr<const MaterialEffectsBase> materialEffects,
  std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack)
  : m_fitQualityOnSurface{}
  , m_trackParameters(std::move(trackParameters))
  , m_measurementOnTrack(std::move(meas))
  , m_materialEffectsOnTrack(std::move(materialEffects))
  , m_alignmentEffectsOnTrack(std::move(alignmentEffectsOnTrack))
{
  assert(isSane());
  setFlags();
}

// full
TrackStateOnSurface::TrackStateOnSurface(
  const FitQualityOnSurface& fitQoS,
  std::unique_ptr<const MeasurementBase> meas,
  std::unique_ptr<const TrackParameters> trackParameters,
  std::unique_ptr<const MaterialEffectsBase> materialEffects,
  const std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>&
    typePattern,
  std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack)
  : m_fitQualityOnSurface(fitQoS)
  , m_trackParameters(std::move(trackParameters))
  , m_measurementOnTrack(std::move(meas))
  , m_materialEffectsOnTrack(std::move(materialEffects))
  , m_alignmentEffectsOnTrack(std::move(alignmentEffectsOnTrack))
  , m_typeFlags(typePattern.to_ulong())
{
  assert(isSane());
}

TrackStateOnSurface::TrackStateOnSurface(
  std::unique_ptr<const MeasurementBase> meas,
  std::unique_ptr<const TrackParameters> trackParameters,
  std::unique_ptr<const MaterialEffectsBase> materialEffects,
  const std::bitset<TrackStateOnSurface::NumberOfTrackStateOnSurfaceTypes>&
    typePattern,
  std::unique_ptr<const AlignmentEffectsOnTrack> alignmentEffectsOnTrack)
  : m_fitQualityOnSurface{}
  , m_trackParameters(std::move(trackParameters))
  , m_measurementOnTrack(std::move(meas))
  , m_materialEffectsOnTrack(std::move(materialEffects))
  , m_alignmentEffectsOnTrack(std::move(alignmentEffectsOnTrack))
  , m_typeFlags(typePattern.to_ulong())
{
  assert(isSane());
}

// copy
TrackStateOnSurface::TrackStateOnSurface(const TrackStateOnSurface& rhs)
  : m_fitQualityOnSurface(rhs.m_fitQualityOnSurface)
  , m_trackParameters(rhs.m_trackParameters ? rhs.m_trackParameters->clone()
                                            : nullptr)
  , m_measurementOnTrack(
      rhs.m_measurementOnTrack ? rhs.m_measurementOnTrack->clone() : nullptr)
  , m_materialEffectsOnTrack(rhs.m_materialEffectsOnTrack
                               ? rhs.m_materialEffectsOnTrack->clone()
                               : nullptr)
  , m_alignmentEffectsOnTrack(
      rhs.m_alignmentEffectsOnTrack
        ? std::make_unique<const AlignmentEffectsOnTrack>(
            *rhs.m_alignmentEffectsOnTrack)
        : nullptr)
  , m_typeFlags(rhs.m_typeFlags)
{
}

// move
TrackStateOnSurface::TrackStateOnSurface(TrackStateOnSurface&& rhs) noexcept
  : m_fitQualityOnSurface(rhs.m_fitQualityOnSurface)
  , m_trackParameters(std::move(rhs.m_trackParameters))
  , m_measurementOnTrack(std::move(rhs.m_measurementOnTrack))
  , m_materialEffectsOnTrack(std::move(rhs.m_materialEffectsOnTrack))
  , m_alignmentEffectsOnTrack(std::move(rhs.m_alignmentEffectsOnTrack))
  , m_typeFlags(rhs.m_typeFlags)
{
}

// copy assignment
TrackStateOnSurface&
TrackStateOnSurface::operator=(const TrackStateOnSurface& rhs)
{
  if (this != &rhs) {
    m_fitQualityOnSurface = rhs.m_fitQualityOnSurface;
    m_trackParameters.reset(
      rhs.m_trackParameters ? rhs.m_trackParameters->clone() : nullptr);
    m_measurementOnTrack.reset(
      rhs.m_measurementOnTrack ? rhs.m_measurementOnTrack->clone() : nullptr);
    m_materialEffectsOnTrack.reset(rhs.m_materialEffectsOnTrack
                                     ? rhs.m_materialEffectsOnTrack->clone()
                                     : nullptr);
    m_alignmentEffectsOnTrack =
      rhs.m_alignmentEffectsOnTrack
        ? std::make_unique<const AlignmentEffectsOnTrack>(
            *rhs.m_alignmentEffectsOnTrack)
        : nullptr;
    m_typeFlags = rhs.m_typeFlags;
    assert(isSane());
  }
  return *this;
}

// move assignment
TrackStateOnSurface&
TrackStateOnSurface::operator=(Trk::TrackStateOnSurface&& rhs) noexcept
{
  if (this != &rhs) {
    m_fitQualityOnSurface = rhs.m_fitQualityOnSurface;
    m_trackParameters = std::move(rhs.m_trackParameters);
    m_measurementOnTrack = std::move(rhs.m_measurementOnTrack);
    m_materialEffectsOnTrack = std::move(rhs.m_materialEffectsOnTrack);
    m_alignmentEffectsOnTrack = std::move(rhs.m_alignmentEffectsOnTrack);
    m_typeFlags = rhs.m_typeFlags;
  }
  return *this;
}

std::string
TrackStateOnSurface::dumpType() const
{
  std::string type;
  const auto& typesSet = types();
  if (typesSet.test(TrackStateOnSurface::Measurement)) {
    type += "Measurement ";
  }
  if (typesSet.test(TrackStateOnSurface::InertMaterial)) {
    type += "InertMaterial ";
  }
  if (typesSet.test(TrackStateOnSurface::BremPoint)) {
    type += "BremPoint ";
  }
  if (typesSet.test(TrackStateOnSurface::Scatterer)) {
    type += "Scatterer ";
  }
  if (typesSet.test(TrackStateOnSurface::Perigee)) {
    type += "Perigee ";
  }
  if (typesSet.test(TrackStateOnSurface::Outlier)) {
    type += "Outlier ";
  }
  if (typesSet.test(TrackStateOnSurface::Hole)) {
    type += "Hole ";
  }
  if (typesSet.test(TrackStateOnSurface::CaloDeposit)) {
    type += "CaloDeposit ";
  }
  if (typesSet.test(TrackStateOnSurface::Parameter)) {
    type += "Parameter ";
  }
  if (typesSet.test(TrackStateOnSurface::FitQuality)) {
    type += "FitQuality ";
  }
  if (typesSet.test(TrackStateOnSurface::Alignment)) {
    type += "Alignment ";
  }
  return type;
}

const Surface&
TrackStateOnSurface::surface() const
{
  if (m_trackParameters) {
    return m_trackParameters->associatedSurface();
  }
  if (m_measurementOnTrack) {
    return m_measurementOnTrack->associatedSurface();
  }
  if (m_materialEffectsOnTrack) {
    return m_materialEffectsOnTrack->associatedSurface();
  }
  throw std::runtime_error("TrackStateOnSurface without Surface!");
}

bool
TrackStateOnSurface::isSane() const
{
  bool surfacesDiffer =
    not Trk::consistentSurfaces(m_trackParameters.get(),
                                m_measurementOnTrack.get(),
                                m_materialEffectsOnTrack.get());
  if (surfacesDiffer) {
    std::cerr << "TrackStateOnSurface::isSane. With :" << '\n';
    std::cerr << "Types : " << types().to_string() << '\n';
    std::cerr << "Hints " << hints().to_string() << '\n';
    std::cerr << "Surfaces differ! " << std::endl;
    if (m_trackParameters) {
      std::cerr << "ParamSurf: [" << &(m_trackParameters->associatedSurface())
                << "] " << m_trackParameters->associatedSurface() << std::endl;
    }
    if (m_measurementOnTrack) {
      std::cerr << "measSurf: [" << &(m_measurementOnTrack->associatedSurface())
                << "] " << m_measurementOnTrack->associatedSurface()
                << std::endl;
    }
    if (m_materialEffectsOnTrack) {
      std::cerr << "matSurf: ["
                << &(m_materialEffectsOnTrack->associatedSurface()) << "] "
                << m_materialEffectsOnTrack->associatedSurface() << std::endl;
    }
    return false;
  }

  return true;
}

void
TrackStateOnSurface::setHints(const uint8_t hints) const
{
  // The extra "hidden" bit we save (1<<NumberOfPersistencyHints)
  // is to dissalow repeated calls to setHints(0).
  uint8_t exp = 0;
  if (!m_hints.compare_exchange_strong(
        exp, hints | (1 << NumberOfPersistencyHints))) {
    throw std::runtime_error(
      "TSOS trying to set again already set Persistification Hints");
  }
}

/**Overload of << operator for both, MsgStream and std::ostream for debug
 * output*/
MsgStream&
operator<<(MsgStream& sl, const TrackStateOnSurface& tsos)
{
  std::string name("TrackStateOnSurface: ");
  sl << name << "\t of type : " << tsos.dumpType() << endmsg;

  if (sl.level() < MSG::INFO) {
    sl << name << "Detailed dump of contained objects follows:" << endmsg;
    sl << (tsos.fitQualityOnSurface()) << "\n (end of FitQualityOnSurface dump)"
       << endmsg;

    if (tsos.trackParameters() != nullptr) {
      sl << *(tsos.trackParameters()) << "\n (end of TrackParameters dump)"
         << endmsg;
    }

    if (tsos.measurementOnTrack() != nullptr) {
      sl << *(tsos.measurementOnTrack()) << "\n (end of MeasurementBase dump"
         << endmsg;
    }

    if (tsos.materialEffectsOnTrack() != nullptr) {
      sl << *(tsos.materialEffectsOnTrack())
         << "\n (end of MaterialEffectsBase dump)" << endmsg;
    }

    if (tsos.alignmentEffectsOnTrack() != nullptr) {
      sl << *(tsos.alignmentEffectsOnTrack())
         << "\n (end of AlignmentEffectsOnTrack dump)" << endmsg;
    }
  }
  return sl;
}

std::ostream&
operator<<(std::ostream& sl, const TrackStateOnSurface& tsos)
{
  std::string name("TrackStateOnSurface: ");
  sl << name << "\t of type : " << tsos.dumpType() << std::endl;

  sl << "\t FitQualityOnSurface(s)." << std::endl;
  sl << "\t \t" << (tsos.fitQualityOnSurface()) << std::endl;

  if (tsos.trackParameters() != nullptr) {
    sl << "\t HAS TrackParameter(s)." << std::endl;
    sl << "\t \t" << *(tsos.trackParameters()) << std::endl;
  } else {
    sl << "\t NO TrackParameters." << std::endl;
  }

  if (tsos.measurementOnTrack() != nullptr) {
    sl << "\t HAS MeasurementBase(s)." << std::endl;
    sl << "\t \t" << *(tsos.measurementOnTrack()) << std::endl;
  } else {
    sl << "\t NO MeasurementBase." << std::endl;
  }

  if (tsos.materialEffectsOnTrack() != nullptr) {
    sl << "\t HAS MaterialEffectsBase." << std::endl;
    sl << "\t \t" << *(tsos.materialEffectsOnTrack()) << std::endl;
  } else {
    sl << "\t NO MaterialEffects." << std::endl;
  } /**return sl; don't return here, the next code becomes dead**/

  if (tsos.alignmentEffectsOnTrack() != nullptr) {
    sl << "\t HAS AlignmentEffectsOnTrack." << std::endl;
    sl << "\t \t" << *(tsos.alignmentEffectsOnTrack()) << std::endl;
  } else {
    sl << "\t NO AlignmentEffectsOnTrack." << std::endl;
  }
  return sl;
}
} // namespace Trk
