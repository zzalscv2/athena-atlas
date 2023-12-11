/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 The Rpc detector = an assembly module = RPC in amdb
 ----------------------------------------------------
***************************************************************************/

#include "MuonReadoutGeometry/RpcReadoutElement.h"

#include <GaudiKernel/IMessageSvc.h>
#include <GeoModelKernel/GeoDefinitions.h>
#include <GeoModelKernel/GeoLogVol.h>
#include <GeoModelKernel/GeoVFullPhysVol.h>
#include <GeoModelKernel/GeoVPhysVol.h>
#include <TString.h>

#include <cmath>
#include <stdexcept>

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelUtilities/GeoVisitVolumes.h"
#include "MuonReadoutGeometry/GenericRPCCache.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/Surface.h"
#include "TrkSurfaces/SurfaceBounds.h"

namespace {
    constexpr double rpc3GapLayerThickness = 11.8;  // gas vol. + ( bakelite + graphite + PET )x2
}

namespace MuonGM {

    RpcReadoutElement::RpcReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, int zi, int /*fi*/, bool is_mirrored,
                                         MuonDetectorManager* mgr) :
        MuonClusterReadoutElement(pv, mgr, Trk::DetectorElemType::Rpc),
        m_mirrored{is_mirrored}  {
        std::string gVersion = manager()->geometryVersion();

        // get the setting of the caching flag from the manager
        setCachingFlag(mgr->cachingFlag());

        m_descratzneg = (zi < 0 && !is_mirrored);


        setStationName(stName);

        if (!mgr->MinimalGeoFlag()) {
            if (GeoFullPhysVol* pvc = dynamic_cast<GeoFullPhysVol*>(pv)) {
                int lgg = 0;
                int llay = 0;
                std::string::size_type npos;
                for (const GeoVolumeVec_t::value_type& p1 : geoGetVolumes(pvc)) {
                    const GeoVPhysVol* pc = p1.first;
                    std::string childname = (pc->getLogVol())->getName();
                    if ((npos = childname.find("layer")) != std::string::npos) {
                        llay++;
                        lgg = 0;
                        for (const GeoVolumeVec_t::value_type& p2 : geoGetVolumes(pc)) {
                            const GeoVPhysVol* pcgv = p2.first;
                            std::string childname1 = (pcgv->getLogVol())->getName();
                            if ((npos = childname1.find("gas volume")) != std::string::npos) {
                                lgg++;
                                GeoTrf::Transform3D trans = p1.second * p2.second * pcgv->getXToChildVol(0);
                                m_Xlg[llay - 1][lgg - 1] = trans;
                            }
                        }
                    }
                }
            } else {
                throw std::runtime_error(Form(
                    "File: %s, Line: %d\nRpcReadoutElement::RpcReadoutElement() - Cannot perform a dynamic cast !", __FILE__, __LINE__));
            }
        }
    }

    RpcReadoutElement::~RpcReadoutElement() { clearCache(); }

    double RpcReadoutElement::localStripSCoord(int doubletZ, int doubletPhi, bool measphi  , int strip) const {
        if ((doubletZ != m_dbZ && m_netastrippanels == 1) || (m_netastrippanels != 1 && (doubletZ < 1 || doubletZ > m_netastrippanels))) {
            throw std::runtime_error(
                Form("File: %s, Line: %d\nRpcReadoutElement::localStripSCoord() - doubletZ %d outside range 1-%d created with m_dbZ=%d for "
                     "stName/Eta/Phi/dbR/dbZ/dbPhi=%s/%d/%d/%d/%d/%d",
                     __FILE__, __LINE__, doubletZ, m_netastrippanels, m_dbZ, getStationName().c_str(), getStationEta(), getStationPhi(),
                     getDoubletR(), getDoubletZ(), getDoubletPhi()));
        }
        bool notintheribs = !inTheRibs();
        if ((doubletPhi != m_dbPhi && m_nphistrippanels == 1 && notintheribs) ||
            (m_nphistrippanels != 1 && (doubletPhi < 1 || doubletPhi > m_nphistrippanels))) {
            throw std::runtime_error(
                Form("File: %s, Line: %d\nRpcReadoutElement::localStripSCoord() - doubletPhi %d outside range 1-%d created with m_dbPhi=%d "
                     "for station %s and RPC type %s",
                     __FILE__, __LINE__, doubletPhi, m_nphistrippanels, m_dbPhi, getStationName().c_str(), getTechnologyName().c_str()));
        }
        int maxstrip = 0;
        if (measphi)
            maxstrip = NphiStrips();
        else
            maxstrip = NetaStrips();
        if (strip < 1 || strip > maxstrip) {
            throw std::runtime_error(
                Form("File: %s, Line: %d\nRpcReadoutElement::localStripSCoord() - strip %d outside range 1-%d for measphi  =%d for "
                     "stName/Eta/Phi/dbR/dbZ/dbPhi=%s/%d/%d/%d/%d/%d",
                     __FILE__, __LINE__, strip, maxstrip, measphi  , getStationName().c_str(), getStationEta(), getStationPhi(),
                     getDoubletR(), getDoubletZ(), getDoubletPhi()));
        }

        double local_s = 0.;
        int dbphi = doubletPhi - 1;
        if (m_nphistrippanels == 1) dbphi = 0;
        if (measphi)
            local_s = m_first_phistrip_s[dbphi] + (strip - 1) * StripPitch(measphi  );
        else
            local_s = m_etastrip_s[dbphi];

#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
        if (log.level() <= MSG::VERBOSE) {
            log << MSG::VERBOSE << "Ssize, ndvs, nstr/pan, spitch, 1st-strp " << m_Ssize << " " << m_nphistrippanels << " "
                << m_nphistripsperpanel << " " << m_phistrippitch << " " << m_first_phistrip_s[doubletPhi - 1] << endmsg;
            log << MSG::VERBOSE << "localStripSCoord: local_s is " << local_s << " for dbZ/dbP/mphi/strip " << doubletZ << " " << doubletPhi
                << " " << measphi   << "/" << strip << endmsg;
        }
#endif
        return local_s;
    }
    double RpcReadoutElement::localStripZCoord(int doubletZ, int doubletPhi, bool measphi  , int strip) const {
        if ((doubletZ != m_dbZ && m_netastrippanels == 1) || (m_netastrippanels != 1 && (doubletZ < 1 || doubletZ > m_netastrippanels))) {
            throw std::runtime_error(
                Form("File: %s, Line: %d\nRpcReadoutElement::localStripZCoord() - doubletZ %d outside range 1-%d created with m_dbZ=%d for "
                     "stName/Eta/Phi/dbR/dbZ/dbPhi=%s/%d/%d/%d/%d/%d",
                     __FILE__, __LINE__, doubletZ, m_netastrippanels, m_dbZ, getStationName().c_str(), getStationEta(), getStationPhi(),
                     getDoubletR(), getDoubletZ(), getDoubletPhi()));
        }
        bool notintheribs = !inTheRibs();
        if ((doubletPhi != m_dbPhi && m_nphistrippanels == 1 && notintheribs) ||
            (m_nphistrippanels != 1 && (doubletPhi < 1 || doubletPhi > m_nphistrippanels))) {
            throw std::runtime_error(
                Form("File: %s, Line: %d\nRpcReadoutElement::localStripZCoord() - doubletPhi %d outside range 1-%d created with m_dbPhi=%d "
                     "for station %s and RPC type %s",
                     __FILE__, __LINE__, doubletPhi, m_nphistrippanels, m_dbPhi, getStationName().c_str(), getTechnologyName().c_str()));
        }
        int maxstrip = 0;
        if (measphi)
            maxstrip = NphiStrips();
        else
            maxstrip = NetaStrips();
        if (strip < 1 || (measphi && strip > maxstrip)) {
            throw std::runtime_error(
                Form("File: %s, Line: %d\nRpcReadoutElement::localStripZCoord() - strip %d outside range 1-%d for measphi  =%d for "
                     "stName/Eta/Phi/dbR/dbZ/dbPhi=%s/%d/%d/%d/%d/%d",
                     __FILE__, __LINE__, strip, maxstrip, measphi  , getStationName().c_str(), getStationEta(), getStationPhi(),
                     getDoubletR(), getDoubletZ(), getDoubletPhi()));
        }

        double local_z = 0.;
        if (measphi   == 0) {
            double xx = m_first_etastrip_z[0];
            if (m_netastrippanels > 1 && doubletZ > 1) xx = m_first_etastrip_z[doubletZ - 1];
            local_z = xx + (strip - 1) * StripPitch(measphi  );
        } else {
            double xx = m_phistrip_z[0];
            if (m_netastrippanels > 1 && doubletZ > 1) xx = m_phistrip_z[doubletZ - 1];
            local_z = xx;
        }

#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
        if (log.level() <= MSG::VERBOSE) {
            log << MSG::VERBOSE << "Zsize, ndvz, nstr/pan, zpitch, 1st-strp " << m_Zsize << " " << m_netastrippanels << " "
                << m_netastripsperpanel << " " << m_etastrippitch << " " << m_first_etastrip_z[doubletZ - 1] << endmsg;
            log << MSG::VERBOSE << "localStripZCoord: local_z is " << local_z << " for dbZ/dbP/mphi/strip " << doubletZ << " " << doubletPhi
                << " " << measphi   << "/" << strip << endmsg;
        }
#endif
        return local_z;
    }

    Amg::Vector3D RpcReadoutElement::stripPos(int doubletR, int doubletZ, int doubletPhi, int gasGap, bool measphi  , int strip) const {
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
        if (log.level() <= MSG::VERBOSE)
            log << MSG::VERBOSE << "stripPos for dbr/dbz/dbp/gg/mp/strip " << doubletR << " " << doubletZ << " " << doubletPhi << " "
                << gasGap << " " << measphi   << " " << strip << endmsg;
#endif

        // global position of a generic strip !!!!!

        const Amg::Vector3D localP = localStripPos(doubletR, doubletZ, doubletPhi, gasGap, measphi  , strip);

        const Amg::Transform3D rpcTrans = absTransform();
#ifndef NDEBUG
        if (log.level() <= MSG::VERBOSE) {
            log << MSG::VERBOSE << "RpcReadoutElement::stripPos got localStripPos " << localP << endmsg;
            Amg::Vector3D trans = rpcTrans * Amg::Vector3D(0., 0., 0);
            log << MSG::VERBOSE << "RpcReadoutElement::stripPos gl. transl. R, phi "
                << std::sqrt(trans.x() * trans.x() + trans.y() * trans.y()) << " " << trans.phi() << " R-Rsize/2 "
                << std::sqrt(trans.x() * trans.x() + trans.y() * trans.y()) - m_Rsize * 0.5 << endmsg;
        }
#endif
        return rpcTrans * localP;
    }

    Amg::Vector3D RpcReadoutElement::localStripPos(int /*doubletR*/, int doubletZ, int doubletPhi, int gasGap, bool measphi  ,
                                                   int strip) const {
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
        if (log.level() <= MSG::VERBOSE)
            log << MSG::VERBOSE << "localstripPos for dbr/dbz/dbp/gg/mp/strip " << doubletZ << " " << doubletPhi << " " << gasGap << " "
                << measphi   << " " << strip << endmsg;
#endif

        // global position of a generic strip !!!!!

        // local coordinates are defined for a module at pos. Z with DED on top !!!!
        // if such module is located at StationPhi=0 then xgeolocal//xglobal
        //                                                ygeolocal//yglobal
        //                                                zgeolocal//yglobal

        // if there's a DED at the bottom, the Rpc is rotated by 180deg around its local y axis
        // gg numbering is swapped
        // except for BI chambers (with 3 gas gaps -> this is taken into account in localTopGasGap()
        // -> eta strip n. 1 (offline id) is "last" eta strip (local)
        int lstrip = strip;
        int lgg = gasGap;
        if (!m_hasDEDontop) {
            if (measphi   == 0) lstrip = NetaStrips() - strip + 1;
            lgg++;
            if (lgg > m_nlayers) lgg = 1;
#ifndef NDEBUG
            if (log.level() <= MSG::VERBOSE)
                log << MSG::VERBOSE << "RpcReadoutElement::localstripos  m_hasDEDontop =" << m_hasDEDontop << " lstrip, lgg " << lstrip
                    << " " << lgg << endmsg;
#endif
        }

        int ldoubletPhi = doubletPhi;
        int ldoubletZ = doubletZ;
        // if the station is mirrored, the Rpc is rotated by 180deg around its local x axis
        // numbering of phi strips must be reversed;
        // numbering of eta strips is unchanged;
        // numbering of doubletPhi must be reversed if m_nphistrippanels>1
        // numbering of doubletZ   is unchanged;
        if (isMirrored()) {
            if (measphi) lstrip = NphiStrips() - lstrip + 1;
            if (m_nphistrippanels != 1) {
                ldoubletPhi = doubletPhi + 1;
                if (ldoubletPhi > 2) ldoubletPhi = 1;
            }
#ifndef NDEBUG
            if (log.level() <= MSG::VERBOSE)
                log << MSG::VERBOSE << "localstrippos  isMirrored =" << isMirrored() << " lstrip, ldoubletPhi " << lstrip << " "
                    << ldoubletPhi << endmsg;
#endif
        }

        // the special case of chambers at Z<0 but not mirrored
        // numbering of eta strips must be reversed;
        // numbering of phi strips is unchanged;
        // numbering of doubletZ must be reversed if  m_netastrippanels>1
        // numbering of doubletPhi   is unchanged;
        if (m_descratzneg) {
            if (m_netastrippanels > 1) {
                ldoubletZ++;
                if (ldoubletZ > 2) ldoubletZ = 1;
            }
            if (measphi   == 0) lstrip = NetaStrips() - lstrip + 1;
#ifndef NDEBUG
            if (log.level() <= MSG::VERBOSE)
                log << MSG::VERBOSE << "localstrippos special not mirrored at eta<0 ="
                    << " lstrip, ldoublerZ " << lstrip << " " << ldoubletZ << endmsg;
#endif
        }

        // the only RPCs in ATLAS which have 3 gasGaps (layers) are BI RPCs and those only have 1 doubletPhi
        if (m_nlayers == 3 && ldoubletPhi != 1) {
#ifdef NDEBUG
            MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
#endif
            if (log.level() <= MSG::WARNING)
                log << MSG::WARNING << "localStripPos() - found ldoubletPhi=" << ldoubletPhi
                    << " for BI RPC which cannot be true, setting to 1" << endmsg;
            ldoubletPhi = 1;
        }

        Amg::Vector3D localP(localGasGapDepth(lgg), localStripSCoord(ldoubletZ, ldoubletPhi, measphi  , lstrip),
                             localStripZCoord(ldoubletZ, ldoubletPhi, measphi  , lstrip));

#ifndef NDEBUG
        if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "localstrippos = " << localP << endmsg;
#endif
        return localP;
    }

    double RpcReadoutElement::localGasGapDepth(int gasGap) const {
        const GenericRPCCache* r = manager()->getGenericRpcDescriptor();
        double xgg = 0;
        if (m_nlayers == 3) {  // the BI RPCs are the only ones with 3 gas gaps, they don't have an inner support structure
            xgg = -rpc3GapLayerThickness + (gasGap - 1) * rpc3GapLayerThickness -
                  0.74;  // the values from MuonGeoModel have an offset of 0.74, TO BE INVESTIGATED, cf. ATLASSIM-5021
        } else {
            xgg = -m_Rsize / 2. + m_exthonthick + r->stripPanelThickness + r->GasGapThickness / 2.;
            if (gasGap == 1) return xgg;
            xgg = xgg + r->rpcLayerThickness + r->centralSupPanelThickness;
        }
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
        if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "localGasGapDepth(" << gasGap << ") selected is " << xgg << endmsg;
#endif
        return xgg;
    }

    Amg::Vector3D RpcReadoutElement::localStripPos(const Identifier& id) const {
        const RpcIdHelper* idh = manager()->rpcIdHelper();
        int doubletR = idh->doubletR(id);
        int doubletZ = idh->doubletZ(id);
        int doubletPhi = idh->doubletPhi(id);
        int gasgap = idh->gasGap(id);
        bool measphi   = idh->measuresPhi(id);
        int strip = idh->strip(id);

        return localStripPos(doubletR, doubletZ, doubletPhi, gasgap, measphi  , strip);
    }

    Amg::Vector3D RpcReadoutElement::stripPos(const Identifier& id) const {
        const RpcIdHelper* idh = manager()->rpcIdHelper();
        int doubletR = idh->doubletR(id);
        int doubletZ = idh->doubletZ(id);
        int doubletPhi = idh->doubletPhi(id);
        int gasgap = idh->gasGap(id);
        bool measphi   = idh->measuresPhi(id);
        int strip = idh->strip(id);
        return stripPos(doubletR, doubletZ, doubletPhi, gasgap, measphi  , strip);
    }

    bool RpcReadoutElement::rotatedRpcModule() const { return (!m_hasDEDontop); }
    bool RpcReadoutElement::rotatedGasGap(const Identifier& id) const { return localTopGasGap(id); }
    bool RpcReadoutElement::rotatedGasGap(int gasGap) const { return localTopGasGap(gasGap); }

    bool RpcReadoutElement::localTopGasGap(int gasGap) const {
        // top gas gap is rotated around y => z coordinates are reversed

        // global position of a generic strip !!!!!

        // local coordinates are defined for a module at pos. Z with DED on top !!!!
        // if such module is located at StationPhi=0 then xgeolocal//xglobal
        //                                                ygeolocal//yglobal
        //                                                zgeolocal//yglobal

        // if there's a DED at the bottom, the Rpc is rotated by 180deg around its local y axis
        // gg numbering is swapped
        // -> eta strip n. 1 (offline id) is "last" eta strip (local)
        int lgg = gasGap;
        if (!m_hasDEDontop) {
            lgg++;
            if (lgg > m_nlayers) lgg = 1;
        }

        bool topgg = false;
        if (lgg == 2 && m_nlayers != 3) topgg = true;  // BI chambers have 3 gaps and are never rotated
        return topgg;
    }

    bool RpcReadoutElement::localTopGasGap(const Identifier& id) const {
        // top gas gap is rotated around y => z coordinates are reversed
        const RpcIdHelper* idh = manager()->rpcIdHelper();
        int gasgap = idh->gasGap(id);

        // global position of a generic strip !!!!!

        // local coordinates are defined for a module at pos. Z with DED on top !!!!
        // if such module is located at StationPhi=0 then xgeolocal//xglobal
        //                                                ygeolocal//yglobal
        //                                                zgeolocal//yglobal

        // if there's a DED at the bottom, the Rpc is rotated by 180deg around its local y axis
        // gg numbering is swapped
        // -> eta strip n. 1 (offline id) is "last" eta strip (local)
        int lgg = gasgap;
        if (!m_hasDEDontop) {
            lgg++;
            if (lgg > m_nlayers) lgg = 1;
        }

        bool topgg = false;
        if (lgg == 2 && m_nlayers != 3) topgg = true;  // BI chambers have 3 gaps and are never rotated
        return topgg;
    }

    Amg::Vector3D RpcReadoutElement::gasGapPos(const Identifier& id) const {
        const RpcIdHelper* idh = manager()->rpcIdHelper();
        int doubletZ = idh->doubletZ(id);
        int doubletPhi = idh->doubletPhi(id);
        int gasgap = idh->gasGap(id);
        return gasGapPos(doubletZ, doubletPhi, gasgap);
    }

    Amg::Vector3D RpcReadoutElement::gasGapPos(int doubletZ, int doubletPhi, int gasgap) const {
        const Amg::Vector3D localP = localGasGapPos(doubletZ, doubletPhi, gasgap);

        const Amg::Transform3D rpcTrans = absTransform();

#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
        if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "RpcReadoutElement::gasGapPos got localGasGapPos" << localP << endmsg;
#endif
        return rpcTrans * localP;
    }
    Amg::Vector3D RpcReadoutElement::localGasGapPos(const Identifier& id) const {
        const RpcIdHelper* idh = manager()->rpcIdHelper();
        int doubletZ = idh->doubletZ(id);
        int doubletPhi = idh->doubletPhi(id);
        int gasgap = idh->gasGap(id);

        return localGasGapPos(doubletZ, doubletPhi, gasgap);
    }
    Amg::Vector3D RpcReadoutElement::localGasGapPos(int doubletZ, int doubletPhi, int gasgap) const {
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
#endif
        // global position of a generic strip !!!!!

        // local coordinates are defined for a module at pos. Z with DED on top !!!!
        // if such module is located at StationPhi=0 then xgeolocal//xglobal
        //                                                ygeolocal//yglobal
        //                                                zgeolocal//yglobal

        // if there's a DED at the bottom, the Rpc is rotated by 180deg around its local y axis
        // gg numbering is swapped
        // -> eta strip n. 1 (offline id) is "last" eta strip (local)
        int lgg = gasgap;
        if (!m_hasDEDontop) {
            lgg++;
            if (lgg > m_nlayers) lgg = 1;
#ifndef NDEBUG
            if (log.level() <= MSG::VERBOSE)
                log << MSG::VERBOSE << "RpcReadoutElement::localgasgapPos  m_hasDEDontop =" << m_hasDEDontop << " lgg " << lgg << endmsg;
#endif
        }

        int ldoubletPhi = doubletPhi;
        int ldoubletZ = doubletZ;
        // if the station is mirrored, the Rpc is rotated by 180deg around its local x axis
        // numbering of phi strips must be reversed;
        // numbering of eta strips is unchanged;
        // numbering of doubletPhi must be reversed if m_nphistrippanels>1
        // numbering of doubletZ   is unchanged;
        if (isMirrored()) {
            if (m_nphistrippanels != 1) {
                ldoubletPhi = doubletPhi + 1;
                if (ldoubletPhi > 2) ldoubletPhi = 1;
            }
#ifndef NDEBUG
            if (log.level() <= MSG::VERBOSE)
                log << MSG::VERBOSE << "RpcReadoutElement::localgasgapPos  isMirrored =" << isMirrored() << " ldoubletPhi " << ldoubletPhi
                    << endmsg;
#endif
        }

        // the special case of chambers at Z<0 but not mirrored
        // numbering of eta strips must be reversed;
        // numbering of phi strips is unchanged;
        // numbering of doubletZ must be reversed if  m_netastrippanels>1
        // numbering of doubletPhi   is unchanged;
        if (m_descratzneg) {
            if (m_netastrippanels > 1) {
                ldoubletZ++;
                if (ldoubletZ > 2) ldoubletZ = 1;
            }
#ifndef NDEBUG
            if (log.level() <= MSG::VERBOSE)
                log << MSG::VERBOSE << "RpcReadoutElement::localgasgapPos  special not mirrored at eta<0 ="
                    << " ldoublerZ " << ldoubletZ << endmsg;
#endif
        }

        double ggwidth = (m_nphigasgaps != 0) ? m_Ssize / m_nphigasgaps : m_Ssize;
        double gglength = (m_netagasgaps != 0) ? m_Zsize / m_netagasgaps : m_Zsize;
        int lggPhi = (m_nphigasgaps == 1) ? 1 : ldoubletPhi;
        int lggZ = (m_netagasgaps == 1) ? 1 : ldoubletZ;
        double local_s = 0;
        double local_z = 0;
        if (m_nphigasgaps != 1) local_s = -m_Ssize / 4. + (lggPhi - 1) * ggwidth;
        if (m_netagasgaps != 1) local_z = -m_Zsize / 4. + (lggZ - 1) * gglength;
        if (m_nlayers == 3) {  // take the numbers set in MuonChamber.cxx for BI RPCs
            local_s = m_y_translation;
            local_z = m_z_translation;
        }
        Amg::Vector3D localPold(localGasGapDepth(lgg), local_s, local_z);
        Amg::Vector3D localP1 = localPold;
        if (manager()->MinimalGeoFlag() == 0) {
            localP1 = m_Xlg[lgg - 1][lggPhi - 1].translation();
            if (std::abs(localP1.x() - localPold.x()) > 0.01 || std::abs(localP1.y() - localPold.y()) > 0.01 ||
                std::abs(localP1.z() - localPold.z()) > 0.01) {
                const RpcIdHelper* idh = manager()->rpcIdHelper();
                const Identifier id = identify();
                throw std::runtime_error(Form(
                    "File: %s, Line: %d\nRpcReadoutElement::localGasGapPos(%d,%d,%d) - position computed here (x,y,z=%.3f,%.3f,%.3f) does "
                    "not match the one retrieved from the GeoPhysVol (x,y,z=%.3f,%.3f,%.3f)\nfor RpcReadoutElement with stationName=%d "
                    "(%s), stationEta=%d, stationPhi=%d, doubletZ=%d, doubletR=%d, doubletPhi=%d, gasGap=%d",
                    __FILE__, __LINE__, doubletZ, doubletPhi, gasgap, localPold.x(), localPold.y(), localPold.z(), localP1.x(), localP1.y(),
                    localP1.z(), idh->stationName(id), idh->stationNameString(idh->stationName(id)).c_str(), idh->stationEta(id),
                    idh->stationPhi(id), idh->doubletZ(id), idh->doubletR(id), idh->doubletPhi(id), idh->gasGap(id)));
            } else {
#ifndef NDEBUG
                MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
                if (log.level() <= MSG::VERBOSE)
                    log << MSG::VERBOSE << "localGasGapPos(" << doubletZ << "," << doubletPhi << "," << gasgap
                        << ") - LocalGasGapPos computed here matches the one retrieved from the GeoPhysVol for rpc RE "
                        << manager()->rpcIdHelper()->show_to_string(identify()) << " and dbZ/dbPhi/gg " << doubletZ << "/" << doubletPhi
                        << "/" << gasgap << endmsg;
                log << MSG::VERBOSE << "Computed here " << localPold << " from GeoPhysVol " << localP1 << endmsg;
#endif
            }
        }

        return localP1;
    }
    Amg::Vector3D RpcReadoutElement::localStripPanelPos(int doubletZ, int doubletPhi, int gasgap) const {
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
#endif
        // global position of a generic strip !!!!!

        // local coordinates are defined for a module at pos. Z with DED on top !!!!
        // if such module is located at StationPhi=0 then xgeolocal//xglobal
        //                                                ygeolocal//yglobal
        //                                                zgeolocal//yglobal

        // if there's a DED at the bottom, the Rpc is rotated by 180deg around its local y axis
        // sp numbering is swapped
        // -> eta strip n. 1 (offline id) is "last" eta strip (local)
        int lsp = gasgap;
        if (!m_hasDEDontop) {
            lsp++;
            if (lsp > m_nlayers) lsp = 1;
#ifndef NDEBUG
            if (log.level() <= MSG::VERBOSE)
                log << MSG::VERBOSE << "localStripPanelPos  m_hasDEDontop =" << m_hasDEDontop << " lsp " << lsp << endmsg;
#endif
        }

        int ldoubletPhi = doubletPhi;
        int ldoubletZ = doubletZ;
        // if the station is mirrored, the Rpc is rotated by 180deg around its local x axis
        // numbering of phi strips must be reversed;
        // numbering of eta strips is unchanged;
        // numbering of doubletPhi must be reversed if m_nphistrippanels>1
        // numbering of doubletZ   is unchanged;
        if (isMirrored()) {
            if (m_nphistrippanels != 1) {
                ldoubletPhi = doubletPhi + 1;
                if (ldoubletPhi > 2) ldoubletPhi = 1;
            }
#ifndef NDEBUG
            if (log.level() <= MSG::VERBOSE)
                log << MSG::VERBOSE << "localgasgapPos  isMirrored =" << isMirrored() << " ldoubletPhi " << ldoubletPhi << endmsg;
#endif
        }

        // the special case of chambers at Z<0 but not mirrored
        // numbering of eta strips must be reversed;
        // numbering of phi strips is unchanged;
        // numbering of doubletZ must be reversed if  m_netastrippanels>1
        // numbering of doubletPhi   is unchanged;
        if (m_descratzneg) {
            if (m_netastrippanels > 1) {
                ldoubletZ++;
                if (ldoubletZ > 2) ldoubletZ = 1;
            }
#ifndef NDEBUG
            if (log.level() <= MSG::VERBOSE)
                log << MSG::VERBOSE << "localgasgapPos  special not mirrored at eta<0 ="
                    << " ldoublerZ " << ldoubletZ << endmsg;
#endif
        }

        double spwidth = m_Ssize;
        double splength = m_Zsize;
        int lspPhi = ldoubletPhi;
        int lspZ = ldoubletZ;
        if (m_nphistrippanels == 1) {
            lspPhi = 1;
        } else {
            spwidth = spwidth / 2.;
        }

        if (m_netastrippanels == 1) {
            lspZ = 1;
        } else {
            splength = splength / 2.;
        }
        double local_s, local_z;
        if (m_nphistrippanels == 1)
            local_s = 0.;
        else
            local_s = -m_Ssize / 4. + (lspPhi - 1) * spwidth;
        if (m_netastrippanels == 1)
            local_z = 0.;
        else
            local_z = -m_Zsize / 4. + (lspZ - 1) * splength;

        Amg::Vector3D localP(localGasGapDepth(lsp), local_s, local_z);
        return localP;
    }

    Amg::Vector3D RpcReadoutElement::SDtoModuleCoords(const Amg::Vector3D& x, const Identifier& id) const {
        const Amg::Vector3D gasgapP = localGasGapPos(id);
        const Amg::Translation3D xfp(gasgapP.x(), gasgapP.y(), gasgapP.z());
        if (rotatedGasGap(id))
            return xfp * Amg::Vector3D(-x.x(), x.y(), -x.z());
        else
            return xfp * x;
    }
    Amg::Vector3D RpcReadoutElement::localToGlobalCoords(const Amg::Vector3D& x, const Identifier& id) const {
        const Amg::Vector3D gasgapP = localGasGapPos(id);
        const Amg::Translation3D xfp(gasgapP.x(), gasgapP.y(), gasgapP.z());
        if (rotatedGasGap(id))
            return absTransform() * xfp * Amg::Vector3D(-x.x(), x.y(), -x.z());
        else
            return absTransform() * xfp * x;
    }
    Amg::Transform3D RpcReadoutElement::localToGlobalTransf(const Identifier& id) const {
        Amg::Vector3D gasgapP = localGasGapPos(id);
        Amg::Translation3D xfp(gasgapP.x(), gasgapP.y(), gasgapP.z());
        Amg::Transform3D trans = absTransform();
        if (rotatedGasGap(id))
            return trans * xfp * Amg::AngleAxis3D(180. * CLHEP::deg, Amg::Vector3D::UnitY());
        else
            return trans * xfp;
    }
    Amg::Transform3D RpcReadoutElement::localToGlobalStripPanelTransf(int dbZ, int dbPhi, int gasGap) const {
        const Amg::Vector3D locP = localStripPanelPos(dbZ, dbPhi, gasGap);
        const Amg::Translation3D xfp(locP.x(), locP.y(), locP.z());
        if (rotatedGasGap(gasGap))
            return absTransform() * xfp * Amg::AngleAxis3D(180. * CLHEP::deg, Amg::Vector3D::UnitY());
        else
            return absTransform() * xfp;
    }
    Amg::Transform3D RpcReadoutElement::localToGlobalTransf(int dbZ, int dbPhi, int gasGap) const {
        const Amg::Vector3D gasgapP = localGasGapPos(dbZ, dbPhi, gasGap);
        const Amg::Translation3D xfp(gasgapP.x(), gasgapP.y(), gasgapP.z());
        if (rotatedGasGap(gasGap))
            return absTransform() * xfp * Amg::AngleAxis3D(180. * CLHEP::deg, Amg::Vector3D::UnitY());
        else
            return absTransform() * xfp;
    }
    Amg::Transform3D RpcReadoutElement::globalToLocalTransf(const Identifier& id) const { return localToGlobalTransf(id).inverse(); }
    Amg::Vector3D RpcReadoutElement::globalToLocalCoords(const Amg::Vector3D& x, Identifier id) const {
        return globalToLocalTransf(id) * x;
    }

    double RpcReadoutElement::distanceToPhiReadout(const Amg::Vector3D& P, const Identifier& id) const {
        // P is a point in the global reference frame
        // we want to have the distance from the side of the phi readout (length travelled along a phi strip) from a signal produced at P)
        // m_set will not be null but be initialized in "initDesign()" earlier
        // if it is null the code should crash! - because we're time-critical here a check for null was not implemented
        unsigned int ndbz{0};
        for (int dbz = 1 ; dbz <= m_idHelper.doubletZMax(identify()); ++dbz) {
            const Identifier dbzId = m_idHelper.channelID(identify(), dbz, 1, 1, 0, 1);
            ndbz+=(manager()->getRpcReadoutElement(dbzId) != nullptr);
        }

        double dist = -999.;
        double zPoint = P.z();
        double Zsizehalf = getZsize() / 2.;
        double recenter = REcenter().z();
        double zLow = recenter - Zsizehalf;
        double zUp = recenter + Zsizehalf;

        if (ndbz == 1) {
            if (zPoint < zLow || zPoint > zUp) {
                const RpcIdHelper* idh = manager()->rpcIdHelper();
                MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
                if (log.level() <= MSG::DEBUG)
                    log << MSG::DEBUG << "RpcReadoutElement with id " << idh->show_to_string(identify())
                        << " ::distanceToPhiReadout --- z of the Point  " << P.z() << " is out of the rpc-module range (" << zLow << ","
                        << zUp << ")"
                        << " /// input id(never used) = " << idh->show_to_string(id) << endmsg;
                // return dist;
                if (zPoint < zLow)
                    zPoint = zLow;
                else if (zPoint > zUp)
                    zPoint = zUp;
            }
            if (sideC())
                dist = zUp - zPoint;
            else
                dist = zPoint - zLow;

        } else {
            if (zPoint < zLow || zPoint > zUp) {
                const RpcIdHelper* idh = manager()->rpcIdHelper();
                MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
                if (log.level() <= MSG::DEBUG)
                    log << MSG::DEBUG << "RpcReadoutElement with id " << idh->show_to_string(identify())
                        << " ::distanceToPhiReadout --- z of the Point  " << P.z() << " is out of the rpc-module range (" << zLow << ","
                        << zUp << ")"
                        << " /// input id(never used) = " << idh->show_to_string(id) << endmsg;
                // return dist;
                if (zPoint < zLow)
                    zPoint = zLow;
                else if (zPoint > zUp)
                    zPoint = zUp;
            }
            if (m_dbZ == 1 || m_dbZ == 3) {
                if (sideC())
                    dist = zUp - zPoint;
                else
                    dist = zPoint - zLow;

            } else if (m_dbZ == 2) {
                if (sideC())
                    dist = zPoint - zLow;
                else
                    dist = zUp - zPoint;
            }
        }
        return dist;
    }

    double RpcReadoutElement::distanceToEtaReadout(const Amg::Vector3D& P) const { return distanceToEtaReadout(P, identify()); }
    double RpcReadoutElement::distanceToPhiReadout(const Amg::Vector3D& P) const { return distanceToPhiReadout(P, identify()); }

    double RpcReadoutElement::distanceToEtaReadout(const Amg::Vector3D& P, const Identifier& /*id*/) const {
        // id is actually never used !!!!!!!!!!!!!!!!!
        double dist = -999999.;
        double pAmdbL = GlobalToAmdbLRSCoords(P).x();
        double myCenterAmdbL = GlobalToAmdbLRSCoords(REcenter()).x();
        double sdistToCenter = pAmdbL - myCenterAmdbL;
        if (std::abs(sdistToCenter) > getSsize() * 0.5) {
#ifndef NDEBUG
            const RpcIdHelper* idh = manager()->rpcIdHelper();
            MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
            if (log.level() <= MSG::DEBUG)
                log << MSG::DEBUG << "RpcReadoutElement with id " << idh->show_to_string(identify())
                    << " ::distanceToEtaReadout --- in amdb local frame x of the point  " << pAmdbL << " is out of the rpc-module range ("
                    << myCenterAmdbL - getSsize() * 0.5 << "," << myCenterAmdbL + getSsize() * 0.5 << ")" << endmsg;
#endif
            if (sdistToCenter > 0) {
                sdistToCenter = getSsize() * 0.5;
#ifndef NDEBUG
                if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "setting distance to " << sdistToCenter << endmsg;
#endif
            } else if (sdistToCenter < 0) {
                sdistToCenter = -getSsize() * 0.5;
#ifndef NDEBUG
                if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "setting distance to " << sdistToCenter << endmsg;
#endif
            }
        }
        if (m_nphistrippanels == 2) {
            dist = getSsize() * 0.5 - std::abs(sdistToCenter);
        } else {
            // assumes readout is at smallest phi
            dist = getSsize() * 0.5 + (sdistToCenter);
        }
        return dist;
    }

    void RpcReadoutElement::initDesign() {
        m_phiDesigns.reserve(NphiStripPanels());
        m_etaDesigns.reserve(NphiStripPanels());

        for (int i = 1; i <= NphiStripPanels(); ++i) {
            Amg::Transform3D gToSurf = MuonClusterReadoutElement::transform(surfaceHash(i, 1, 1)).inverse();
            Amg::Vector3D stripPos1 = stripPos(getDoubletR(), getDoubletZ(), i, 1, 1, 1);
            Amg::Vector3D stripPos2 = stripPos(getDoubletR(), getDoubletZ(), i, 1, 1, 2);
            Amg::Vector3D locStripPos1 = gToSurf * stripPos1;
            Amg::Vector3D locStripPos2 = gToSurf * stripPos2;

            Amg::Transform3D gToSurfEta = MuonClusterReadoutElement::transform(surfaceHash(i, 1, 0)).inverse();
            Amg::Vector3D stripPosEta1 = stripPos(getDoubletR(), getDoubletZ(), i, 1, 0, 1);
            Amg::Vector3D stripPosEta2 = stripPos(getDoubletR(), getDoubletZ(), i, 1, 0, 2);
            Amg::Vector3D locStripPosEta1 = gToSurfEta * stripPosEta1;
            Amg::Vector3D locStripPosEta2 = gToSurfEta * stripPosEta2;

            MuonStripDesign phiDesign;
            phiDesign.nstrips = NphiStrips();
            phiDesign.stripPitch = StripPitch(1);
            phiDesign.stripLength = StripLength(1);
            phiDesign.stripWidth = StripWidth(1);

            phiDesign.firstStripPos[0] = locStripPos1.x();
            phiDesign.firstStripPos[1] = locStripPos1.y();
            if (locStripPos2.x() - locStripPos1.x() < 0.) phiDesign.stripPitch *= -1.;
            phiDesign.invStripPitch = 1. / phiDesign.stripPitch;
            phiDesign.readoutLocY = 0.;
            phiDesign.signY = 0.;

            Amg::Vector2D pos1;
            pos1.setZero();
            phiDesign.stripPosition(1, pos1);
            Amg::Vector2D pos2;
            pos2.setZero();
            phiDesign.stripPosition(2, pos2);

            if (std::abs(pos1.x() - locStripPos1.x()) > 1e-6) {
                MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
                log << MSG::INFO << " bad local strip pos " << endmsg;
                log << MSG::INFO << " phi local strip positions " << locStripPos1 << "   " << locStripPos2 << " first strip "
                    << phiDesign.firstStripPos << " pitch " << phiDesign.stripPitch << " from calc " << locStripPos2.x() - locStripPos1.x()
                    << endmsg;
                log << MSG::INFO << " checking strip position: phi design  " << pos1 << " " << pos2 << endmsg;
            }

            m_phiDesigns.push_back(phiDesign);

            MuonStripDesign etaDesign;
            etaDesign.nstrips = NetaStrips();
            etaDesign.stripPitch = StripPitch(0);
            etaDesign.stripLength = StripLength(0);
            etaDesign.stripWidth = StripWidth(0);

            etaDesign.firstStripPos[0] = locStripPosEta1.x();
            etaDesign.firstStripPos[1] = locStripPosEta1.y();
            if (locStripPosEta2.x() - locStripPosEta1.x() < 0.) etaDesign.stripPitch *= -1.;
            etaDesign.invStripPitch = 1. / etaDesign.stripPitch;
            etaDesign.readoutLocY = 0.;
            etaDesign.signY = 0.;

            etaDesign.stripPosition(1, pos1);
            etaDesign.stripPosition(2, pos2);

            if (std::abs(pos1.x() - locStripPosEta1.x()) > 1e-6) {
                MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
                log << MSG::INFO << " bad local strip pos " << endmsg;
                log << MSG::INFO << " eta local strip positions " << locStripPosEta1 << "   " << locStripPosEta2 << " first strip "
                    << etaDesign.firstStripPos << " pitch " << etaDesign.stripPitch << " from calc "
                    << locStripPosEta2.x() - locStripPosEta1.x() << endmsg;
                log << MSG::INFO << " checking strip position: eta design  " << pos1 << " " << pos2 << endmsg;
            }

            m_etaDesigns.push_back(etaDesign);
        }
    }

