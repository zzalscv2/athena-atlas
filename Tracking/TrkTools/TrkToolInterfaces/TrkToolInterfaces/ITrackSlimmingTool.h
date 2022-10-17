/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ITrackSlimmingTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#ifndef ITRKTRACKSLIMMINGTOOL_H
#define ITRKTRACKSLIMMINGTOOL_H

#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/IAlgTool.h"

namespace Trk

{
class Track;

static const InterfaceID IID_ITrackSlimmingTool("Trk::ITrackSlimmingTool",
                                                1,
                                                0);

/** @brief Interface for constructing 'slimmed' Tracks from complete tracks.

    @author edward.moyse@cern.ch
    @author  Christos Anastopoulos Athena MT modifications
*/
class ITrackSlimmingTool : virtual public IAlgTool
{
public:
  static const InterfaceID& interfaceID();
  /**This method 'skims' interesting information from the passed track.
   * @param track A const reference to the track to be skimmed
   *
   * For compatibility reasons it can do two different things
   * depending on so called setPersistificationHints.
   *
   * When setPersistificationHints is not to be used
   * @return A 'slimmed' copy of the input 'track'.
   *
   * When setPersistificationHints = True
   * it sets persistification hints
   * @return nullptr
   *
   */
  virtual Trk::Track* slim(const Trk::Track& track) const = 0;
  /**
   * Slim/skim a non const Track.
   * @param track A reference to the track to be skimmed.
   * It will be modified.
   *
   * When setPersistificationHints = True
   * it sets persistification hints
   *
   */
  virtual void slimTrack(Trk::Track& track) const = 0;
};

inline const InterfaceID&
Trk::ITrackSlimmingTool::interfaceID()
{
  return IID_ITrackSlimmingTool;
}

} // end of namespace

#endif
