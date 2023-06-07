/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 MuonGeoModel description
 -----------------------------------------
 ***************************************************************************/

#include "MuonGeoModelTest/MuonGMCheck.h"

#include <boost/format.hpp>
#include <fstream>
#include <sstream>

#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "MuonAlignmentData/ALinePar.h"
#include "MuonAlignmentData/BLinePar.h"
#include "MuonDigitContainer/MdtDigit.h"
#include "MuonDigitContainer/MdtDigitCollection.h"
#include "MuonDigitContainer/MdtDigitContainer.h"
#include "MuonDigitContainer/RpcDigit.h"
#include "MuonDigitContainer/RpcDigitCollection.h"
#include "MuonDigitContainer/RpcDigitContainer.h"
#include "MuonGeoModelTest/PerfUtils.h"
#include "MuonIdHelpers/MdtIdHelper.h"
#include "MuonReadoutGeometry/CscReadoutElement.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonReadoutGeometry/RpcReadoutSet.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "MuonReadoutGeometry/TgcReadoutParams.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "TrkSurfaces/Surface.h"

typedef std::istringstream mystream;

namespace {
    constexpr double const invRad = 180. / M_PI;
    // the tube number of a tube in a tubeLayer is encoded in the GeoSerialIdentifier (modulo maxNTubesPerLayer)
    constexpr unsigned int maxNTubesPerLayer = MdtIdHelper::maxNTubesPerLayer;

}  // namespace

using namespace MuonGM;

MuonGMCheck::MuonGMCheck(const std::string& name, ISvcLocator* pSvcLocator) : AthAlgorithm(name, pSvcLocator), p_MuonMgr(nullptr) {
    m_mem = 0;
    m_cpu[0] = 0;
    m_cpu[1] = 0;

    
}

StatusCode MuonGMCheck::initialize() {
    ATH_MSG_DEBUG(" starting up");
    showVmemCpu("initialize");

    // first get helpers
    ATH_MSG_DEBUG("Get Muon Id Helpers from the det store (through their converters)");

    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_fixedIdTool.retrieve());

    ATH_MSG_DEBUG(" Muon Id Helper retrieved from handle ");
    showVmemCpu("initialize (IDHELPER retrieved from handle)");

    // now get the MuonDetManager
    ATH_CHECK(detStore()->retrieve(p_MuonMgr));
    // test when is the geometry really loaded
    ATH_MSG_DEBUG(" MuonDetectorManager  is retrieved ");
    showVmemCpu("initialize (MGManager retrieved)");
    ATH_MSG_INFO("# of Mdt/Rpc/Tgc/Csc ReadoutElements " << p_MuonMgr->nMdtRE() << "/" << p_MuonMgr->nRpcRE() << "/" << p_MuonMgr->nTgcRE()
                                                         << "/" << p_MuonMgr->nCscRE());
    showVmemCpu("initialize (MGManager queried for nRE)");

    if (m_event_loop == 0) {
        if (m_check_csc) checkreadoutcscgeo();
        if (m_check_mdt) checkreadoutmdtgeo();
        if (m_check_stgc) checkreadoutstgcgeo();
        if (m_check_mm) checkreadoutmmgeo();
        if (m_check_tgc) checkreadouttgcgeo();
        if (m_check_rpc) checkreadoutrpcgeo();
        if (m_check_rpcrsmap) buildRpcRegionSelectorMap();
        if (m_check_mdtrsmap) buildMdtRegionSelectorMap();
        if (m_check_tgcrsmap) buildTgcRegionSelectorMap();
        if (m_check_cscrsmap) buildCscRegionSelectorMap();
        if (m_check_parent) checkParentStation();
    }

    return StatusCode::SUCCESS;
}

void MuonGMCheck::test_MM_IdHelpers() { ATH_MSG_INFO("Executing ..."); }

void MuonGMCheck::test_sTGC_IdHelpers() { ATH_MSG_INFO("Executing ..."); }

StatusCode MuonGMCheck::execute() {
    ATH_MSG_DEBUG("Executing");
    if (m_event_loop == 0) {
        // on first event test MM and sTGC helpers
        test_MM_IdHelpers();
        test_sTGC_IdHelpers();

        ATH_MSG_INFO("Don't need to loop over events --- just doing nothing");
        return StatusCode::SUCCESS;
    }

    showVmemCpu("execute (start new event)");
    if (m_testMdtCache) testMdtCache();
    showVmemCpu("execute (after MdtCacheFilling)");
    if (m_testRpcCache) testRpcCache();
    showVmemCpu("execute (after RpcCacheFilling)");
    if (m_testTgcCache) testTgcCache();
    showVmemCpu("execute (after TgcCacheFilling)");
    if (m_testCscCache) testCscCache();
    showVmemCpu("execute (after CscCacheFilling)");

    if (m_check_csc) checkreadoutcscgeo();
    if (m_check_mdt) checkreadoutmdtgeo();
    if (m_check_tgc) checkreadouttgcgeo();
    if (m_check_rpc) checkreadoutrpcgeo();

    if (m_testMdtDetectorElementHash) testMdtDetectorElementHash();
    if (m_testRpcDetectorElementHash) testRpcDetectorElementHash();
    if (m_testTgcDetectorElementHash) testTgcDetectorElementHash();
    if (m_testCscDetectorElementHash) testCscDetectorElementHash();

    if (p_MuonMgr->cachingFlag() == 0) p_MuonMgr->clearCache();
    showVmemCpu("execute (after clearing cache)");

    return StatusCode::SUCCESS;
}

void MuonGMCheck::clearCache() {
    if (p_MuonMgr->cachingFlag() == 0) p_MuonMgr->clearCache();
}

