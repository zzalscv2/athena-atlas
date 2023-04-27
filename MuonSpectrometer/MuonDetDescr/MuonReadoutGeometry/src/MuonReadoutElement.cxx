/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 A Muon GeoVDetectorElement
 -----------------------------------------
***************************************************************************/

#include "MuonReadoutGeometry/MuonReadoutElement.h"

#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "TrkSurfaces/CylinderBounds.h"
#include "TrkSurfaces/StraightLineSurface.h"

namespace {
    std::string to_string(const Trk::DetectorElemType type ) {
        if (type == Trk::DetectorElemType::Mdt) return "Mdt";
        else if (type == Trk::DetectorElemType::Rpc) return "Rpc";
        else if (type == Trk::DetectorElemType::Tgc) return "Tgc";
        else if (type == Trk::DetectorElemType::sTgc) return "sTgc";
        else if (type == Trk::DetectorElemType::MM) return "Mm";
        else if (type == Trk::DetectorElemType::Csc) return "Csc";
        return "Unknown";
    }
}
namespace MuonGM {

    MuonReadoutElement::MuonReadoutElement(GeoVFullPhysVol* pv, MuonDetectorManager* mgr, Trk::DetectorElemType detType) :
        TrkDetElementBase(pv), AthMessaging{to_string(detType)+"MuonReadoutElement"}, m_type{detType}, m_muon_mgr{mgr} {
            if (!m_idHelperSvc.retrieve().isSuccess()) {
                ATH_MSG_FATAL("Failed to retrieve the MuonIdHelperSvc");
                throw std::runtime_error("Invalid MuonIdHelperSvc");
            }
        }

    MuonReadoutElement::~MuonReadoutElement() = default;

    const Amg::Vector3D MuonReadoutElement::globalPosition() const { return absTransform().translation(); }

