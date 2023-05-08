/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 test MuonGeoModel from digits to pos. in space
 ----------------------------------------------
 ***************************************************************************/

#ifndef MUONGEOMODEL_MUONGMCHECK_H
#define MUONGEOMODEL_MUONGMCHECK_H

#include <cmath>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonCalibITools/IIdToFixedIdTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace MuonGM {
class MuonReadoutElement;
class MuonDetectorManager;
class RpcReadoutElement;
}  // namespace MuonGM

class MuonGMCheck : public AthAlgorithm {
   public:
    MuonGMCheck(const std::string& name, ISvcLocator* pSvcLocator);
    ~MuonGMCheck() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;
    void clearCache();

   private:
    // User setable properties
    Gaudi::Property<bool> m_event_loop{this, "EventLoopMode", false};
    Gaudi::Property<bool> m_minimal_checks{this, "minimalChecks", true};
    Gaudi::Property<bool> m_check_mdt{this, "check_mdt", false};
    Gaudi::Property<bool> m_check_rpc{this, "check_rpc", false};
    Gaudi::Property<bool> m_check_tgc{this, "check_tgc", false};
    Gaudi::Property<bool> m_check_csc{this, "check_csc", false};
    Gaudi::Property<bool> m_check_stgc{this, "check_stgc", false};
    Gaudi::Property<bool> m_check_mm{this, "check_mm", false};
    Gaudi::Property<bool> m_check_cscrsmap{this, "buildCscRegionSelectorMap",
                                           false};
    Gaudi::Property<bool> m_check_rpcrsmap{this, "buildRpcRegionSelectorMap",
                                           false};
    Gaudi::Property<bool> m_check_mdtrsmap{this, "buildMdtRegionSelectorMap",
                                           false};
    Gaudi::Property<bool> m_check_tgcrsmap{this, "buildTgcRegionSelectorMap",
                                           false};
    Gaudi::Property<bool> m_testMdtCache{this, "testMdtCache", false};
    Gaudi::Property<bool> m_testRpcCache{this, "testRpcCache", false};
    Gaudi::Property<bool> m_testTgcCache{this, "testTgcCache", false};
    Gaudi::Property<bool> m_testCscCache{this, "testCscCache", false};
    Gaudi::Property<bool> m_testMdtDetectorElementHash{
        this, "testMdtDetectorElementHash", false};
    Gaudi::Property<bool> m_testRpcDetectorElementHash{
        this, "testRpcDetectorElementHash", false};
    Gaudi::Property<bool> m_testTgcDetectorElementHash{
        this, "testTgcDetectorElementHash", false};
    Gaudi::Property<bool> m_testCscDetectorElementHash{
        this, "testCscDetectorElementHash", false};
    Gaudi::Property<int> m_check_first_last{this, "check_first_last", 1};
    Gaudi::Property<bool> m_check_parent{this, "check_ParentStation", false};
    Gaudi::Property<bool> m_check_blines{this, "check_Blines", false};
    Gaudi::Property<bool> m_check_surfaces{this, "check_surfaces", false};
    Gaudi::Property<bool> m_check_surfaces_details{
        this, "check_surfaces_details", false};
    Gaudi::Property<bool> m_check_rpc_distToReadout{
        this, "check_rpc_distToReadout", false};
    Gaudi::Property<bool> m_tgcgood{this, "print_mdt_good_hits", false};
    Gaudi::Property<bool> m_rpcgood{this, "print_rpc_good_hits", false};
    Gaudi::Property<bool> m_mdtgood{this, "print_tgc_good_hits", false};

    MuonGM::MuonDetectorManager* p_MuonMgr{nullptr};

    ToolHandle<MuonCalib::IIdToFixedIdTool> m_fixedIdTool{
        this, "idTool", "MuonCalib::IdToFixedIdTool"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    int m_mem{0};  //<! counter for memory allocated VmSize values read from
                   ///proc/<pid>/status
    std::array<int, 2>
        m_cpu{};  //<! counter for cpu time read from /proc/<pid>/cpu
    void showVmemCpu(const std::string& message);
    void getVmemCpu(int& dVmem, int& duCpu, int& dsCpu);

    void testMdtCache();
    void testRpcCache();
    void testTgcCache();
    void testCscCache();

    void test_MM_IdHelpers();
    void test_sTGC_IdHelpers();

    void getEtaPhiPanelBoundaries(const MuonGM::RpcReadoutElement* rpc,
                                  Identifier& chid, double& etamin,
                                  double& etamax, double& phimin,
                                  double& phimax);
    void getZPhiPanelBoundaries(const MuonGM::RpcReadoutElement* rpc,
                                Identifier& chid, double& zmin, double& zmax,
                                double& phimin, double& phimax);
    void getEtaPhiActivePanelBoundaries(const MuonGM::RpcReadoutElement* rpc,
                                        Identifier& chid, double& etamin,
                                        double& etamax, double& phimin,
                                        double& phimax);
    void getZPhiActivePanelBoundaries(const MuonGM::RpcReadoutElement* rpc,
                                      Identifier& chid, double& zmin,
                                      double& zmax, double& phimin,
                                      double& phimax);
    void getPanelBoundaries(const MuonGM::RpcReadoutElement* rpc,
                            Identifier& chid, double& etamin, double& etamax,
                            double& phimin, double& phimax, double& zmin,
                            double& zmax);
    void getActivePanelBoundaries(const MuonGM::RpcReadoutElement* rpc,
                                  Identifier& chid, double& etamin,
                                  double& etamax, double& phimin,
                                  double& phimax, double& zmin, double& zmax);
    void getPanelEdgeCenter(const MuonGM::RpcReadoutElement* rpc,
                            Identifier& chid, double& xC, double& yC,
                            double& zC, double& xFirstPhiS, double& yFirstPhiS,
                            double& zFirstPhiS, double& xLastPhiS,
                            double& yLastPhiS, double& zLastPhiS);

    void buildCscRegionSelectorMap();
    void buildRpcRegionSelectorMap();
    void buildMdtRegionSelectorMap();
    void buildTgcRegionSelectorMap();
    void checkreadoutstgcgeo();
    void checkreadoutmmgeo();
    void checkreadoutmdtgeo();
    void checkreadoutcscgeo();
    void checkreadoutrpcgeo();
    void checkParentStation();
    void checkreadouttgcgeo();
    void testMdtDetectorElementHash();
    void testRpcDetectorElementHash();
    void testTgcDetectorElementHash();
    void testCscDetectorElementHash();

    void coercePositivePhi(double& phi);
};
inline void MuonGMCheck::coercePositivePhi(double& phi) {
    if (phi < 0)
        phi += 2 * M_PI;
}

#endif  // MUONGEOMODEL_MUONGMCHECK_H
