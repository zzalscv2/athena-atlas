/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonReadoutGeometry/MuonDetectorManager.h"

#include <fstream>
#include <utility>

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "MuonAlignmentData/ALinePar.h"
#include "MuonAlignmentData/BLinePar.h"
#include "MuonAlignmentData/CscInternalAlignmentPar.h"
#include "MuonReadoutGeometry/CscReadoutElement.h"
#include "MuonReadoutGeometry/GlobalUtilities.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"

#ifndef SIMULATIONBASE
#include "MuonCondSvc/NSWCondUtils.h"
#endif

namespace MuonGM {

    MuonDetectorManager::MuonDetectorManager(): AthMessaging{"MGM::MuonDetectorManager"} { 
        setName("Muon");
        if (m_idHelperSvc.retrieve().isFailure()) {
            throw std::runtime_error("MuonDetectorManager() - No IdHelper svc is available");
        } 
        loadStationIndices();
    }

    MuonDetectorManager::~MuonDetectorManager() {
        for (unsigned int p = 0; p < m_envelope.size(); ++p) { m_envelope[p]->unref(); }
    }
    template <typename read_out, size_t N> void MuonDetectorManager::clearCache(std::array<std::unique_ptr<read_out>, N>& array) {
        for (std::unique_ptr<read_out>& ele : array) {
            if (ele) ele->clearCache();
        }
    }
    template <typename read_out, size_t N> void MuonDetectorManager::fillCache(std::array<std::unique_ptr<read_out>, N>& array) {
        for (std::unique_ptr<read_out>& ele : array) {
            if (ele) ele->fillCache();
        }
    }
    template <typename read_out, size_t N> void MuonDetectorManager::refreshCache(std::array<std::unique_ptr<read_out>, N>& array) {
        for (std::unique_ptr<read_out>& ele : array) {
            if (!ele) continue;
            ele->clearCache();
            ele->fillCache();
        }
    }
    void MuonDetectorManager::clearCache() {
        clearMdtCache();
        clearRpcCache();
        clearTgcCache();
        clearCscCache();
    }

    void MuonDetectorManager::refreshCache() {
        refreshMdtCache();
        refreshRpcCache();
        refreshTgcCache();
        refreshCscCache();
    }
    void MuonDetectorManager::refreshMdtCache() {
        // NEED to fill since FillCacheInitTime = 1 is the default now.
        refreshCache(m_mdtArray);
    }
    void MuonDetectorManager::refreshRpcCache() { refreshCache(m_rpcArray); }
    void MuonDetectorManager::refreshTgcCache() { refreshCache(m_tgcArray); }
    void MuonDetectorManager::refreshCscCache() {
        if (nCscRE()) refreshCache(m_cscArray);
    }
    void MuonDetectorManager::refreshMMCache() {
        if (nMMRE()) refreshCache(m_mmcArray);
    }
    void MuonDetectorManager::refreshsTgcCache() {
        if (nsTgcRE()) refreshCache(m_stgArray);
    }

    void MuonDetectorManager::clearMdtCache() { clearCache(m_mdtArray); }
    void MuonDetectorManager::clearRpcCache() { clearCache(m_rpcArray); }
    void MuonDetectorManager::clearTgcCache() { clearCache(m_tgcArray); }
    void MuonDetectorManager::clearCscCache() {
        if (nCscRE()) clearCache(m_cscArray);
    }
    void MuonDetectorManager::clearMMCache() {
        if (nMMRE()) clearCache(m_mmcArray);
    }
    void MuonDetectorManager::clearsTgcCache() {
        if (nsTgcRE()) clearCache(m_stgArray);
    }
    void MuonDetectorManager::fillMMCache() {
        if (nMMRE()) fillCache(m_mmcArray);
    }
    void MuonDetectorManager::fillsTgcCache() {
        if (nsTgcRE()) fillCache(m_stgArray);
    }
    void MuonDetectorManager::fillCache() {
       
        ATH_MSG_INFO( "Filling cache" );
        fillMdtCache();
        fillRpcCache();
        fillTgcCache();
        fillCscCache();
    }
    void MuonDetectorManager::fillMdtCache() { fillCache(m_mdtArray); }
    void MuonDetectorManager::fillRpcCache() { fillCache(m_rpcArray); }
    void MuonDetectorManager::fillTgcCache() { fillCache(m_tgcArray); }
    void MuonDetectorManager::fillCscCache() {
        if (nCscRE()) fillCache(m_cscArray);
    }

    unsigned int MuonDetectorManager::getNumTreeTops() const { return m_envelope.size(); }

    PVConstLink MuonDetectorManager::getTreeTop(unsigned int i) const { return m_envelope[i]; }

    void MuonDetectorManager::addTreeTop(PVLink pV) {
        pV->ref();
        m_envelope.push_back(pV);
    }

    void MuonDetectorManager::addMuonStation(MuonStation* mst) {
        std::string key = muonStationKey(mst->getStationType(), mst->getEtaIndex(), mst->getPhiIndex());
        m_MuonStationMap[key] = std::unique_ptr<MuonStation>(mst);
    }

    std::string MuonDetectorManager::muonStationKey(const std::string& stName, int statEtaIndex, int statPhiIndex) const {
        std::string key;
        if (statEtaIndex < 0)
            key = stName.substr(0, 3) + "_C_zi" + MuonGM::buildString(std::abs(statEtaIndex), 2) + "fi" +
                  MuonGM::buildString(statPhiIndex, 2);
        else
            key = stName.substr(0, 3) + "_A_zi" + MuonGM::buildString(std::abs(statEtaIndex), 2) + "fi" +
                  MuonGM::buildString(statPhiIndex, 2);
        return key;
    }

    const MuonStation* MuonDetectorManager::getMuonStation(const std::string& stName, int stEtaIndex, int stPhiIndex) const {
        std::string key = muonStationKey(stName, stEtaIndex, stPhiIndex);

        std::map<std::string, std::unique_ptr<MuonStation>>::const_iterator it = m_MuonStationMap.find(key);
        if (it != m_MuonStationMap.end())
            return (*it).second.get();
        else
            return nullptr;
    }

    MuonStation* MuonDetectorManager::getMuonStation(const std::string& stName, int stEtaIndex, int stPhiIndex) {
        std::string key = muonStationKey(stName, stEtaIndex, stPhiIndex);

        std::map<std::string, std::unique_ptr<MuonStation>>::const_iterator it = m_MuonStationMap.find(key);
        if (it != m_MuonStationMap.end())
            return (*it).second.get();
        else
            return nullptr;
    }

