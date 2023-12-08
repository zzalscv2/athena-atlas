/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ClusterTimeProjectionMMClusterBuilderTool_h
#define ClusterTimeProjectionMMClusterBuilderTool_h


#include "AthenaBaseComps/AthAlgTool.h"
#include "MMClusterization/IMMClusterBuilderTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MMPrepData.h"
#include "MuonCondData/NswErrorCalibData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include <array>

namespace Muon {
    class ClusterTimeProjectionMMClusterBuilderTool : virtual public IMMClusterBuilderTool, public AthAlgTool {
    public:
        ClusterTimeProjectionMMClusterBuilderTool(const std::string&, const std::string&, const IInterface*);

        virtual ~ClusterTimeProjectionMMClusterBuilderTool() = default;
        StatusCode initialize() override;

        StatusCode getClusters(const EventContext& ctx,
                              std::vector<Muon::MMPrepData>&& stripsVect,
                              std::vector<std::unique_ptr<Muon::MMPrepData>>& clustersVect) const override;

        virtual RIO_Author getCalibratedClusterPosition(const EventContext& ctx,
                                                        const std::vector<NSWCalib::CalibratedStrip>& calibratedStrips,
                                                        const Amg::Vector3D& directionEstimate, 
                                                        Amg::Vector2D& clusterLocalPosition,
                                                        Amg::MatrixX& covMatrix) const override;

    private:
        using LaySortedPrds = std::array<std::vector<Muon::MMPrepData>, 8>;
        /// Muon Detector Descriptor
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        
        SG::ReadCondHandleKey<NswErrorCalibData> m_uncertCalibKey{this, "ErrorCalibKey", "NswUncertData",
                                                                 "Key of the parametrized NSW uncertainties"};

        Gaudi::Property<bool> m_writeStripProperties{this, "writeStripProperties" , true};
        Gaudi::Property<uint> m_maxHoleSize{this, "maxHoleSize", 1};

        LaySortedPrds sortHitsToLayer(std::vector<Muon::MMPrepData>&& MMprds) const;

        std::vector<std::vector<uint>> clusterLayer(const std::vector<Muon::MMPrepData>& MMPrdsPerLayer) const;

        std::pair<double, double> getClusterPositionPRD(const NswErrorCalibData& errorCalibDB,
                                                        uint8_t author,
                                                        const std::vector<NSWCalib::CalibratedStrip>& features, 
                                                        const Amg::Vector3D& thetaEstimate) const;

        StatusCode writeClusterPrd(const EventContext& ctx,
                                   const std::vector<Muon::MMPrepData>& constituents, 
                                   const double clustersPosition, 
                                   const double clustersPositionErrorSq,
                                   std::vector<std::unique_ptr<Muon::MMPrepData>>& mergedClust) const;

        uint channel(const Identifier& id) const { return m_idHelperSvc->mmIdHelper().channel(id); }
        uint channel(const MMPrepData& strip) const { return channel(strip.identify()); }
    };  // class ClusterTimeProjectionMMClusterBuilderTool

}  // namespace Muon
#endif  // ClusterTimeProjectionMMClusterBuilderTool_h