    bool MuonReadoutElement::largeSector() const {
        // this doesn't apply to TGC
        if(m_statname.size() >= 3){
            char c = m_statname[2];
            if (c == 'L')
                return true;
            else if (c == 'S')
                return false;
            else {
                if (c == 'E' || c == 'F' || c == 'G') return false;
                if (c == 'M' || c == 'R') return true;
            }
        }
        ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" largeSector() - is this station a larger sector answer is no for readout element "<<m_idHelperSvc->toStringDetEl(identify()));
        throw std::runtime_error("Unknown sector");
        return false;
    }

    bool MuonReadoutElement::smallSector() const {
        // this doesn't apply to TGC
        return (!largeSector());
    }

    void MuonReadoutElement::setParentStationPV(const PVConstLink& x) {
        m_parentStationPV = x;
        setIndexOfREinMuonStation();
    }

    void MuonReadoutElement::setParentStationPV() {
        if (m_parentStationPV) return;

        std::string::size_type npos;
        PVConstLink pStat = PVConstLink(nullptr);
        PVConstLink myphysvol(getMaterialGeom());

        std::string name = (myphysvol->getLogVol())->getName();
        if ((npos = name.find("Station")) != std::string::npos) {
            pStat = myphysvol;
        } else {
            for (unsigned int k = 0; k < 10; k++) {
                pStat = myphysvol->getParent();
                if (pStat == PVConstLink(nullptr)) break;
                name = (pStat->getLogVol())->getName();
                if ((npos = name.find("Station")) != std::string::npos) { break; }
                myphysvol = pStat;
            }
        }
        m_parentStationPV = pStat;
        setIndexOfREinMuonStation();
    }

    PVConstLink MuonReadoutElement::parentStationPV() const { return m_parentStationPV; }

    int MuonReadoutElement::getIndexOfREinMuonStation() const { return m_indexOfREinMuonStation; }

    void MuonReadoutElement::setIndexOfREinMuonStation() {
        PVConstLink par = parentStationPV();
        if (par == PVConstLink(nullptr)) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" No parent station found for "<<m_idHelperSvc->toStringDetEl(identify()));
            throw std::runtime_error("Parent station is a nullptr");
        }
        Query<unsigned int> c = par->indexOf(getMaterialGeom());
        if (c.isValid()) {
            m_indexOfREinMuonStation = (int)c;
        } else
            m_indexOfREinMuonStation = -999;
    }

    Amg::Transform3D MuonReadoutElement::toParentStation() const {
        PVConstLink par = parentStationPV();
        if (par == PVConstLink(nullptr)) {
            ATH_MSG_FATAL(__FILE__<<":"<<__LINE__<<" No parent station found for "<<m_idHelperSvc->toStringDetEl(identify()));
            throw std::runtime_error("Parent station is a nullptr");
        }

        if (m_indexOfREinMuonStation >= 0) return par->getXToChildVol((unsigned int)m_indexOfREinMuonStation);
        return GeoTrf::Transform3D::Identity();
    }

    void MuonReadoutElement::setParentMuonStation(const MuonStation* mstat) { m_parentMuonStation = mstat; }

    const MuonStation* MuonReadoutElement::parentMuonStation() const { return m_parentMuonStation; }

    Amg::Vector3D MuonReadoutElement::parentMuonStationPos() const {
        return Amg::CLHEPTransformToEigen(parentMuonStation()->getTransform()).translation();       
    }

    Amg::Vector3D MuonReadoutElement::AmdbLRSToGlobalCoords(const Amg::Vector3D& x) const {       
        return AmdbLRSToGlobalTransform() * x;
    }

    Amg::Transform3D MuonReadoutElement::AmdbLRSToGlobalTransform() const {
        HepGeom::Transform3D msToGlobal = parentMuonStation()->getTransform();             // native_MuonStation to global
        const HepGeom::Transform3D* msToAmdb = parentMuonStation()->getNativeToAmdbLRS();  // native_MuonStation to Amdb local (szt)
        return Amg::CLHEPTransformToEigen(msToGlobal * (msToAmdb->inverse()));
    }

    Amg::Vector3D MuonReadoutElement::GlobalToAmdbLRSCoords(const Amg::Vector3D& x) const {       
        return GlobalToAmdbLRSTransform() * x;
    }

    Amg::Transform3D MuonReadoutElement::GlobalToAmdbLRSTransform() const {
        HepGeom::Transform3D msToGlobal = parentMuonStation()->getTransform();             // native_MuonStation to global
        const HepGeom::Transform3D* msToAmdb = parentMuonStation()->getNativeToAmdbLRS();  // native_MuonStation to Amdb local (szt)
        return Amg::CLHEPTransformToEigen((*msToAmdb) * (msToGlobal.inverse()));
    }
    void MuonReadoutElement::setIdentifier(const Identifier& id) {
        m_id = id;
        if (!m_idHelperSvc->isMuon(id)) {
            ATH_MSG_FATAL("The Identifier "<<m_idHelperSvc->toString(id)<<" is not a muon one.");
            throw std::runtime_error("Invalid Identifier set");
        }
        auto loadIdFields = [this](const MuonIdHelper& idHelper) {
            if (idHelper.get_detectorElement_hash(m_id, m_detectorElIdhash) || 
                static_cast<unsigned int>(m_detectorElIdhash) >= idHelper.detectorElement_hash_max()){
                 ATH_MSG_FATAL("Failed to get a valid detector Element hash for "<<m_idHelperSvc->toStringDetEl(identify())
                                <<". Extracted hash "<<m_detectorElIdhash<<". Maximum allowed "<<idHelper.detectorElement_hash_max());
                 throw std::runtime_error("Invalid Detector Element Hash");
            }
            if (idHelper.get_module_hash(m_id, m_idhash) || 
                static_cast<unsigned int>(m_idhash) >= idHelper.module_hash_max()){
                 ATH_MSG_FATAL("Failed to get a valid module hash for "<<m_idHelperSvc->toStringDetEl(identify())
                                <<". Extracted hash "<<m_idhash<<". Maximum allowed "<<idHelper.module_hash_max());
                 throw std::runtime_error("Invalid Module Hash");
            }
            m_stIdx = idHelper.stationName(identify());
            m_eta = idHelper.stationEta(identify());
            m_phi = idHelper.stationPhi(identify());
        };
        if (m_idHelperSvc->isMdt(identify())) loadIdFields(m_idHelperSvc->mdtIdHelper());
        else if (m_idHelperSvc->isRpc(identify())) loadIdFields(m_idHelperSvc->rpcIdHelper());
        else if (m_idHelperSvc->isTgc(identify())) loadIdFields(m_idHelperSvc->tgcIdHelper());
        else if (m_idHelperSvc->isCsc(identify())) loadIdFields(m_idHelperSvc->cscIdHelper());
        else if (m_idHelperSvc->issTgc(identify())) loadIdFields(m_idHelperSvc->stgcIdHelper());
        else if (m_idHelperSvc->isMM(identify())) loadIdFields(m_idHelperSvc->mmIdHelper());
    }
    void MuonReadoutElement::setTechnologyName(const std::string& str) { m_techname = str; }
    void MuonReadoutElement::setStationName(const std::string& str) { m_statname = str; }
    void MuonReadoutElement::setStationS(double v) { m_stationS = v; }
    void MuonReadoutElement::setLongSsize(double v) { m_LongSsize = v; }
    void MuonReadoutElement::setLongRsize(double v) { m_LongRsize = v; }
    void MuonReadoutElement::setLongZsize(double v) { m_LongZsize = v; }
    void MuonReadoutElement::setSsize(double v) { m_Ssize = v; }
    void MuonReadoutElement::setRsize(double v) { m_Rsize = v; }
    void MuonReadoutElement::setZsize(double v) { m_Zsize = v; }
    void MuonReadoutElement::setCachingFlag(int value) { m_caching = value; }
}  // namespace MuonGM