    void MuonDetectorManager::addRpcReadoutElement(RpcReadoutElement* x) {
        const Identifier id = x->identify();
        // add RE to map by RE hash
        const IdentifierHash Idhash = x->detectorElementHash();
        if (Idhash >= RpcRElMaxHash) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" with hash Id"
                                  <<Idhash<<" exceeding the allowed boundaries 0-"<<RpcRElMaxHash);
            throw std::runtime_error("Invalid hash assignment");
        } else {
            if (m_rpcArrayByHash[Idhash]) {
                ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been added before by"<<
                              m_idHelperSvc->toStringDetEl(m_rpcArrayByHash[Idhash]->identify()));
                throw std::runtime_error("Double assignment of the Hash");
            }
            m_rpcArrayByHash[Idhash] = x;
        }
        int dbz_index{-1};
        int idx = rpcIdentToArrayIdx(id, dbz_index);
        if (m_rpcArray[idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been already added.");
            throw std::runtime_error("Double readout element assignment");               
        }
        m_rpcArray[idx] = std::unique_ptr<RpcReadoutElement>(x);
        ++m_n_rpcRE;
    }

    const RpcReadoutElement* MuonDetectorManager::getRpcReadoutElement(const Identifier& id) const {
        int idx = rpcIdentToArrayIdx(id);
        return m_rpcArray[idx].get();
    }

    void MuonDetectorManager::addMMReadoutElement(MMReadoutElement* x) {
        const int array_idx = mmIdenToArrayIdx(x->identify());
        if (m_mmcArray[array_idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(x->identify())<<" which has been already added.");
            throw std::runtime_error("Double readout element assignment"); 
        }
        m_mmcArray[array_idx] = std::unique_ptr<MMReadoutElement>(x);
        ++m_n_mmcRE;
    }
    
    void MuonDetectorManager::addsTgcReadoutElement(sTgcReadoutElement* x) {       
        const int array_idx = stgcIdentToArrayIdx(x->identify());
        if (m_stgArray[array_idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(x->identify())<<" which has been already added.");
            throw std::runtime_error("Double readout element assignment"); 
        }
        m_stgArray[array_idx] = std::unique_ptr<sTgcReadoutElement>(x);
        ++m_n_stgRE;
    }

    void MuonDetectorManager::addMdtReadoutElement(MdtReadoutElement* x) {       
       const Identifier id = x->identify();
        // add here the MdtReadoutElement to the array by RE hash
        // use already known RE hash
        const IdentifierHash Idhash = x->detectorElementHash();
        if (Idhash >= MdtRElMaxHash) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" with hash Id"
                                  <<Idhash<<" exceeding the allowed boundaries 0-"<<MdtRElMaxHash);
            throw std::runtime_error("Invalid hash assignment");
        } else {
            if (m_mdtArrayByHash[Idhash]) {
                ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been added before by"<<
                              m_idHelperSvc->toStringDetEl(m_mdtArrayByHash[Idhash]->identify()));
                throw std::runtime_error("Double assignment of the Hash");
            }
            m_mdtArrayByHash[Idhash] = x;
        }       
        const int arrayIdx = mdtIdentToArrayIdx(id);
        if (m_mdtArray[arrayIdx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been already added.");
            throw std::runtime_error("Double readout element assignment");
        }
        m_mdtArray[arrayIdx] = std::unique_ptr<MdtReadoutElement>(x);
        ++m_n_mdtRE;
    }

    const MdtReadoutElement* MuonDetectorManager::getMdtReadoutElement(const Identifier& id) const {
        const int arrayIdx = mdtIdentToArrayIdx(id);
        return m_mdtArray[arrayIdx].get();
    }

     MdtReadoutElement* MuonDetectorManager::getMdtReadoutElement(const Identifier& id) {
        const int arrayIdx = mdtIdentToArrayIdx(id);
        return m_mdtArray[arrayIdx].get();
    }

    void MuonDetectorManager::addCscReadoutElement(CscReadoutElement* x) {
        const Identifier id = x->identify();
        // add here RE to array by hash
        const IdentifierHash Idhash = x->detectorElementHash();
        if (Idhash >= CscRElMaxHash) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" with hash Id"
                                  <<Idhash<<" exceeding the allowed boundaries 0-"<<CscRElMaxHash);
            throw std::runtime_error("Invalid hash assignment");
        } else {
            if (m_cscArrayByHash[Idhash]) {
                ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been added before by"<<
                              m_idHelperSvc->toStringDetEl(m_cscArrayByHash[Idhash]->identify()));
                throw std::runtime_error("Double assignment of the Hash");
            }
            m_cscArrayByHash[Idhash] = x;
        }

        const int array_idx = cscIdentToArrayIdx(id);
        if (m_cscArray[array_idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been already added.");
            throw std::runtime_error("Double readout element assignment");
        }
        m_cscArray[array_idx] = std::unique_ptr<CscReadoutElement>(x);
        ++m_n_cscRE;
    }

    const CscReadoutElement* MuonDetectorManager::getCscReadoutElement(const Identifier& id) const {
        const int array_idx = cscIdentToArrayIdx(id);
        return m_cscArray[array_idx].get();
    }

     CscReadoutElement* MuonDetectorManager::getCscReadoutElement(const Identifier& id) {
        const int array_idx = cscIdentToArrayIdx(id);
        return m_cscArray[array_idx].get();
    }
    
    void MuonDetectorManager::addTgcReadoutElement(TgcReadoutElement* x) {
        const Identifier id = x->identify();
        // add RE to array by RE hash
        const IdentifierHash Idhash = x->detectorElementHash();
        if (Idhash >= TgcRElMaxHash) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" with hash Id"
                                  <<Idhash<<" exceeding the allowed boundaries 0-"<<TgcRElMaxHash);
            throw std::runtime_error("Invalid hash assignment");
        }            
        
        if (m_tgcArrayByHash[Idhash]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been added before by"<<
                            m_idHelperSvc->toStringDetEl(m_tgcArrayByHash[Idhash]->identify()));
            throw std::runtime_error("Double assignment of the Hash");
        }
        m_tgcArrayByHash[Idhash] = x;

        const int array_idx = tgcIdentToArrayIdx(id);
        if (m_tgcArray[array_idx]) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" Trying to add ReadoutElement "<<m_idHelperSvc->toStringDetEl(id)<<" which has been already added.");
            throw std::runtime_error("Double readout element assignment");
        }

        m_tgcArray[array_idx] = std::unique_ptr<TgcReadoutElement>(x);
        ++m_n_tgcRE;
    }

    const TgcReadoutElement* MuonDetectorManager::getTgcReadoutElement(const Identifier& id) const {
        const int array_idx = tgcIdentToArrayIdx(id);
        return m_tgcArray[array_idx].get();
    }
    TgcReadoutElement* MuonDetectorManager::getTgcReadoutElement(const Identifier& id) {
        const int array_idx = tgcIdentToArrayIdx(id);
        return m_tgcArray[array_idx].get();
    }   
    const MMReadoutElement* MuonDetectorManager::getMMReadoutElement(const Identifier& id) const {
        const int array_idx = mmIdenToArrayIdx(id);
        return m_mmcArray[array_idx].get();
    }
    const sTgcReadoutElement* MuonDetectorManager::getsTgcReadoutElement(const Identifier& id) const {
        const int array_idx = stgcIdentToArrayIdx(id);
        return m_stgArray[array_idx].get();
    }   
    int MuonDetectorManager::mmIdenToArrayIdx(const Identifier& id) const {
        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
        return mmIdenToArrayIdx(idHelper.isSmall(id), idHelper.stationEta(id), idHelper.stationPhi(id),
                                idHelper.multilayer(id));
    }
    int MuonDetectorManager::mmIdenToArrayIdx(const int isSmall, const int stEta, const int stPhi, const int ml) const {
        const int steta_index = stEta + NMMcStEtaOffset - (stEta > 0);
        const int stphi_index = 2 * (stPhi - 1) + (isSmall == 1);
        const int ml_index = ml - 1;
#ifndef NDEBUG
        if (steta_index < 0 || steta_index >= NMMcStatEta) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" eta index is out of range "<<steta_index<<" allowed 0-"<<(NMMcStatEta-1));
            throw std::runtime_error("Out of range eta index");
        }
        if (stphi_index < 0 || stphi_index >= NMMcStatPhi) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" phi index is out of range "<<stphi_index<<" allowed 0-"<<(NMMcStatPhi-1));
            throw std::runtime_error("Out of range phi index");
        }
        if (ml_index < 0 || ml_index >= NMMcChamberLayer) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" ml index is out of range "<<ml_index<<" allowed 0-"<<(NMMcChamberLayer-1));
            throw std::runtime_error("Out of range multi layer index");           
        }
#endif
        constexpr int C = NMMcChamberLayer;
        constexpr int BxC = NMMcStatPhi * C;
        const int array_idx = steta_index * BxC + stphi_index * C + ml_index;
        return array_idx;
    }
    int MuonDetectorManager::stgcIdentToArrayIdx(const Identifier& id) const {
        const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
        return stgcIdentToArrayIdx(idHelper.isSmall(id), idHelper.stationEta(id), idHelper.stationPhi(id),
                                   idHelper.multilayer(id));
    }
    int MuonDetectorManager::stgcIdentToArrayIdx(const int isSmall, const int stEta, const int stPhi, const int ml) const {
        /// Next the array indeces
        const int steta_index = stEta + NsTgStEtaOffset - (stEta > 0);
        const int stphi_index = 2 * (stPhi - 1) + (isSmall == 1);
        const int ml_index = ml - 1;
#ifndef NDEBUG
        if (steta_index < 0 || steta_index >= NsTgStatEta) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" eta index is out of range "<<steta_index<<" allowed 0-"<<(NsTgStatEta-1));
            throw std::runtime_error("Out of range eta index");
        }
        if (stphi_index < 0 || stphi_index >= NsTgStatPhi) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" phi index is out of range "<<stphi_index<<" allowed 0-"<<(NsTgStatPhi-1));
            throw std::runtime_error("Out of range phi index");            
        }
        if (ml_index < 0 || ml_index >= NsTgChamberLayer) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" ml index is out of range "<<ml_index<<" allowed 0-"<<(NsTgChamberLayer-1));
            throw std::runtime_error("Out of range multi layer index"); 
        }
