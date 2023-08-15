/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKTRACK_ALIGNMENTEFFECTSONTRACK_H
#define TRKTRACK_ALIGNMENTEFFECTSONTRACK_H

#include "Identifier/Identifier.h"
#include "TrkSurfaces/SurfaceHolders.h"
#include <iostream>
#include <vector>

class MsgStream;

namespace Trk {

class Surface;
class TrackStateOnSurface;

/// Class to represent misalignments or 'discontinuities' on tracks
/// These have a surface where the z axis is aligned with the direction of the
/// translation, and the angle of the rotation is with respect to this.
class AlignmentEffectsOnTrack : public SurfacePtrHolder
{
public:
  AlignmentEffectsOnTrack(
    double deltaTranslation,
    double m_sigmaDeltaTranslation,
    double deltaAngle,
    double sigmaDeltaAngle,
    const std::vector<Identifier>& identifiersOfAffectedTSOS,
    const Trk::Surface& surf);

  AlignmentEffectsOnTrack(const Trk::AlignmentEffectsOnTrack& rhs) = default;

  Trk::AlignmentEffectsOnTrack& operator=(
    const Trk::AlignmentEffectsOnTrack& rhs) = default;

  AlignmentEffectsOnTrack(Trk::AlignmentEffectsOnTrack&& rhs) noexcept =
    default;

  Trk::AlignmentEffectsOnTrack& operator=(
    Trk::AlignmentEffectsOnTrack&& rhs) noexcept = default;

  ~AlignmentEffectsOnTrack() = default;

  /// returns the \f$ \Delta X \f$
  double deltaTranslation() const;

  /// returns the \f$ \sigma\Delta X \f$
  double sigmaDeltaTranslation() const;

  /// returns the \f$ \Delta \alpha \f$
  double deltaAngle() const;

  /// returns the \f$ \sigma\Delta \alpha \f$
  double sigmaDeltaAngle() const;

  /// Returns a vector of the affected TSOS in the track. Obviously this must
  /// not be invalidated by removing TSOS from the track.
  const std::vector<Identifier>& vectorOfAffectedTSOS() const;

  /// Updates the vector of the affected TSOS in the track.
  void updateVectorOfAffectedTSOS(std::vector<Identifier>&);

  /// Returns true if the effects of this  AlignmentEffectsOnTrack apply to all
  /// remaining TrackStatesOnSurface of the Track.
  bool effectsLastFromNowOn() const { return m_affectedTSOS.empty(); }

  /// The surface on which this offset is expressed.
  const Trk::Surface& associatedSurface() const;

private:
  double m_deltaTranslation{0.};
  double m_sigmaDeltaTranslation{0.};
  double m_deltaAngle{0.};
  double m_sigmaDeltaAngle{0.};
  std::vector<Identifier> m_affectedTSOS;
};

/**Overload of << operator for MsgStream for debug output*/
MsgStream&
operator<<(MsgStream& sl, const AlignmentEffectsOnTrack& tsos);

/**Overload of << operator for std::ostream for debug output*/
std::ostream&
operator<<(std::ostream& sl, const AlignmentEffectsOnTrack& tsos);
}

inline double
Trk::AlignmentEffectsOnTrack::deltaTranslation() const
{
  return m_deltaTranslation;
}

inline double
Trk::AlignmentEffectsOnTrack::sigmaDeltaTranslation() const
{
  return m_sigmaDeltaTranslation;
}

inline double
Trk::AlignmentEffectsOnTrack::deltaAngle() const
{
  return m_deltaAngle;
}

inline double
Trk::AlignmentEffectsOnTrack::sigmaDeltaAngle() const
{
  return m_sigmaDeltaAngle;
}

inline const std::vector<Identifier>&
Trk::AlignmentEffectsOnTrack::vectorOfAffectedTSOS() const
{
  return m_affectedTSOS;
}

inline void
Trk::AlignmentEffectsOnTrack::updateVectorOfAffectedTSOS(
  std::vector<Identifier>& affectedTSOS)
{
  m_affectedTSOS = affectedTSOS;
}

inline const Trk::Surface&
Trk::AlignmentEffectsOnTrack::associatedSurface() const
{
  return *m_associatedSurface;
}

#endif
