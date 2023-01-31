/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IEXTENDEDTRACKSUMMARYHELPERTOOL
#define IEXTENDEDTRACKSUMMARYHELPERTOOL

#include "TrkToolInterfaces/ITrackSummaryHelperTool.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include <vector>
#include <bitset>


class Identifier;

namespace Trk {

  static const InterfaceID IID_IExtendedTrackSummaryHelperTool("Trk::IExtendedTrackSummaryHelperTool", 1, 0);

/** @class ITrackSummaryHelperTool
  @brief Interface for structuring the summary creation into sub-detector
    specific tools.

    This tool is aimed to be implemented once in the Inner Detector and
    once in the Muon Spectrometer, allowing each implementation to access
    detector-specific information while a master tool can decide at run-time
    which implementation to call, depending on the type of hit on the track.

    @author Edward Moyse, Martin Siebel <http://consult.cern.ch/xwho>
*/

  class IExtendedTrackSummaryHelperTool : virtual public ITrackSummaryHelperTool
  {
  public:
    static const InterfaceID& interfaceID();
    using ITrackSummaryHelperTool::addDetailedTrackSummary;
    using ITrackSummaryHelperTool::analyse;

    /* Expand/Extend the interface , with methods  using the EventContext
     */

     /*
     * For now due to client compatibility 
     * we provide a default  implementations
     * in terms of the the older interface
     */

    virtual void analyse(
      const EventContext& ctx,
      const Trk::Track& track,
      const RIO_OnTrack* rot,
      const TrackStateOnSurface* tsos,
      std::vector<int>& information,
      std::bitset<Trk::numberOfDetectorTypes>& hitPattern) const
    {
      (void)ctx;
      analyse(track, rot, tsos, information, hitPattern);
    };

    virtual void analyse(
      const EventContext& ctx,
      const Trk::Track& track,
      const CompetingRIOsOnTrack* crot,
      const TrackStateOnSurface* tsos,
      std::vector<int>& information,
      std::bitset<Trk::numberOfDetectorTypes>& hitPattern) const
    {
      (void)ctx;
      analyse(track, crot, tsos, information, hitPattern);
    }

    virtual void addDetailedTrackSummary(const EventContext& ctx,
                                         const Track& track,
                                         Trk::TrackSummary& summary) const
    {
      (void)ctx;
      addDetailedTrackSummary(track,summary);
    };

    /*
     * Implement the ITrackSummaryHelperTool part
     * of the interface for  the methods with the same
     * name as the method above.
     */
    virtual void analyse(
      const Trk::Track& track,
      const RIO_OnTrack* rot,
      const TrackStateOnSurface* tsos,
      std::vector<int>& information,
      std::bitset<Trk::numberOfDetectorTypes>& hitPattern) const override
    {
      analyse(Gaudi::Hive::currentContext(),
              track,
              rot,
              tsos,
              information,
              hitPattern);
    }

    virtual void analyse(
      const Trk::Track& track,
      const CompetingRIOsOnTrack* crot,
      const TrackStateOnSurface* tsos,
      std::vector<int>& information,
      std::bitset<Trk::numberOfDetectorTypes>& hitPattern) const override
    {
      analyse(Gaudi::Hive::currentContext(),
              track,
              crot,
              tsos,
              information,
              hitPattern);
    }

    virtual void addDetailedTrackSummary(
      const Track& track,
      Trk::TrackSummary& summary) const override
    {
      addDetailedTrackSummary(Gaudi::Hive::currentContext(), track, summary);
    }

  };

  inline const InterfaceID& Trk::IExtendedTrackSummaryHelperTool::interfaceID()
  {
    return IID_ITrackSummaryHelperTool;
  }

}
#endif