#endif
        constexpr int C = NsTgChamberLayer;
        constexpr int BxC = NsTgStatPhi * C;
        const int array_idx = steta_index * BxC + stphi_index * C + ml_index;
        return array_idx;
    }

    int MuonDetectorManager::rpcIdentToArrayIdx(const Identifier& id) const {
        int dbl_z{-1};
        return rpcIdentToArrayIdx(id, dbl_z);
    }
    int MuonDetectorManager::rpcIdentToArrayIdx(const Identifier& id, int& dbz_index) const {
        const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
        const int stationName = idHelper.stationName(id);
        const int stationEta = idHelper.stationEta(id);
        const int doubletPhi = idHelper.doubletPhi(id);
        const int doubletZ = idHelper.doubletZ(id);
        const int doubletR = idHelper.doubletR(id);
        const int stname_index = rpcStationTypeIdx(stationName);
        const int steta_index = stationEta + NRpcStEtaOffset;
        const int stphi_index = idHelper.stationPhi(id) - 1;
        const int dbr_index = doubletR - 1;
        dbz_index = doubletZ - 1;

        // BMS 5/ |stEta|= 2 / dbR = 1 and 2 / dbZ = 3
        // BMS 6/ |stEta|= 4 / dbR = 2 / dbZ = 3
        // BMS 6/ |stEta|= 4 / dbR = 1 / dbZ = 2
        // these are the special cases where we want the rpc at doubletPhi = 2
        // to be addressed with a dbz_index=dbZ+1
        if (stname_index == RpcStatType::BMS) {
            if (std::abs(stationEta) == 2 && doubletZ == 3 && doubletPhi == 2)
                ++dbz_index;
            else if (std::abs(stationEta) == 4 && doubletR == 2 && doubletZ == 3 && doubletPhi == 2)
                ++dbz_index;
            else if (std::abs(stationEta) == 4 && doubletR == 1 && doubletZ == 2 && doubletPhi == 2)
                ++dbz_index;
        }
#ifndef NDEBUG
        if (stname_index < 0 || stname_index >= NRpcStatType) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" station name index is out of range "<<stname_index<<" allowed 0-"<<(NRpcStatType-1));
            throw std::runtime_error("Out of range station index index");
        }
        if (steta_index < 0 || steta_index >= NRpcStatEta) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" eta index is out of range "<<steta_index<<" allowed 0-"<<(NRpcStatEta-1));
            throw std::runtime_error("Out of range eta index");
        }
        if (stphi_index < 0 || stphi_index >= NRpcStatPhi) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" phi index is out of range "<<stphi_index<<" allowed 0-"<<(NRpcStatPhi-1));
            throw std::runtime_error("Out of range phi index");            
       }
        if (dbr_index < 0 || dbr_index >= NDoubletR) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" doublet R index is out of range "<<dbr_index<<" allowed 0-"<<(NDoubletR-1));
            throw std::runtime_error("Out of doublet R index");
        }
        if (dbz_index < 0 || dbz_index >= NDoubletZ) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" doublet Z index is out of range "<<dbz_index<<" allowed 0-"<<(NDoubletZ-1));
            throw std::runtime_error("Out of doublet Z index");
       }
#endif
        /// Unfold the array by
        /// [A][B][C][D][E]
        /// a * BxCxDxE + b * CxDxE + c*DxE +d*E +e
        constexpr int E = NDoubletZ;
        constexpr int DxE = NDoubletR * E;
        constexpr int CxDxE = NRpcStatPhi * DxE;
        constexpr int BxCxDxE = NRpcStatEta * CxDxE;
        const int arrayIdx = stname_index * BxCxDxE + steta_index * CxDxE + stphi_index * DxE + dbr_index * E + dbz_index;
        return arrayIdx;
    }
    int MuonDetectorManager::mdtIdentToArrayIdx(const Identifier& id) const {
       const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
       return mdtIdentToArrayIdx(idHelper.stationName(id), idHelper.stationEta(id), 
                                 idHelper.stationPhi(id), idHelper.multilayer(id));
    }
    int MuonDetectorManager::mdtIdentToArrayIdx(const int stName, const int stEta, const int stPhi, const int ml) const {
        int stname_index = stName;
        if (stName == m_mdt_EIS_stName) {
            stname_index = NMdtStatType - 4;
        } else if (stName == m_mdt_BIM_stName) {
            stname_index = NMdtStatType - 3;
        } else if (stName == m_mdt_BME_stName) {
            stname_index = NMdtStatType - 2;
        } else if (stName == m_mdt_BMG_stName) {
            stname_index = NMdtStatType - 1;
        }
        int steta_index = stEta + NMdtStEtaOffset;
        int stphi_index = stPhi - 1;
        int ml_index = ml - 1;
#ifndef NDEBUG
        if (stname_index < 0 || stname_index >= NMdtStatType) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" station name index is out of range "<<stname_index<<" allowed 0-"<<(NMdtStatType-1));
            throw std::runtime_error("Out of range station index index"); 
        }
        if (steta_index < 0 || steta_index >= NMdtStatEta) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" eta index is out of range "<<steta_index<<" allowed 0-"<<(NMdtStatEta-1));
            throw std::runtime_error("Out of range eta index");
        }
        if (stphi_index < 0 || stphi_index >= NMdtStatPhi) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" phi index is out of range "<<stphi_index<<" allowed 0-"<<(NMdtStatPhi-1));
            throw std::runtime_error("Out of range phi index");
         }
        if (ml_index < 0 || ml_index >= NMdtMultilayer) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" multilayer index is out of range "<<ml_index<<" allowed 0-"<<(NMdtMultilayer-1));
            throw std::runtime_error("Out of range multilayer index");
        }
#endif
        /// Unfold the array by
        /// [A][B][C][D]
        /// a * BxCxD + b * CxD+ c*D +d
        constexpr int D = NMdtMultilayer;
        constexpr int CxD = NMdtStatPhi * D;
        constexpr int BxCxD = NMdtStatEta * CxD;
        const int arrayIdx = stname_index * BxCxD + steta_index * CxD + stphi_index * D + ml_index;
        return arrayIdx;
    }
    int MuonDetectorManager::tgcIdentToArrayIdx(const Identifier& id) const {
        const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
        return tgcIdentToArrayIdx(idHelper.stationName(id), idHelper.stationEta(id), idHelper.stationPhi(id));
    }
    int MuonDetectorManager::tgcIdentToArrayIdx(const int stationName, const int stationEta, const int stationPhi) const {
        const int stname_index = stationName + NTgcStatTypeOff;
        const int zi = stationEta;
        const int steta_index = zi + NTgcStEtaOffset - (zi > 0);
        const int stphi_index = stationPhi - 1;
#ifndef NDEBUG
        if (stname_index < 0 || stname_index >= NTgcStatType) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" station name index is out of range "<<stname_index<<" allowed 0-"<<(NTgcStatType-1));
            throw std::runtime_error("Out of range station index index");
        }
        if (steta_index < 0 || steta_index >= NTgcStatEta) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" eta index is out of range "<<steta_index<<" allowed 0-"<<(NTgcStatEta-1));
            throw std::runtime_error("Out of range eta index");
        }
        if (stphi_index < 0 || stphi_index >= NTgcStatPhi) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" phi index is out of range "<<stphi_index<<" allowed 0-"<<(NTgcStatPhi-1));
            throw std::runtime_error("Out of range phi index");
        }
#endif
        /// NTgcStatType * NTgcStatEta * NTgcStatPhi
        /// Unfold the array by
        /// [A][B][C]
        /// a * BxC + b * C + c
        constexpr int C = NTgcStatPhi;
        constexpr int BxC = NTgcStatEta * C;
        return stname_index * BxC + steta_index * C + stphi_index;
    }
    int MuonDetectorManager::cscIdentToArrayIdx(const Identifier& id) const {
        const CscIdHelper& idHelper{m_idHelperSvc->cscIdHelper()};
        return cscIdentToArrayIdx(idHelper.stationName(id), idHelper.stationEta(id), idHelper.stationPhi(id),
                                  idHelper.chamberLayer(id));
    }
    int MuonDetectorManager::cscIdentToArrayIdx(const int stName, const int stEta, const int stPhi, const int ml) const {
        const int stname_index = stName + NCscStatTypeOff;
        int steta_index = stEta + NCscStEtaOffset;
        if (steta_index == 2) steta_index = 1;
        const int stphi_index = stPhi - 1;
        const int ml_index = ml - 1;
#ifndef NDEBUG
        if (stname_index < 0 || stname_index >= NCscStatType) {
             ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" station name index is out of range "<<stname_index<<" allowed 0-"<<(NCscStatType-1));
            throw std::runtime_error("Out of range station index index");
        }
        if (steta_index < 0 || steta_index >= NCscStatEta) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" eta index is out of range "<<steta_index<<" allowed 0-"<<(NCscStatEta-1));
            throw std::runtime_error("Out of range eta index");
        }
        if (stphi_index < 0 || stphi_index >= NCscStatPhi) {
           ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" phi index is out of range "<<stphi_index<<" allowed 0-"<<(NCscStatPhi-1));
            throw std::runtime_error("Out of range phi index");
        }
        if (ml_index < 0 || ml_index >= NCscChamberLayer) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" multilayer index is out of range "<<ml_index<<" allowed 0-"<<(NMdtMultilayer-1));
            throw std::runtime_error("Out of range multilayer index");
        }
