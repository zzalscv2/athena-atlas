/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackSlimmingTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKTRACKSLIMMINGTOOL_H
#define TRKTRACKSLIMMINGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"
#include "TrkToolInterfaces/ITrackSlimmingTool.h"
#include "TrkTrack/Track.h"
#include <memory>
class AtlasDetectorID;
class Identifier;

namespace Trk {
class TrackStateOnSurface;

/** @class TrackSlimmingTool

  A tool to produce 'slimmed' Tracks from a reference track.

  By default, this slimmed track will include all the measurements, and the
  Perigee parameters (currently these are assumed to exist)

  @author  Edward Moyse <Edward.Moysecern.ch>
  @author  Christos Anastopoulos Athena MT modifications

*/
class TrackSlimmingTool final
  : virtual public ITrackSlimmingTool
  , public AthAlgTool
{
public:
  TrackSlimmingTool(const std::string&, const std::string&, const IInterface*);

  /** default destructor */
  virtual ~TrackSlimmingTool();

  /** standard Athena-Algorithm method */
  virtual StatusCode initialize() override;

  /** standard Athena-Algorithm method */
  virtual StatusCode finalize() override;

  /**This method 'skims' interesting information from the passed track.
   * @param track A const reference to the track to be skimmed
   * For compatibility reasons in can do two different things
   * depending on the value of m_setPersistificationHints.
   *
   * When  setPersistificationHints = False
   * @return A 'slimmed' copy of the  'track'.
   *
   * When setPersistificationHints = True
   * it sets persistification hints
   * and returns nullptr
   *
   */
  Trk::Track* slim(const Trk::Track& track) const override final;

  /**
   * Slim/skim a non const Track.
   * @param track A reference to the track to be skimmed.
   *
   * When setPersistificationHints = True
   * it sets persistification hints
   */
  void slimTrack(Trk::Track& track) const override final;

private:
  /** any CaloDeposit with its adjacent MEOT's will be kept on the slimmed track
   * (combined muon property) */
  bool m_keepCaloDeposit;

  /** If true, Outliers will be kept on the slimmed track*/
  bool m_keepOutliers;

  /** If true, the first and last parameters of ID & MS subsystems will be kept
   * on the slimmed track*/
  bool m_keepParameters;

  /** Do not create slimmed tracks but only set hints for the persistifier to
   * drop information while writing.
   */
  bool m_setPersistificationHints;

  /**atlas id helper*/
  const AtlasDetectorID* m_detID;

  /**
   * This method just set persistification
   * Hints
   */
  void setHints(const Trk::Track& track) const;

  /*
   * This method resets the TSOS of a Track 
   * keeping only the ones we want
   */
  void resetTSOS(Trk::Track& track) const;
  /**
   * This method always creates a std::unique_ptr<Trk::Track> with information
   * removed calling the resetTSOS above
   */
  std::unique_ptr<Trk::Track> slimCopy(const Trk::Track& track) const;


  void checkForValidMeas(const Trk::TrackStateOnSurface* tsos,
                         bool& isIDmeas,
                         bool& isMSmeas) const;

  void findLastValidTSoS(
    const DataVector<const Trk::TrackStateOnSurface>* oldTrackStates,
    const Trk::TrackStateOnSurface*& lastValidIDTSOS,
    const TrackStateOnSurface*& lastValidMSTSOS) const;

  bool keepParameters(const Trk::TrackStateOnSurface* TSoS,
                      const TrackStateOnSurface*& firstValidIDTSOS,
                      const TrackStateOnSurface*& lastValidIDTSOS,
                      const TrackStateOnSurface*& firstValidMSTSOS,
                      const TrackStateOnSurface*& lastValidMSTSOS) const;
};
} // end of namespace

#endif