void MuonGMCheck::checkreadoutrpcgeo() {
    ATH_MSG_INFO(" *************************** Global Check for Rpc");

    if (p_MuonMgr->nRpcRE() == 0) {
        ATH_MSG_INFO(" No RpcReadoutElements found ");
        return;
    } else
        ATH_MSG_INFO(p_MuonMgr->nRpcRE() << " RpcReadoutElements found ");

    std::string gVersion = p_MuonMgr->geometryVersion();
    std::string fileName = "rpc_current_" + gVersion;
    std::string fileName1 = "testPadPos_" + gVersion;
    std::string fileNamePad = "rpc_current_padPos_" + gVersion;
    std::string fileNamePanel = "rpc_current_panelID_" + gVersion;
    std::string fileNamePanelHashIds = "rpc_current_panelHashID_" + gVersion;

    std::ofstream fout(fileName.c_str());
    ATH_MSG_INFO(" ***** Writing file " << fileName);
    std::ofstream fout1(fileName1.c_str());
    ATH_MSG_INFO(" ***** Writing file " << fileName);
    std::ofstream fpanelid(fileNamePanel.c_str());
    ATH_MSG_INFO(" ***** Writing file " << fileNamePanel);
    std::ofstream fpad(fileNamePad.c_str());
    ATH_MSG_INFO(" ***** Writing file " << fileNamePad);
    std::ofstream fpanelidh(fileNamePanelHashIds.c_str());
    ATH_MSG_INFO(" ***** Writing file " << fileNamePanelHashIds);

    fout << setiosflags(std::ios::fixed) << std::setprecision(4) << std::endl;
    fout1 << setiosflags(std::ios::fixed) << std::setprecision(4) << std::endl;
    fpanelid << setiosflags(std::ios::fixed) << std::setprecision(4) << std::endl;
    fpad << setiosflags(std::ios::fixed) << std::setprecision(4) << std::endl;
    fpanelidh << setiosflags(std::ios::fixed) << std::setprecision(4) << std::endl;

    double phistr_pitch{0.}, etastr_pitch{0.};
    int nphistr{0}, netastr{0};

    for (int sname_index = 0; sname_index < MuonDetectorManager::NRpcStatType; ++sname_index) {
        for (int seta_index = 0; seta_index < MuonDetectorManager::NRpcStatEta; ++seta_index) {
            for (int sphi_index = 0; sphi_index < MuonDetectorManager::NRpcStatPhi; ++sphi_index) {
                for (int dbr_index = 0; dbr_index < MuonDetectorManager::NDoubletR; ++dbr_index) {
                    double keepxC{0.}, keepyC{0.}, keepzC{0.}, keepxFS{0.}, keepyFS{0.}, keepzFS{0.}, keepxLS{0.}, keepyLS{0.}, keepzLS{0.};

                    for (int dbz_index = 0; dbz_index < MuonDetectorManager::NDoubletZ; ++dbz_index) {
                        fout << " ///////////////////// Looking for a RpcReadoutElement for indices = " << sname_index << " " << seta_index
                             << " " << sphi_index << " " << dbr_index << " " << dbz_index << std::endl;
                        int stationName = p_MuonMgr->rpcStationName(sname_index);
                        bool isValid = false;
                        Identifier id =
                            p_MuonMgr->rpcIdHelper()->channelID(stationName, seta_index, sphi_index, dbr_index, dbz_index, 1, 1, 1, 1, isValid);  
                                                                            // last 4 arguments are: int doubletPhi, int gasGap, int
                                                                            // measuresPhi, int strip, bool& isValid
                        if (!isValid) continue;
                        const RpcReadoutElement* rpc = p_MuonMgr->getRpcReadoutElement(id);

                        if (!rpc) continue;
                        fout << " ///////////////////// Found a RpcReadoutElement for indices = " << sname_index << " " << seta_index << " "
                             << sphi_index << " " << dbr_index << " " << dbz_index << std::endl;
                        Identifier idr = rpc->identify();
                        int ndbphi = rpc->NphiStripPanels();
                        fout << " its offline hash Id = " << rpc->identifyHash() << std::endl;
                        fout << " its offline Id = " << m_idHelperSvc->rpcIdHelper().show_to_string(idr)
                             << " ////////////////// belongs to module " << rpc->getTechnologyName() << "/" << rpc->getStationName()
                             << " # doubletPhi = " << ndbphi << std::endl;
                        Identifier idp = m_idHelperSvc->rpcIdHelper().parentID(idr);
                        fout << "      parent Id = " << m_idHelperSvc->rpcIdHelper().show_to_string(idp) << std::endl;
                        fout << " Center of the RpcReadoutElement at " << rpc->center() << std::endl;
                        int doubletR = m_idHelperSvc->rpcIdHelper().doubletR(idr);
                        int doubletZ = m_idHelperSvc->rpcIdHelper().doubletZ(idr);
                        int stEta = rpc->getStationEta();
                        int stPhi = rpc->getStationPhi();
                        int stNameInt = m_idHelperSvc->rpcIdHelper().stationName(idr);
                        std::string stNameString = m_idHelperSvc->rpcIdHelper().stationNameString(stNameInt);

                        RpcReadoutSet Set(p_MuonMgr, idr);
                        int ndbz = Set.NdoubletZ();
                        fout1 << " its offline Id = " << m_idHelperSvc->rpcIdHelper().show_to_string(idr) << " means " << stNameString
                              << " eta " << stEta << " phi " << stPhi << " doubletR " << doubletR << " dbZ " << doubletZ << "(out of "
                              << ndbz << " in the set); n. of dbPhi in this chamber = " << ndbphi << std::endl;

                        const MuonStation* ms = rpc->parentMuonStation();
                        if (ms)
                            fout << "Parent MuonStation found " << std::endl;
                        else
                            fout << "Parent MuonStation NOT found for element " << m_idHelperSvc->rpcIdHelper().show_to_string(idr)
                                 << std::endl;

                        fout << " For this Module,  gasGapSsize() " << rpc->gasGapSsize() << " and stripPanelSsize() "
                             << rpc->stripPanelSsize(1) << " phi-strip-pitch " << rpc->StripPitch(1) << " *Nstrip "
                             << rpc->StripPitch(1) * rpc->NphiStrips() - 2. * CLHEP::mm + 2 * rpc->StripPanelDead(1) - 2 * 9. * CLHEP::mm
                             << std::endl;
                        fout << " For this Module,  gasGapZsize() " << rpc->gasGapZsize() << " and stripPanelZsize() "
                             << rpc->stripPanelZsize(1) << " eta-strip-pitch " << rpc->StripPitch(0) << " *Nstrip "
                             << rpc->StripPitch(0) * rpc->NetaStrips() - 2. * CLHEP::mm + 2 * rpc->StripPanelDead(1) - 2 * 9. * CLHEP::mm
                             << std::endl;

                        for (int idbphi = 1; idbphi <= ndbphi; ++idbphi) {
                            int dbp = m_idHelperSvc->rpcIdHelper().doubletPhi(idr);
                            if (ndbphi > 1 && idbphi > 1) dbp = idbphi;
                            fout << " Changing doubletPhi for  " << m_idHelperSvc->rpcIdHelper().show_to_string(idr) << " "
                                 << rpc->getTechnologyName() << "/" << rpc->getStationName() << " dbr, dbz, dbp " << doubletR << " "
                                 << m_idHelperSvc->rpcIdHelper().doubletZ(idr) << " " << dbp << std::endl;

                            for (int igg = 1; igg < 3; igg++) {
                                //
                                fout << " Changing gas-gap  " << igg << " for  " << m_idHelperSvc->rpcIdHelper().show_to_string(idr) << " "
                                     << rpc->getTechnologyName() << "/" << rpc->getStationName() << " dbr, dbz, dbp " << doubletR << " "
                                     << m_idHelperSvc->rpcIdHelper().doubletZ(idr) << " " << dbp << std::endl;
                                int measphi = 1;
                                fout << " Gas Gap " << igg << " measphi = " << measphi << " phi strip pitch = " << rpc->StripPitch(measphi)
                                     << " n. phi strips = " << rpc->NphiStrips() << std::endl;
                                phistr_pitch += rpc->StripPitch(measphi) * rpc->NphiStrips();
                                nphistr += rpc->NphiStrips();

                                bool localToGlobal_rpc = true;
                                if (localToGlobal_rpc) {
                                    // here (gasgap-level) perform checks on local to global transform
                                    // BMF1 at stEta = 3 stPhi = 6, dbR=1,dbZ=1,dbPhi=1->should be 2 gg=1 or 2
                                    Amg::Vector3D aLocalPoint = Amg::Vector3D(-1., 62.882732, -47.88338);
                                    Identifier idgg = m_idHelperSvc->rpcIdHelper().channelID(
                                        idp, m_idHelperSvc->rpcIdHelper().doubletZ(idr), dbp, igg, measphi, 1);
                                    Amg::Vector3D aGlobalPoint = rpc->localToGlobalCoords(aLocalPoint, idgg);
                                    fout << " Global point = " << aGlobalPoint << std::endl;
                                    double gPhi = aGlobalPoint.phi();
                                    double gTheta = aGlobalPoint.theta();
                                    fout << " Global phi/theta = " << gPhi << "/" << gTheta << std::endl;
                                    // BMF1 at stEta = 3 stPhi = 6, dbR=1,dbZ=1,dbPhi=1->should be 2 gg=1 or 2
                                    fout << " Global phi/theta differences = " << gPhi - (-2.628506) << " " << gTheta - 1.1838122
                                         << std::endl;
                                    // end here (gasgap-level)     checks on local to global transform
                                }

                                double etamin, etamax, phimin, phimax, zmin, zmax;
                                int stripStep = 1;
                                if (m_check_first_last) stripStep = rpc->NphiStrips() - 1;
                                for (int strip = 1; strip <= rpc->NphiStrips();) {
                                    Identifier chid = m_idHelperSvc->rpcIdHelper().channelID(
                                        idp, m_idHelperSvc->rpcIdHelper().doubletZ(idr), dbp, igg, measphi, strip);
                                    if (strip == 1) {
                                        fpanelidh << "IdCodes " << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " "
                                                  << m_idHelperSvc->rpcIdHelper().stationNameString(
                                                         m_idHelperSvc->rpcIdHelper().stationName(chid))
                                                  << " " << chid << " " << chid.get_identifier32().get_compact() << " "
                                                  << chid.get_compact() << " n phi strips = " << rpc->NphiStrips() << std::endl;
                                        getEtaPhiActivePanelBoundaries(rpc, chid, etamin, etamax, phimin, phimax);
                                        getZPhiActivePanelBoundaries(rpc, chid, zmin, zmax, phimin, phimax);
                                        int layerType = 0;
                                        if (doubletR == 2) layerType = 1;
                                        if ((stNameInt > 3 && (stNameInt != 53 && stNameInt != 8)) && doubletR == 1) layerType = 2;
                                        layerType = layerType * 2 + m_idHelperSvc->rpcIdHelper().gasGap(chid);
                                        fpanelid << layerType << " " << stNameString << " phi "
                                                 << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " "
                                                 << chid.get_identifier32().get_compact() << " " << etamin << " " << etamax << " " << phimin
                                                 << " " << phimax << " " << zmin << " " << zmax << std::endl;
                                        bool anyPad = true;
                                        std::string planeString = "confirm";
                                        if (doubletR == 2) planeString = "pivot";
                                        if ((stNameString == "BMS" || stNameString == "BMF" || stNameString == "BME") && doubletR == 2)
                                            anyPad = false;
                                        if (stNameString == "BML" && doubletR == 1) anyPad = false;
                                        if (doubletR > 2) anyPad = false;
                                        if (anyPad) {
                                            if (measphi == 1 && igg == 1) {
                                                if (stNameString == "BMS" || stNameString == "BMF" || stNameString == "BML" ||
                                                    stNameString == "BME" || stNameString == "BOL") {
                                                    double xC, yC, zC, xFirstPhiS, yFirstPhiS, zFirstPhiS, xLastPhiS, yLastPhiS, zLastPhiS;
                                                    getPanelEdgeCenter(rpc, chid, xC, yC, zC, xFirstPhiS, yFirstPhiS, zFirstPhiS, xLastPhiS,
                                                                       yLastPhiS, zLastPhiS);
                                                    fout1 << "dbR, dbZ, dbP = " << doubletR << " " << doubletZ << " " << dbp << " panel c "
                                                          << xC << " " << yC << " " << zC << " panel fs " << xFirstPhiS << " " << yFirstPhiS
                                                          << " " << zFirstPhiS << " ls " << xLastPhiS << " " << yLastPhiS << " "
                                                          << zLastPhiS << std::endl;
                                                    if (ndbz == 1 ||
                                                        (stNameString.substr(0, 3) == "BMS" && abs(stEta) == 4 && dbz_index == 0)) {
                                                        fpanelid << "Pad box position " << stNameString.substr(0, 3) << " eta/phi " << stEta
                                                                 << "/" << stPhi << " " << planeString
                                                                 << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " " << xC << " "
                                                                 << yC << " " << zC << std::endl;
                                                        fout1 << "Pad box position " << stNameString.substr(0, 3) << " eta/phi " << stEta
                                                              << "/" << stPhi << " " << planeString
                                                              << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " " << xC << " " << yC
                                                              << " " << zC << std::endl;
                                                        fpad << stNameString.substr(0, 3) << " " << xC << " " << yC << " " << zC
                                                             << std::endl;

                                                    } else if (ndbz > 1 && dbz_index == 0) {
                                                        if (dbp == 1) {
                                                            keepxFS = xC;
                                                            keepyFS = yC;
                                                            keepzFS = zC;
                                                            fout1 << "keep center for dbp1" << std::endl;
                                                        } else {
                                                            keepxLS = xC;
                                                            keepyLS = yC;
                                                            keepzLS = zC;
                                                            fout1 << "keep center for dbp2" << std::endl;
                                                            keepxC = rpc->center().x();
                                                            keepyC = rpc->center().y();
                                                            keepzC = rpc->center().z();
                                                            fout1 << "keep center for center " << keepxC << " " << keepyC << " " << keepzC
                                                                  << std::endl;
                                                        }
                                                    } else if (ndbz > 1 && dbz_index == 1 &&
                                                               (!(stNameString.substr(0, 3) == "BMS" && abs(stEta) == 4))) {
                                                        if (dbp == 1) {
                                                            xC = 0.5 * (keepxFS + xC);
                                                            yC = 0.5 * (keepyFS + yC);
                                                            zC = 0.5 * (keepzFS + zC);
                                                        } else {
                                                            xC = 0.5 * (keepxLS + xC);
                                                            yC = 0.5 * (keepyLS + yC);
                                                            zC = 0.5 * (keepzLS + zC);
                                                        }
                                                        fpanelid << "Pad box position " << stNameString.substr(0, 3) << " eta/phi " << stEta
                                                                 << "/" << stPhi << " " << planeString
                                                                 << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " " << xC << " "
                                                                 << yC << " " << zC << std::endl;
                                                        fout1 << "Pad box position " << stNameString.substr(0, 3) << " eta/phi " << stEta
                                                              << "/" << stPhi << " " << planeString
                                                              << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " " << xC << " " << yC
                                                              << " " << zC << std::endl;
                                                        fpad << stNameString.substr(0, 3) << " " << xC << " " << yC << " " << zC
                                                             << std::endl;
                                                    }
                                                } else if (stNameString == "BOS" || stNameString == "BOG" || stNameString == "BOF") {
                                                    double xC, yC, zC, xFirstPhiS, yFirstPhiS, zFirstPhiS, xLastPhiS, yLastPhiS, zLastPhiS;
                                                    getPanelEdgeCenter(rpc, chid, xC, yC, zC, xFirstPhiS, yFirstPhiS, zFirstPhiS, xLastPhiS,
                                                                       yLastPhiS, zLastPhiS);
                                                    fout1 << "dbR, dbZ, dbP = " << doubletR << " " << doubletZ << " " << dbp << " panel c "
                                                          << xC << " " << yC << " " << zC << " panel fs " << xFirstPhiS << " " << yFirstPhiS
                                                          << " " << zFirstPhiS << " ls " << xLastPhiS << " " << yLastPhiS << " "
                                                          << zLastPhiS << std::endl;
                                                    if (ndbz == 1) {
                                                        if (dbp == 1) {
                                                            fpanelid << "Pad box position " << stNameString.substr(0, 3) << " eta/phi "
                                                                     << stEta << "/" << stPhi << " " << planeString
                                                                     << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " "
                                                                     << xFirstPhiS << " " << yFirstPhiS << " " << zFirstPhiS << std::endl;
                                                            fout1 << "Pad box position " << stNameString.substr(0, 3) << " eta/phi "
                                                                  << stEta << "/" << stPhi << " " << planeString
                                                                  << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " " << xFirstPhiS
                                                                  << " " << yFirstPhiS << " " << zFirstPhiS << std::endl;
                                                            fpad << stNameString.substr(0, 3) << " " << xFirstPhiS << " " << yFirstPhiS
                                                                 << " " << zFirstPhiS << std::endl;
                                                        } else {
                                                            fpanelid << "Pad box position " << stNameString.substr(0, 3) << " eta/phi "
                                                                     << stEta << "/" << stPhi << " " << planeString
                                                                     << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " "
                                                                     << xLastPhiS << " " << yLastPhiS << " " << zLastPhiS << std::endl;
                                                            fout1 << "Pad box position " << stNameString.substr(0, 3) << " eta/phi "
                                                                  << stEta << "/" << stPhi << " " << planeString
                                                                  << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " " << xLastPhiS
                                                                  << " " << yLastPhiS << " " << zLastPhiS << std::endl;
                                                            fpad << stNameString.substr(0, 3) << " " << xLastPhiS << " " << yLastPhiS << " "
                                                                 << zLastPhiS << std::endl;
                                                            xC = rpc->center().x();
                                                            yC = rpc->center().y();
                                                            zC = rpc->center().z();
                                                        }

                                                    } else if (ndbz > 1 && dbz_index == 0) {
                                                        if (dbp == 1) {
                                                            keepxFS = xFirstPhiS;
                                                            keepyFS = yFirstPhiS;
                                                            keepzFS = zFirstPhiS;
                                                            fout1 << " first strip kept" << std::endl;
                                                        } else {
                                                            keepxLS = xLastPhiS;
                                                            keepyLS = yLastPhiS;
                                                            keepzLS = zLastPhiS;
                                                            fout1 << " last strip kept" << std::endl;
                                                            keepxC = rpc->center().x();
                                                            keepyC = rpc->center().y();
                                                            keepzC = rpc->center().z();
                                                        }
                                                        fout1 << " center kept" << std::endl;
                                                    } else if (ndbz > 1 && dbz_index == 1) {
                                                        if (dbp == 1) {
                                                            xFirstPhiS = 0.5 * (keepxFS + xFirstPhiS);
                                                            yFirstPhiS = 0.5 * (keepyFS + yFirstPhiS);
                                                            zFirstPhiS = 0.5 * (keepzFS + zFirstPhiS);
                                                            fpanelid << "Pad box position " << stNameString.substr(0, 3) << " eta/phi "
                                                                     << stEta << "/" << stPhi << " " << planeString
                                                                     << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " "
                                                                     << xFirstPhiS << " " << yFirstPhiS << " " << zFirstPhiS << std::endl;
                                                            fpad << stNameString.substr(0, 3) << " " << xFirstPhiS << " " << yFirstPhiS
                                                                 << " " << zFirstPhiS << std::endl;
                                                            fout1 << "Pad box position " << stNameString.substr(0, 3) << " eta/phi "
                                                                  << stEta << "/" << stPhi << " " << planeString
                                                                  << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " " << xFirstPhiS
                                                                  << " " << yFirstPhiS << " " << zFirstPhiS << std::endl;
                                                        } else {
                                                            xLastPhiS = 0.5 * (keepxLS + xLastPhiS);
                                                            yLastPhiS = 0.5 * (keepyLS + yLastPhiS);
                                                            zLastPhiS = 0.5 * (keepzLS + zLastPhiS);
                                                            fpanelid << "Pad box position " << stNameString.substr(0, 3) << " eta/phi "
                                                                     << stEta << "/" << stPhi << " " << planeString
                                                                     << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " "
                                                                     << xLastPhiS << " " << yLastPhiS << " " << zLastPhiS << std::endl;
                                                            fout1 << "Pad box position " << stNameString.substr(0, 3) << " eta/phi "
                                                                  << stEta << "/" << stPhi << " " << planeString
                                                                  << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " " << xLastPhiS
                                                                  << " " << yLastPhiS << " " << zLastPhiS << std::endl;
                                                            fpad << stNameString.substr(0, 3) << " " << xLastPhiS << " " << yLastPhiS << " "
                                                                 << zLastPhiS << std::endl;
                                                            xC = 0.5 * (keepxC + rpc->center().x());
                                                            yC = 0.5 * (keepyC + rpc->center().y());
                                                            zC = 0.5 * (keepzC + rpc->center().z());
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    fout << " StripGlobalPosition " << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " pos "
                                         << rpc->stripPos(chid) << " Local Position = " << rpc->localStripPos(chid) << std::endl;
                                    Amg::Vector3D xxSD = rpc->globalToLocalCoords(rpc->stripPos(chid), chid);
                                    Amg::Vector3D xxMod = rpc->SDtoModuleCoords(xxSD, chid);
                                    fout << " this location in the Module frame " << xxMod << " camparing with localStripPos "
                                         << rpc->localStripPos(chid) << std::endl;
                                    if (Amg::Vector3D(rpc->localStripPos(chid) - xxMod).perp() > 0.1)
                                        fout << " a problem here " << std::endl;

                                    if (strip == 1 && m_check_rpc_distToReadout > 0) {
                                        Identifier chtest0 = m_idHelperSvc->rpcIdHelper().channelID(
                                            idp, m_idHelperSvc->rpcIdHelper().doubletZ(idr), dbp, igg, measphi, 1);
                                        Identifier chtest1 = m_idHelperSvc->rpcIdHelper().channelID(
                                            idp, m_idHelperSvc->rpcIdHelper().doubletZ(idr), dbp, igg, measphi, rpc->NphiStrips());
                                        Identifier chtest2 = m_idHelperSvc->rpcIdHelper().channelID(
                                            idp, m_idHelperSvc->rpcIdHelper().doubletZ(idr), dbp, igg, 0, 1);
                                        Identifier chtest3 = m_idHelperSvc->rpcIdHelper().channelID(
                                            idp, m_idHelperSvc->rpcIdHelper().doubletZ(idr), dbp, igg, 0, rpc->NetaStrips());
                                        fout << "distance to Phi/Eta RO for center of strip "
                                             << m_idHelperSvc->rpcIdHelper().show_to_string(chtest0) << " are "
                                             << rpc->distanceToPhiReadout(rpc->stripPos(chtest0), chtest0) << " "
                                             << rpc->distanceToEtaReadout(rpc->stripPos(chtest0), chtest0) << std::endl;
                                        fout << "distance to Phi/Eta RO for center of strip "
                                             << m_idHelperSvc->rpcIdHelper().show_to_string(chtest1) << " are "
                                             << rpc->distanceToPhiReadout(rpc->stripPos(chtest1), chtest1) << " "
                                             << rpc->distanceToEtaReadout(rpc->stripPos(chtest1), chtest1) << std::endl;
                                        fout << "distance to Phi/Eta RO for center of strip "
                                             << m_idHelperSvc->rpcIdHelper().show_to_string(chtest2) << " are "
                                             << rpc->distanceToPhiReadout(rpc->stripPos(chtest2), chtest2) << " "
                                             << rpc->distanceToEtaReadout(rpc->stripPos(chtest2), chtest2) << std::endl;
                                        fout << "distance to Phi/Eta RO for center of strip "
                                             << m_idHelperSvc->rpcIdHelper().show_to_string(chtest3) << " are "
                                             << rpc->distanceToPhiReadout(rpc->stripPos(chtest3), chtest3) << " "
                                             << rpc->distanceToEtaReadout(rpc->stripPos(chtest3), chtest3) << std::endl;
                                    }

                                    if (m_check_surfaces) {
                                        if (strip == 1) {
                                            fout << "tracking local x(phi) axis (surface->transform) = "
                                                 << (rpc->surface(chid)).transform().linear() * Amg::Vector3D::UnitX() << std::endl;
                                            fout << "tracking local y(phi) axis (surface->transform) = "
                                                 << (rpc->surface(chid)).transform().linear() * Amg::Vector3D::UnitY()<< std::endl;
                                            fout << "tracking local z(phi) axis (surface->transform) = "
                                                 << (rpc->surface(chid)).transform().linear() * Amg::Vector3D::UnitZ() << std::endl;
                                            Amg::Vector2D lpos(0., 0.);
                                            fout << "Check surface.localToGlobal for loc point " << lpos << std::endl;
                                            fout << "                                global    "
                                                 << static_cast<const Trk::Surface&>(rpc->surface(chid)).localToGlobal(lpos) << std::endl;
                                            lpos = Amg::Vector2D(100., 0.);
                                            fout << "Check surface.localToGlobal for loc point " << lpos << std::endl;
                                            fout << "                                global    "
                                                 << static_cast<const Trk::Surface&>(rpc->surface(chid)).localToGlobal(lpos) << std::endl;
                                            lpos = Amg::Vector2D(0., 100.);
                                            fout << "Check surface.localToGlobal for loc point " << lpos << std::endl;
                                            fout << "                                global    "
                                                 << static_cast<const Trk::Surface&>(rpc->surface(chid)).localToGlobal(lpos) << std::endl;
                                        }
                                    }
                                    strip += stripStep;
                                }
                                measphi = 0;
                                fout << " Gas Gap " << igg << " measphi = " << measphi << " eta strip pitch = " << rpc->StripPitch(measphi)
                                     << " n. eta strips = " << rpc->NetaStrips() << std::endl;
                                etastr_pitch += rpc->StripPitch(measphi) * rpc->NetaStrips();
                                netastr += rpc->NetaStrips();

                                stripStep = 1;
                                if (m_check_first_last) stripStep = rpc->NetaStrips() - 1;
                                for (int strip = 1; strip <= rpc->NetaStrips();) {
                                    Identifier chid = m_idHelperSvc->rpcIdHelper().channelID(
                                        idp, m_idHelperSvc->rpcIdHelper().doubletZ(idr), dbp, igg, measphi, strip);
                                    if (strip == 1) {
                                        fpanelidh << "IdCodes " << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " "
                                                  << m_idHelperSvc->rpcIdHelper().stationNameString(
                                                         m_idHelperSvc->rpcIdHelper().stationName(chid))
                                                  << " " << chid << " " << chid.get_identifier32().get_compact() << " "
                                                  << chid.get_compact() << " n eta strips = " << rpc->NetaStrips() << std::endl;

                                        int layerType = 0;
                                        if (doubletR == 2) layerType = 1;
                                        if ((stNameInt > 3 && (stNameInt != 53 && stNameInt != 8)) && doubletR == 1) layerType = 2;
                                        layerType = layerType * 2 + m_idHelperSvc->rpcIdHelper().gasGap(chid);

                                        fpanelid << layerType << " " << stNameString << " eta "
                                                 << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " "
                                                 << chid.get_identifier32().get_compact() << " " << etamin << " " << etamax << " " << phimin
                                                 << " " << phimax << " " << zmin << " " << zmax << std::endl;
                                    }

                                    fout << " StripGlobalPosition " << m_idHelperSvc->rpcIdHelper().show_to_string(chid) << " pos "
                                         << rpc->stripPos(chid) << " Local Position = " << rpc->localStripPos(chid) << std::endl;

                                    if (m_check_surfaces) {
                                        if (strip == 1) {
                                            fout << "tracking local x(eta) axis (surface->transform) = "
                                                 << (rpc->surface(chid)).transform().linear() * Amg::Vector3D::UnitX() << std::endl;
                                            fout << "tracking local y(eta) axis (surface->transform) = "
                                                 << (rpc->surface(chid)).transform().linear() * Amg::Vector3D::UnitY()<< std::endl;
                                            fout << "tracking local z(eta) axis (surface->transform) = "
                                                 << (rpc->surface(chid)).transform().linear() * Amg::Vector3D::UnitZ() << std::endl;
                                            Amg::Vector2D lpos(0., 0.);
                                            fout << "Check surface.localToGlobal for loc point " << lpos << std::endl;
                                            fout << "                                global    "
                                                 << static_cast<const Trk::Surface&>(rpc->surface(chid)).localToGlobal(lpos) << std::endl;
                                            lpos = Amg::Vector2D(100., 0.);
                                            fout << "Check surface.localToGlobal for loc point " << lpos << std::endl;
                                            fout << "                                global    "
                                                 << static_cast<const Trk::Surface&>(rpc->surface(chid)).localToGlobal(lpos) << std::endl;
                                            lpos = Amg::Vector2D(0., 100.);
                                            fout << "Check surface.localToGlobal for loc point " << lpos << std::endl;
                                            fout << "                                global    "
                                                 << static_cast<const Trk::Surface&>(rpc->surface(chid)).localToGlobal(lpos) << std::endl;
                                        }
                                    }
                                    strip += stripStep;
                                }
                                if (m_check_surfaces_details) {
                                    for (int strip = 1; strip <= rpc->NphiStrips(); strip++) {
                                        Identifier chid = m_idHelperSvc->rpcIdHelper().channelID(
                                            idp, m_idHelperSvc->rpcIdHelper().doubletZ(idr), dbp, igg, 1, strip);
                                        // Global position
                                        Amg::Vector3D tempGlobalPosition = rpc->stripPos(chid);
                                        fout << "GG: " << igg << " dbZ: " << m_idHelperSvc->rpcIdHelper().doubletZ(idr) << " dbP: " << dbp
                                             << " Phi strip: " << strip << " glob.pos. " << tempGlobalPosition.x() << ", "
                                             << tempGlobalPosition.y() << ", " << tempGlobalPosition.z() << " ";
                                        // Local position
                                        std::optional<Amg::Vector2D> locPosition =
                                            rpc->surface(chid).Trk::Surface::globalToLocal(tempGlobalPosition);
                                        fout << "loc.pos. " << locPosition->x() << " " << locPosition->y();
                                        const Amg::Vector3D gPtrk = static_cast<const Trk::Surface&>(rpc->surface(chid)).localToGlobal(*locPosition);
                                        fout << " Trk::glob.pos. " << gPtrk.x() << " " << gPtrk.y() << " " << gPtrk.z() << std::endl;
                                    }
                                    for (int strip = 1; strip <= rpc->NetaStrips(); strip++) {
                                        Identifier chid = m_idHelperSvc->rpcIdHelper().channelID(
                                            idp, m_idHelperSvc->rpcIdHelper().doubletZ(idr), dbp, igg, 0, strip);
                                        // Global position
                                        Amg::Vector3D tempGlobalPosition = rpc->stripPos(chid);
                                        fout << "GG: " << igg << " dbZ: " << m_idHelperSvc->rpcIdHelper().doubletZ(idr) << " dbP: " << dbp
                                             << " Eta strip: " << strip << " glob.pos. " << tempGlobalPosition.x() << ", "
                                             << tempGlobalPosition.y() << ", " << tempGlobalPosition.z() << " ";
                                        // Local position
                                        std::optional<Amg::Vector2D> locPosition =
                                            rpc->surface(chid).Trk::Surface::globalToLocal(tempGlobalPosition);
                                        fout << "loc.pos. " << locPosition->x() << " " << locPosition->y();
                                        const Amg::Vector3D gPtrk = static_cast<const Trk::Surface&>(rpc->surface(chid)).localToGlobal(*locPosition);
                                        fout << " Trk::glob.pos. " << gPtrk.x() << " " << gPtrk.y() << " " << gPtrk.z() << std::endl;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    etastr_pitch = etastr_pitch / (double)netastr;
    phistr_pitch = phistr_pitch / (double)nphistr;
    fout << " average Eta strip pitch = " << etastr_pitch << " for " << netastr << " eta strips in total" << std::endl;
    fout << " average Phi strip pitch = " << phistr_pitch << " for " << nphistr << " phi strips in total" << std::endl;
    fout.close();
    fout1.close();
    fpanelid.close();
    fpad.close();
    fpanelidh.close();
    ATH_MSG_INFO(" CheckReadoutRpc done !");
}

void MuonGMCheck::getEtaPhiPanelBoundaries(const MuonGM::RpcReadoutElement* rpc, Identifier& chid, double& etamin, double& etamax,
                                           double& phimin, double& phimax) {
    double zmin{0.}, zmax{0.};
    return getPanelBoundaries(rpc, chid, etamin, etamax, phimin, phimax, zmin, zmax);
}
void MuonGMCheck::getZPhiPanelBoundaries(const MuonGM::RpcReadoutElement* rpc, Identifier& chid, double& zmin, double& zmax, double& phimin,
                                         double& phimax) {
    double etamin;
    double etamax;
    return getPanelBoundaries(rpc, chid, etamin, etamax, phimin, phimax, zmin, zmax);
}
void MuonGMCheck::getEtaPhiActivePanelBoundaries(const MuonGM::RpcReadoutElement* rpc, Identifier& chid, double& etamin, double& etamax,
                                                 double& phimin, double& phimax) {
    double zmin;
    double zmax;
    return getActivePanelBoundaries(rpc, chid, etamin, etamax, phimin, phimax, zmin, zmax);
}
void MuonGMCheck::getZPhiActivePanelBoundaries(const MuonGM::RpcReadoutElement* rpc, Identifier& chid, double& zmin, double& zmax,
                                               double& phimin, double& phimax) {
    double etamin;
    double etamax;
    return getActivePanelBoundaries(rpc, chid, etamin, etamax, phimin, phimax, zmin, zmax);
}
void MuonGMCheck::getPanelEdgeCenter(const MuonGM::RpcReadoutElement* rpc, Identifier& chid, double& xC, double& yC, double& zC,
                                     double& xFirstPhiS, double& yFirstPhiS, double& zFirstPhiS, double& xLastPhiS, double& yLastPhiS,
                                     double& zLastPhiS) {
    int n_strips = 0;  // in phi or eta dir. depening on measphi-field of chid
    int view = -1;
    if (m_idHelperSvc->rpcIdHelper().measuresPhi(chid)) {
        // phi panel
        view = 1;
        n_strips = rpc->NphiStrips();
    } else {
        // eta panel
        view = 0;
        n_strips = rpc->NetaStrips();
    }
    Identifier idp = m_idHelperSvc->rpcIdHelper().parentID(chid);
    Identifier chidFirst = m_idHelperSvc->rpcIdHelper().channelID(idp, m_idHelperSvc->rpcIdHelper().doubletZ(chid),
                                                                  m_idHelperSvc->rpcIdHelper().doubletPhi(chid),
                                                                  m_idHelperSvc->rpcIdHelper().gasGap(chid), view, 1);
    Identifier chidLast = m_idHelperSvc->rpcIdHelper().channelID(idp, m_idHelperSvc->rpcIdHelper().doubletZ(chid),
                                                                 m_idHelperSvc->rpcIdHelper().doubletPhi(chid),
                                                                 m_idHelperSvc->rpcIdHelper().gasGap(chid), view, n_strips);

    xFirstPhiS = yFirstPhiS = zFirstPhiS = xLastPhiS = yLastPhiS = zLastPhiS = xC = yC = zC = 0.;
    if (view == 1) {
        xFirstPhiS = rpc->stripPos(chidFirst).x();
        yFirstPhiS = rpc->stripPos(chidFirst).y();
        zFirstPhiS = rpc->stripPos(chidFirst).z();

        xLastPhiS = rpc->stripPos(chidLast).x();
        yLastPhiS = rpc->stripPos(chidLast).y();
        zLastPhiS = rpc->stripPos(chidLast).z();

        const Amg::Vector3D vecCenter = 0.5 * (rpc->stripPos(chidFirst) + rpc->stripPos(chidLast));
        xC = vecCenter.x();
        yC = vecCenter.y();
        zC = vecCenter.z();
    }

    return;
}
void MuonGMCheck::getPanelBoundaries(const MuonGM::RpcReadoutElement* rpc, Identifier& chid, double& etamin, double& etamax, double& phimin,
                                     double& phimax, double& zmin, double& zmax) {
    int n_strips = 0;  // in phi or eta dir. depening on measphi-field of chid
    int n_stripsOtherView = 0;
    int view = -1;
    int otherview = -1;
    if (m_idHelperSvc->rpcIdHelper().measuresPhi(chid)) {
        // phi panel
        view = 1;
        otherview = 0;
        n_strips = rpc->NphiStrips();
        n_stripsOtherView = rpc->NetaStrips();
    } else {
        // eta panel
        view = 0;
        otherview = 1;
        n_strips = rpc->NetaStrips();
        n_stripsOtherView = rpc->NphiStrips();
    }
    Identifier idp = m_idHelperSvc->rpcIdHelper().parentID(chid);
    Identifier chidFirst = m_idHelperSvc->rpcIdHelper().channelID(idp, m_idHelperSvc->rpcIdHelper().doubletZ(chid),
                                                                  m_idHelperSvc->rpcIdHelper().doubletPhi(chid),
                                                                  m_idHelperSvc->rpcIdHelper().gasGap(chid), view, 1);
    Identifier chidLast = m_idHelperSvc->rpcIdHelper().channelID(idp, m_idHelperSvc->rpcIdHelper().doubletZ(chid),
                                                                 m_idHelperSvc->rpcIdHelper().doubletPhi(chid),
                                                                 m_idHelperSvc->rpcIdHelper().gasGap(chid), view, n_strips);
    Identifier chidFirstOtherView = m_idHelperSvc->rpcIdHelper().channelID(idp, m_idHelperSvc->rpcIdHelper().doubletZ(chid),
                                                                           m_idHelperSvc->rpcIdHelper().doubletPhi(chid),
                                                                           m_idHelperSvc->rpcIdHelper().gasGap(chid), otherview, 1);
    Identifier chidLastOtherView = m_idHelperSvc->rpcIdHelper().channelID(
        idp, m_idHelperSvc->rpcIdHelper().doubletZ(chid), m_idHelperSvc->rpcIdHelper().doubletPhi(chid),
        m_idHelperSvc->rpcIdHelper().gasGap(chid), otherview, n_stripsOtherView);

    double phiFirstStrip{0.}, phiLastStrip{0.}, etaFirstStrip{0.}, etaLastStrip{0.}, zFirstStrip{0.}, zLastStrip{0.};
    if (view == 1) {
        phiFirstStrip = rpc->stripPos(chidFirst).phi();
        phiLastStrip = rpc->stripPos(chidLast).phi();
        etaFirstStrip = rpc->stripPos(chidFirstOtherView).eta();
        etaLastStrip = rpc->stripPos(chidLastOtherView).eta();
        zFirstStrip = rpc->stripPos(chidFirstOtherView).z();
        zLastStrip = rpc->stripPos(chidLastOtherView).z();
    } else {
        phiFirstStrip = rpc->stripPos(chidFirstOtherView).phi();
        phiLastStrip = rpc->stripPos(chidLastOtherView).phi();
        etaFirstStrip = rpc->stripPos(chidFirst).eta();
        etaLastStrip = rpc->stripPos(chidLast).eta();
        zFirstStrip = rpc->stripPos(chidFirst).z();
        zLastStrip = rpc->stripPos(chidLast).z();
    }

    if (etaFirstStrip < etaLastStrip) {
        etamin = etaFirstStrip;
        etamax = etaLastStrip;
        zmin = zFirstStrip;
        zmax = zLastStrip;
    } else {
        etamin = etaLastStrip;
        etamax = etaFirstStrip;
        zmin = zLastStrip;
        zmax = zFirstStrip;
    }

    if (phiFirstStrip < phiLastStrip) {
        phimin = phiFirstStrip;
        phimax = phiLastStrip;
    } else {
        phimin = phiLastStrip;
        phimax = phiFirstStrip;
    }

    return;
}

void MuonGMCheck::getActivePanelBoundaries(const MuonGM::RpcReadoutElement* rpc, Identifier& chid, double& etamin, double& etamax,
                                           double& phimin, double& phimax, double& zmin, double& zmax) {
    int n_strips = 0;  // in phi or eta dir. depening on measphi-field of chid
    int n_stripsOtherView = 0;
    int view = -1;
    int otherview = -1;
    if (m_idHelperSvc->rpcIdHelper().measuresPhi(chid)) {
        // phi panel
        view = 1;
        otherview = 0;
        n_strips = rpc->NphiStrips();
        n_stripsOtherView = rpc->NetaStrips();
    } else {
        // eta panel
        view = 0;
        otherview = 1;
        n_strips = rpc->NetaStrips();
        n_stripsOtherView = rpc->NphiStrips();
    }
    Identifier idp = m_idHelperSvc->rpcIdHelper().parentID(chid);
    Identifier chidFirst = m_idHelperSvc->rpcIdHelper().channelID(idp, m_idHelperSvc->rpcIdHelper().doubletZ(chid),
                                                                  m_idHelperSvc->rpcIdHelper().doubletPhi(chid),
                                                                  m_idHelperSvc->rpcIdHelper().gasGap(chid), view, 1);
    Identifier chidLast = m_idHelperSvc->rpcIdHelper().channelID(idp, m_idHelperSvc->rpcIdHelper().doubletZ(chid),
                                                                 m_idHelperSvc->rpcIdHelper().doubletPhi(chid),
                                                                 m_idHelperSvc->rpcIdHelper().gasGap(chid), view, n_strips);
    Identifier chidFirstOtherView = m_idHelperSvc->rpcIdHelper().channelID(idp, m_idHelperSvc->rpcIdHelper().doubletZ(chid),
                                                                           m_idHelperSvc->rpcIdHelper().doubletPhi(chid),
                                                                           m_idHelperSvc->rpcIdHelper().gasGap(chid), otherview, 1);
    Identifier chidLastOtherView = m_idHelperSvc->rpcIdHelper().channelID(
        idp, m_idHelperSvc->rpcIdHelper().doubletZ(chid), m_idHelperSvc->rpcIdHelper().doubletPhi(chid),
        m_idHelperSvc->rpcIdHelper().gasGap(chid), otherview, n_stripsOtherView);

    const Amg::Vector3D firstStripPos = rpc->stripPos(chidFirst);
    const Amg::Vector3D lastStripPos = rpc->stripPos(chidLast);
    const Amg::Vector3D firstStripPosOtherView = rpc->stripPos(chidFirstOtherView);
    const Amg::Vector3D lastStripPosOtherView = rpc->stripPos(chidLastOtherView);
    Amg::Vector3D halfStripShift = (1. / (2. * double(n_strips))) * (lastStripPos - firstStripPos);
    Amg::Vector3D halfStripShiftOtherView = (1. / (2. * double(n_stripsOtherView))) * (lastStripPosOtherView - firstStripPosOtherView);
    Amg::Vector3D firstStripEdge = firstStripPos - halfStripShift;
    Amg::Vector3D lastStripEdge = lastStripPos + halfStripShift;
    Amg::Vector3D firstStripEdgeOtherView = firstStripPosOtherView - halfStripShiftOtherView;
    Amg::Vector3D lastStripEdgeOtherView = lastStripPosOtherView + halfStripShiftOtherView;

    double phiFirstStrip{0.}, phiLastStrip{0.}, etaFirstStrip{0.}, etaLastStrip{0.}, zFirstStrip{0.}, zLastStrip{0.};
    if (view == 1) {
        phiFirstStrip = firstStripEdge.phi();
        phiLastStrip = lastStripEdge.phi();
        etaFirstStrip = firstStripEdgeOtherView.eta();
        etaLastStrip = lastStripEdgeOtherView.eta();
        zFirstStrip = firstStripEdgeOtherView.z();
        zLastStrip = lastStripEdgeOtherView.z();
    } else {
        phiFirstStrip = firstStripEdgeOtherView.phi();
        phiLastStrip = lastStripEdgeOtherView.phi();
        etaFirstStrip = firstStripEdge.eta();
        etaLastStrip = lastStripEdge.eta();
        zFirstStrip = firstStripEdge.z();
        zLastStrip = lastStripEdge.z();
    }

    if (etaFirstStrip < etaLastStrip) {
        etamin = etaFirstStrip;
        etamax = etaLastStrip;
        zmin = zFirstStrip;
        zmax = zLastStrip;
    } else {
        etamin = etaLastStrip;
        etamax = etaFirstStrip;
        zmin = zLastStrip;
        zmax = zFirstStrip;
    }

    if (phiFirstStrip < phiLastStrip) {
        phimin = phiFirstStrip;
        phimax = phiLastStrip;
    } else {
        phimin = phiLastStrip;
        phimax = phiFirstStrip;
    }

    return;
}

void MuonGMCheck::checkParentStation() {
    ATH_MSG_INFO(" *************************** Global Check for Mdt");
    for (unsigned int hashInt =0 ; hashInt < MuonDetectorManager::MdtRElMaxHash; ++hashInt) {
        const IdentifierHash hash{hashInt};
        const MdtReadoutElement* mdt = p_MuonMgr->getMdtReadoutElement(hash);
        if (!mdt) continue;
        ATH_MSG_INFO(" ///////////////////// Found a MdtReadoutElement ");
        Identifier idr = mdt->identify();
        ATH_MSG_INFO(" its offline Id = " << m_idHelperSvc->toString(idr)
                                            << " ////////////////// belongs to module " << mdt->getTechnologyName() << "/"
                                            << mdt->getStationName());
        Identifier idp = m_idHelperSvc->mdtIdHelper().parentID(idr);
        ATH_MSG_INFO("      parent Id = " << m_idHelperSvc->toString(idp));
        ATH_MSG_ERROR(" now the positions ");
        Amg::Vector3D stc = mdt->parentMuonStationPos();
        ATH_MSG_INFO("     Station centre is at " << stc << " cyl. coords R,phi,Z " << stc.perp() << " " << stc.phi() << " "
                                                    << stc.z());                   
        Amg::Vector3D elc = mdt->globalPosition();
        ATH_MSG_INFO(" Element centre is at " << elc << " cyl. coords R,phi,Z " << elc.perp() << " " << elc.phi() << " "
                                                << elc.z());
    }
}

void MuonGMCheck::checkreadoutmmgeo() {
    ATH_MSG_INFO(" *************************** Global Check for MM");

    if (p_MuonMgr->nMMRE() == 0) {
        ATH_MSG_INFO(" No MMReadoutElements found ");
        return;
    } else
        ATH_MSG_INFO(p_MuonMgr->nMMRE() << " MMReadoutElements found ");
    const MmIdHelper& helper = m_idHelperSvc->mmIdHelper();

    std::string gVersion = p_MuonMgr->geometryVersion();
    std::string fileName = "MM_current_" + gVersion;
    std::ofstream fout(fileName.c_str());
    ATH_MSG_INFO(" ***** Writing file " << fileName);
    fout << setiosflags(std::ios::fixed) << std::setw(11) << std::setprecision(4) << std::endl;

    for (int ieta = helper.stationEtaMin(); ieta <= helper.stationEtaMax(); ++ieta) {
        if (ieta ==0) continue;
        for (int iphi = helper.stationPhiMin(); iphi <= helper.stationPhiMax(); ++iphi) {
            for (int iml = 1; iml <= 2; ++iml) {
                // first large
                fout << "\n ieta, iphi, iml, " << ieta << " " << iphi << " " << iml;
                
                const Identifier id_c = helper.channelID("MML", ieta, iphi, iml, 1 ,1);
                const MMReadoutElement* mmC = p_MuonMgr->getMMReadoutElement(id_c);
                const Identifier id_a = helper.channelID("MMS", ieta, iphi, iml, 1 ,1);              
                const MMReadoutElement* mmA = p_MuonMgr->getMMReadoutElement(id_a);
                if (mmC && mmA) {
                    Identifier idA = mmA->identify();
                    Identifier idC = mmC->identify();
                    fout << "Found MMRE in A side; identified with: " << helper.show_to_string(idA) << " From Id: StName/Eta/Phi/ML = <"
                         << helper.stationNameString(helper.stationName(idA)) << ">=" << helper.stationName(idA) << "/"
                         << helper.stationEta(idA) << "/" << helper.stationPhi(idA) << "/" << helper.multilayer(idA)
                         << " CollHash = " << mmA->identifyHash() << " REhash = " << mmA->detectorElementHash()
                         << " number of layers(from geo)= " << mmA->numberOfLayers(true) << std::endl;

                    fout << "Found MMRE in C side; identified with: " << helper.show_to_string(idC) << " From Id: StName/Eta/Phi/ML = <"
                         << helper.stationNameString(helper.stationName(idA)) << ">=" << helper.stationName(idC) << "/"
                         << helper.stationEta(idC) << "/" << helper.stationPhi(idC) << "/" << helper.multilayer(idC)
                         << " CollHash = " << mmC->identifyHash() << " REhash = " << mmC->detectorElementHash()
                         << " number of layers(from geo)= " << mmC->numberOfLayers(true) << std::endl;
                    fout << helper.show_to_string(idA) << " # of gas gaps = " << helper.gasGapMax(idA) - helper.gasGapMin(idA) + 1
                         << " ggMax = " << helper.gasGapMax(idA) << " number of layers(from geo)= " << mmA->numberOfLayers(true)
                         << " nStrips = " << mmA->numberOfStrips(idA) << std::endl;
                    fout << helper.show_to_string(idC) << " # of gas gaps = " << helper.gasGapMax(idC) - helper.gasGapMin(idC) + 1
                         << " ggMax = " << helper.gasGapMax(idC) << " number of layers(from geo)= " << mmC->numberOfLayers(true)
                         << " nStrips = " << mmC->numberOfStrips(idC) << std::endl;
                    Amg::Vector3D chCenterC = (mmC->absTransform()).translation();
                    Amg::Vector3D chCenterA = (mmA->absTransform()).translation();
                    fout << "center of the chamber on the A-side = " << chCenterA << " cyl coord (r,phi)-> " << chCenterA.perp() << " "
                         << chCenterA.phi() * invRad << std::endl;
                    fout << "center of the chamber on the C-side = " << chCenterC << " cyl coord (r,phi)-> " << chCenterC.perp() << " "
                         << chCenterC.phi() * invRad << std::endl;
                    Amg::Vector2D lpos(0., 0.);
                    for (int igg = 1; igg < mmA->numberOfLayers(true) + 1; ++igg) {
                        Identifier idgg_fA = helper.channelID(helper.parentID(idA), iml + 1, igg, 1);
                        Identifier idgg_lA = helper.channelID(helper.parentID(idA), iml + 1, igg, mmA->numberOfStrips(idA));
                        Identifier idgg_fC = helper.channelID(helper.parentID(idC), iml + 1, igg, 1);
                        Identifier idgg_lC = helper.channelID(helper.parentID(idC), iml + 1, igg, mmC->numberOfStrips(idC));
                        const Amg::Vector3D chCenter_fA = static_cast<const Trk::Surface&>(mmA->surface(idgg_fA)).localToGlobal(lpos);
                        const Amg::Vector3D chCenter_lA = static_cast<const Trk::Surface&>(mmA->surface(idgg_lA)).localToGlobal(lpos);
                        const Amg::Vector3D chCenter_fC = static_cast<const Trk::Surface&>(mmC->surface(idgg_fC)).localToGlobal(lpos);
                        const Amg::Vector3D chCenter_lC = static_cast<const Trk::Surface&>(mmC->surface(idgg_lC)).localToGlobal(lpos);
                        fout << "A-side: center of surface for gg " << igg << " 1st ch, " << mmA->numberOfStrips(idA) << "-th ch: r "
                             << chCenter_fA.perp() << " " << chCenter_lA.perp() << " phi " << chCenter_fA.phi() * invRad << " "
                             << chCenter_lA.phi() * invRad << " z " << chCenter_fA.z() << " " << chCenter_lA.z() << " "
                             << helper.show_to_string(idgg_fA) << std::endl;
                        fout << "C-side: center of surface for gg " << igg << " 1st ch, " << mmC->numberOfStrips(idC) << "-th ch: r "
                             << chCenter_fC.perp() << " " << chCenter_lC.perp() << " phi " << chCenter_fC.phi() * invRad << " "
                             << chCenter_lC.phi() * invRad << " z " << chCenter_fC.z() << " " << chCenter_lC.z() << " "
                             << helper.show_to_string(idgg_fC) << std::endl;
                    }
                }
            }
        }
    }

    fout.close();
}
void MuonGMCheck::checkreadoutstgcgeo() {
    ATH_MSG_INFO(" *************************** Global Check for sTgc");

    if (p_MuonMgr->nsTgcRE() == 0) {
        ATH_MSG_INFO(" No sTgcReadoutElements found ");
        return;
    } else
        ATH_MSG_INFO(p_MuonMgr->nsTgcRE() << " sTgcReadoutElements found ");
    std::string gVersion = p_MuonMgr->geometryVersion();
    std::string fileName = "sTgc_current_" + gVersion;
    std::ofstream fout(fileName.c_str());
    ATH_MSG_INFO(" ***** Writing file " << fileName);
    const sTgcIdHelper& helper = m_idHelperSvc->stgcIdHelper();

    for (int ieta = helper.stationEtaMin(); ieta <= helper.stationEtaMax(); ++ieta) {
        if (ieta ==0) continue;
        for (int iphi = helper.stationPhiMin(); iphi <= helper.stationPhiMax(); ++iphi) {
            for (int iml = 1; iml <= 2; ++iml) {
                // first large
                fout << " ieta, iphi, iml, " << ieta << " " << iphi << " " << iml << std::endl;
                const Identifier idc = helper.channelID("STL", ieta, iphi , iml, 1, 0 ,1);
                const sTgcReadoutElement* mmC = p_MuonMgr->getsTgcReadoutElement(idc);
                const Identifier ida = helper.channelID("STS", ieta, iphi , iml, 1, 0 ,1);
                const sTgcReadoutElement* mmA = p_MuonMgr->getsTgcReadoutElement(ida);

                if (mmC && mmA) {
                    Identifier idA = mmA->identify();
                    Identifier idC = mmC->identify();
                    fout << "Found MMRE in A side; identified with: " << helper.show_to_string(idA) << " From Id: StName/Eta/Phi/ML = <"
                         << helper.stationNameString(helper.stationName(idA)) << ">=" << helper.stationName(idA) << "/"
                         << helper.stationEta(idA) << "/" << helper.stationPhi(idA) << "/" << helper.multilayer(idA)
                         << " CollHash = " << mmA->identifyHash()
                         << " REhash = " /*<<mmA->detectorElementHash()<<" number of layers= "<<mmA->numberOfLayers()*/
                         << std::endl;
                    fout << "Found MMRE in C side; identified with: " << helper.show_to_string(idC) << " From Id: StName/Eta/Phi/ML = <"
                         << helper.stationNameString(helper.stationName(idA)) << ">=" << helper.stationName(idC) << "/"
                         << helper.stationEta(idC) << "/" << helper.stationPhi(idC) << "/" << helper.multilayer(idC)
                         << " CollHash = " << mmC->identifyHash()
                         << " REhash = " /*<<mmC->detectorElementHash()<<" number of layers= "<<mmC->numberOfLayers()*/
                         << std::endl;
                    fout << "# of gas gaps = " << helper.gasGapMax(idA) - helper.gasGapMin(idA) + 1 << std::endl;

                    Amg::Vector3D chCenterC = (mmC->absTransform()).translation();
                    Amg::Vector3D chCenterA = (mmA->absTransform()).translation();
                    fout << "center of the chamber on the A-side = " << chCenterA << " cyl coord (r,phi)-> " << chCenterA.perp() << " "
                         << chCenterA.phi() * invRad << std::endl;
                    fout << "center of the chamber on the C-side = " << chCenterC << " cyl coord (r,phi)-> " << chCenterC.perp() << " "
                         << chCenterC.phi() * invRad << std::endl;
                }
            }
        }
    }

    fout.close();
}

void MuonGMCheck::checkreadoutmdtgeo() {
    ATH_MSG_INFO(" *************************** Global Check for Mdt");

    if (p_MuonMgr->nMdtRE() == 0) {
        ATH_MSG_INFO(" No MdtReadoutElements found ");
        return;
    } else
        ATH_MSG_INFO(p_MuonMgr->nMdtRE() << " MdtReadoutElements found ");

    std::string gVersion = p_MuonMgr->geometryVersion();
    std::string fileName = "mdt_current_" + gVersion;
    std::string fileNameEP = "mdt_current_EP_" + gVersion;
    std::string fileNameEP2 = "mdt_current_EP2_" + gVersion;
    std::ofstream fout(fileName.c_str());
    std::ofstream fendpoints;
    if (m_check_blines) fendpoints.open(fileNameEP.c_str());
    std::ofstream fendpoints2(fileNameEP2.c_str());
    fendpoints2 << "id/I:str_id/C:px/D:py:pz:zx:zy:zz:mx:my:mz" << std::endl;
    ATH_MSG_INFO(" ***** Writing file " << fileName);
    fout << setiosflags(std::ios::fixed) << std::setprecision(4) << std::endl;

    bool doHeader = true;

    for (unsigned int hashInt = 0; hashInt < MuonDetectorManager::MdtRElMaxHash; ++ hashInt) {
        bool doChHeader = true;
        const IdentifierHash hash{hashInt};
        const MdtReadoutElement* mdt = p_MuonMgr->getMdtReadoutElement(hashInt);
        if (!mdt) continue;

        fout << " ///////////////////// Found a MdtReadoutElement " << std::endl;
        Identifier idr = mdt->identify();
        fout << " its offline hash Id = " << mdt->identifyHash() << std::endl;
        fout << " its offline Id = " << m_idHelperSvc->toString(idr)
                << " ////////////////// belongs to module " << mdt->getTechnologyName() << "/" << mdt->getStationName();
        if (mdt->hasCutouts())
            fout << " ---- it HAS CUTOUTS" << std::endl;
        else
            fout << std::endl;

        // here get B-line
        const BLinePar* bLine = nullptr;
        bLine = mdt->getBLinePar();  //

        Identifier idp = m_idHelperSvc->mdtIdHelper().parentID(idr);
        fout << "      parent Id = " << m_idHelperSvc->toString(idp) << std::endl;

        const MuonStation* pms = mdt->parentMuonStation();

        if (mdt->getStationName() == "BIL2" && mdt->getStationPhi() == 3 && mdt->getStationEta() == 1) {
            fout << " ^^^^^^^^^^^^^^^^^ start checking transform to amdb lrs MULTILAYER = " << mdt->getMultilayer()
                    << std::endl;
            fout << " center in the parent station RF " << mdt->toParentStation().translation() << std::endl;
            Amg::Transform3D gToStation = mdt->GlobalToAmdbLRSTransform();
            for (int tl = 1; tl <= mdt->getNLayers(); tl++) {
                fout << " tube layer = " << tl << std::endl;
                for (int tube = 1; tube < mdt->getNtubesperlayer(); tube++) {
                    Identifier chid = m_idHelperSvc->mdtIdHelper().channelID(idp, mdt->getMultilayer(), tl, tube);
                    Amg::Vector3D locPos = gToStation * mdt->tubePos(chid);
                    fout << " locPos = " << locPos << std::endl;
                }
            }
            fout << " ^^^^^^^^^^^^^^^^^ end   checking transform to amdb lrs " << std::endl;
        }

        fout << " Multilayer    = " << mdt->getMultilayer();
        fout << " N tube layers = " << mdt->getNLayers();
        fout << "   tubes/layer = " << mdt->getNtubesperlayer() << std::endl;

        Amg::Vector3D elc = mdt->globalPosition();
        fout << " Element centre is at " << elc << " cyl. coords R,phi,Z " << elc.perp() << " " << elc.phi() << " " << elc.z()
                << std::endl;

        Identifier chid;
        for (int tl = 1; tl <= mdt->getNLayers(); tl++) {
            fout << " A new tube layer " << tl << std::endl;
            int tubeStep = 1;
            if (m_check_first_last == 1) tubeStep = mdt->getNtubesperlayer() - 1;
            if (m_check_first_last == 2) tubeStep = int(mdt->getNtubesperlayer() / 2.);
            if (m_check_first_last > 2) tubeStep = m_check_first_last;
            if (mdt->hasCutouts()) tubeStep = 1;
            for (int tube = 1; tube < mdt->getNtubesperlayer() + 1;) {
                if (mdt->hasCutouts()) {  // check that the tube exists if there are cutouts
                    PVConstLink cv = mdt->getMaterialGeom();
                    int nGrandchildren = cv->getNChildVols();
                    if (nGrandchildren <= 0) continue;
                    bool tubefound = false;
                    for (unsigned int kk = 0; kk < cv->getNChildVols(); kk++) {
                        int tubegeo = cv->getIdOfChildVol(kk) % maxNTubesPerLayer;
                        int layergeo = (cv->getIdOfChildVol(kk) - tubegeo) / maxNTubesPerLayer;
                        if (tubegeo == tube && layergeo == tl) {
                            tubefound = true;
                            break;
                        }
                        if (layergeo > tl) break;
                    }
                    if (!tubefound) {
                        tube += tubeStep;
                        continue;
                    }
                }
                chid = m_idHelperSvc->mdtIdHelper().channelID(idp, mdt->getMultilayer(), tl, tube);
                fout << m_idHelperSvc->toString(chid) << " wire global pos " << mdt->tubePos(chid);
                fout << " Tube length is " << mdt->tubeLength(chid);
                if (mdt->hasCutouts())
                    fout << " HAS CUTOUTS" << std::endl;
                else
                    fout << std::endl;
                
                // check B-lines
                if (m_check_blines) {
                    double zRO = mdt->signedRODistanceFromTubeCentre(chid);
                    double halfTubeL = mdt->getWireLength(tl, tube) / 2.;

                    double z1000RO = 1000.;
                    if (zRO < 0) z1000RO = -1000.;
                    Amg::Vector3D pzRO = mdt->transform(chid) * Amg::Vector3D(0., 0., zRO);
                    Amg::Vector3D pzHV = mdt->transform(chid) * Amg::Vector3D(0., 0., -zRO);
                    Amg::Vector3D pzEPsplus = mdt->transform(chid) * Amg::Vector3D(0., 0., halfTubeL);
                    Amg::Vector3D pzEPsminus = mdt->transform(chid) * Amg::Vector3D(0., 0., -halfTubeL);
                    Amg::Vector3D pzEPstmp;
                    double pzROamdbS = (mdt->GlobalToAmdbLRSCoords(pzRO)).x();
                    if (pzROamdbS < 0) {
                        pzEPstmp = pzEPsminus;
                        pzEPsminus = pzEPsplus;
                        pzEPsplus = pzEPstmp;
                    }
                    Amg::Vector3D pz0 = mdt->center(chid);
                    fout << " point on the wire at 1m from the center @ RO side "
                            << mdt->transform(chid) * Amg::Vector3D(0., 0., z1000RO) << std::endl;
                    fout << " point on the wire at 1m from the center @ HV side "
                            << mdt->transform(chid) * Amg::Vector3D(0., 0., -z1000RO) << std::endl;
                    fout << " RO side tube end-point                            " << pzRO << std::endl;
                    fout << " HV side tube end-point                            " << pzHV << std::endl;
                    fout << " s+ half-tube end-point                            " << pzEPsplus << std::endl;
                    fout << " s- half-tube end-point                            " << pzEPsminus << std::endl;
                    if (doHeader) {
                        doHeader = false;
                        fendpoints << "=================================================" << std::endl;

                        fendpoints << "   Atlas id                                 s+ end-point x,y,z               center "
                                        "x,y,z                                s-  end-point x,y,z    halfTubeLength"
                                    << std::endl;
                        fendpoints << "=================================================" << std::endl;
                    }
                    if (doChHeader) {
                        doChHeader = false;

                        fendpoints << " ----------- New MDT Station --------- id = "
                                    << m_idHelperSvc->mdtIdHelper().print_to_string(chid)
                                    << "----------------------------------------------------------------------- " << std::endl;
                        fendpoints << " A-line in use is s,z,t rots,z,t " << std::setw(10) << setiosflags(std::ios::fixed)
                                    << std::setprecision(7) << pms->getALine_tras() << " " << pms->getALine_traz() << " "
                                    << pms->getALine_trat() << " " << pms->getALine_rots() << " " << pms->getALine_rotz() << " "
                                    << pms->getALine_rott() << std::setw(10) << setiosflags(std::ios::fixed)
                                    << std::setprecision(4) << std::endl;
                        if (!bLine)
                            fendpoints << m_idHelperSvc->toString(chid) << " B-line is NOT defined "
                                        << std::endl;
                        else {
                            fendpoints << m_idHelperSvc->toString(chid)
                                        << " B-line in use is: bz,bp,bn,sp,sn,tw,pg,tr,eg,ep,en  " << std::setw(10)
                                        << setiosflags(std::ios::fixed) << std::setprecision(7) << bLine->bz() << " "
                                        << bLine->bp() << " " << bLine->bn() << " " << bLine->sp() << " " << bLine->sn() << " "
                                        << bLine->tw() << " " << bLine->pg() << " " << bLine->tr() << " " << bLine->eg() << " "
                                        << bLine->ep() << " " << bLine->en() << std::setw(10) << setiosflags(std::ios::fixed)
                                        << std::setprecision(4) << std::endl;
                        }
                        if (mdt->barrel())
                            fendpoints << m_idHelperSvc->toString(chid)
                                        << " MDT envelop Ssize, LongSsize, Rsize=Height, Zsize=Length " << pms->Ssize() << " "
                                        << pms->LongSsize() << " " << pms->RsizeMdtStation() << " " << pms->ZsizeMdtStation()
                                        << std::endl;
                        else
                            fendpoints << m_idHelperSvc->toString(chid)
                                        << " MDT envelop Ssize, LongSsize, Zsize=Height, Rsize=Length " << pms->Ssize() << " "
                                        << pms->LongSsize() << " " << pms->ZsizeMdtStation() << " " << pms->RsizeMdtStation()
                                        << std::endl;
                        HepGeom::Point3D<double> temp = pms->getUpdatedBlineFixedPointInAmdbLRS();
                        Amg::Vector3D bLineFixedPointAMDBl(temp[0], temp[1], temp[2]);
                        Amg::Vector3D aLineFixedPoint = mdt->AmdbLRSToGlobalCoords(Amg::Vector3D::Zero());
                        Amg::Vector3D bLineFixedPoint = mdt->AmdbLRSToGlobalCoords(bLineFixedPointAMDBl);
                        fendpoints << m_idHelperSvc->toString(chid)
                                    << " A-line szt frame origine:               " << std::setw(15)
                                    << setiosflags(std::ios::fixed) << std::setprecision(4) << " " << aLineFixedPoint.x() << " "
                                    << aLineFixedPoint.y() << " " << aLineFixedPoint.z() << std::endl;
                        fendpoints << m_idHelperSvc->toString(chid)
                                    << " B-line fixed point in A-line szt frame: " << std::setw(15)
                                    << setiosflags(std::ios::fixed) << std::setprecision(4) << " " << bLineFixedPointAMDBl.x()
                                    << " " << bLineFixedPointAMDBl.y() << " " << bLineFixedPointAMDBl.z() << std::endl;
                        fendpoints << m_idHelperSvc->toString(chid)
                                    << " B-line fixed point:                     " << std::setw(15)
                                    << setiosflags(std::ios::fixed) << std::setprecision(4) << " " << bLineFixedPoint.x() << " "
                                    << bLineFixedPoint.y() << " " << bLineFixedPoint.z() << std::endl;
                    }

                    fendpoints << std::setw(20) << setiosflags(std::ios::fixed)
                                << m_idHelperSvc->toString(chid) << "     s+ " << std::setw(12)
                                << setiosflags(std::ios::fixed) << std::setprecision(4) << " " << pzEPsplus.x() << " "
                                << pzEPsplus.y() << " " << pzEPsplus.z() << "   "
                                << "     s=0 " << std::setw(12) << setiosflags(std::ios::fixed) << std::setprecision(4) << " "
                                << pz0.x() << " " << pz0.y() << " " << pz0.z() << "     s- " << std::setw(12)
                                << setiosflags(std::ios::fixed) << std::setprecision(4) << " " << pzEPsminus.x() << " "
                                << pzEPsminus.y() << " " << pzEPsminus.z() << "    halfTubeLength "
                                << halfTubeL  //<<" "<<pzROamdbS
                                << std::endl;

                    fendpoints2 << boost::format(
                                        "%10d %25s   %12.4f %12.4f %12.4f   %12.4f %12.4f %12.4f   %12.4f %12.4f %12.4f\n") %
                                        m_fixedIdTool->idToFixedId(chid).getIdInt() %
                                        m_idHelperSvc->toString(chid) % pzEPsplus.x() % pzEPsplus.y() %
                                        pzEPsplus.z() % pz0.x() % pz0.y() % pz0.z() % pzEPsminus.x() % pzEPsminus.y() %
                                        pzEPsminus.z();

                    fout << " A-line in use is s,z,t rots,z,t " << std::setw(10) << setiosflags(std::ios::fixed)
                            << std::setprecision(7) << pms->getALine_tras() << " " << pms->getALine_traz() << " "
                            << pms->getALine_trat() << " " << pms->getALine_rots() << " " << pms->getALine_rotz() << " "
                            << pms->getALine_rott() << std::setw(10) << setiosflags(std::ios::fixed) << std::setprecision(4)
                            << std::endl;
                    if (!bLine)
                        fout << " B-line is NOT defined " << std::endl;
                    else {
                        fout << " B-line in use is: bz,bp,bn,sp,sn,tw,pg,tr,eg,ep,en  " << std::setw(10)
                                << setiosflags(std::ios::fixed) << std::setprecision(7) << bLine->bz() << " " << bLine->bp() << " "
                                << bLine->bn() << " " << bLine->sp() << " " << bLine->sn() << " " << bLine->tw() << " "
                                << bLine->pg() << " " << bLine->tr() << " " << bLine->eg() << " " << bLine->ep() << " "
                                << bLine->en() << std::setw(10) << setiosflags(std::ios::fixed) << std::setprecision(4)
                                << std::endl;
                    }
                }
                if (m_check_first_last == 2 && tube == tubeStep)
                    tube = mdt->getNtubesperlayer();
                else if (m_check_first_last == 2 && tube == 1)
                    tube = tubeStep;
                else {
                    tube += tubeStep;
                }
            }

            if (!m_minimal_checks) {
                // temporary
                Amg::Vector3D myPloc = Amg::Vector3D(0., 0., 10.);  //
                Amg::Vector3D myP = mdt->localToGlobalTransf(chid)*myPloc;
                double distRO = mdt->tubeFrame_localROPos(chid).z();
                double distToRO = mdt->distanceFromRO(myP, chid);
                double propagationDistance = distToRO - mdt->RODistanceFromTubeCentre(chid);
                double position_along_wire = myPloc.z();
                double sign(-1.);
                if (distRO < 0.) sign = 1.;
                double propagation_delay = sign * position_along_wire;
                fout << " ***** distRO, distToRO, propagationDistance " << distRO << " " << distToRO << " "
                        << propagationDistance << " propagationDist (sim) " << propagation_delay
                        << " inner CLHEP::rad = " << mdt->innerTubeRadius() << std::endl;
                // end temporary
            }
        }
    }
    
    fout.close();
    if (m_check_blines) {
        fendpoints2 << std::flush;
        fendpoints2.close();
        fendpoints.close();
    }
    ATH_MSG_INFO(" CheckReadoutMdt done !");
}
void MuonGMCheck::checkreadouttgcgeo() {
    ATH_MSG_INFO(" *************************** Global Check for Tgc");

    if (p_MuonMgr->nTgcRE() == 0) {
        ATH_MSG_INFO(" No TgcReadoutElements found ");
        return;
    } else
        ATH_MSG_INFO(p_MuonMgr->nTgcRE() << " TgcReadoutElements found ");

    std::string gVersion = p_MuonMgr->geometryVersion();
    std::string fileName = "tgc_current_" + gVersion;
    std::ofstream fout(fileName.c_str());
    ATH_MSG_INFO(" ***** Writing file " << fileName);
    fout << setiosflags(std::ios::fixed) << std::setprecision(4) << std::endl;

    for (unsigned int hashId = 0; hashId < MuonDetectorManager::TgcRElMaxHash; ++ hashId) {
        const IdentifierHash hash{hashId};
        const TgcReadoutElement* tgc = p_MuonMgr->getTgcReadoutElement(hashId);
        if (!tgc) continue;
        fout << " ///////////////////// Found a TgcReadoutElement " << std::endl;
        Identifier idr = tgc->identify();
        fout << " its offline hash Id = " << tgc->identifyHash() << std::endl;
        fout << " its offline Id = " << m_idHelperSvc->toString(idr) << " ////////////////// belongs to module "
                << tgc->getTechnologyName() << "/" << tgc->getStationName() << " stEta/stPhi " << tgc->getStationEta() << " "
                << tgc->getStationPhi() << std::endl;
        Identifier idp = m_idHelperSvc->tgcIdHelper().parentID(idr);
        Identifier idp1 = m_idHelperSvc->tgcIdHelper().elementID(m_idHelperSvc->tgcIdHelper().stationName(idp),
                                                                    -m_idHelperSvc->tgcIdHelper().stationEta(idp),
                                                                    m_idHelperSvc->tgcIdHelper().stationPhi(idp));
        fout << "      parent Id = " << m_idHelperSvc->toString(idp);
        fout << " opposite stEta = " << m_idHelperSvc->toString(idp1);
        const TgcReadoutElement* tgc1 = p_MuonMgr->getTgcReadoutElement(idp1);
        if (!tgc1) {
            ATH_MSG_FATAL(" TGC readout Element at z<0, with id = " << m_idHelperSvc->toString(idp1)
                                                                    << " not found ");
            ATH_MSG_FATAL(" is the geometry COMPLETE ? Any StationSelection active ? Exiting");
            return;
        }
        fout << " its offline hash Id = " << tgc1->identifyHash() << std::endl;

        const MuonStation* ms = tgc->parentMuonStation();
        if (ms)
            fout << "Parent MuonStation found for element at z>0" << std::endl;
        else
            fout << "Parent MuonStation NOT found for element " << m_idHelperSvc->toString(idp) << std::endl;

        const MuonStation* ms1 = tgc1->parentMuonStation();
        if (ms1)
            fout << "Parent MuonStation found for element at z<0" << std::endl;
        else
            fout << "Parent MuonStation NOT found for element " << m_idHelperSvc->toString(idp1) << std::endl;

        fout << " N gasgaps layers = " << tgc->NwirePlanes();
        fout << " N strip planes   = " << tgc->NstripPlanes() << std::endl;

        const TgcReadoutParams* rpar = tgc->getReadoutParams();
        fout << "****** read type " << tgc->getReadoutType() << " getNGaps() " << tgc->getNGaps() << " d.a. " << rpar->nGaps()
                << std::endl;
        for (int ngg = 0; ngg < tgc->NwirePlanes(); ++ngg) {
            fout << "****** getnGangs(gg) " << tgc->getNGangs(ngg + 1) << "  getnStrips(gg) " << tgc->getNStrips(ngg + 1)
                    << std::endl;
            fout << "****** getnWire(gg,1/2/3) " << tgc->getNWires(ngg + 1, 1) << "/" << tgc->getNWires(ngg + 1, 2) << "/"
                    << tgc->getNWires(ngg + 1, 3) << " d.a. " << rpar->nWires(ngg + 1, 1) << "/" << rpar->nWires(ngg + 1, 2) << "/"
                    << rpar->nWires(ngg + 1, 3) << std::endl;

            Identifier idch = m_idHelperSvc->tgcIdHelper().channelID(idp, ngg + 1, 0, 1);
            Amg::Vector3D ggcentre = tgc->localToGlobalCoords(Amg::Vector3D::Zero(), idch);
            fout << " For Chamber id " << m_idHelperSvc->toString(idch) << " gasgap centre is " << ggcentre
                    << std::endl;
            idch = m_idHelperSvc->tgcIdHelper().channelID(idp1, ngg + 1, 0, 1);
            ggcentre = tgc1->localToGlobalCoords(Amg::Vector3D::Zero(), idch);
            fout << " For Chamber id " << m_idHelperSvc->toString(idch) << " gasgap centre is " << ggcentre
                    << std::endl;
            idch = m_idHelperSvc->tgcIdHelper().channelID(idp, ngg + 1, 1, 1);
            ggcentre = tgc->localToGlobalCoords(Amg::Vector3D::Zero(), idch);
            fout << " For Chamber id " << m_idHelperSvc->toString(idch) << " gasgap centre is " << ggcentre
                    << std::endl;
            idch = m_idHelperSvc->tgcIdHelper().channelID(idp1, ngg + 1, 1, 1);
            ggcentre = tgc1->localToGlobalCoords(Amg::Vector3D::Zero(), idch);
            fout << " For Chamber id " << m_idHelperSvc->toString(idch) << " gasgap centre is " << ggcentre
                    << std::endl;

            Identifier fg = m_idHelperSvc->tgcIdHelper().channelID(idp, ngg + 1, 0, 1);
            Identifier lg = m_idHelperSvc->tgcIdHelper().channelID(idp, ngg + 1, 0, tgc->getNGangs(ngg + 1));
            Identifier fgn = m_idHelperSvc->tgcIdHelper().channelID(idp1, ngg + 1, 0, 1);
            Identifier lgn = m_idHelperSvc->tgcIdHelper().channelID(idp1, ngg + 1, 0, tgc1->getNGangs(ngg + 1));

            Amg::Vector3D xfg = tgc->channelPos(fg);
            Amg::Vector3D xlg = tgc->channelPos(lg);
            Amg::Vector3D xfgn = tgc1->channelPos(fgn);
            Amg::Vector3D xlgn = tgc1->channelPos(lgn);
            fout << "\n gg " << ngg + 1 << " " << m_idHelperSvc->toString(fg)
                    << "GM:: first eta gang z>0 r,p,z " << xfg.perp() << " " << xfg.phi() * invRad << " " << xfg.z() << " last "
                    << xlg.perp() << " " << xlg.phi() * invRad << " " << xlg.z() << std::endl;
            fout << "gg " << ngg + 1 << " " << m_idHelperSvc->toString(fgn) << "GM:: first eta gang z<0 r,p,z "
                    << xfgn.perp() << " " << xfgn.phi() * invRad << " " << xfgn.z() << " last " << xlgn.perp() << " "
                    << xlgn.phi() * invRad << " " << xlgn.z() << std::endl;

            Identifier fs = m_idHelperSvc->tgcIdHelper().channelID(idp, ngg + 1, 1, 1);
            Identifier ls = m_idHelperSvc->tgcIdHelper().channelID(idp, ngg + 1, 1, tgc->getNStrips(ngg + 1));
            Identifier fsn = m_idHelperSvc->tgcIdHelper().channelID(idp1, ngg + 1, 1, 1);
            Identifier lsn = m_idHelperSvc->tgcIdHelper().channelID(idp1, ngg + 1, 1, tgc1->getNStrips(ngg + 1));
            xfg = tgc->channelPos(fs);
            xlg = tgc->channelPos(ls);
            xfgn = tgc1->channelPos(fsn);
            xlgn = tgc1->channelPos(lsn);
            fout << "\n gg " << ngg + 1 << " " << m_idHelperSvc->toString(fs)
                    << "GM:: first phi strip z>0 r,p,z " << xfg.perp() << " " << xfg.phi() * invRad << " " << xfg.z() << " last "
                    << xlg.perp() << " " << xlg.phi() * invRad << " " << xlg.z() << std::endl;
            fout << "gg " << ngg + 1 << " " << m_idHelperSvc->toString(fsn) << "GM:: first phi strip z<0 r,p,z "
                    << xfgn.perp() << " " << xfgn.phi() * invRad << " " << xfgn.z() << " last " << xlgn.perp() << " "
                    << xlgn.phi() * invRad << " " << xlgn.z() << std::endl;
            xfg = tgc->localChannelPos(fs);
            xlg = tgc->localChannelPos(ls);
            xfgn = tgc1->localChannelPos(fsn);
            xlgn = tgc1->localChannelPos(lsn);
            fout << "\n gg " << ngg + 1 << " " << m_idHelperSvc->toString(fs)
                    << "GM:: first p_S local z>0 r,p,z " << xfg.perp() << " " << xfg.phi() * invRad << " " << xfg.z() << " last "
                    << xlg.perp() << " " << xlg.phi() * invRad << " " << xlg.z() << std::endl;
            fout << "gg " << ngg + 1 << " " << m_idHelperSvc->toString(fsn) << "GM:: first p_S local z<0 r,p,z "
                    << xfgn.perp() << " " << xfgn.phi() * invRad << " " << xfgn.z() << " last " << xlgn.perp() << " "
                    << xlgn.phi() * invRad << " " << xlgn.z() << std::endl;

            if (m_check_surfaces) {
                fout << "A-side: Phi surface centre = " << (tgc->surface(fs)).center() << std::endl;
                fout << "A-side: Eta surface centre = " << (tgc->surface(fg)).center() << std::endl;
                fout << "C-side: Phi surface centre = " << (tgc1->surface(fsn)).center() << std::endl;
                fout << "C-side: Eta surface centre = " << (tgc1->surface(fgn)).center() << std::endl;
                // A side: fs(phi view), fg(eta view)
                fout << "A-side: tracking local x(phi) axis (surface->transform) = "
                        << (tgc->surface(fs)).transform().linear() * Amg::Vector3D::UnitX() << std::endl;
                fout << "A-side: tracking local y(phi) axis (surface->transform) = "
                        << (tgc->surface(fs)).transform().linear() * Amg::Vector3D::UnitY()<< std::endl;
                fout << "A-side: tracking local z(phi) axis (surface->transform) = "
                        << (tgc->surface(fs)).transform().linear() * Amg::Vector3D::UnitZ() << std::endl;
                Amg::Vector2D lpos(0., 0.);
                fout << "A-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "A-side:                                 global    " << static_cast<const Trk::Surface&>(tgc->surface(fs)).localToGlobal(lpos)
                        << std::endl;
                lpos = Amg::Vector2D(100., 0.);
                fout << "A-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "A-side:                                 global    " << static_cast<const Trk::Surface&>(tgc->surface(fs)).localToGlobal(lpos)
                        << std::endl;
                lpos = Amg::Vector2D(0., 100.);
                fout << "A-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "A-side:                                 global    " << static_cast<const Trk::Surface&>(tgc->surface(fs)).localToGlobal(lpos)
                        << std::endl;
                // C side: fsn(phi view), fgn(eta view)
                fout << "C-side: tracking local x(phi) axis (surface->transform) = "
                        << (tgc1->surface(fsn)).transform().linear() * Amg::Vector3D::UnitX() << std::endl;
                fout << "C-side: tracking local y(phi) axis (surface->transform) = "
                        << (tgc1->surface(fsn)).transform().linear() * Amg::Vector3D::UnitY()<< std::endl;
                fout << "C-side: tracking local z(phi) axis (surface->transform) = "
                        << (tgc1->surface(fsn)).transform().linear() * Amg::Vector3D::UnitZ() << std::endl;
                lpos = Amg::Vector2D(0., 0.);
                fout << "C-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "C-side:                                 global    "
                        << static_cast<const Trk::Surface&>(tgc1->surface(fsn)).localToGlobal(lpos) << std::endl;
                lpos = Amg::Vector2D(100., 0.);
                fout << "C-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "C-side:                                 global    "
                        << static_cast<const Trk::Surface&>(tgc1->surface(fsn)).localToGlobal(lpos) << std::endl;
                lpos = Amg::Vector2D(0., 100.);
                fout << "C-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "C-side:                                 global    "
                        << static_cast<const Trk::Surface&>(tgc1->surface(fsn)).localToGlobal(lpos) << std::endl;
                // A side: fs(phi view), fg(eta view)
                fout << "A-side: tracking local x(eta) axis (surface->transform) = "
                        << (tgc->surface(fg)).transform().linear() * Amg::Vector3D::UnitX() << std::endl;
                fout << "A-side: tracking local y(eta) axis (surface->transform) = "
                        << (tgc->surface(fg)).transform().linear() * Amg::Vector3D::UnitY()<< std::endl;
                fout << "A-side: tracking local z(eta) axis (surface->transform) = "
                        << (tgc->surface(fg)).transform().linear() * Amg::Vector3D::UnitZ() << std::endl;
                lpos = Amg::Vector2D(0., 0.);
                fout << "A-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "A-side:                                 global    " << static_cast<const Trk::Surface&>(tgc->surface(fg)).localToGlobal(lpos)
                        << std::endl;
                lpos = Amg::Vector2D(100., 0.);
                fout << "A-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "A-side:                                 global    " << static_cast<const Trk::Surface&>(tgc->surface(fg)).localToGlobal(lpos)
                        << std::endl;
                lpos = Amg::Vector2D(0., 100.);
                fout << "A-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "A-side:                                 global    " << static_cast<const Trk::Surface&>(tgc->surface(fg)).localToGlobal(lpos)
                        << std::endl;
                // C side: fsn(phi view), fgn(eta view)
                fout << "C-side: tracking local x(eta) axis (surface->transform) = "
                        << (tgc1->surface(fgn)).transform().linear() * Amg::Vector3D::UnitX() << std::endl;
                fout << "C-side: tracking local y(eta) axis (surface->transform) = "
                        << (tgc1->surface(fgn)).transform().linear() * Amg::Vector3D::UnitY()<< std::endl;
                fout << "C-side: tracking local z(eta) axis (surface->transform) = "
                        << (tgc1->surface(fgn)).transform().linear() * Amg::Vector3D::UnitZ() << std::endl;
                lpos = Amg::Vector2D(0., 0.);
                fout << "C-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "C-side:                                 global    "
                        << static_cast<const Trk::Surface&>(tgc1->surface(fgn)).localToGlobal(lpos) << std::endl;
                lpos = Amg::Vector2D(100., 0.);
                fout << "C-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "C-side:                                 global    "
                        << static_cast<const Trk::Surface&>(tgc1->surface(fgn)).localToGlobal(lpos) << std::endl;
                lpos = Amg::Vector2D(0., 100.);
                fout << "C-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "C-side:                                 global    "
                        << static_cast<const Trk::Surface&>(tgc1->surface(fgn)).localToGlobal(lpos) << std::endl;
            }

            if (m_check_surfaces_details) {
                for (int strip = 1; strip <= tgc->getNStrips(ngg + 1); strip++) {
                    Identifier chid = m_idHelperSvc->tgcIdHelper().channelID(idp, ngg + 1, 1, strip);
                    Identifier chid1 = m_idHelperSvc->tgcIdHelper().channelID(idp1, ngg + 1, 1, strip);
                    // Global position
                    Amg::Vector3D tempGlobalPosition = tgc->channelPos(chid);
                    Amg::Vector3D tempGlobalPosition1 = tgc1->channelPos(chid1);
                    fout << "Z>0 - GG: " << ngg << " Phi strip: " << strip << " glob.pos. " << tempGlobalPosition.x() << ", "
                            << tempGlobalPosition.y() << ", " << tempGlobalPosition.z() << " ";
                    // Local position
                    std::optional<Amg::Vector2D> locPosition = tgc->surface(chid).Trk::Surface::globalToLocal(tempGlobalPosition);
                    fout << " Z>0 - loc.pos. " << locPosition->x() << " " << locPosition->y();
                    const Amg::Vector3D gPtrk = static_cast<const Trk::Surface&>(tgc->surface(chid)).localToGlobal(*locPosition);
                    fout << " Z>0 - Trk::glob.pos. " << gPtrk.x() << " " << gPtrk.y() << " " << gPtrk.z() << std::endl;
                    // global
                    fout << "Z<0 - GG: " << ngg << " Phi strip: " << strip << " glob.pos. " << tempGlobalPosition1.x() << ", "
                            << tempGlobalPosition1.y() << ", " << tempGlobalPosition1.z() << " ";
                    std::optional<Amg::Vector2D> locPosition1 =
                        tgc1->surface(chid1).Trk::Surface::globalToLocal(tempGlobalPosition1);
                    fout << " Z<0 - loc.pos. " << locPosition1->x() << " " << locPosition1->y();
                    const Amg::Vector3D gPtrk1 = static_cast<const Trk::Surface&>(tgc1->surface(chid1)).localToGlobal(*locPosition1);
                    fout << " Z<0 - Trk::glob.pos. " << gPtrk1.x() << " " << gPtrk1.y() << " " << gPtrk1.z() << std::endl;
                }
                for (int gang = 1; gang <= tgc->getNGangs(ngg + 1); gang++) {
                    Identifier chid = m_idHelperSvc->tgcIdHelper().channelID(idp, ngg + 1, 0, gang);
                    Identifier chid1 = m_idHelperSvc->tgcIdHelper().channelID(idp1, ngg + 1, 0, gang);
                    // Global position
                    Amg::Vector3D tempGlobalPosition = tgc->channelPos(chid);
                    Amg::Vector3D tempGlobalPosition1 = tgc1->channelPos(chid1);
                    fout << "Z>0 - GG: " << ngg << " Eta gang: " << gang << " glob.pos. " << tempGlobalPosition.x() << ", "
                            << tempGlobalPosition.y() << ", " << tempGlobalPosition.z() << " ";
                    // Local position
                    std::optional<Amg::Vector2D> locPosition = tgc->surface(chid).Trk::Surface::globalToLocal(tempGlobalPosition);
                    fout << " Z>0 - loc.pos. " << locPosition->x() << " " << locPosition->y();
                    const Amg::Vector3D gPtrk = static_cast<const Trk::Surface&>(tgc->surface(chid)).localToGlobal(*locPosition);
                    fout << " Z>0 - Trk::glob.pos. " << gPtrk.x() << " " << gPtrk.y() << " " << gPtrk.z() << std::endl;
                    // Global position
                    fout << "Z<0 - GG: " << ngg << " Eta gang: " << gang << " glob.pos. " << tempGlobalPosition1.x() << ", "
                            << tempGlobalPosition1.y() << ", " << tempGlobalPosition1.z() << " ";
                    // Local position
                    std::optional<Amg::Vector2D> locPosition1 =
                        tgc1->surface(chid1).Trk::Surface::globalToLocal(tempGlobalPosition1);
                    fout << " Z<0 - loc.pos. " << locPosition1->x() << " " << locPosition1->y();
                    const Amg::Vector3D gPtrk1 = static_cast<const Trk::Surface&>(tgc1->surface(chid1)).localToGlobal(*locPosition1);
                    fout << " Z<0 - Trk::glob.pos. " << gPtrk1.x() << " " << gPtrk1.y() << " " << gPtrk1.z() << std::endl;
                }
            }
        }
    }
        
    
    fout.close();
    ATH_MSG_INFO(" CheckReadoutTgc done !");
}
void MuonGMCheck::checkreadoutcscgeo() {
    ATH_MSG_INFO(" *************************** Global Check for Csc");

    if (p_MuonMgr->nCscRE() == 0) {
        ATH_MSG_INFO(" No CscReadoutElements found ");
        return;
    } else
        ATH_MSG_INFO(p_MuonMgr->nCscRE() << " CscReadoutElements found ");

    std::string gVersion = p_MuonMgr->geometryVersion();
    std::string fileName = "csc_current_" + gVersion;
    std::ofstream fout(fileName.c_str());
    ATH_MSG_INFO(" ***** Writing file " << fileName);
    fout << setiosflags(std::ios::fixed) << std::setprecision(4) << std::endl;

    for (unsigned int hashInt = 0; hashInt < MuonDetectorManager::CscRElMaxHash; ++hashInt) {
        const IdentifierHash hash{hashInt};
        const CscReadoutElement* csc = p_MuonMgr->getCscReadoutElement(hash);
        if (!csc) continue;
        
        fout << " ///////////////////// Found a CscReadoutElement "<< std::endl;
        Identifier idr = csc->identify();
        fout << " its offline hash Id = " << csc->identifyHash() << std::endl;
        fout << " its offline Id = " << m_idHelperSvc->toString(idr)
                << " ////////////////// belongs to module " << csc->getTechnologyName() << "/" << csc->getStationName()
                << " centre at " << (csc->transform()).translation() << std::endl;
        Identifier idp = m_idHelperSvc->cscIdHelper().parentID(idr);
        fout << "      parent Id = " << m_idHelperSvc->toString(idp) << std::endl;

        Identifier idp1 = m_idHelperSvc->cscIdHelper().elementID(m_idHelperSvc->cscIdHelper().stationName(idp),
                                                                    -m_idHelperSvc->cscIdHelper().stationEta(idp),
                                                                    m_idHelperSvc->cscIdHelper().stationPhi(idp));

        Identifier idp1ch = m_idHelperSvc->cscIdHelper().channelID(idp1, csc->ChamberLayer(), 1, 0, 1);
        const CscReadoutElement* csc1 = p_MuonMgr->getCscReadoutElement(idp1ch);
        if (!csc1) {
            ATH_MSG_FATAL(" CSC readout Element at z<0, with id = " << m_idHelperSvc->toString(idp1ch)
                                                                    << " not found ");
            ATH_MSG_FATAL(" is the geometry COMPLETE ? Any StationSelection active ? Exiting");
            return;
        }
        unsigned int ml = csc->ChamberLayer();
        fout << " at opposite z  = " << m_idHelperSvc->toString(csc1->identify())
                << " ////////////////// belongs to module " << csc1->getTechnologyName() << "/" << csc1->getStationName()
                << " centre at " << (csc1->transform()).translation() << std::endl;
        fout << " at opposite z the offline hash Id = " << csc1->identifyHash() << std::endl;

        const MuonStation* ms = csc->parentMuonStation();
        if (ms)
            fout << "Parent MuonStation found for element at z>0" << std::endl;
        else
            fout << "Parent MuonStation NOT found for element " << m_idHelperSvc->toString(idp)
                    << std::endl;

        const MuonStation* ms1 = csc1->parentMuonStation();
        if (ms1)
            fout << "Parent MuonStation found for element at z<0" << std::endl;
        else
            fout << "Parent MuonStation NOT found for element " << m_idHelperSvc->toString(idp1)
                    << std::endl;
        int netastrips = csc->NetaStrips(1);
        int nphistrips = csc->NphiStrips(1);
        fout << "N or eta / phi strips in this chamber = " << netastrips << " " << nphistrips << " --- pitch in eta/phi "
                << csc->StripPitch(0) << " " << csc->StripPitch(1) << std::endl;

        for (int gg = 1; gg <= csc->Ngasgaps(); ++gg) {
            Amg::Vector3D ggNomCenter = csc->nominalCenter(gg);
            Amg::Vector3D ggCenter = csc->center(gg - 1);
            Amg::Vector3D localWireLayerPos = csc->localWireLayerPos(gg);
            Amg::Vector3D localEtaStripLayerPos = csc->localStripLayerPos(csc->ChamberLayer(), gg, 0, 0);
            Amg::Vector3D localPhiStripLayerPos = csc->localStripLayerPos(csc->ChamberLayer(), gg, 1, 0);

            Amg::Vector3D ggNomCenter1 = csc1->nominalCenter(gg);
            Amg::Vector3D ggCenter1 = csc1->center(gg - 1);
            Amg::Vector3D localWireLayerPos1 = csc1->localWireLayerPos(gg);
            Amg::Vector3D localEtaStripLayerPos1 = csc1->localStripLayerPos(csc1->ChamberLayer(), gg, 0, 0);
            Amg::Vector3D localPhiStripLayerPos1 = csc1->localStripLayerPos(csc1->ChamberLayer(), gg, 1, 0);

            fout << "-------------------- new gasGap       =  " << gg << std::endl;
            fout << "------- Side-A: Internal alignment Params are =  s,z,t " << csc->getGasGapIntAlign_s(gg) << " "
                    << csc->getGasGapIntAlign_z(gg) << " " << csc->getGasGapIntAlign_t(gg) << "   rots,z,t "
                    << csc->getGasGapIntAlign_rots(gg) << " " << csc->getGasGapIntAlign_rotz(gg) << " "
                    << csc->getGasGapIntAlign_rott(gg) << " " << std::endl;

            fout << "Side-A: Gas Gap nominal center (origin for internal alignment) = " << ggNomCenter << std::endl;
            fout << "Side-A: Gas Gap center         (after      internal alignment) = " << ggCenter << std::endl;
            fout << "Side-A: localWireLayerPosition                                 = " << localWireLayerPos << std::endl;
            fout << "Side-A: localEtaStripLayerPosition                             = " << localEtaStripLayerPos << std::endl;
            fout << "Side-A: localPhiStripLayerPosition                             = " << localPhiStripLayerPos << std::endl;
            fout << "Side-A: Now converting all the previous quantities in the AmdbLRS frame "
                    "----------------------------------------"
                    << std::endl;
            fout << "Side-A: in AmdbLRS Gas Gap nominal center (origin for internal alignment) = "
                    << csc->GlobalToAmdbLRSCoords(ggNomCenter) << std::endl;
            fout << "Side-A: in AmdbLRS Gas Gap center         (after      internal alignment) = "
                    << csc->GlobalToAmdbLRSCoords(ggCenter) << std::endl;

            fout << "------- Side-C: Internal alignment Params are =  s,z,t " << csc1->getGasGapIntAlign_s(gg) << " "
                    << csc1->getGasGapIntAlign_z(gg) << " " << csc1->getGasGapIntAlign_t(gg) << "   rots,z,t "
                    << csc1->getGasGapIntAlign_rots(gg) << " " << csc1->getGasGapIntAlign_rotz(gg) << " "
                    << csc1->getGasGapIntAlign_rott(gg) << " " << std::endl;
            fout << "Side-C: Gas Gap nominal center (origin for internal alignment) = " << ggNomCenter1 << std::endl;
            fout << "Side-C: Gas Gap center         (after      internal alignment) = " << ggCenter1 << std::endl;
            fout << "Side-C: localWireLayerPosition                                 = " << localWireLayerPos1 << std::endl;
            fout << "Side-C: localEtaStripLayerPosition                             = " << localEtaStripLayerPos1 << std::endl;
            fout << "Side-C: localPhiStripLayerPosition                             = " << localPhiStripLayerPos1 << std::endl;
            fout << "Side-C: Now converting all the previous quantities in the AmdbLRS frame "
                    "----------------------------------------"
                    << std::endl;
            fout << "Side-C: in AmdbLRS Gas Gap nominal center (origin for internal alignment) = "
                    << csc1->GlobalToAmdbLRSCoords(ggNomCenter1) << std::endl;
            fout << "Side-C: in AmdbLRS Gas Gap center         (after      internal alignment) = "
                    << csc1->GlobalToAmdbLRSCoords(ggCenter1) << std::endl;
            
            Identifier fwzp = m_idHelperSvc->cscIdHelper().channelID(idp, ml, gg, 0, 1);
            Identifier fszp = m_idHelperSvc->cscIdHelper().channelID(idp, ml, gg, 1, 1);
            Identifier lwzp = m_idHelperSvc->cscIdHelper().channelID(idp, ml, gg, 0, csc->NetaStrips(gg));
            Identifier lszp = m_idHelperSvc->cscIdHelper().channelID(idp, ml, gg, 1, csc->NphiStrips(gg));
            Identifier fwzp1 = m_idHelperSvc->cscIdHelper().channelID(idp1, ml, gg, 0, 1);
            Identifier fszp1 = m_idHelperSvc->cscIdHelper().channelID(idp1, ml, gg, 1, 1);
            Identifier lwzp1 = m_idHelperSvc->cscIdHelper().channelID(idp1, ml, gg, 0, csc1->NetaStrips(gg));
            Identifier lszp1 = m_idHelperSvc->cscIdHelper().channelID(idp1, ml, gg, 1, csc1->NphiStrips(gg));
            Amg::Vector3D AoriginGlobalF = csc->AmdbLRSToGlobalCoords(Amg::Vector3D::Zero());
            Amg::Vector3D AoriginTrkF = csc->transform(fszp).inverse() * AoriginGlobalF;
            fout << " Side-A: A-line origin Global Frame       " << AoriginGlobalF.x() << " " << AoriginGlobalF.y() << " "
                    << AoriginGlobalF.z() << std::endl;
            fout << " Side-A: A-line origin Tracking Phi Frame " << AoriginTrkF << std::endl;
            Amg::Vector3D AoriginGlobalF1 = csc1->AmdbLRSToGlobalCoords(Amg::Vector3D::Zero());
            Amg::Vector3D AoriginTrkF1 = csc1->transform(fszp1).inverse() * AoriginGlobalF1;
            fout << " Side-C: A-line origin Global Frame       " << AoriginGlobalF1.x() << " " << AoriginGlobalF1.y() << " "
                    << AoriginGlobalF1.z() << std::endl;
            fout << " Side-C: A-line origin Tracking Phi Frame " << AoriginTrkF1 << std::endl;

            Amg::Vector3D xfwzp = csc->stripPos(fwzp);
            Amg::Vector3D xfszp = csc->stripPos(fszp);
            Amg::Vector3D xlwzp = csc->stripPos(lwzp);
            Amg::Vector3D xlszp = csc->stripPos(lszp);
            Amg::Vector3D xfwzpNom = csc->nominalStripPos(fwzp);
            Amg::Vector3D xfszpNom = csc->nominalStripPos(fszp);
            Amg::Vector3D xlwzpNom = csc->nominalStripPos(lwzp);
            Amg::Vector3D xlszpNom = csc->nominalStripPos(lszp);
            Amg::Vector3D xfwzp1 = csc1->stripPos(fwzp1);
            Amg::Vector3D xfszp1 = csc1->stripPos(fszp1);
            Amg::Vector3D xlwzp1 = csc1->stripPos(lwzp1);
            Amg::Vector3D xlszp1 = csc1->stripPos(lszp1);
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fwzp)
                    << "GM::  first eta wire is at z>0 local coord " << csc->localStripPos(fwzp) << " last "
                    << csc->localStripPos(lwzp) << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fwzp)
                    << "GM::  first eta wire is at z>0 AMDB lcoord " << csc->GlobalToAmdbLRSCoords(csc->stripPos(fwzp)) << " last "
                    << csc->GlobalToAmdbLRSCoords(csc->stripPos(lwzp)) << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fwzp1)
                    << "GM::  first eta wire is at z<0 AMDB lcoord " << csc1->GlobalToAmdbLRSCoords(csc1->stripPos(fwzp1))
                    << " last " << csc1->GlobalToAmdbLRSCoords(csc1->stripPos(lwzp1)) << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fwzp)
                    << "Nom:: first eta wire is at z>0 local coord " << csc->nominalLocalStripPos(fwzp) << " last "
                    << csc->nominalLocalStripPos(lwzp) << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fwzp)
                    << "TRK:: first eta wire on    TrackingSurface " << csc->stripPosOnTrackingSurface(fwzp) << " last "
                    << csc->stripPosOnTrackingSurface(lwzp) << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fwzp) << "GM::  first eta wire is at z>0 "
                    << xfwzp << " last " << xlwzp << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fwzp) << "Nom:: first eta wire is at z>0 "
                    << xfwzpNom << " last " << xlwzpNom << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fwzp1) << "GM:: first eta wire is at z<0 "
                    << xfwzp1 << " last " << xlwzp1 << std::endl;

            fout << "gg " << gg << " " << m_idHelperSvc->toString(fszp)
                    << "GM::  first phi strip is at z>0 local coord " << csc->localStripPos(fszp) << " last "
                    << csc->localStripPos(lszp) << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fszp)
                    << "GM::  first phi wire is at z>0 AMDB lcoord " << csc->GlobalToAmdbLRSCoords(csc->stripPos(fszp)) << " last "
                    << csc->GlobalToAmdbLRSCoords(csc->stripPos(lszp)) << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fszp1)
                    << "GM::  first phi wire is at z<0 AMDB lcoord " << csc1->GlobalToAmdbLRSCoords(csc->stripPos(fszp1))
                    << " last " << csc1->GlobalToAmdbLRSCoords(csc1->stripPos(lszp1)) << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fszp)
                    << "Nom:: first phi strip is at z>0 local coord " << csc->nominalLocalStripPos(fszp) << " last "
                    << csc->nominalLocalStripPos(lszp) << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fwzp)
                    << "TRK:: first phi strip    on TrackingSurface " << csc->stripPosOnTrackingSurface(fszp) << " last "
                    << csc->stripPosOnTrackingSurface(lszp) << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fszp)
                    << "GM::  first phi strip is at z>0 " << xfszp << " last " << xlszp << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fszp)
                    << "Nom:: first phi strip is at z>0 " << xfszpNom << " last " << xlszpNom << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fszp1)
                    << "GM:: first phi strip is at z<0 " << xfszp1 << " last " << xlszp1 << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fwzp) << "GM::  first eta gang z>0 r,p,z "
                    << xfwzp.perp() << " " << xfwzp.phi() * invRad << " " << xfwzp.z() << " last " << xlwzp.perp() << " "
                    << xlwzp.phi() * invRad << " " << xlwzp.z() << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fwzp1) << "GM:: first eta wire z<0 r,p,z "
                    << xfwzp1.perp() << " " << xfwzp1.phi() * invRad << " " << xfwzp1.z() << " last " << xlwzp1.perp() << " "
                    << xlwzp1.phi() * invRad << " " << xlwzp1.z() << std::endl;

            fout << "gg " << gg << " " << m_idHelperSvc->toString(fszp)
                    << "GM::  first phi strip z>0 r,p,z " << xfszp.perp() << " " << xfszp.phi() * invRad << " " << xfszp.z()
                    << " last " << xlszp.perp() << " " << xlszp.phi() * invRad << " " << xlszp.z() << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fszp1)
                    << "GM:: first phi strip z<0 r,p,z " << xfszp1.perp() << " " << xfszp1.phi() * invRad << " " << xfszp1.z()
                    << " last " << xlszp1.perp() << " " << xlszp1.phi() * invRad << " " << xlszp1.z() << std::endl;

            Amg::Vector3D lxfszp = csc->localStripPos(fszp);
            Amg::Vector3D lxlszp = csc->localStripPos(lszp);
            Amg::Vector3D lxfszp1 = csc1->localStripPos(fszp1);
            Amg::Vector3D lxlszp1 = csc1->localStripPos(lszp1);

            fout << "gg " << gg << " " << m_idHelperSvc->toString(fszp)
                    << "GM:: first phi_S local z>0 r,p,z " << lxfszp.perp() << " " << lxfszp.phi() * invRad << " " << lxfszp.z()
                    << " last " << lxlszp.perp() << " " << lxlszp.phi() * invRad << " " << lxlszp.z() << std::endl;
            fout << "gg " << gg << " " << m_idHelperSvc->toString(fszp1)
                    << "GM:: first phi_S local z<0 r,p,z " << lxfszp1.perp() << " " << lxfszp1.phi() * invRad << " " << lxfszp1.z()
                    << " last " << lxlszp1.perp() << " " << lxlszp1.phi() * invRad << " " << lxlszp1.z() << std::endl;

            if (m_check_surfaces) {
                // A side: fs(phi view), fg(eta view)
                fout << "A-side: tracking local x(phi) axis (surface->transform) = "
                        << (csc->surface(fszp)).transform().linear() * Amg::Vector3D::UnitX() << std::endl;
                fout << "A-side: tracking local y(phi) axis (surface->transform) = "
                        << (csc->surface(fszp)).transform().linear() * Amg::Vector3D::UnitY()<< std::endl;
                fout << "A-side: tracking local z(phi) axis (surface->transform) = "
                        << (csc->surface(fszp)).transform().linear() * Amg::Vector3D::UnitZ() << std::endl;
                Amg::Vector2D lpos(0., 0.);
                fout << "A-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "A-side:                                 global    "
                        << static_cast<const Trk::Surface&>(csc->surface(fszp)).localToGlobal(lpos) << std::endl;
                lpos = Amg::Vector2D(100., 0.);
                fout << "A-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "A-side:                                 global    "
                        << static_cast<const Trk::Surface&>(csc->surface(fszp)).localToGlobal(lpos) << std::endl;
                lpos = Amg::Vector2D(0., 100.);
                fout << "A-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "A-side:                                 global    "
                        <<static_cast<const Trk::Surface&> (csc->surface(fszp)).localToGlobal(lpos) << std::endl;
                // C side: fsn(phi view), fgn(eta view)
                fout << "C-side: tracking local x(phi) axis (surface->transform) = "
                        << (csc1->surface(fszp1)).transform().linear() * Amg::Vector3D::UnitX() << std::endl;
                fout << "C-side: tracking local y(phi) axis (surface->transform) = "
                        << (csc1->surface(fszp1)).transform().linear() * Amg::Vector3D::UnitY()<< std::endl;
                fout << "C-side: tracking local z(phi) axis (surface->transform) = "
                        << (csc1->surface(fszp1)).transform().linear() * Amg::Vector3D::UnitZ() << std::endl;
                lpos = Amg::Vector2D(0., 0.);
                fout << "C-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "C-side:                                 global    "
                        << static_cast<const Trk::Surface&>(csc1->surface(fszp1)).localToGlobal(lpos) << std::endl;
                lpos = Amg::Vector2D(100., 0.);
                fout << "C-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "C-side:                                 global    "
                        << static_cast<const Trk::Surface&>(csc1->surface(fszp1)).localToGlobal(lpos) << std::endl;
                lpos = Amg::Vector2D(0., 100.);
                fout << "C-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "C-side:                                 global    "
                        << static_cast<const Trk::Surface&>(csc1->surface(fszp1)).localToGlobal(lpos) << std::endl;
                // A side: fs(phi view), fg(eta view)
                fout << "A-side: tracking local x(eta) axis (surface->transform) = "
                        << (csc->surface(fwzp)).transform().linear() * Amg::Vector3D::UnitX() << std::endl;
                fout << "A-side: tracking local y(eta) axis (surface->transform) = "
                        << (csc->surface(fwzp)).transform().linear() * Amg::Vector3D::UnitY()<< std::endl;
                fout << "A-side: tracking local z(eta) axis (surface->transform) = "
                        << (csc->surface(fwzp)).transform().linear() * Amg::Vector3D::UnitZ() << std::endl;
                lpos = Amg::Vector2D(0., 0.);
                fout << "A-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "A-side:                                 global    "
                        << static_cast<const Trk::Surface&>(csc->surface(fwzp)).localToGlobal(lpos) << std::endl;
                lpos = Amg::Vector2D(100., 0.);
                fout << "A-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "A-side:                                 global    "
                        << static_cast<const Trk::Surface&>(csc->surface(fwzp)).localToGlobal(lpos) << std::endl;
                lpos = Amg::Vector2D(0., 100.);
                fout << "A-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "A-side:                                 global    "
                        << static_cast<const Trk::Surface&>(csc->surface(fwzp)).localToGlobal(lpos) << std::endl;
                // C side: fsn(phi view), fgn(eta view)
                fout << "C-side: tracking local x(eta) axis (surface->transform) = "
                        << (csc1->surface(fwzp1)).transform().linear() * Amg::Vector3D::UnitX() << std::endl;
                fout << "C-side: tracking local y(eta) axis (surface->transform) = "
                        << (csc1->surface(fwzp1)).transform().linear() * Amg::Vector3D::UnitY()<< std::endl;
                fout << "C-side: tracking local z(eta) axis (surface->transform) = "
                        << (csc1->surface(fwzp1)).transform().linear() * Amg::Vector3D::UnitZ() << std::endl;
                lpos = Amg::Vector2D(0., 0.);
                fout << "C-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "C-side:                                 global    "
                        << static_cast<const Trk::Surface&>(csc1->surface(fwzp1)).localToGlobal(lpos) << std::endl;
                lpos = Amg::Vector2D(100., 0.);
                fout << "C-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "C-side:                                 global    "
                        << static_cast<const Trk::Surface&>(csc1->surface(fwzp1)).localToGlobal(lpos) << std::endl;
                lpos = Amg::Vector2D(0., 100.);
                fout << "C-side: Check surface.localToGlobal for loc point " << lpos << std::endl;
                fout << "C-side:                                 global    "
                        << static_cast<const Trk::Surface&>(csc1->surface(fwzp1)).localToGlobal(lpos) << std::endl;
            }
        }
    }
   
    fout.close();
    ATH_MSG_INFO(" CheckReadoutCsc done !");
}
void MuonGMCheck::buildRpcRegionSelectorMap() {
    std::vector<Identifier>::const_iterator idfirst = m_idHelperSvc->rpcIdHelper().module_begin();
    std::vector<Identifier>::const_iterator idlast = m_idHelperSvc->rpcIdHelper().module_end();

    IdContext rpcModuleContext = m_idHelperSvc->rpcIdHelper().module_context();

    std::ofstream fout0("RPCMapIdAndHash.txt");
    fout0 << "RPC ExtendedId HashId Etamin Etamax Phimin Phimax " << std::endl;

    for (std::vector<Identifier>::const_iterator i = idfirst; i != idlast; ++i) {
        Identifier Id = *i;
        IdentifierHash Idhash;
        int gethash_code = m_idHelperSvc->rpcIdHelper().get_hash(Id, Idhash, &rpcModuleContext);
        std::string extid = m_idHelperSvc->rpcIdHelper().show_to_string(Id);
        ATH_MSG_INFO(" Identifier = " << extid);
        if (gethash_code == 0)
            ATH_MSG_INFO(" its hash Id is " << Idhash);
        else
            ATH_MSG_INFO("     hash Id NOT computed " << Idhash);

        std::string new_extid = "";
        int aux0, aux1, aux2, aux3, aux4, aux5;
        char dot[5];
        std::string::size_type loc_o;
        std::string::size_type loc_c;
        std::string leftover = "";
        if ((loc_o = extid.find('[')) != std::string::npos) {
            if ((loc_c = extid.find(']', loc_o + 1)) != std::string::npos) {
                mystream rpcid_stream(extid.substr(loc_o + 1, loc_c - loc_o - 1));
                rpcid_stream >> aux0 >> dot[0] >> aux1 >> dot[1] >> aux2 >> dot[2] >> aux3 >> dot[3] >> aux4 >> dot[4] >> aux5 >> leftover;
                std::ostringstream rpcid_nstr;
                rpcid_nstr << aux0 << "/" << aux1 << "/" << aux2 << "/" << aux3 << "/" << aux4 << "/" << aux5;
                new_extid = rpcid_nstr.str();
            }
        }
        ATH_MSG_INFO(extid << " hash Id " << Idhash << " new format " << new_extid);

        RpcReadoutSet Set(p_MuonMgr, Id);
        int nmod = Set.NreadoutElements();
        ATH_MSG_INFO(" Number of modules  in this RpcSet " << nmod);
        ATH_MSG_INFO(" Number of doubletZ in this RpcSet " << Set.NdoubletZ());
        int ndbz = Set.NdoubletZ();
        ATH_MSG_INFO(" Number of modules in Phi/DoubletZ: ");
        for (int i = 1; i <= ndbz; i++) ATH_MSG_INFO(Set.NPhimodules(i) << " ");
        ATH_MSG_INFO(" Number of modules  in this RpcSet " << nmod << " Number of doubletZ in this RpcSet " << Set.NdoubletZ());
        ATH_MSG_INFO(" Number of modules in Phi/DoubletZ: ");
        for (int i = 1; i <= ndbz; i++) ATH_MSG_INFO(Set.NPhimodules(i) << " ");

        double zmin{ 99999999}, zmax{ -99999999}, phimin { 999999}, phimax { -999999};
        Amg::Vector3D Pzmin(0, 0, zmin);
        Amg::Vector3D Pzmax(0, 0, zmax);
        unsigned int nmodules = 0;
        for (int dbz = 1; dbz <= ndbz; dbz++) {
            const RpcReadoutElement* rpcold = nullptr;
            int ndbp = Set.NPhimodules(dbz);
            for (int dbp = 1; dbp <= ndbp; dbp++) {
                ATH_MSG_INFO(" dbz, dbp = " << dbz << " " << dbp);
                const RpcReadoutElement* rpc = Set.readoutElement(dbz, dbp);
                ATH_MSG_INFO("_rpc = " << rpc);
                if (rpc != rpcold) {
                    nmodules++;
                    rpcold = rpc;
                    // here a new module
                    Amg::Vector3D rpcPos = rpc->center();
                    double zminMod = rpcPos.z() - rpc->getZsize() / 2.;
                    double zmaxMod = rpcPos.z() + rpc->getZsize() / 2.;
                    double dphi = atan2(rpc->getSsize() / 2., rpcPos.perp());
                    double pminMod = rpcPos.phi() - dphi;
                    double pmaxMod = rpcPos.phi() + dphi;

                    if (zminMod < zmin) {
                        zmin = zminMod;
                        Pzmin = rpcPos;
                        Pzmin[2] = zmin;
                    }
                    if (zmaxMod > zmax) {
                        zmax = zmaxMod;
                        Pzmax = rpcPos;
                        Pzmax[2] = zmax;
                    }
                    if (pminMod < phimin) phimin = pminMod;
                    if (pmaxMod > phimax) phimax = pmaxMod;
                }
            }
        }
        if (nmodules != Set.NreadoutElements()) ATH_MSG_INFO(" nmod = " << nmodules << " != NreadoutElements() " << nmod);
        // here define the eta and phi(0-2*pi) ranges
        coercePositivePhi(phimin);
        coercePositivePhi(phimax);
        double eta_min = Pzmin.eta();
        double eta_max = Pzmax.eta();
        ATH_MSG_INFO("eta range " << eta_min << " " << eta_max << " phi range " << phimin << " " << phimax);

        fout0 << new_extid << setiosflags(std::ios::fixed) << std::setprecision(0) << std::setw(6) << Idhash << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << eta_min << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12)
              << eta_max << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12) << phimin << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << phimax << std::endl;
    }

    fout0.close();
}