#endif
        constexpr int D = NCscChamberLayer;
        constexpr int CxD = NCscStatPhi * D;
        constexpr int BxCxD = NCscStatEta * CxD;
        const int array_idx = stname_index * BxCxD + steta_index * CxD + stphi_index * D + ml_index;
        return array_idx;
    }

    void MuonDetectorManager::initABlineContainers() {
        m_aLineContainer.clear();
        m_bLineContainer.clear();
        ATH_MSG_DEBUG("Init A/B Line Containers - pointers are <" << (uintptr_t)&m_aLineContainer << "> and <"
                << (uintptr_t)&m_bLineContainer << ">");

        // loop over stations to fill the A-line map at start-up
        for (auto& ist : m_MuonStationMap) {
            MuonStation* ms = ist.second.get();
            int jff = ms->getPhiIndex();
            int jzz = ms->getEtaIndex();
            std::string stType = ms->getStationType();

            ALinePar newALine;
            newALine.setAmdbId(stType, jff, jzz, 0);
            if (ms->hasALines()) {
                newALine.setParameters(ms->getALine_tras(), ms->getALine_traz(), ms->getALine_trat(), ms->getALine_rots(),
                                       ms->getALine_rotz(), ms->getALine_rott());
            } else {
                ATH_MSG_DEBUG("No starting A-lines for Station " << stType << " Jzz/Jff " << jzz << "/" << jff);
                newALine.setParameters(0., 0., 0., 0., 0., 0.);
            }
            newALine.isNew(true);

            Identifier id{0};
            if (m_idHelperSvc->hasTGC() && stType.at(0) == 'T') {
                // TGC case
                int stPhi = MuonGM::stationPhiTGC(stType, jff, jzz, geometryVersion());
                int stEta = 1;            // stEta for the station is stEta for the first component chamber
                if (jzz < 0) stEta = -1;  // stEta for the station is stEta for the first component chamber
                id = m_idHelperSvc->tgcIdHelper().elementID(stType, stEta, stPhi);
                ATH_MSG_DEBUG("Filling A-line container with entry for key = " << m_idHelperSvc->toString(id));
            } else if (m_idHelperSvc->hasCSC() && stType.at(0) == 'C') {
                // CSC case
                id = m_idHelperSvc->cscIdHelper().elementID(stType, jzz, jff);
                ATH_MSG_DEBUG( "Filling A-line container with entry for key = " << m_idHelperSvc->toString(id));
            } else if (m_idHelperSvc->hasRPC() && stType.substr(0, 3) == "BML" && std::abs(jzz) == 7) {
                // RPC case
                id = m_idHelperSvc->rpcIdHelper().elementID(stType, jzz, jff, 1);
                ATH_MSG_DEBUG("Filling A-line container with entry for key = " << m_idHelperSvc->toString(id));
            } else if (m_idHelperSvc->hasMDT()) {
                id = m_idHelperSvc->mdtIdHelper().elementID(stType, jzz, jff);
                ATH_MSG_DEBUG("Filling A-line container with entry for key = " << m_idHelperSvc->toString(id));
            }
            m_aLineContainer.emplace(id, std::move(newALine));
            ATH_MSG_DEBUG("<Filling A-line container with entry for key >" << m_idHelperSvc->toString(id));
        }

#ifndef SIMULATIONBASE
        if (!m_NSWABLineAsciiPath.empty()) {
	        ATH_MSG_DEBUG("Using NSW AB lines from file: " << m_NSWABLineAsciiPath);
            ALineMapContainer writeALines;
            BLineMapContainer writeBLines;
            MuonCalib::NSWCondUtils::setNSWABLinesFromAscii(m_NSWABLineAsciiPath, writeALines, writeBLines, stgcIdHelper(), mmIdHelper());
            for (auto it = writeALines.cbegin(); it != writeALines.cend(); ++it) {
                Identifier id = it->first;
                ALinePar aline = it->second;
                m_aLineContainer.emplace(id, std::move(aline));
            }

            for (auto it = writeBLines.cbegin(); it != writeBLines.cend(); ++it) {
                Identifier id = it->first;
                BLinePar bline = it->second;
                m_bLineContainer.emplace(id, std::move(bline));
            }
        }
#endif

        ATH_MSG_INFO("Init A/B Line Containers - done - size is respectively " << m_aLineContainer.size() << "/"
            << m_bLineContainer.size());
    }

    StatusCode MuonDetectorManager::updateAlignment(const ALineMapContainer& alineData, bool isData) {
#ifdef TESTBLINES
        {
            for (auto& it : m_MuonStationMap) {
                MuonStation* station = it.second.get();
                station->setDelta_fromAline(0., 0., 0., 0., 0.,
                                            0.);  // double tras, double traz, double trat, double rots, double rotz, double rott
                if (cacheFillingFlag()) {
                    station->clearCache();
                    station->fillCache();
                } else {
                    station->refreshCache();
                }
            }
        }
#endif
        if (alineData.empty()) {
            if (isData) {
                ATH_MSG_INFO("Empty temporary A-line container - nothing to do here");
            } else {
               ATH_MSG_DEBUG("Got empty A-line container (expected for MC), not applying A-lines...");
            }
            return StatusCode::SUCCESS;
        } else
            ATH_MSG_INFO("temporary A-line container with size = " << alineData.size());

        // loop over the container of the updates passed by the MuonAlignmentDbTool
        unsigned int nLines = 0;
        unsigned int nUpdates = 0;
        for (const auto& [ALineId, ALine] : alineData) {
            nLines++;
            std::string stType{""};
            int jff{0}, jzz{0}, job{0};
            ALine.getAmdbId(stType, jff, jzz, job);

            if (!ALine.isNew()) {
                ATH_MSG_WARNING("ALinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job << " is not new *** skipping" );
                continue;
            }            
            
           
            ATH_MSG_DEBUG("ALinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job << " is new. ID = " << m_idHelperSvc->toString(ALineId) );

            //********************
            // NSW Cases
            //********************
            
            if (stType[0] == 'M' || stType[0] == 'S') {
            
                if (!nMMRE() || !nsTgcRE()) {
                    ATH_MSG_WARNING("Unable to set A-line; the manager does not contain NSW readout elements" );
                    continue;
                }
                            
                if (!m_NSWABLineAsciiPath.empty()) {
                    ATH_MSG_INFO( "NSW A-lines are already set via external ascii file " << m_NSWABLineAsciiPath );
                    continue;
                }

                if (stType[0] == 'M') {
                    // Micromegas                        
                    const int array_idx  = mmIdenToArrayIdx(ALineId);
                    MMReadoutElement* RE = m_mmcArray[array_idx].get();

                    if (!RE) {
                        ATH_MSG_WARNING("AlinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job << " *** No MM readout element found\n"
                            << "PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and Geometry Layout in use");
                        return StatusCode::FAILURE;
                    }
                
                    RE->setDelta(ALine);

                } else if (stType[0] == 'S') {
                    // sTGC
                    const int array_idx    = stgcIdentToArrayIdx(ALineId);
                    sTgcReadoutElement* RE = m_stgArray[array_idx].get();

                    if (!RE) {
                        ATH_MSG_WARNING("AlinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job << " *** No sTGC readout element found\n"
                            << "PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and Geometry Layout in use");
                        return StatusCode::FAILURE;
                    }
                
                    RE->setDelta(ALine);
                }

                // record this A-line in the historical A-line container
                auto [it, flag] = m_aLineContainer.insert_or_assign(ALineId, ALine);
                if (flag)
                    ATH_MSG_DEBUG( "New A-line entry for Station " << stType << " at Jzz/Jff/Job " << jzz << "/" << jff << "/" << job );
                else 
                    ATH_MSG_DEBUG( "Updating existing A-line for Station " << stType << " at Jzz/Jff/Job " << jzz << "/" << jff << "/" << job );
                
                
                continue;
            }
             

            //********************
            // Non-NSW Cases
            //********************

            MuonStation* thisStation = getMuonStation(stType, jzz, jff);
            if (!thisStation) {
                ATH_MSG_WARNING("ALinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job << "*** No MuonStation found\n"
                    << "PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and Geometry Layout in use"
                    );
                continue;
            }

            if (job != 0) {
                // job different than 0 (standard for TGC conditions for Sept 2010 repro.)
                if (stType.at(0) == 'T') {                    
                    ATH_MSG_DEBUG( "ALinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job
                        << " has JOB not 0 - this is expected for TGC" );
                } else {
                    ATH_MSG_WARNING("ALinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job
                        << " has JOB not 0 - this is NOT EXPECTED yet for non TGC chambers - skipping this A-line" );
                    continue;
                }
            }

            // record this A-line in the historical A-line container
            auto [it, flag] = m_aLineContainer.insert_or_assign(ALineId, ALine);
            ALinePar& newALine = it->second;
            if (flag) {                
                ATH_MSG_DEBUG( "               New entry in A-line container for Station " << stType << " at Jzz/Jff " << jzz
                    << "/" << jff << " --- in the container with key " << m_idHelperSvc->toString(ALineId) );
            } else {                
                ATH_MSG_DEBUG( "Updating extisting entry in A-line container for Station " << stType << " at Jzz/Jff " << jzz
                    << "/" << jff );
            }

            if (job == 0) {
                float s, z, t, ths, thz, tht;
                newALine.getParameters(s, z, t, ths, thz, tht);
                if (m_controlAlines % 10 == 0) tht = 0.;
                if (int(m_controlAlines / 10) % 10 == 0) thz = 0.;
                if (int(m_controlAlines / 100) % 10 == 0) ths = 0.;
                if (int(m_controlAlines / 1000) % 10 == 0) t = 0.;
                if (int(m_controlAlines / 10000) % 10 == 0) z = 0.;
                if (int(m_controlAlines / 100000) % 10 == 0) s = 0.;
                if (m_controlAlines != 111111) newALine.setParameters(s, z, t, ths, thz, tht);
                
                ATH_MSG_DEBUG( "Setting delta transform for Station " << stType << " " << jzz << " " << jff << " "
                    << " params are = " << s << " " << z << " " << t << " " << ths << " " << thz << " " << tht );
                thisStation->setDelta_fromAline(s, z, t, ths, thz, tht);
#ifdef TESTBLINES
                newALine.setParameters(0., 0., 0., 0., 0., 0.);
                thisStation->setDelta_fromAline(0., 0., 0., 0., 0., 0.);
#endif
                if (cacheFillingFlag()) {
                    thisStation->clearCache();
                    thisStation->fillCache();
                } else {
                    thisStation->refreshCache();
                }
            } else {
                // job different than 0 (standard for TGC conditions for Sept 2010 repro.)
                float s, z, t, ths, thz, tht;
                newALine.getParameters(s, z, t, ths, thz, tht);
                if (m_controlAlines % 10 == 0) tht = 0.;
                if (int(m_controlAlines / 10) % 10 == 0) thz = 0.;
                if (int(m_controlAlines / 100) % 10 == 0) ths = 0.;
                if (int(m_controlAlines / 1000) % 10 == 0) t = 0.;
                if (int(m_controlAlines / 10000) % 10 == 0) z = 0.;
                if (int(m_controlAlines / 100000) % 10 == 0) s = 0.;
                if (m_controlAlines != 111111) newALine.setParameters(s, z, t, ths, thz, tht);
                
                ATH_MSG_DEBUG( "Setting delta transform for component " << job << " of Station " << stType << " " << jzz << " "
                    << jff << " "
                    << " params are = " << s << " " << z << " " << t << " " << ths << " " << thz << " " << tht );
                thisStation->setDelta_fromAline_forComp(job, s, z, t, ths, thz, tht);
                if (cacheFillingFlag()) {
                    thisStation->getMuonReadoutElement(job)->clearCache();
                    thisStation->getMuonReadoutElement(job)->fillCache();
                } else {
                    thisStation->getMuonReadoutElement(job)->refreshCache();
                }
            }
            nUpdates++;
        }
        ATH_MSG_INFO( "# of A-lines read from the ALineMapContainer in StoreGate is " << nLines );
        ATH_MSG_INFO( "# of deltaTransforms updated according to A-lines         is " << nUpdates );
        ATH_MSG_INFO( "# of entries in the A-lines historical container          is " << ALineContainer()->size() );

        return StatusCode::SUCCESS;
    }

    StatusCode MuonDetectorManager::updateDeformations(const BLineMapContainer& blineData, bool isData) {
#ifdef TESTBLINES
        {
            for (auto& it : m_MuonStationMap) {
                MuonStation* station = it.second.get();
                station->clearBLineCache();
                BLinePar* BLine = new BLinePar();
                BLine->setParameters(0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.);
                station->setBline(BLine);
                if (cacheFillingFlag()) station->fillBLineCache();
            }
        }
#endif
       
        ATH_MSG_INFO( "In updateDeformations()" );
        if (!applyMdtDeformations()) ATH_MSG_INFO( "Mdt deformations are disabled; will only apply NSW deformations" );
        
        if (blineData.empty()) {
            if (isData) {
                ATH_MSG_INFO( "Empty temporary B-line container - nothing to do here" );
            } else {
                ATH_MSG_DEBUG( "Got empty B-line container (expected for MC), not applying B-lines..." );
            }
            return StatusCode::SUCCESS;
        } else
            ATH_MSG_INFO( "temporary B-line container with size = " << blineData.size() );

        // loop over the container of the updates passed by the MuonAlignmentDbTool
        unsigned int nLines = 0;
        unsigned int nUpdates = 0;
        for (auto [BLineId, BLine] : blineData) {
            nLines++;
            std::string stType{""};
            int jff{0}, jzz{0}, job{0};
            BLine.getAmdbId(stType, jff, jzz, job);

            //********************
            // NSW Cases
            //********************
            
            if (stType[0] == 'M' || stType[0] == 'S') {
            
                if (!nMMRE() || !nsTgcRE()) {
                    ATH_MSG_WARNING("Unable to set B-line; the manager does not contain NSW readout elements" );
                    continue;
                }
                            
                if (!m_NSWABLineAsciiPath.empty()) {
                    ATH_MSG_INFO( "NSW B-lines are already set via external ascii file " << m_NSWABLineAsciiPath );
                    continue;
                }
                 // record this B-line in the historical B-line container
                auto [it, flag] = m_bLineContainer.insert_or_assign(BLineId, BLine);
                 {
                    if (flag)
                        ATH_MSG_DEBUG( "New B-line entry for Station " << stType << " at Jzz/Jff/Job " << jzz << "/" << jff << "/" << job );
                    else 
                        ATH_MSG_DEBUG( "Updating existing B-line for Station " << stType << " at Jzz/Jff/Job " << jzz << "/" << jff << "/" << job );
                }

                if (stType[0] == 'M') {
                    // Micromegas                        
                    const int array_idx  = mmIdenToArrayIdx(BLineId);
                    MMReadoutElement* RE = m_mmcArray[array_idx].get();

                    if (!RE) {
                        ATH_MSG_WARNING("BlinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job << " *** No MM readout element found\n"
                            << "PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and Geometry Layout in use");
                        return StatusCode::FAILURE;
                    }
                    RE->setBLinePar(it->second);
                } else if (stType[0] == 'S') {
                    // sTGC
                    const int array_idx    = stgcIdentToArrayIdx(BLineId);
                    sTgcReadoutElement* RE = m_stgArray[array_idx].get();

                    if (!RE) {
                        ATH_MSG_WARNING("BlinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job << " *** No sTGC readout element found\n"
                            << "PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and Geometry Layout in use");
                        return StatusCode::FAILURE;
                    }
                    RE->setBLinePar(it->second);
                }
                continue;
            }
            
            //********************
            // MDT Cases
            //********************    

            if (!applyMdtDeformations()) continue; // nothing to more to do
        
#ifdef TESTBLINES
            BLine.setParameters(0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.);
#endif
            if (mdtDeformationFlag() > 999999) {
                // first reset everything
                BLine.setParameters(0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.);
                // now apply user choice
                int choice = mdtDeformationFlag();
                if (int(choice % 10) > 0)
                    BLine.setParameters(0., 0., 0., BLine.sp(), BLine.sn(), BLine.tw(), 0., 0., BLine.eg(), BLine.ep(), 100.);
                if (int(choice % 100) > 9)
                    BLine.setParameters(0., 0., 0., BLine.sp(), BLine.sn(), BLine.tw(), 0., 0., BLine.eg(), 100., BLine.en());
                if (int(choice % 1000) > 99)
                    BLine.setParameters(0., 0., 0., BLine.sp(), BLine.sn(), BLine.tw(), 0., 0., 100., BLine.ep(), BLine.en());
                if (int(choice % 10000) > 999)
                    BLine.setParameters(0., 0., 0., BLine.sp(), BLine.sn(), 100., 0., 0., BLine.eg(), BLine.ep(), BLine.en());
                if (int(choice % 100000) > 9999)
                    BLine.setParameters(0., 0., 0., BLine.sp(), 100., BLine.tw(), 0., 0., BLine.eg(), BLine.ep(), BLine.en());
                if (int(choice % 1000000) > 99999)
                    BLine.setParameters(0., 0., 0., 100., BLine.sn(), BLine.tw(), 0., 0., BLine.eg(), BLine.ep(), BLine.en());
                
                ATH_MSG_DEBUG( "Testing B-lines: control flag " << choice << " hard coding Bline = ( bz=" << BLine.bz()
                    << " bp=" << BLine.bp() << " bn=" << BLine.bn() << " sp=" << BLine.sp() << " sn=" << BLine.sn()
                    << " tw=" << BLine.tw() << " pg=" << BLine.pg() << " tr=" << BLine.tr() << " eg=" << BLine.eg()
                    << " ep=" << BLine.ep() << " en=" << BLine.en() << ")" );
            }

            if (stType.at(0) == 'T' || stType.at(0) == 'C' || (stType.substr(0, 3) == "BML" && std::abs(jzz) == 7)) {
                
                ATH_MSG_DEBUG( "BLinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job
                    << " is not a MDT station - skipping" );
                continue;
            }
            if (mdtDeformationFlag() == 2 &&
                (stType.substr(0, 3) == "BEE" || stType.at(0) == 'E'))  // MDT deformations are requested for Barrel(ASAP) only !!!!
            {
                
                ATH_MSG_DEBUG( " mdtDeformationFlag()==" << mdtDeformationFlag() << " stName = " << stType.substr(0, 3)
                    << " barrel / ec initial = " << stType.substr(0, 1) << " 	 skipping this b-line" );
                continue;  // MDT deformations are requested for Barrel(ASAP) only !!!!
            }
            if (mdtDeformationFlag() == 3 && (stType.substr(0, 3) != "BEE" && stType.at(0) == 'B')) {
                
                ATH_MSG_DEBUG( " mdtDeformationFlag()==" << mdtDeformationFlag() << " stName = " << stType.substr(0, 3)
                    << " barrel / ec initial = " << stType.substr(0, 1) << " 	 skipping this b-line" );
                continue;  // MDT deformations are requested for Endcap(ARAMYS) only !!!!
            }
            if (mdtDeformationFlag() == 0) {
                 ATH_MSG_DEBUG( " mdtDeformationFlag()==0 skipping this b-line" );
                continue;  // should never happen...
            }
            if (!BLine.isNew()) {
                ATH_MSG_WARNING("BLinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job
                    << " is not new *** skipping" );
                continue;
            }
            
            ATH_MSG_DEBUG( "BLinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job
                << " is new ID = " << m_idHelperSvc->toString(BLineId) );
            if (job == 0) {
                MuonStation* thisStation = getMuonStation(stType, jzz, jff);
                if (!thisStation) {
                    ATH_MSG_WARNING("BLinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job
                        << " *** No MuonStation found \n PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and "
                           "Geometry Layout in use");
                    continue;
                }

                // record this B-line in the historical B-line container
                auto [it, flag] = m_bLineContainer.insert_or_assign(BLineId, BLine);
                if (flag) {                    
                    ATH_MSG_DEBUG( "               New entry in B-line container for Station " << stType << " at Jzz/Jff " << jzz
                        << "/" << jff << " --- in the container with key " << m_idHelperSvc->toString(BLineId) );
                } else {                    
                    ATH_MSG_DEBUG( "Updating existing entry in B-line container for Station " << stType << " at Jzz/Jff " << jzz
                        << "/" << jff );
                }

                
                ATH_MSG_DEBUG( "Setting deformation parameters for Station " << stType << " " << jzz << " " << jff << " ");
                thisStation->clearBLineCache();
                thisStation->setBline(&it->second);
                if (cacheFillingFlag()) thisStation->fillBLineCache();
                nUpdates++;
            } else {
                ATH_MSG_WARNING("BLinePar with AmdbId " << stType << " " << jzz << " " << jff << " " << job << " has JOB not 0 ");
                return StatusCode::FAILURE;
            }
        }
        ATH_MSG_INFO( "# of B-lines read from the ALineMapContainer in StoreGate   is " << nLines );
        ATH_MSG_INFO( "# of deform-Transforms updated according to B-lines         is " << nUpdates );
        ATH_MSG_INFO( "# of entries in the B-lines historical container            is " << BLineContainer()->size() );

        return StatusCode::SUCCESS;
    }

    void MuonDetectorManager::storeTgcReadoutParams(std::unique_ptr<const TgcReadoutParams> x) {
        m_TgcReadoutParamsVec.push_back(std::move(x));
    }

    StatusCode MuonDetectorManager::initCSCInternalAlignmentMap() {
       

        if (!m_useCscIlinesFromGM) {
            ATH_MSG_INFO( "Init of CSC I-Lines will be done via Conditions DB" );
            m_cscALineContainer.clear();
            const CscIdHelper& idHelper{m_idHelperSvc->cscIdHelper()};

            for (auto& ist : m_MuonStationMap) {
                MuonStation* ms = ist.second.get();
                std::string stType = ms->getStationType();
                if (stType[0] != 'C') continue;

                int jff{ms->getPhiIndex()}, jzz{ms->getEtaIndex()}, job{3};  // it's always like this for CSCs

                for (unsigned int wlay = 1; wlay < 5; ++wlay) {
                    CscInternalAlignmentPar newILine;
                    newILine.setAmdbId(stType, jff, jzz, job, wlay);
                    
                    ATH_MSG_DEBUG( "No starting I-Lines or reseting them for Station " << stType << " Jzz/Jff/Wlay " << jzz << "/"
                        << jff << "/" << wlay );
                    // there is no way to check if the RE already has parameters set - always overwriting them.
                    newILine.setParameters(0., 0., 0., 0., 0., 0.);
                    newILine.isNew(true);
                    Identifier idp = idHelper.parentID(ms->getMuonReadoutElement(job)->identify());
                    Identifier id = idHelper.channelID(idp, 2, wlay, 0, 1);
                    
                    ATH_MSG_DEBUG( "<Filling I-Line container with entry for key >" << m_idHelperSvc->toString(id));
                    m_cscALineContainer.emplace(id, newILine);
                }
            }
            ATH_MSG_INFO( "Init I-Line Container - done - size is respectively " << m_cscALineContainer.size() );
        }
        
        ATH_MSG_DEBUG( "Init CSC I-Line Containers - pointer is <" << (uintptr_t)&m_cscALineContainer << ">" );

        ATH_MSG_INFO( "I-Line for CSC wire layers loaded (Csc Internal Alignment)" );
        if (m_useCscIntAlign)
            ATH_MSG_INFO( "According to configuration they WILL be used " );
        else
            ATH_MSG_INFO( "According to configuration parameters they WILL BE UPDATED FROM CONDDB " );
        return StatusCode::SUCCESS;
    }
    StatusCode MuonDetectorManager::updateCSCInternalAlignmentMap(const CscInternalAlignmentMapContainer& ilineData) {
       
        if (ilineData.empty()) {
            ATH_MSG_WARNING("Empty temporary CSC I-line container - nothing to do here" );
            return StatusCode::SUCCESS;
        } else
            ATH_MSG_INFO( "temporary CSC I-line container with size = " << ilineData.size() );

        // loop over the container of the updates passed by the MuonAlignmentDbTool
        unsigned int nLines{0}, nUpdates{0};
        for (const auto& [ILineId, ILine] : ilineData) {
            nLines++;
            std::string stType = "";
            int jff{0}, jzz{0}, job{0}, jlay{0};
            ILine.getAmdbId(stType, jff, jzz, job, jlay);
            if (!ILine.isNew()) {
                ATH_MSG_WARNING("CscInternalAlignmentPar with AmdbId " << stType << " " << jzz << " " << jff << " " << job << " "
                    << jlay << " is not new *** skipping" );
                continue;
            }
            
            ATH_MSG_DEBUG( "CscInternalAlignmentPar with AmdbId " << stType << " " << jzz << " " << jff << " " << job << " "
                << jlay << " is new ID = " << m_idHelperSvc->toString(ILineId) );
            if (job == 3) {
                MuonStation* thisStation = getMuonStation(stType, jzz, jff);
                if (!thisStation) {
                    ATH_MSG_WARNING("CscInternalAlignmentPar with AmdbId " << stType << " " << jzz << " " << jff << " " << job << " "
                        << jlay
                        << " *** No MuonStation found \n PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and "
                           "Geometry Layout in use");
                    continue;
                }

                auto [it, flag] = m_cscALineContainer.insert_or_assign(ILineId, ILine);
                if (flag) {                    
                    ATH_MSG_DEBUG( "               New entry in CSC I-line container for Station " << stType
                        << " at Jzz/Jff/Jlay " << jzz << "/" << jff << "/" << jlay << " --- in the container with key "
                        << m_idHelperSvc->toString(ILineId) );
                } else {                    
                    ATH_MSG_DEBUG( "Updating extisting entry in CSC I-line container for Station " << stType
                        << " at Jzz/Jff/Jlay " << jzz << "/" << jff << "/" << jlay );
                }

                CscInternalAlignmentPar& newILine = it->second;
                float tras{0.f}, traz{0.f}, trat{0.f}, rots{0.f}, rotz{0.f}, rott{0.f};
                newILine.getParameters(tras, traz, trat, rots, rotz, rott);
                int choice = CscIlinesFlag();
                if (choice % 10 == 0) tras = 0.;
                if (int(choice / 10) % 10 == 0) rotz = 0.;
                if (int(choice / 100) % 10 == 0) rots = 0.;
                if (int(choice / 1000) % 10 == 0) trat = 0.;
                if (int(choice / 10000) % 10 == 0) traz = 0.;
                if (int(choice / 100000) % 10 == 0) traz = 0.;
                if (m_controlCscIlines != 111111) newILine.setParameters(tras, traz, trat, rots, rotz, rott);
                
                ATH_MSG_DEBUG( "Setting CSC I-Lines for Station " << stType << " " << jzz << " " << jff << " " << job << " "
                    << jlay << " "
                    << " params are = " << tras << " " << traz << " " << trat << " " << rots << " " << rotz << " " << rott );
                CscReadoutElement* CscRE = dynamic_cast<CscReadoutElement*>(thisStation->getMuonReadoutElement(job));
                if (!CscRE)
                    ATH_MSG_ERROR( "The CSC I-lines container includes stations which are no CSCs! This is impossible." );
                else {
                    CscRE->setCscInternalAlignmentPar(newILine);
                }
                if (cacheFillingFlag()) {
                    thisStation->clearCache();
                    thisStation->fillCache();
                } else {
                    thisStation->refreshCache();
                }
                nUpdates++;

            } else {
                ATH_MSG_ERROR( "job for CSC I-Lines= " << job << " is not 3 => This is not valid." );
            }
        }
        ATH_MSG_INFO( "# of CSC I-lines read from the ILineMapContainer in StoreGate is " << nLines );
        ATH_MSG_INFO( "# of deltaTransforms updated according to A-lines             is " << nUpdates );
        ATH_MSG_INFO( "# of entries in the CSC I-lines historical container          is " << CscInternalAlignmentContainer()->size());

        return StatusCode::SUCCESS;
    }
    StatusCode MuonDetectorManager::updateMdtAsBuiltParams(const MdtAsBuiltMapContainer& asbuiltData) {
       
        if (asbuiltData.empty()) {
            ATH_MSG_WARNING("Empty temporary As-Built container - nothing to do here" );
            return StatusCode::SUCCESS;
        } else
            ATH_MSG_INFO( "temporary As-Built container with size = " << asbuiltData.size() );

        // loop over the container of the updates passed by the MuonAlignmentDbTool
        unsigned int nLines{0}, nUpdates{0};
        for (const auto& [AsBuiltId, AsBuiltPar] : asbuiltData) {
            nLines++;
            std::string stType = "";
            int jff{0}, jzz{0}, job{0};
            AsBuiltPar.getAmdbId(stType, jff, jzz, job);
            if (!AsBuiltPar.isNew()) {                
                ATH_MSG_DEBUG( "MdtAsBuiltPar with AmdbId " << stType << " " << jzz << " " << jff << " " << job
                    << " is not new *** skipping" );
                continue;
            }

            auto [it, flag] = m_AsBuiltParamsMap.insert_or_assign(AsBuiltId, AsBuiltPar);
            if (flag) {                
                ATH_MSG_DEBUG( "New entry in AsBuilt container for Station " << stType << " at Jzz/Jff " << jzz << "/" << jff
                    << " --- in the container with key " << m_idHelperSvc->toString(AsBuiltId) );
            } else {                
                ATH_MSG_DEBUG( "Updating extisting entry in AsBuilt container for Station " << stType << " at Jzz/Jff " << jzz
                    << "/" << jff );
            }

            
            ATH_MSG_DEBUG( "MdtAsBuiltPar with AmdbId " << stType << " " << jzz << " " << jff << " " << job
                    << " is new ID = " << m_idHelperSvc->toString(AsBuiltId) );

            MuonStation* thisStation = getMuonStation(stType, jzz, jff);
            if (thisStation) {
                
                ATH_MSG_DEBUG( "Setting as-built parameters for Station " << stType << " " << jzz << " " << jff << " " );
                thisStation->clearBLineCache();
                thisStation->setMdtAsBuiltParams(&it->second);
                if (cacheFillingFlag()) thisStation->fillBLineCache();
                nUpdates++;
            } else {
                ATH_MSG_WARNING("MdtAsBuiltPar with AmdbId " << stType << " " << jzz << " " << jff << " " << job
                    << " *** No MuonStation found \n PLEASE CHECK FOR possible MISMATCHES between alignment constants from COOL and "
                       "Geometry Layout in use");
                continue;
            }
        }
        ATH_MSG_INFO( "# of MDT As-Built read from the MdtAsBuiltMapContainer in StoreGate is " << nLines );
        ATH_MSG_INFO( "# of deltaTransforms updated according to As-Built                  is " << nUpdates );
        ATH_MSG_INFO( "# of entries in the MdtAsBuilt historical container                 is " << MdtAsBuiltContainer()->size());

        return StatusCode::SUCCESS;
    }
    void MuonDetectorManager::storeCscInternalAlignmentParams(const CscInternalAlignmentPar& x) {
       

        std::string stName = "XXX";
        int jff{0}, jzz{0}, job{0}, wlayer{0};
        x.getAmdbId(stName, jff, jzz, job, wlayer);
        // chamberLayer is always 2 => job is always 3
        int chamberLayer = 2;
        if (job != 3)
            ATH_MSG_WARNING("job = " << job << " is not 3 => chamberLayer should be 1 - not existing ! setting 2" );
        Identifier id = m_idHelperSvc->cscIdHelper().channelID(stName, jzz, jff, chamberLayer, wlayer, 0, 1);

        m_cscALineContainer.emplace(id, x);
        ATH_MSG_DEBUG( "Adding Aline for CSC wire layer: " << m_idHelperSvc->toString(id) );
        ATH_MSG_DEBUG( "CscInternalAlignmentMapContainer has currently size " << m_cscALineContainer.size() );
        
    }

    void MuonDetectorManager::storeMdtAsBuiltParams(const MdtAsBuiltPar& params) {
       

        std::string stName = "XXX";
        int jff{0}, jzz{0}, job{0};
        params.getAmdbId(stName, jff, jzz, job);
        Identifier id = m_idHelperSvc->mdtIdHelper().elementID(stName, jzz, jff);
        if (!id.is_valid()) {
            ATH_MSG_ERROR( "Invalid MDT identifiers: sta=" << stName << " eta=" << jzz << " phi=" << jff );
            return;
        }

        if (m_AsBuiltParamsMap.insert_or_assign(id, params).second) {            
            ATH_MSG_DEBUG( "New entry in AsBuilt container for Station " << stName << " at Jzz/Jff " << jzz << "/" << jff
                << " --- in the container with key " << m_idHelperSvc->toString(id) );
        } else {            
            ATH_MSG_DEBUG( "Updating extisting entry in AsBuilt container for Station " << stName << " at Jzz/Jff " << jzz << "/"
                << jff );
        }

        return;
    }

    const MdtAsBuiltPar* MuonDetectorManager::getMdtAsBuiltParams(const Identifier& id) const {
        if (!MdtAsBuiltContainer()) {           
            ATH_MSG_DEBUG( "No Mdt AsBuilt parameter container available" );
            return nullptr;
        }
        ciMdtAsBuiltMap iter = m_AsBuiltParamsMap.find(id);
        if (iter == m_AsBuiltParamsMap.end()) {           
            ATH_MSG_DEBUG( "No Mdt AsBuilt parameters for station " << m_idHelperSvc->toString(id) );
            return nullptr;
        }
        return &iter->second;
    }

    void MuonDetectorManager::setMMAsBuiltCalculator(const NswAsBuiltDbData* nswAsBuiltData) {
        if (m_NSWAsBuiltAsciiOverrideMM) return; // test-mode using AsBuilt conditions from an ascii file
#ifndef SIMULATIONBASE
        m_MMAsBuiltCalculator.reset();  // unset any previous instance
        m_MMAsBuiltCalculator = std::make_unique<NswAsBuilt::StripCalculator>();
        std::string mmJson="";
        if(!nswAsBuiltData->getMmData(mmJson)){          
           ATH_MSG_WARNING(" Cannot retrieve MM as-built conditions data from detector store!" );
        }
        m_MMAsBuiltCalculator->parseJSON(mmJson);
#else
        // just to silence the warning about an unused parameter
        (void)nswAsBuiltData;
#endif
    }

    void MuonDetectorManager::setStgcAsBuiltCalculator(const NswAsBuiltDbData* nswAsBuiltData) {
        if (m_NSWAsBuiltAsciiOverrideSTgc) return; // test-mode using AsBuilt conditions from an ascii file
#ifndef SIMULATIONBASE
        m_StgcAsBuiltCalculator.reset();  // unset any previous instance
        m_StgcAsBuiltCalculator = std::make_unique<NswAsBuilt::StgcStripCalculator>();
        std::string stgcJson="";
        if(!nswAsBuiltData->getSTgcData(stgcJson)){          
           ATH_MSG_WARNING(" Cannot retrieve sTGC as-built conditions data from detector store!" );
        }
        m_StgcAsBuiltCalculator->parseJSON(stgcJson);
#else
        // just to silence the warning about an unused parameter
        (void)nswAsBuiltData;
#endif
    }

    const MdtReadoutElement* MuonDetectorManager::getMdtReadoutElement(const IdentifierHash& id) const {
#ifndef NDEBUG
        if (id >= MdtRElMaxHash) {           
            ATH_MSG_WARNING(" try to getMdtReadoutElement with hashId " << (unsigned int)id << " outside range 0-"
                << MdtRElMaxHash - 1 );
            return nullptr;
        }
#endif
        return m_mdtArrayByHash[id];
    }

    const RpcReadoutElement* MuonDetectorManager::getRpcReadoutElement(const IdentifierHash& id) const {
#ifndef NDEBUG
        if (id >= RpcRElMaxHash) {           
            ATH_MSG_WARNING(" try to getRpcReadoutElement with hashId " << (unsigned int)id << " outside range 0-"
                << RpcRElMaxHash - 1 );
            return nullptr;
        }
#endif
        return m_rpcArrayByHash[id];
    }

    const TgcReadoutElement* MuonDetectorManager::getTgcReadoutElement(const IdentifierHash& id) const {
#ifndef NDEBUG
        if (id >= TgcRElMaxHash) {           
            ATH_MSG_WARNING(" try to getTgcReadoutElement with hashId " << (unsigned int)id << " outside range 0-"
                << TgcRElMaxHash - 1 );
            return nullptr;
        }
#endif
        return m_tgcArrayByHash[id];
    }

    const CscReadoutElement* MuonDetectorManager::getCscReadoutElement(const IdentifierHash& id) const {
#ifndef NDEBUG
        if (id >= CscRElMaxHash) {           
            ATH_MSG_WARNING(" try to getCscReadoutElement with hashId " << (unsigned int)id << " outside range 0-"
                << CscRElMaxHash - 1 );
            return nullptr;
        }
#endif
        return m_cscArrayByHash[id];
    }

    unsigned int MuonDetectorManager::rpcStationTypeIdx(const int stationName) const {
        std::map<int, int>::const_iterator itr = m_rpcStatToIdx.find(stationName);
        if (itr != m_rpcStatToIdx.end()) return itr->second;
        return RpcStatType::UNKNOWN;
    }

    int MuonDetectorManager::rpcStationName(const int stationIndex) const {
        std::map<int, int>::const_iterator itr = m_rpcIdxToStat.find(stationIndex);
        if (itr != m_rpcIdxToStat.end()) return itr->second;
        return -1;
    }
    void MuonDetectorManager::loadStationIndices() {
        
        const MdtIdHelper& mdtHelper{m_idHelperSvc->mdtIdHelper()};
        m_mdt_EIS_stName = mdtHelper.stationNameIndex("EIS");
        m_mdt_BIM_stName = mdtHelper.stationNameIndex("BIM");
        m_mdt_BME_stName = mdtHelper.stationNameIndex("BME");
        m_mdt_BMG_stName = mdtHelper.stationNameIndex("BMG");
  
        const RpcIdHelper& rpcHelper{m_idHelperSvc->rpcIdHelper()};
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BML"), RpcStatType::BML));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BMS"), RpcStatType::BMS));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BOL"), RpcStatType::BOL));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BOS"), RpcStatType::BOS));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BMF"), RpcStatType::BMF));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BOF"), RpcStatType::BOF));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BOG"), RpcStatType::BOG));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BME"), RpcStatType::BME));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BIR"), RpcStatType::BIR));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BIM"), RpcStatType::BIM));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BIL"), RpcStatType::BIL));
        m_rpcStatToIdx.insert(std::make_pair(rpcHelper.stationNameIndex("BIS"), RpcStatType::BIS));

        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BML, rpcHelper.stationNameIndex("BML")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BMS, rpcHelper.stationNameIndex("BMS")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BOL, rpcHelper.stationNameIndex("BOL")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BOS, rpcHelper.stationNameIndex("BOS")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BMF, rpcHelper.stationNameIndex("BMF")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BOF, rpcHelper.stationNameIndex("BOF")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BOG, rpcHelper.stationNameIndex("BOG")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BME, rpcHelper.stationNameIndex("BME")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BIR, rpcHelper.stationNameIndex("BIR")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BIM, rpcHelper.stationNameIndex("BIM")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BIL, rpcHelper.stationNameIndex("BIL")));
        m_rpcIdxToStat.insert(std::make_pair(RpcStatType::BIS, rpcHelper.stationNameIndex("BIS")));
    }
    int MuonDetectorManager::mdtStationName(const int stationIndex) const {
        if (stationIndex == NMdtStatType - 4)
            return m_mdt_EIS_stName;
        else if (stationIndex == NMdtStatType - 3)
            return  m_mdt_BIM_stName;
        else if (stationIndex == NMdtStatType - 2)
            return m_mdt_BME_stName ;
        else if (stationIndex == NMdtStatType - 1)
            return m_mdt_BMG_stName;
        return stationIndex;
    }

    // functions that override standard condition input for tests
    void MuonDetectorManager::setNSWABLineAsciiPath(const std::string& str) { m_NSWABLineAsciiPath = str; }
    void MuonDetectorManager::setNSWAsBuiltAsciiPath(const std::string &strMM, const std::string &strSTgc) {
        if (!strMM.empty()) {           
            ATH_MSG_INFO( "Overriding standard MM As-Built conditions with an external ascii file" );
            std::ifstream thefile(strMM);
            std::stringstream buffer;
            buffer << thefile.rdbuf();
            std::string str = buffer.str();
            thefile.close();
            std::unique_ptr<NswAsBuiltDbData> readNswAsBuilt = std::make_unique<NswAsBuiltDbData>();
            readNswAsBuilt->setMmData(str);
            setMMAsBuiltCalculator(readNswAsBuilt.get());
            m_NSWAsBuiltAsciiOverrideMM = true;
        }

        if (!strSTgc.empty()) {           
            ATH_MSG_INFO( "Overriding standard sTGC As-Built conditions with an external ascii file" );
            std::ifstream thefile(strSTgc);
            std::stringstream buffer;
            buffer << thefile.rdbuf();
            std::string str = buffer.str();
            thefile.close();
            std::unique_ptr<NswAsBuiltDbData> readNswAsBuilt = std::make_unique<NswAsBuiltDbData>();
            readNswAsBuilt->setSTgcData(str);
            setStgcAsBuiltCalculator(readNswAsBuilt.get());
            m_NSWAsBuiltAsciiOverrideSTgc = true;
        }
    }
    
    void MuonDetectorManager::setCacheFillingFlag(int value) { m_cacheFillingFlag = value; }
    void MuonDetectorManager::setCachingFlag(int value) { m_cachingFlag = value; }
    void MuonDetectorManager::set_DBMuonVersion(const std::string& version) { m_DBMuonVersion = version; }
    void MuonDetectorManager::setGeometryVersion(const std::string& version) { m_geometryVersion = std::move(version); }
    void MuonDetectorManager::setMinimalGeoFlag(int flag) { m_minimalgeo = flag; }
    void MuonDetectorManager::setCutoutsFlag(int flag) { m_includeCutouts = flag; }
    void MuonDetectorManager::setCutoutsBogFlag(int flag) { m_includeCutoutsBog = flag; }
    void MuonDetectorManager::setGenericTgcDescriptor(const GenericTGCCache& tc) {
        m_genericTGC.frame_h = tc.frame_h;
        m_genericTGC.frame_ab = tc.frame_ab;
        m_genericTGC.nlayers = tc.nlayers;
        for (unsigned int i = 0; i < (tc.materials).size(); i++) {
            m_genericTGC.materials[i] = tc.materials[i];
            m_genericTGC.positions[i] = tc.positions[i];
            m_genericTGC.tck[i] = tc.tck[i];
        }
    }
    void MuonDetectorManager::setGenericCscDescriptor(const GenericCSCCache& cc) {
        m_genericCSC.dummy1 = cc.dummy1;
        m_genericCSC.dummy2 = cc.dummy2;
    }
    void MuonDetectorManager::setGenericMdtDescriptor(const GenericMDTCache& mc) {
        m_genericMDT.innerRadius = mc.innerRadius;
        m_genericMDT.outerRadius = mc.outerRadius;
    }
    void MuonDetectorManager::setGenericRpcDescriptor(const GenericRPCCache& rc) {
        m_genericRPC.stripSeparation = rc.stripSeparation;
        m_genericRPC.stripPanelThickness = rc.stripPanelThickness;
        m_genericRPC.rpcLayerThickness = rc.rpcLayerThickness;
        m_genericRPC.centralSupPanelThickness = rc.centralSupPanelThickness;
        m_genericRPC.GasGapThickness = rc.GasGapThickness;
        m_genericRPC.frontendBoardWidth = rc.frontendBoardWidth;
    }

}  // namespace MuonGM
