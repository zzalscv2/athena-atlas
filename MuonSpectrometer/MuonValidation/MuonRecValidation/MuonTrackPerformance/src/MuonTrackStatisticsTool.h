/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTRACKSTATISTICSTOOL_MUONTRACKSTATISTICSTOOL_H
#define MUONTRACKSTATISTICSTOOL_MUONTRACKSTATISTICSTOOL_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "TrkToolInterfaces/ITruthToTrack.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTruthData/DetailedTrackTruthCollection.h"

/* namespace Muon { */
/*   class IMuonEDMHelperSvc; */
/* }     */

/* namespace Trk { */
/*   class ITruthToTrack; */
/* } */

static const InterfaceID IID_MuonTrackStatisticsTool("MuonTrackStatisticsTool", 1, 0);

class MuonTrackStatisticsTool : public AthAlgTool {
public:
    // AlgTool Constructor
    MuonTrackStatisticsTool(const std::string&, const std::string&, const IInterface*);
    ~MuonTrackStatisticsTool(){};

    // tool interface
    static const InterfaceID& interfaceID() { return IID_MuonTrackStatisticsTool; }

    // Gaudi algorithm hooks
    StatusCode initialize();
    StatusCode finalize();
    // statistics to be counted
    struct TrackCounters {
        TrackCounters(const std::string & trkLoc) :
            trackLocation(trkLoc),
            nEvents(0),
            nTracks(0),
            nHits(0),
            nEtaHits(0),
            nEtaTrig(0),
            nPhiHits(0),
            nScatter(0),
            nHoles(0),
            nPsudo(0),
            summedchi2(0) {}

        std::string trackLocation;
        int nEvents;
        int nTracks;
        int nHits;
        int nEtaHits;
        int nEtaTrig;
        int nPhiHits;
        int nScatter;
        int nHoles;
        int nPsudo;
        double summedchi2;
    };

    struct TruthTrackCounters {
        TruthTrackCounters(const std::string & trkLoc) : trackLocation(trkLoc), nEvents(0), nTracks(0) {
            for (unsigned int i = 0; i < 3; i++) {
                nPIXELhits[i] = 0;
                nSCThits[i] = 0;
                nTRThits[i] = 0;
                nMDThits[i] = 0;
                nRPChits[i] = 0;
                nTGChits[i] = 0;
                nCSChits[i] = 0;
            }
        };

        std::string trackLocation;
        int nEvents;
        int nTracks;
        int nPIXELhits[3];
        int nSCThits[3];
        int nTRThits[3];
        int nMDThits[3];
        int nRPChits[3];
        int nTGChits[3];
        int nCSChits[3];
    };

    // function to access tracks and update counters
    // First function is an interface to MuonTrackStatisticsAlg, which passes in the retrieved collection from SG and the name, which it
    // uses to find the appropriate counter The counter and collection are then passed to the 2nd function, which does the updating
    StatusCode updateTrackCounters(const std::string& name, const TrackCollection* tracks);
    StatusCode updateTrackCounters(TrackCounters& counters, const TrackCollection& tracks);

    StatusCode updateTruthTrackCounters(const std::string& name, const DetailedTrackTruthCollection* truthMap);
    StatusCode updateTruthTrackCounters(TruthTrackCounters& counters, const DetailedTrackTruthCollection& TruthMap);

    void addTrackCounters(const std::string& trkLoc);
    //  void addTruthTrackCounters( std::string trkLoc ) const;
    std::string printTrackCounters() const;

private:
    // access to Id Helpers
    ServiceHandle<Muon::IMuonEDMHelperSvc> m_edmHelperSvc{this, "edmHelper", "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc",
                                                          "Handle to the service providing the IMuonEDMHelperSvc interface"};
   
    bool m_doTruth;

    std::vector<MuonTrackStatisticsTool::TrackCounters*> m_allCounters;
    std::vector<MuonTrackStatisticsTool::TruthTrackCounters*> m_allTruthCounters;

    void storeTruthTracks(void);
};

#endif  // MUONTRACKSTATISTICSTOOL_MUONTRACKSTATISTICSTOOL_H