void MuonGMCheck::buildMdtRegionSelectorMap() {
    std::vector<Identifier>::const_iterator idfirst = m_idHelperSvc->mdtIdHelper().module_begin();
    std::vector<Identifier>::const_iterator idlast = m_idHelperSvc->mdtIdHelper().module_end();

    IdContext mdtModuleContext = m_idHelperSvc->mdtIdHelper().module_context();

    std::ofstream fout0("MDTMapIdAndHash.txt");
    fout0 << "MDT ExtendedId HashId Etamin Etamax Phimin Phimax " << std::endl;

    for (std::vector<Identifier>::const_iterator i = idfirst; i != idlast; ++i) {
        Identifier Id = *i;
        IdentifierHash Idhash;
        int gethash_code = m_idHelperSvc->mdtIdHelper().get_hash(Id, Idhash, &mdtModuleContext);
        std::string extid = m_idHelperSvc->toString(Id);
        ATH_MSG_INFO("\n Identifier = " << extid);
        if (gethash_code == 0)
            ATH_MSG_INFO(" its hash Id is " << Idhash);
        else
            ATH_MSG_INFO("     hash Id NOT computed " << Idhash);

        std::string new_extid = "";
        int aux0, aux1, aux2, aux3, aux4, aux5;
        char dot[5];
        std::string::size_type loc_o;
        std::string::size_type loc_c;
        std::string leftover = "";
        if ((loc_o = extid.find('[')) != std::string::npos) {
            if ((loc_c = extid.find(']', loc_o + 1)) != std::string::npos) {
                mystream mdtid_stream(extid.substr(loc_o + 1, loc_c - loc_o - 1));
                mdtid_stream >> aux0 >> dot[0] >> aux1 >> dot[1] >> aux2 >> dot[2] >> aux3 >> dot[3] >> aux4 >> dot[4] >> aux5 >> leftover;
                std::ostringstream mdtid_nstr;
                mdtid_nstr << aux0 << "/" << aux1 << "/" << aux2 << "/" << aux3 << "/" << aux4;
                new_extid = mdtid_nstr.str();
            }
        }
        ATH_MSG_INFO(extid << " hash Id " << Idhash << " new format " << new_extid);

        IdContext mdtChannelContext = m_idHelperSvc->mdtIdHelper().channel_context();
        // get the element corresponding to multilayer = 1
        const MdtReadoutElement* mdt1 = p_MuonMgr->getMdtReadoutElement(Id);
        if (!mdt1) {
            ATH_MSG_INFO(" Mdt Readout Element not found for Id = " << m_idHelperSvc->toString(Id) << " go to next ");
            continue;
        }
        ATH_MSG_INFO(" Station name / technology " << mdt1->getStationName() << " / " << mdt1->getTechnologyName());
        Identifier Id2 = m_idHelperSvc->mdtIdHelper().channelID(Id, 2, 1, 1);
        // get the element corresponding to multilayer = 2
        const MdtReadoutElement* mdt2 = p_MuonMgr->getMdtReadoutElement(Id2);
        double tubePitch = mdt1->tubePitch();
        int ntlay = mdt1->getNLayers();
        int ntubesl1 = mdt1->getNtubesperlayer();
        int ntubesl2 = 0;
        if (!mdt2) {
            ATH_MSG_INFO(" Mdt Readout Element not found for Id = " << m_idHelperSvc->toString(Id2));
        } else
            ntubesl2 = mdt2->getNtubesperlayer();

        std::array<Identifier,4> Idv{};
        Idv[0] = m_idHelperSvc->mdtIdHelper().channelID(Id, 1, 1, 1);
        Idv[1] = m_idHelperSvc->mdtIdHelper().channelID(Id, 1, 1, ntubesl1);
        Idv[2] = m_idHelperSvc->mdtIdHelper().channelID(Id, 2, ntlay, 1);
        Idv[3] = m_idHelperSvc->mdtIdHelper().channelID(Id, 2, ntlay, ntubesl2);

        ATH_MSG_INFO(" Number of tube layers " << ntlay);
        ATH_MSG_INFO(" Number of tubes / layer (1 ,2) " << ntubesl1 << ", " << ntubesl2);

        double rmin{ 99999999.}, rmax{ -99999999.}, zmin{ 99999999.}, zmax{ -99999999.}, zpos21{0.},
                emin{ 99999999.}, emax{ -99999999.}, phimin {999999.}, phimax {-999999.};

        Identifier Idsl = m_idHelperSvc->mdtIdHelper().channelID(Id, 1, 2, 1);
        if (mdt1->barrel()) {
            zpos21 = (mdt1->tubePos(Idsl)).z() - (mdt1->tubePos(Idv[0])).z();
            if (zpos21 > 1.)
                ATH_MSG_INFO(" staggering this way >>>>> ");
            else if (zpos21 < -1.)
                ATH_MSG_INFO(" staggering this way <<<<< ");
            else
                ATH_MSG_INFO(" zpos 21 = 0 ??? " << zpos21);
        } else {
            zpos21 = (mdt1->tubePos(Idsl)).perp() - (mdt1->tubePos(Idv[0])).perp();
            if (zpos21 > 1.)
                ATH_MSG_INFO(" staggering this way /\\ ");
            else if (zpos21 < -1.)
                ATH_MSG_INFO(" staggering this way \\/ ");
            else
                ATH_MSG_INFO(" zpos 21 = 0 ??? " << zpos21);
        }

        for (int i = 0; i < 4; i++) {
            //
            const MdtReadoutElement* mdt = nullptr;
            i < 2 ? mdt = mdt1 : mdt = mdt2;
            if (!mdt) {
                ATH_MSG_INFO(" element not found for index i = " << i << " --------- ");
                if (i == 2) {
                    Idv[2] = m_idHelperSvc->mdtIdHelper().channelID(Id, 1, ntlay, 1);
                    mdt = p_MuonMgr->getMdtReadoutElement(Idv[2]);
                } else if (i == 3) {
                    Idv[3] = m_idHelperSvc->mdtIdHelper().channelID(Id, 1, ntlay, ntubesl1);
                    mdt = p_MuonMgr->getMdtReadoutElement(Idv[3]);
                }
            }
            if (!mdt) {
                ATH_MSG_INFO(" Skipping element; i = " << i << " ----- ");
                continue;
            }
            Amg::Vector3D mdtPos = mdt->tubePos(Idv[i]);
            ATH_MSG_INFO(m_idHelperSvc->toString(Idv[i])
                         << " index " << i << " posx,y,z " << mdtPos << " R = " << mdtPos.perp());
            //
            Amg::Vector3D mdtPos1 = mdtPos;
            Amg::Vector3D mdtPos2 = mdtPos;
            double scaleMin = (mdtPos.perp() - tubePitch / 2.) / mdtPos.perp();
            double scalePlus = (mdtPos.perp() + tubePitch / 2.) / mdtPos.perp();
            if (mdt->barrel()) {
                // these are z ranges of the first or last tube layer
                mdtPos1[2] = mdtPos.z() - tubePitch / 2.;
                mdtPos2[2] = mdtPos.z() + tubePitch / 2.;
                // correct the z ranges of the first or last tube layer to account for tube staggering
                if (zpos21 > 1.) {
                    mdtPos2[2] = mdtPos2.z() + tubePitch / 2.;
                } else if (zpos21 < -1.) {
                    mdtPos1[2] = mdtPos1.z() - tubePitch / 2.;
                }

                if (i < 2) {
                    mdtPos1[0] *= scaleMin;
                    mdtPos1[1] *= scaleMin;
                    mdtPos2[0] *= scaleMin;
                    mdtPos2[1] *= scaleMin;
                } else {
                    mdtPos1[0] *= scalePlus;
                    mdtPos1[1] *= scalePlus;
                    mdtPos2[0] *= scalePlus;
                    mdtPos2[1] *= scalePlus;
                }
            } else {
                // these are z ranges of the first or last tube layer
                mdtPos1[0] *= scaleMin;
                mdtPos1[1] *= scaleMin;
                mdtPos2[0] *= scalePlus;
                mdtPos2[1] *= scalePlus;
                // correct the z ranges of the first or last tube layer to account for tube staggering
                if (zpos21 > 1.) {
                    scalePlus = (mdtPos2.perp() + tubePitch / 2.) / mdtPos2.perp();
                    mdtPos2[0] *= scalePlus;
                    mdtPos2[1] *= scalePlus;
                } else if (zpos21 < -1.) {
                    scaleMin = (mdtPos1.perp() - tubePitch / 2.) / mdtPos1.perp();
                    mdtPos1[0] *= scaleMin;
                    mdtPos1[1] *= scaleMin;
                }
                if (i < 2) {
                    if (mdt->sideA()) {
                        mdtPos1[2] = mdtPos.z() - tubePitch / 2.;
                        mdtPos2[2] = mdtPos.z() - tubePitch / 2.;
                    } else {
                        mdtPos1[2] = mdtPos.z() + tubePitch / 2.;
                        mdtPos2[2] = mdtPos.z() + tubePitch / 2.;
                    }
                } else {
                    if (mdt->sideA()) {
                        mdtPos1[2] = mdtPos.z() + tubePitch / 2.;
                        mdtPos2[2] = mdtPos.z() + tubePitch / 2.;
                    } else {
                        mdtPos1[2] = mdtPos.z() - tubePitch / 2.;
                        mdtPos2[2] = mdtPos.z() - tubePitch / 2.;
                    }
                }
            }

            double eminMod{0.}, emaxMod{0.}, zminMod{0.}, zmaxMod{0.}, rminMod{0.}, rmaxMod{0.}, dphi{0.};
            if (mdt->barrel()) {
                eminMod = mdtPos1.eta();
                emaxMod = mdtPos2.eta();

                zminMod = mdtPos1.z();
                zmaxMod = mdtPos2.z();

                rminMod = mdtPos1.perp();
                rmaxMod = mdtPos2.perp();

                dphi = atan2(mdt->getSsize() / 2., (mdtPos.perp() - tubePitch / 2.));
            } else {
                if (mdt->sideA()) {
                    eminMod = mdtPos2.eta();
                    emaxMod = mdtPos1.eta();

                    zminMod = mdtPos2.z();
                    zmaxMod = mdtPos1.z();

                    rminMod = mdtPos1.perp();
                    rmaxMod = mdtPos2.perp();
                } else {
                    eminMod = mdtPos1.eta();
                    emaxMod = mdtPos2.eta();

                    zminMod = mdtPos1.z();
                    zmaxMod = mdtPos2.z();

                    rminMod = mdtPos1.perp();
                    rmaxMod = mdtPos2.perp();
                }
                dphi = atan2(mdt->tubeLength(Idv[i]) / 2., (mdtPos.perp() - tubePitch / 2.));
            }
            double pminMod = mdtPos.phi() - dphi;
            double pmaxMod = mdtPos.phi() + dphi;
            ATH_MSG_INFO(" Stack  zmin, max " << zminMod << " " << zmaxMod << " phimin, max " << pminMod << " " << pmaxMod
                                              << "   Rmin, max " << rminMod << " " << rmaxMod << " etamin, max " << eminMod << " "
                                              << emaxMod);

            if (zminMod < zmin) { zmin = zminMod; }
            if (zmaxMod > zmax) { zmax = zmaxMod; }
            if (pminMod < phimin) phimin = pminMod;
            if (pmaxMod > phimax) phimax = pmaxMod;
            if (eminMod < emin) emin = eminMod;
            if (emaxMod > emax) emax = emaxMod;
            if (rminMod < rmin) rmin = rminMod;
            if (rmaxMod > rmax) rmax = rmaxMod;
            ATH_MSG_INFO(" Module emin - emax " << emin << " " << emax << " phimin - phimax " << phimin << " " << phimax);
        }

        // here define the eta and phi(0-2*pi) ranges
        coercePositivePhi(phimin);
        coercePositivePhi(phimax);

        ATH_MSG_INFO(" ***** Z range " << zmin << " " << zmax << " ***** R range " << rmin << " " << rmax << " --- eta range " << emin
                                       << " " << emax << " phi range " << phimin << " " << phimax);

        fout0 << new_extid << setiosflags(std::ios::fixed) << std::setprecision(0) << std::setw(6) << Idhash << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << emin << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12)
              << emax << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12) << phimin << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << phimax << std::endl;
    }

    fout0.close();
}
void MuonGMCheck::buildTgcRegionSelectorMap() {
    std::vector<Identifier>::const_iterator idfirst = m_idHelperSvc->tgcIdHelper().module_begin();
    std::vector<Identifier>::const_iterator idlast = m_idHelperSvc->tgcIdHelper().module_end();

    const IdContext tgcModuleContext = m_idHelperSvc->tgcIdHelper().module_context();

    std::ofstream fout0("TGCMapIdAndHash.txt");
    fout0 << "TGC ExtendedId HashId Etamin Etamax Phimin Phimax " << std::endl;

    for (std::vector<Identifier>::const_iterator i = idfirst; i != idlast; ++i) {
        Identifier elemId = *i;
        IdentifierHash hashId;

        int gethash_code = m_idHelperSvc->tgcIdHelper().get_hash(elemId, hashId, &tgcModuleContext);

        std::string extid = m_idHelperSvc->toString(elemId);
        ATH_MSG_INFO("\n Identifier = " << extid);
        if (gethash_code == 0)
            ATH_MSG_INFO(" its hash Id is " << hashId);
        else
            ATH_MSG_INFO("     hash Id NOT computed " << hashId);

        std::string new_extid = "";
        int aux0, aux1, aux2, aux3, aux4, aux5;
        char dot[5];
        std::string::size_type loc_o;
        std::string::size_type loc_c;
        std::string leftover = "";
        if ((loc_o = extid.find('[')) != std::string::npos) {
            if ((loc_c = extid.find(']', loc_o + 1)) != std::string::npos) {
                std::istringstream tgcid_stream(extid.substr(loc_o + 1, loc_c - loc_o - 1));
                tgcid_stream >> aux0 >> dot[0] >> aux1 >> dot[1] >> aux2 >> dot[2] >> aux3 >> dot[3] >> aux4 >> dot[4] >> aux5 >> leftover;
                std::ostringstream mdtid_nstr;
                mdtid_nstr << aux0 << "/" << aux1 << "/" << aux2 << "/" << aux3 << "/" << aux4;
                new_extid = mdtid_nstr.str();
            }
        }
        ATH_MSG_INFO(extid << " hash Id " << hashId << " new format " << new_extid);

        const TgcReadoutElement* tgc = p_MuonMgr->getTgcReadoutElement(elemId);
        if (!tgc) {
            ATH_MSG_INFO(" Tgc Readout Element not found for Id = " << m_idHelperSvc->toString(elemId)
                                                                    << " go to next ");
            continue;
        }
        ATH_MSG_INFO(" Station name / technology " << tgc->getStationName() << " / " << tgc->getTechnologyName());

        Amg::Vector3D posmin, posmax;
        float etamin, etamax;
        int gapMin = m_idHelperSvc->tgcIdHelper().gasGapMin(elemId);
        int gapMax = m_idHelperSvc->tgcIdHelper().gasGapMax(elemId);

        Identifier chId;
        chId = m_idHelperSvc->tgcIdHelper().channelID(elemId, gapMin, 0, 1);
        const int chmax = m_idHelperSvc->tgcIdHelper().channelMax(chId);
        posmax = tgc->channelPos(gapMin, 0, chmax);  // gapMax gives posmax!
        chId = m_idHelperSvc->tgcIdHelper().channelID(elemId, gapMax, 0, 1);
        const int chmin = m_idHelperSvc->tgcIdHelper().channelMin(chId);
        posmin = tgc->channelPos(gapMax, 0, chmin);  // gapMin gives posmin!

        // calculation based on active sensitive area
        float activeheight;
        Amg::Vector3D posctr;
        posctr = tgc->globalPosition();
        activeheight = tgc->length();
        etamin = -logf(tan(atan((posctr.perp() - activeheight / 2.) / std::abs(posmin.z())) / 2.));
        etamax = -logf(tan(atan((posctr.perp() + activeheight / 2.) / std::abs(posmax.z())) / 2.));
        if (m_idHelperSvc->tgcIdHelper().stationEta(elemId) < 0) {
            etamin = -etamin;
            etamax = -etamax;
        }

        float phimin, phimax;

        // calculation based on active sensitive area
        float activelongside = tgc->longWidth() - tgc->frameXwidth() * 2.;
        phimin = atan2f(posctr.y(), posctr.x()) - atan2f(activelongside / 2., posctr.perp() + activeheight / 2.);
        phimax = atan2f(posctr.y(), posctr.x()) + atan2f(activelongside / 2., posctr.perp() + activeheight / 2.);
        if (phimin < 0) phimin += 2. * M_PI;
        if (phimax < 0) phimax += 2. * M_PI;

        fout0 << new_extid << setiosflags(std::ios::fixed) << std::setprecision(0) << std::setw(6) << hashId << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << etamin << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12)
              << etamax << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12) << phimin << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << phimax << std::endl;
    }
    fout0.close();
}

