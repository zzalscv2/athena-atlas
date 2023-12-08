/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUPATHITTOOL_H
#define MUPATHITTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuPatPrimitives/MuPatHit.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "MuonRecToolInterfaces/IMdtDriftCircleOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonClusterOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonCompetingClustersOnTrackCreator.h"
#include "TrkExInterfaces/IPropagator.h"
#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkToolInterfaces/IResidualPullCalculator.h"


namespace Muon {

    class MuonEDMPrinterTool;
    class MuonSegment;

    class MuPatHitTool : public AthAlgTool {
    public:
        /** default AlgTool constructor */
        MuPatHitTool(const std::string&, const std::string&, const IInterface*);

        /** destructor */
        ~MuPatHitTool();

        /** initialize method, method taken from bass-class AlgTool */
        StatusCode initialize() override;

        /** @brief access to tool interface */
        static const InterfaceID& interfaceID() {
            static const InterfaceID IID_MuPatHitTool("Muon::MuPatHitTool", 1, 0);
            return IID_MuPatHitTool;
        }

        /** @brief create a MCTBList from a MuonSegment
            @param seg the MuonSegment
            @param hitList the list to be filled
            @return true if creation succeded
        */
        bool create(const EventContext& ctx, const MuonSegment& seg, MuPatHitList& hitList) const;

        /** @brief create a MuPatHitList from a Track
            @param track the input track
            @param hitList the list to be filled
            @return true if creation succeded
        */
        bool create(const Trk::Track& track, MuPatHitList& hitList) const;

        /** @brief create a MuPatHitList from a Track
            @param pars the input parameters
            @param measVec the list of measurements
            @param hitList the list to be filled
            @return true if creation succeded
        */
        bool create(const EventContext& ctx, const Trk::TrackParameters& pars, const std::vector<const Trk::MeasurementBase*>& measVec,
                    MuPatHitList& hitList) const;

        /** @brief merge two MuPatHitLists into a new one
            @param hitList1 the first  list
            @param hitList2 the second list
            @param outList  the resulting list
            @return true if merge succeded
        */
        static MuPatHitList merge(const MuPatHitList& hitList1, const MuPatHitList& hitList2) ;

        /** @brief merge two MuPatHitLists into a new one. The first list will be added to the second
            @param hitList1 the first  list
            @param hitList2 the second list
            @return true if merge succeded
        */

        static bool extract(const MuPatHitList& hitList, std::vector<const Trk::MeasurementBase*>& measVec, bool usePreciseHits = true,
                     bool getReducedTrack = false) ;

        /** @brief check whether the list is correctly sorted */
        static bool isSorted(const MuPatHitList& hitList) ;

        /** print the list of hits, with optional parts of the printout (position,direction,momentum) */
        std::string print(const MuPatHitList& hitList, bool printPos = true, bool printDir = true, bool printMom = true) const;

        /** remove hit with a give Identifier */
        static bool remove(const Identifier& id, MuPatHitList& hitList) ;

        /** remove hit containing give measurement (uses pointer comparison) */
        bool remove(const Trk::MeasurementBase& meas, MuPatHitList& hitList) const;

        /** update hit list for a give track */
        bool update(const Trk::Track& track, MuPatHitList& hitList) const;

    private:
        /** @brief get hit type */
        MuPatHit::Type getHitType(const Identifier& id) const;

        /** @brief get hit info */
        MuPatHit::Info getHitInfo(const Trk::MeasurementBase& meas) const;

        /** @brief calculate broad measurement for a give precise measurement */
        std::unique_ptr<const Trk::MeasurementBase> createBroadMeasurement(const Trk::MeasurementBase& preciseMeas,
                                                                           const MuPatHit::Info& hitInfo) const;

        /** @brief calculate the residuals */
        void calculateResiduals(const Trk::TrackStateOnSurface* tsos, Trk::ResidualPull::ResidualType type, double& residual,
                                            double& residualPull) const;
        ToolHandle<Trk::IPropagator> m_propagator{
            this,
            "AtlasRungeKuttaPropagator",
            "Trk::RungeKuttaPropagator/AtlasRungeKuttaPropagator",
        };
        ToolHandle<IMdtDriftCircleOnTrackCreator> m_mdtRotCreator{
            this,
            "MdtRotCreator",
            "Muon::MdtDriftCircleOnTrackCreator/MdtDriftCircleOnTrackCreator",
        };  //<! tool to calibrate MDT hits
        ToolHandle<IMuonClusterOnTrackCreator> m_cscRotCreator{
            this,
            "CscRotCreator",
            "Muon::CscClusterOnTrackCreator/CscClusterOnTrackCreator",
        };  //<! tool to calibrate CSC hits
        
        ToolHandle<Trk::IResidualPullCalculator> m_pullCalculator{
            this,
            "ResidualPullCalculator",
            "Trk::ResidualPullCalculator/ResidualPullCalculator",
        };  //<! tool to calculate residuals and pulls
        PublicToolHandle<MuonEDMPrinterTool> m_printer{
            this,
            "Printer",
            "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool",
        };  //<! tool to print EDM objects

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
            this,
            "MuonIdHelperSvc",
            "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
        };
        ServiceHandle<IMuonEDMHelperSvc> m_edmHelperSvc{
            this,
            "edmHelper",
            "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc",
            "Handle to the service providing the IMuonEDMHelperSvc interface",
        };  //<! multipurpose helper tool

        Trk::MagneticFieldProperties m_magFieldProperties{Trk::NoField};  //!< magnetic field properties

        Gaudi::Property<bool> m_isCosmic{this, "isCosmic", false,
                                         "Toggle whether the job runs on cosmic data. That influences the hit sorting on track"};
    };

}  // namespace Muon

#endif
