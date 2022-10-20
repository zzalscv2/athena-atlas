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
  /**
   * Slim a non const Track.
   * @param track A reference to the track to be slimmed.
   *
   * The method sets persistification hints
   * in the Track's TrackStateOnSurfaces
   * So a slimmed version is written to disk
   * 
   * The properties are modified
   * setTrackProperties(TrackInfo::SlimmedTrack);
   */
  virtual void slimTrack(Trk::Track& track) const = 0;
  /**
   *
   * Slim a non const Track.
   * @param track A const reference to the track to be slimmed.
   * The method sets persistification hints
   * in the Track's TrackStateOnSurfaces
   * So a slimmed version is written to disk
   *
   * The track properties of a const track can not be modified.
   *
   */
  virtual void slimConstTrack(const Trk::Track& track) const = 0;
 };

inline const InterfaceID&
Trk::ITrackSlimmingTool::interfaceID()
{
  return IID_ITrackSlimmingTool;
}

} // end of namespace

#endif