void MuonGMCheck::buildCscRegionSelectorMap() {
    std::vector<Identifier>::const_iterator idfirst = m_idHelperSvc->cscIdHelper().module_begin();
    std::vector<Identifier>::const_iterator idlast = m_idHelperSvc->cscIdHelper().module_end();

    IdContext cscModuleContext = m_idHelperSvc->cscIdHelper().module_context();

    std::ofstream fout0("CSCMapIdAndHash.txt");
    fout0 << "CSC ExtendedId HashId Etamin Etamax Phimin Phimax " << std::endl;

    for (std::vector<Identifier>::const_iterator i = idfirst; i != idlast; ++i) {
        Identifier Id = *i;
        IdentifierHash Idhash;
        int gethash_code = m_idHelperSvc->cscIdHelper().get_hash(Id, Idhash, &cscModuleContext);
        std::string extid = m_idHelperSvc->toString(Id);
        ATH_MSG_INFO("\n Identifier = " << extid);
        if (gethash_code == 0)
            ATH_MSG_INFO(" its hash Id is " << Idhash);
        else
            ATH_MSG_INFO("     hash Id NOT computed " << Idhash);

        std::string new_extid = "";
        int aux0, aux1 = 0, aux2, aux3 = 0, aux4, aux5;
        char dot[5];
        std::string::size_type loc_o;
        std::string::size_type loc_c;
        std::string leftover = "";
        if ((loc_o = extid.find('[')) != std::string::npos) {
            if ((loc_c = extid.find(']', loc_o + 1)) != std::string::npos) {
                mystream cscid_stream(extid.substr(loc_o + 1, loc_c - loc_o - 1));
                cscid_stream >> aux0 >> dot[0] >> aux1 >> dot[1] >> aux2 >> dot[2] >> aux3 >> dot[3] >> aux4 >> dot[4] >> aux5 >> leftover;
                std::ostringstream cscid_nstr;
                cscid_nstr << aux0 << "/" << aux1 << "/" << aux2 << "/" << aux3 << "/" << aux4;
                new_extid = cscid_nstr.str();
            }
        }
        // aux1 is module type (50,51)
        // aux2 is eta region (-1,1)
        // aux3 is phi region (1,8)
        if (aux1 == 0 || aux3 == 0) {
            ATH_MSG_ERROR("There is something wrong in buildCscRegionSelectorMap!");
            ATH_MSG_ERROR("Variables aux1 and/or aux3 not initialized - Taking emergency exit!");
            throw std::runtime_error("There is something wrong in buildCscRegionSelectorMap! "
                                     "Variables aux1 and/or aux3 not initialized");
                                     
        }

        ATH_MSG_INFO(extid << " hash Id " << Idhash << " new format " << new_extid);

        // retrieve CscReadoutElement
        Identifier Id2ndLayer;
        int chamberLayer = 2;
        const CscReadoutElement* csc;
        csc = p_MuonMgr->getCscReadoutElement(Id);
        if (!csc) {
            ATH_MSG_INFO("Csc Readout Element not found for this Id ---- try 2nd layer");
            Id2ndLayer = m_idHelperSvc->cscIdHelper().parentID(Id);
            Id2ndLayer = m_idHelperSvc->cscIdHelper().channelID(Id2ndLayer, chamberLayer, 1, 0, 1);
            csc = p_MuonMgr->getCscReadoutElement(Id2ndLayer);
            if (!csc) {
                ATH_MSG_INFO("Csc Readout Element not found for 2nd layer too ---- skip");
                continue;
            } else {
                ATH_MSG_INFO("Csc Readout Element found for 2nd layer");
            }
        }

        ATH_MSG_INFO("number of chamber layers : " << m_idHelperSvc->cscIdHelper().chamberLayerMin(Id) << ","
                                                   << m_idHelperSvc->cscIdHelper().chamberLayerMax(Id));
        ATH_MSG_INFO("number of wire layers : " << m_idHelperSvc->cscIdHelper().wireLayerMin(Id) << ","
                                                << m_idHelperSvc->cscIdHelper().wireLayerMax(Id));
        ATH_MSG_INFO("strip number : " << m_idHelperSvc->cscIdHelper().stripMin(Id) << "," << m_idHelperSvc->cscIdHelper().stripMax(Id));

        double eta_min { 99999999.}, eta_max { -99999999.}, phi_min { 999999.}, phi_max { -999999.}, phi_test{0.};
        Identifier Id_phi_max{0}, Id_phi_min{0}, Id_eta_max{0}, Id_eta_min{0};

        for (int chlayer = 1; chlayer <= m_idHelperSvc->cscIdHelper().chamberLayerMax(Id); chlayer++) {
            for (int wlayer = 1; wlayer <= m_idHelperSvc->cscIdHelper().wireLayerMax(Id); wlayer++) {
                for (int phis = 1; phis <= csc->NphiStrips(wlayer); phis++) {
                    Identifier phis_id = m_idHelperSvc->cscIdHelper().channelID(Id, chlayer, wlayer, 1, phis);
                    Amg::Vector3D phis_x = csc->localStripPos(phis_id);
                    double phis_lenght = csc->stripLength(phis_id);

                    Amg::Vector3D phis_x1(phis_x[0], phis_x[1], 0);
                    Amg::Vector3D phis_x2(phis_x[0], phis_x[1], 0);

                    phis_x1[2] = phis_x.z() - phis_lenght / 2.0;
                    phis_x2[2] = phis_x.z() + phis_lenght / 2.0;

                    Amg::Vector3D gphis_x1 = csc->globalPos(phis_x1);
                    Amg::Vector3D gphis_x2 = csc->globalPos(phis_x2);

                    phi_test = gphis_x1.phi();
                    // for detector in (-0.25,0.25) phi interval use +-3,14 phi interval
                    if (!(aux1 == 51 && aux3 == 1))
                        if (phi_test < 0) phi_test += 2. * M_PI;

                    // phi
                    if (phi_test > phi_max) {
                        Id_phi_max = phis_id;
                        phi_max = phi_test;
                    }
                    if (phi_test < phi_min) {
                        Id_phi_min = phis_id;
                        phi_min = phi_test;
                    }
                    // eta
                    if (gphis_x1.eta() > eta_max) {
                        Id_eta_max = phis_id;
                        eta_max = gphis_x1.eta();
                    }
                    if (gphis_x1.eta() < eta_min) {
                        Id_eta_min = phis_id;
                        eta_min = gphis_x1.eta();
                    }

                    phi_test = gphis_x2.phi();
                    // for detector in (-0.25,0.25) phi interval use +-3,14 phi interval
                    if (!(aux1 == 51 && aux3 == 1))
                        if (phi_test < 0) phi_test += 2. * M_PI;

                    // phi
                    if (phi_test > phi_max) {
                        Id_phi_max = phis_id;
                        phi_max = phi_test;
                    }
                    if (phi_test < phi_min) {
                        Id_phi_min = phis_id;
                        phi_min = phi_test;
                    }
                    // eta
                    if (gphis_x2.eta() > eta_max) {
                        Id_eta_max = phis_id;
                        eta_max = gphis_x2.eta();
                    }
                    if (gphis_x2.eta() < eta_min) {
                        Id_eta_min = phis_id;
                        eta_min = gphis_x2.eta();
                    }
                    ATH_MSG_INFO("pm 1 n " << phis << " phi1 " << gphis_x1.phi() << " phi2 " << gphis_x2.phi() << " phi_min " << phi_min
                                           << " phi_max " << phi_max);

                    ATH_MSG_INFO("pm 1 n " << phis << " eta1 " << gphis_x1.eta() << " eta2 " << gphis_x2.eta() << " eta_min " << eta_min
                                           << " eta_max " << eta_max);
                    ATH_MSG_INFO("------------------------------------------------------------------------------------------------");
                }
                for (int etas = 1; etas <= csc->NetaStrips(wlayer); etas++) {
                    Identifier etas_id = m_idHelperSvc->cscIdHelper().channelID(Id, chlayer, wlayer, 0, etas);
                    Amg::Vector3D etas_x = csc->localStripPos(etas_id);
                    double etas_lenght = csc->stripLength(etas_id);

                    Amg::Vector3D etas_x1 = etas_x;
                    Amg::Vector3D etas_x2 = etas_x;

                    etas_x1[1] = etas_x.y() - etas_lenght / 2.0;
                    etas_x2[1] = etas_x.y() + etas_lenght / 2.0;

                    Amg::Vector3D getas_x1 = csc->globalPos(etas_x1);
                    Amg::Vector3D getas_x2 = csc->globalPos(etas_x2);

                    phi_test = getas_x1.phi();
                    // for detector in (-0.25,0.25) phi interval use +-3,14 phi interval
                    if (!(aux1 == 51 && aux3 == 1))
                        if (phi_test < 0) phi_test += 2. * M_PI;
                    // phi
                    if (phi_test > phi_max) {
                        Id_phi_max = etas_id;
                        phi_max = phi_test;
                    }
                    if (phi_test < phi_min) {
                        Id_phi_min = etas_id;
                        phi_min = phi_test;
                    }
                    // eta
                    if (getas_x1.eta() > eta_max) {
                        Id_eta_max = etas_id;
                        eta_max = getas_x1.eta();
                    }
                    if (getas_x1.eta() < eta_min) {
                        Id_eta_min = etas_id;
                        eta_min = getas_x1.eta();
                    }

                    phi_test = getas_x2.phi();
                    // for detector in (-0.25,0.25) phi interval use +-3,14 phi interval
                    if (!(aux1 == 51 && aux3 == 1))
                        if (phi_test < 0) phi_test += 2. * M_PI;
                    // phi
                    if (phi_test > phi_max) {
                        Id_phi_max = etas_id;
                        phi_max = phi_test;
                    }
                    if (phi_test < phi_min) {
                        Id_phi_min = etas_id;
                        phi_min = phi_test;
                    }
                    // eta
                    if (getas_x2.eta() > eta_max) {
                        Id_eta_max = etas_id;
                        eta_max = getas_x2.eta();
                    }
                    if (getas_x2.eta() < eta_min) {
                        Id_eta_min = etas_id;
                        eta_min = getas_x2.eta();
                    }

                    ATH_MSG_INFO("pm 0 n " << etas << " phi1 " << getas_x1.phi() << " phi2 " << getas_x2.phi() << " phi_min " << phi_min
                                           << " phi_max " << phi_max);

                    ATH_MSG_INFO("pm 0 n " << etas << " eta1 " << getas_x1.eta() << " eta2 " << getas_x2.eta() << " eta_min " << eta_min
                                           << " eta_max " << eta_max);
                    ATH_MSG_INFO("------------------------------------------------------------------------------------------------");
                }
            }  // gas gaps
        }      // chamber layers

        int cl_phi_min = m_idHelperSvc->cscIdHelper().chamberLayer(Id_phi_min);
        int cl_phi_max = m_idHelperSvc->cscIdHelper().chamberLayer(Id_phi_max);
        int cl_eta_min = m_idHelperSvc->cscIdHelper().chamberLayer(Id_eta_min);
        int cl_eta_max = m_idHelperSvc->cscIdHelper().chamberLayer(Id_eta_max);

        int wl_phi_min = m_idHelperSvc->cscIdHelper().wireLayer(Id_phi_min);
        int wl_phi_max = m_idHelperSvc->cscIdHelper().wireLayer(Id_phi_max);
        int wl_eta_min = m_idHelperSvc->cscIdHelper().wireLayer(Id_eta_min);
        int wl_eta_max = m_idHelperSvc->cscIdHelper().wireLayer(Id_eta_max);

        int N_phi_min = m_idHelperSvc->cscIdHelper().strip(Id_phi_min);
        int N_phi_max = m_idHelperSvc->cscIdHelper().strip(Id_phi_max);
        int N_eta_min = m_idHelperSvc->cscIdHelper().strip(Id_eta_min);
        int N_eta_max = m_idHelperSvc->cscIdHelper().strip(Id_eta_max);

        int mp_phi_min = m_idHelperSvc->cscIdHelper().measuresPhi(Id_phi_min);
        int mp_phi_max = m_idHelperSvc->cscIdHelper().measuresPhi(Id_phi_max);
        int mp_eta_min = m_idHelperSvc->cscIdHelper().measuresPhi(Id_eta_min);
        int mp_eta_max = m_idHelperSvc->cscIdHelper().measuresPhi(Id_eta_max);

        ATH_MSG_INFO("--------> phi_min " << phi_min << " mp " << mp_phi_min << " chl " << cl_phi_min << " wl " << wl_phi_min << " strip "
                                          << N_phi_min);
        ATH_MSG_INFO("--------> phi_max " << phi_max << " mp " << mp_phi_max << " chl " << cl_phi_max << " wl " << wl_phi_max << " strip "
                                          << N_phi_max);
        ATH_MSG_INFO("--------> eta_min " << eta_min << " mp " << mp_eta_min << " chl " << cl_eta_min << " wl " << wl_eta_min << " strip "
                                          << N_eta_min);
        ATH_MSG_INFO("--------> eta_max " << eta_max << " mp " << mp_eta_max << " chl " << cl_eta_max << " wl " << wl_eta_max << " strip "
                                          << N_eta_max);
        ATH_MSG_INFO("--------> Dphi " << std::abs(phi_max - phi_min) << " Deta " << std::abs(eta_max - eta_min));

        if (aux1 == 51 && aux3 == 1)
            if (phi_min < 0) phi_min += 2. * M_PI;
        if (aux1 == 51 && aux3 == 1)
            if (phi_max < 0) phi_max += 2. * M_PI;

        fout0 << new_extid << setiosflags(std::ios::fixed) << std::setprecision(0) << std::setw(6) << Idhash << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << eta_min << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12)
              << eta_max << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12) << phi_min << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << phi_max << std::endl;
    }  // modules
    fout0.close();
}