#if defined(FLATTEN)
    // We compile this package with optimization, even in debug builds; otherwise,
    // the heavy use of Eigen makes it too slow.  However, from here we may call
    // to out-of-line Eigen code that is linked from other DSOs; in that case,
    // it would not be optimized.  Avoid this by forcing all Eigen code
    // to be inlined here if possible.
    ATH_FLATTEN
#endif
    void
    RpcReadoutElement::fillCache() {

        if (!m_surfaceData)
            m_surfaceData = std::make_unique<SurfaceData>();
        else {
            MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
            log << MSG::WARNING << "calling fillCache on an already filled cache" << endmsg;
            return;
        }
        const RpcIdHelper* idh = manager()->rpcIdHelper();
        Identifier parentID = idh->parentID(identify());
        if (NetaStripPanels() != 1) {
            MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
            log << MSG::WARNING << "more than one eta strip pannel " << NetaStripPanels() << " " << idh->print_to_string(identify())
                << endmsg;
        }
        for (int dbPhi = 1; dbPhi <= NphiStripPanels(); ++dbPhi) {
            for (int gasGap = 1; gasGap <= numberOfLayers(true); ++gasGap) {
                const Amg::Vector3D locP = localStripPanelPos(getDoubletZ(), dbPhi, gasGap);
                const Amg::Translation3D xfp(locP.x(), locP.y(), locP.z());
                Amg::Transform3D trans3D = rotatedGasGap(gasGap)
                                               ? absTransform() * xfp * Amg::AngleAxis3D(180. * CLHEP::deg, Amg::Vector3D::UnitY())
                                               : absTransform() * xfp;

                // surface()
                bool hasSpecialRot = (rotatedGasGap(gasGap) && (!rotatedRpcModule())) || (!rotatedGasGap(gasGap) && (rotatedRpcModule()));
                Amg::RotationMatrix3D muonTRotation(trans3D.rotation());
                if (isMirrored()) muonTRotation = muonTRotation * Amg::AngleAxis3D(180. * CLHEP::deg, Amg::Vector3D::UnitX());
                if (hasSpecialRot) muonTRotation = muonTRotation * Amg::AngleAxis3D(180. * CLHEP::deg, Amg::Vector3D::UnitY());

                Amg::RotationMatrix3D surfaceTRotation;
                surfaceTRotation.col(0) = muonTRotation.col(1);
                surfaceTRotation.col(1) = muonTRotation.col(2);
                surfaceTRotation.col(2) = muonTRotation.col(0);

                for (bool measphi  : { true, false}) {
                    Identifier id = idh->channelID(parentID, getDoubletZ(), dbPhi, gasGap, measphi  , 1);

                    Amg::Transform3D trans(surfaceTRotation);
                    if (measphi   == 0) trans *= Amg::AngleAxis3D(M_PI / 2., Amg::Vector3D::UnitZ());
                    trans.pretranslate(trans3D.translation());

                    m_surfaceData->m_layerTransforms.push_back(trans);
                    m_surfaceData->m_layerSurfaces.emplace_back(std::make_unique<Trk::PlaneSurface>(*this, id));

                    if (measphi) {
                        m_surfaceData->m_layerCenters.push_back(m_surfaceData->m_layerTransforms.back() * Amg::Vector3D::Zero());
                        m_surfaceData->m_layerNormals.emplace_back(m_surfaceData->m_layerTransforms.back().linear() *
                                                                Amg::Vector3D::UnitZ());
                    }
                }
            }
        }

        // phi at index=0
        m_surfaceData->m_surfBounds.emplace_back(
            std::make_unique<Trk::RectangleBounds>((m_Ssize / m_nphistrippanels) / 2., (m_Zsize / m_netastrippanels) / 2.));
        m_surfaceData->m_surfBounds.emplace_back(
            std::make_unique<Trk::RectangleBounds>((m_Zsize / m_netastrippanels) / 2., (m_Ssize / m_nphistrippanels) / 2.));
    }

    bool RpcReadoutElement::containsId(const Identifier& id) const {
        const RpcIdHelper* idh = manager()->rpcIdHelper();
        int doubletR = idh->doubletR(id);
        if (doubletR != getDoubletR()) return false;

        int doubletZ = idh->doubletZ(id);
        if (doubletZ != getDoubletZ()) { return false; }

        int doubletPhi = idh->doubletPhi(id);
        if (doubletPhi != getDoubletPhi() && m_nphistrippanels == 1) { return false; }
        if (doubletPhi < 1 || doubletPhi > m_nphistrippanels) {
            if (doubletPhi != 2 || !inTheRibs()) { return false; }
        }

        int gasgap = idh->gasGap(id);
        if (gasgap < 1 || gasgap > m_nlayers) return false;

        bool measphi   = idh->measuresPhi(id);
        int strip = idh->strip(id);
        if (!measphi) {
            if (strip < 1 || strip > NetaStrips()) return false;
        /// Measures phi is true so we can directly check the strip range
        } else if (strip < 1 || strip > NphiStrips()) return false;       

        return true;
    }

    bool RpcReadoutElement::inTheRibs() const {
        return ((getStationName().substr(0, 3) == "BMS") && (getTechnologyName() == "RPC07" || getTechnologyName() == "RPC08"));
    }

    int RpcReadoutElement::surfaceHash(int dbPhi, int gasGap, bool measphi  ) const {
        // if there is only one doublet phi we should always use one in the hash calculation
        if (m_nphistrippanels == 1) dbPhi = 1;
        if (dbPhi > NphiStripPanels() || gasGap > numberOfLayers(true)) {
            MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
            log << MSG::WARNING << " surfaceHash: identifier out of range dbphi " << dbPhi << " max " << NphiStripPanels() << " ch dbphi "
                << getDoubletPhi() << " gp " << gasGap << " max " << numberOfLayers() << endmsg;
            return -1;
        }
        return (dbPhi - 1) * (2 * NphiStripPanels()) + 2 * (gasGap - 1) + (measphi   ? 0 : 1);
    }

    int RpcReadoutElement::layerHash(int dbPhi, int gasGap) const {
        if (m_nphistrippanels == 1) dbPhi = 1;

        if (dbPhi > NphiStripPanels() || gasGap > numberOfLayers(true)) {
            MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
            log << MSG::WARNING << " layerHash: identifier out of range dbphi " << dbPhi << " max " << NphiStripPanels() << " ch dbphi "
                << getDoubletPhi() << " gp " << gasGap << " max " << numberOfLayers() << endmsg;
            return -1;
        }
        return (dbPhi - 1) * (NphiStripPanels()) + (gasGap - 1);
    }

    const MuonStripDesign* RpcReadoutElement::getDesign(const Identifier& id) const {
        int phipanel = m_nphistrippanels == 1 ? 1 : manager()->rpcIdHelper()->doubletPhi(id);
        if (phipanel > (int)m_phiDesigns.size()) {
            MsgStream log(Athena::getMessageSvc(), "RpcReadoutElement");
            log << MSG::WARNING << " bad identifier, no MuonStripDesign found " << endmsg;
            return nullptr;
        }
        return manager()->rpcIdHelper()->measuresPhi(id) ? &m_phiDesigns[phipanel - 1] : &m_etaDesigns[phipanel - 1];
    }

}  // namespace MuonGM
