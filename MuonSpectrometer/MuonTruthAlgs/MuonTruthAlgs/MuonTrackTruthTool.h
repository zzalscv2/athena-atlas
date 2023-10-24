/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_MUONTRACKTRUTHTOOL_H
#define MUON_MUONTRACKTRUTHTOOL_H

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonRecToolInterfaces/IMuonTrackTruthTool.h"
#include "MuonSimData/CscSimDataCollection.h"
#include "MuonSimData/MuonSimDataCollection.h"
#include "TrackRecord/TrackRecordCollection.h"
#include "TrkToolInterfaces/ITruthTrajectoryBuilder.h"
#include "TrkTrack/TrackCollection.h"

class TruthTrajectory;

namespace MuonGM {
    class MuonDetectorManager;
}

namespace Muon {
    class MuonSegment;
}

namespace Trk {
    class Track;
    class MeasurementBase;
}  // namespace Trk

namespace Muon {

    /**
       @brief Tool to calculate track truth

    */
    class MuonTrackTruthTool : virtual public IMuonTrackTruthTool, public AthAlgTool {
    public:
        struct SortResultByMatchedHits {
            bool operator()(const MatchResult& r1, const MatchResult& r2) const;
            bool operator()(const SegmentMatchResult& r1, const SegmentMatchResult& r2) const;
            bool operator()(const MuonTrackTruth& r1, const MuonTrackTruth& r2) const;
        };

    public:
        /** @brief constructor */
        MuonTrackTruthTool(const std::string&, const std::string&, const IInterface*);

        /** @brief destructor */
        ~MuonTrackTruthTool() = default;

        /** @brief AlgTool initilize */
        StatusCode initialize();

        /** @brief perform truth matching for a given set of tracks */
        ResultVec match(const TruthTree& truth_tree, const TrackCollection& tracks) const;

        /** @brief perform truth matching for a given set of segments */
        SegmentResultVec match(const TruthTree& truth_tree, const std::vector<const MuonSegment*>& segments) const;

        /** @brief get track truth */
        MuonTrackTruth getTruth(const TruthTree& truth_tree, const Trk::Track& track, bool restrictedTruth = false) const;

        /** @brief get segment truth for a list of segments, the segments will be considered to belong to the same muon */
        MuonTrackTruth getTruth(const TruthTree& truth_tree, const std::vector<const MuonSegment*>& segments,
                                bool restrictedTruth = false) const;

        /** @brief get segment truth */
        MuonTrackTruth getTruth(const TruthTree& truth_tree, const Muon::MuonSegment& segment) const;

        /** @brief get truth for a give set of hits. If restrictedTruth is set to true only missed hits
            in chambers with hits will be counted.
        */
        MuonTrackTruth getTruth(const TruthTree& truth_tree, const std::vector<const Trk::MeasurementBase*>& measurements,
                                bool restrictedTruth = false) const;

        /** create truth tree from sim data */
        const TruthTree createTruthTree(const TrackRecordCollection* truthTrackCol, const McEventCollection* mcEventCollection,
                                        const std::vector<const MuonSimDataCollection*>& muonSimData,
                                        const CscSimDataCollection* cscSimDataMap) const;

        /// Returns the mother particle of the particle with barcodeIn if it is found in the truth trajectory.
        /// It traces the decay chain until if finds the first particle that is different flavor from the starting one.
        HepMC::ConstGenParticlePtr getMother(const TruthTrajectory& traj, const int barcodeIn) const;

        /// Returns the ancestor particle of the particle with barcodeIn if it is found in the truth trajectory.
        /// Ancestor here means the last particle at generator level that has a status code different from final state, e.g. Z
        HepMC::ConstGenParticlePtr getAncestor(const TruthTrajectory& traj, const int barcodeIn) const;

        /// Returns the initial particle of the particle with barcodeIn if it is found in the truth trajectory.
        /// For example a mu undergoing a mubrem would create a second mu, in which case this method returns the mu prior to bremsstrahlung.
        /// This interface calls the method getInitialPair.
        HepMC::ConstGenParticlePtr getInitial(const TruthTrajectory& traj, const int barcodeIn) const;

        /// Returns the number of steps a particle took while maintaining its PDG ID.
        /// This method calls getInitialPair for calculating this number.
        unsigned int getNumberOfScatters(const TruthTrajectory& traj, const int barcodeIn) const;

    private:
        MuonTrackTruth getTruth(const std::vector<const Trk::MeasurementBase*>& measurements, const TruthTreeEntry& truthEntry,
                                bool restrictedTruth) const;

        void addSimDataToTree(TruthTree& truth_tree, std::map<int, int>& barcode_map, const MuonSimDataCollection* simDataCol) const;
        void addCscSimDataToTree(TruthTree& truth_tree, std::map<int, int>& barcode_map, const CscSimDataCollection* simDataCol) const;

        void addMdtTruth(MuonTechnologyTruth& trackTruth, const Identifier& id, const Trk::MeasurementBase& meas,
                         const MuonSimDataCollection& simCol) const;

        void addClusterTruth(MuonTechnologyTruth& trackTruth, const Identifier& id, const Trk::MeasurementBase& meas,
                             const MuonSimDataCollection& simCol) const;
        void addClusterTruth(MuonTechnologyTruth& trackTruth, const Identifier& id, const Trk::MeasurementBase& meas,
                             const CscSimDataCollection& simCol) const;

        void addMissedHits(MuonTechnologyTruth& truth, const std::set<Identifier>& ids, const std::set<Identifier>& chids,
                           const MuonSimDataCollection& simCol, bool restrictedTruth) const;

        void addMissedHits(MuonTechnologyTruth& truth, const std::set<Identifier>& ids, const std::set<Identifier>& chids,
                           const CscSimDataCollection& simCol, bool restrictedTruth) const;

        int manipulateBarCode(int barcode) const;

        bool selectPdg(int pdg) const { return m_selectedPdgs.count(pdg); }

        /// Returns the initial particle of the particle with barcodeIn if it is found in the truth trajectory.
        /// For example a mu undergoing a mubrem would create a second mu, in which case this method returns the mu prior to bremsstrahlung.
        /// The number of such scatters is returned in the .second.
        const std::pair<HepMC::ConstGenParticlePtr, unsigned int> getInitialPair(const TruthTrajectory& traj, const int barcodeIn) const;

        const MuonGM::MuonDetectorManager* m_detMgr;

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        ToolHandle<Muon::MuonEDMPrinterTool> m_printer{this, "Printer", "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool"};
        ToolHandle<Trk::ITruthTrajectoryBuilder> m_truthTrajectoryBuilder{
            this, "TruthTrajectoryBuilder", "Muon::MuonDecayTruthTrajectoryBuilder/MuonDecayTruthTrajectoryBuilder"};

        Gaudi::Property<bool> m_manipulateBarCode{this, "ManipulateBarCode", false};
        Gaudi::Property<bool> m_doSummary{this, "DoSummary", false};
        Gaudi::Property<bool> m_matchAllParticles{this, "MatchAllParticles", true};
        Gaudi::Property<unsigned int> m_minHits{this, "MinHits", 4};
        Gaudi::Property<std::vector<int>> m_pdgsToBeConsidered{this, "ConsideredPDGs", {}};

        std::set<int> m_selectedPdgs;  // set storing particle PDG's considered for matching
    };

}  // namespace Muon

#endif  // MUON_MDTONTRACKTOOL_H