void MuonGMCheck::testRpcCache() {
    ATH_MSG_INFO(" *************************** Building Rpc cache");
    p_MuonMgr->fillRpcCache();
}

void MuonGMCheck::testTgcCache() {
    ATH_MSG_INFO(" *************************** Building Tgc cache");
    p_MuonMgr->fillTgcCache();
}
void MuonGMCheck::testCscCache() {
    ATH_MSG_INFO(" *************************** Building Csc cache");
    p_MuonMgr->fillCscCache();
}
void MuonGMCheck::testMdtCache() {
    ATH_MSG_INFO(" *************************** Building Mdt cache");
    p_MuonMgr->fillMdtCache();
}

void MuonGMCheck::testMdtDetectorElementHash() {
    ATH_MSG_INFO(" start running ");
    std::vector<Identifier>::const_iterator idfirst = m_idHelperSvc->mdtIdHelper().detectorElement_begin();
    std::vector<Identifier>::const_iterator idlast = m_idHelperSvc->mdtIdHelper().detectorElement_end();

    IdContext mdtDetElemContext = m_idHelperSvc->mdtIdHelper().detectorElement_context();

    std::ofstream fout0("testDEtElemHash_MDT.txt");
    fout0 << "MDT ExtendedId HashId Etamin Etamax Phimin Phimax " << std::endl;

    for (std::vector<Identifier>::const_iterator i = idfirst; i != idlast; ++i) {
        Identifier Id = *i;
        IdentifierHash Idhash;
        int gethash_code = m_idHelperSvc->mdtIdHelper().get_hash(Id, Idhash, &mdtDetElemContext);
        std::string extid = m_idHelperSvc->toString(Id);
        ATH_MSG_DEBUG("MDT Identifier = " << extid);
        if (gethash_code == 0)
            ATH_MSG_DEBUG(" its hash Id is " << Idhash);
        else
            ATH_MSG_ERROR("     hash Id NOT computed " << Idhash << " for Id " << extid);

        std::string new_extid = "";
        int aux0 = 0, aux1 = 0, aux2 = 0, aux3 = 0, aux4 = 0, aux5 = 0, aux6 = 0;
        char dot[6];
        std::string::size_type loc_o;
        std::string::size_type loc_c;
        std::string leftover = "";
        if ((loc_o = extid.find('[')) != std::string::npos) {
            if ((loc_c = extid.find(']', loc_o + 1)) != std::string::npos) {
                mystream mdtid_stream(extid.substr(loc_o + 1, loc_c - loc_o - 1));
                mdtid_stream >> aux0 >> dot[0] >> aux1 >> dot[1] >> aux2 >> dot[2] >> aux3 >> dot[3] >> aux4 >> dot[4] >> aux5 >> dot[5] >>
                    aux6 >> leftover;
                std::ostringstream mdtid_nstr;
                mdtid_nstr << aux0 << "/" << aux1 << "/" << aux2 << "/" << aux3 << "/" << aux4 << "/" << aux5 << "/" << aux6;
                new_extid = mdtid_nstr.str();
            } else {
                ATH_MSG_ERROR(extid << "There is sth wrong with CSC hash Id " << Idhash << " new format " << new_extid
                                    << " ... skipping ... ");
                continue;
            }
        } else {
            ATH_MSG_ERROR(extid << "There is sth wrong with CSC hash Id " << Idhash << " new format " << new_extid << " ... skipping ... ");
            continue;
        }
        ATH_MSG_VERBOSE(extid << " hash Id " << Idhash << " new format " << new_extid);

        const MdtReadoutElement* mdt = p_MuonMgr->getMdtReadoutElement(Idhash);
        if (!mdt) {
            ATH_MSG_ERROR("MuonManager->getMdtReadoutElement(Idhash) fails ! for id = " << extid << " detElemhash " << Idhash);
            continue;
        }

        // here a new module
        Amg::Vector3D mdtPos = mdt->center();
        double zminMod = mdtPos.z() - mdt->getZsize() / 2.;
        double zmaxMod = mdtPos.z() + mdt->getZsize() / 2.;
        double dphi = atan2(mdt->getSsize() / 2., mdtPos.perp());
        double phimin = mdtPos.phi() - dphi;
        double phimax = mdtPos.phi() + dphi;
        Amg::Vector3D Pzmin = mdtPos;
        Pzmin[2] = zminMod;
        Amg::Vector3D Pzmax = mdtPos;
        Pzmax[2] = zmaxMod;

        coercePositivePhi(phimin);
        coercePositivePhi(phimax);
        double eta_min = Pzmin.eta();
        double eta_max = Pzmax.eta();
        ATH_MSG_VERBOSE("eta range " << eta_min << " " << eta_max << " phi range " << phimin << " " << phimax);
        //
        fout0 << new_extid << setiosflags(std::ios::fixed) << std::setprecision(0) << std::setw(6) << Idhash << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << eta_min << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12)
              << eta_max << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12) << phimin << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << phimax << std::endl;
    }
    ATH_MSG_INFO(" end running ");

    fout0.close();
}
void MuonGMCheck::testRpcDetectorElementHash() {
    ATH_MSG_INFO(" start running ");
    std::vector<Identifier>::const_iterator idfirst = m_idHelperSvc->rpcIdHelper().detectorElement_begin();
    std::vector<Identifier>::const_iterator idlast = m_idHelperSvc->rpcIdHelper().detectorElement_end();

    IdContext rpcDetElemContext = m_idHelperSvc->rpcIdHelper().detectorElement_context();

    std::ofstream fout0("testDEtElemHash_RPC.txt");
    fout0 << "RPC ExtendedId HashId Etamin Etamax Phimin Phimax " << std::endl;

    for (std::vector<Identifier>::const_iterator i = idfirst; i != idlast; ++i) {
        Identifier Id = *i;
        IdentifierHash Idhash;
        int gethash_code = m_idHelperSvc->rpcIdHelper().get_hash(Id, Idhash, &rpcDetElemContext);
        std::string extid = m_idHelperSvc->rpcIdHelper().show_to_string(Id);
        ATH_MSG_DEBUG("RPC  Identifier = " << extid);
        if (gethash_code == 0)
            ATH_MSG_DEBUG(" its hash Id is " << Idhash);
        else
            ATH_MSG_ERROR("     hash Id NOT computed " << Idhash);

        std::string new_extid = "";
        int aux0, aux1, aux2, aux3, aux4, aux5, aux6, aux7;
        char dot[7];
        std::string::size_type loc_o;
        std::string::size_type loc_c;
        std::string leftover = "";
        if ((loc_o = extid.find('[')) != std::string::npos) {
            if ((loc_c = extid.find(']', loc_o + 1)) != std::string::npos) {
                mystream rpcid_stream(extid.substr(loc_o + 1, loc_c - loc_o - 1));
                rpcid_stream >> aux0 >> dot[0] >> aux1 >> dot[1] >> aux2 >> dot[2] >> aux3 >> dot[3] >> aux4 >> dot[4] >> aux5 >> dot[5] >>
                    aux6 >> dot[6] >> aux7 >> leftover;
                std::ostringstream rpcid_nstr;
                rpcid_nstr << aux0 << "/" << aux1 << "/" << aux2 << "/" << aux3 << "/" << aux4 << "/" << aux5 << "/" << aux6 << "/" << aux7;
                new_extid = rpcid_nstr.str();
            }
        }
        ATH_MSG_VERBOSE(extid << " hash Id " << Idhash << " new format " << new_extid);

        const RpcReadoutElement* rpc = p_MuonMgr->getRpcReadoutElement(Idhash);
        if (!rpc) {
            ATH_MSG_ERROR("MuonManager->getRpcReadoutElement(Idhash) fails ! for id = " << extid << " detElemhash " << Idhash);
            continue;
        }

        // here a new module
        Amg::Vector3D rpcPos = rpc->center();
        double zminMod = rpcPos.z() - rpc->getZsize() / 2.;
        double zmaxMod = rpcPos.z() + rpc->getZsize() / 2.;
        double dphi = atan2(rpc->getSsize() / 2., rpcPos.perp());
        double phimin = rpcPos.phi() - dphi;
        double phimax = rpcPos.phi() + dphi;
        Amg::Vector3D Pzmin = rpcPos;
        Pzmin[2] = zminMod;
        Amg::Vector3D Pzmax = rpcPos;
        Pzmax[2] = zmaxMod;

        coercePositivePhi(phimin);
        coercePositivePhi(phimax);
        double eta_min = Pzmin.eta();
        double eta_max = Pzmax.eta();
        ATH_MSG_VERBOSE("eta range " << eta_min << " " << eta_max << " phi range " << phimin << " " << phimax);
        //
        fout0 << new_extid << setiosflags(std::ios::fixed) << std::setprecision(0) << std::setw(6) << Idhash << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << eta_min << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12)
              << eta_max << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12) << phimin << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << phimax << std::endl;
    }
    ATH_MSG_INFO(" end running ");

    fout0.close();
}

void MuonGMCheck::testTgcDetectorElementHash() {
    ATH_MSG_INFO(" start running ");

    std::vector<Identifier>::const_iterator idfirst = m_idHelperSvc->tgcIdHelper().detectorElement_begin();
    std::vector<Identifier>::const_iterator idlast = m_idHelperSvc->tgcIdHelper().detectorElement_end();

    IdContext tgcDetElemContext = m_idHelperSvc->tgcIdHelper().detectorElement_context();

    std::ofstream fout0("testDEtElemHash_TGC.txt");
    fout0 << "TGC ExtendedId HashId Etamin Etamax Phimin Phimax " << std::endl;

    for (std::vector<Identifier>::const_iterator i = idfirst; i != idlast; ++i) {
        Identifier Id = *i;
        IdentifierHash Idhash;
        int gethash_code = m_idHelperSvc->tgcIdHelper().get_hash(Id, Idhash, &tgcDetElemContext);
        std::string extid = m_idHelperSvc->toString(Id);
        ATH_MSG_DEBUG("TGC Identifier = " << extid);
        if (gethash_code == 0)
            ATH_MSG_DEBUG(" its hash Id is " << Idhash);
        else
            ATH_MSG_ERROR("     hash Id NOT computed " << Idhash);

        std::string new_extid = "";
        int aux0, aux1, aux2, aux3, aux4, aux5;
        char dot[5];
        std::string::size_type loc_o;
        std::string::size_type loc_c;
        std::string leftover = "";
        if ((loc_o = extid.find('[')) != std::string::npos) {
            if ((loc_c = extid.find(']', loc_o + 1)) != std::string::npos) {
                mystream tgcid_stream(extid.substr(loc_o + 1, loc_c - loc_o - 1));
                tgcid_stream >> aux0 >> dot[0] >> aux1 >> dot[1] >> aux2 >> dot[2] >> aux3 >> dot[3] >> aux4 >> dot[4] >> aux5 >> leftover;
                std::ostringstream tgcid_nstr;
                tgcid_nstr << aux0 << "/" << aux1 << "/" << aux2 << "/" << aux3 << "/" << aux4 << "/" << aux5;
                new_extid = tgcid_nstr.str();
            }
        }
        ATH_MSG_VERBOSE(extid << " hash Id " << Idhash << " new format " << new_extid);

        const TgcReadoutElement* tgc = p_MuonMgr->getTgcReadoutElement(Idhash);
        if (!tgc) {
            ATH_MSG_ERROR("MuonManager->getTgcReadoutElement(Idhash) fails ! for id = " << extid << " detElemhash " << Idhash);
            continue;
        }

        // here a new module
        Amg::Vector3D tgcPos = tgc->center();
        double zminMod = tgcPos.z() - tgc->getZsize() / 2.;
        double zmaxMod = tgcPos.z() + tgc->getZsize() / 2.;
        double dphi = atan2(tgc->getSsize() / 2., tgcPos.perp());
        double phimin = tgcPos.phi() - dphi;
        double phimax = tgcPos.phi() + dphi;
        Amg::Vector3D Pzmin = tgcPos;
        Pzmin[2] = zminMod;
        Amg::Vector3D Pzmax = tgcPos;
        Pzmax[2] = zmaxMod;

        coercePositivePhi(phimin);
        coercePositivePhi(phimax);
        double eta_min = Pzmin.eta();
        double eta_max = Pzmax.eta();
        ATH_MSG_VERBOSE("eta range " << eta_min << " " << eta_max << " phi range " << phimin << " " << phimax);
        //
        fout0 << new_extid << setiosflags(std::ios::fixed) << std::setprecision(0) << std::setw(6) << Idhash << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << eta_min << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12)
              << eta_max << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12) << phimin << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << phimax << std::endl;
    }
    ATH_MSG_INFO(" end running ");

    fout0.close();
}
void MuonGMCheck::testCscDetectorElementHash() {
    ATH_MSG_INFO(" start running ");

    std::vector<Identifier>::const_iterator idfirst = m_idHelperSvc->cscIdHelper().detectorElement_begin();
    std::vector<Identifier>::const_iterator idlast = m_idHelperSvc->cscIdHelper().detectorElement_end();

    IdContext cscDetElemContext = m_idHelperSvc->cscIdHelper().detectorElement_context();

    std::ofstream fout0("testDEtElemHash_CSC.txt");
    fout0 << "CSC ExtendedId HashId Etamin Etamax Phimin Phimax " << std::endl;

    for (std::vector<Identifier>::const_iterator i = idfirst; i != idlast; ++i) {
        Identifier Id = *i;
        IdentifierHash Idhash;
        int gethash_code = m_idHelperSvc->cscIdHelper().get_hash(Id, Idhash, &cscDetElemContext);
        std::string extid = m_idHelperSvc->toString(Id);
        ATH_MSG_DEBUG("CSC Identifier = " << extid);
        if (gethash_code == 0)
            ATH_MSG_DEBUG(" its hash Id is " << Idhash);
        else
            ATH_MSG_ERROR("     hash Id NOT computed " << Idhash);

        std::string new_extid = "";
        int aux0, aux1, aux2, aux3, aux4, aux5, aux6;
        char dot[6];
        std::string::size_type loc_o;
        std::string::size_type loc_c;
        std::string leftover = "";
        if ((loc_o = extid.find('[')) != std::string::npos) {
            if ((loc_c = extid.find(']', loc_o + 1)) != std::string::npos) {
                mystream cscid_stream(extid.substr(loc_o + 1, loc_c - loc_o - 1));
                cscid_stream >> aux0 >> dot[0] >> aux1 >> dot[1] >> aux2 >> dot[2] >> aux3 >> dot[3] >> aux4 >> dot[4] >> aux5 >> dot[5] >>
                    aux6 >> leftover;
                std::ostringstream cscid_nstr;
                cscid_nstr << aux0 << "/" << aux1 << "/" << aux2 << "/" << aux3 << "/" << aux4 << "/" << aux5 << "/" << aux6;
                new_extid = cscid_nstr.str();
            }
        }
        ATH_MSG_VERBOSE(extid << " hash Id " << Idhash << " new format " << new_extid);

        const CscReadoutElement* csc = p_MuonMgr->getCscReadoutElement(Idhash);
        if (!csc) {
            ATH_MSG_ERROR("MuonManager->getCscReadoutElement(Idhash) fails ! for id = " << extid << " detElemhash " << Idhash);
            continue;
        }

        // here a new module
        Amg::Vector3D cscPos = csc->center();
        double zminMod = cscPos.z() - csc->getZsize() / 2.;
        double zmaxMod = cscPos.z() + csc->getZsize() / 2.;
        double dphi = atan2(csc->getSsize() / 2., cscPos.perp());
        double phimin = cscPos.phi() - dphi;
        double phimax = cscPos.phi() + dphi;
        Amg::Vector3D Pzmin = cscPos;
        Pzmin[2] = zminMod;
        Amg::Vector3D Pzmax = cscPos;
        Pzmax[2] = zmaxMod;

        coercePositivePhi(phimin);
        coercePositivePhi(phimax);
        double eta_min = Pzmin.eta();
        double eta_max = Pzmax.eta();
        ATH_MSG_VERBOSE("eta range " << eta_min << " " << eta_max << " phi range " << phimin << " " << phimax);
        //
        fout0 << new_extid << setiosflags(std::ios::fixed) << std::setprecision(0) << std::setw(6) << Idhash << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << eta_min << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12)
              << eta_max << setiosflags(std::ios::fixed) << std::setprecision(3) << std::setw(12) << phimin << setiosflags(std::ios::fixed)
              << std::setprecision(3) << std::setw(12) << phimax << std::endl;
    }
    ATH_MSG_INFO(" end running ");

    fout0.close();
}

void MuonGMCheck::showVmemCpu(const std::string& message) {
    int dmem = 0;
    int ducpu = 0;
    int dscpu = 0;
    getVmemCpu(dmem, ducpu, dscpu);

    ATH_MSG_INFO(message << " Delta_VirtualMemory      " << dmem << " kB --- current Vmem " << m_mem << " kB");
    ATH_MSG_INFO(message << " Delta_CPU_time u/s/total " << ducpu << "/" << dscpu << "/" << ducpu + dscpu << " ms --- current user time "
                         << m_cpu[0] << " ms");
}
void MuonGMCheck::getVmemCpu(int& dVmem, int& dUCpu, int& dSCpu) {
    std::array<int, 2> cpu_now;
    int uTime = 0;
    int sTime = 0;
    int mem_now = PerfUtils::getMem();  // kB
    PerfUtils::getCpu(uTime, sTime);    // ms
    cpu_now[0] = uTime;
    cpu_now[1] = sTime;

    dVmem = mem_now - m_mem;
    m_mem = mem_now;
    dUCpu = cpu_now[0] - m_cpu[0];
    m_cpu[0] = cpu_now[0];
    dSCpu = cpu_now[1] - m_cpu[1];
    m_cpu[1] = cpu_now[1];

    return;
}
