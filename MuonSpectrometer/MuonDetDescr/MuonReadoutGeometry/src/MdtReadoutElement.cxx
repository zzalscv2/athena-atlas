/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 The Mdt detector = a multilayer = MDT in amdb
 ----------------------------------------------------
 ***************************************************************************/

#include "MuonReadoutGeometry/MdtReadoutElement.h"

#include <TString.h>  // for Form

#include <limits>
#include <utility>

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelUtilities/GeoGetIds.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "MuonAlignmentData/BLinePar.h"
#include "MuonIdHelpers/MdtIdHelper.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "TrkSurfaces/CylinderBounds.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"

// From Dan Levin: MDT
// linear density of wire: lambda=wireLinearDensity=19.3 [gm/cm^3] * PI*
//(25 *10^-4 )^2 [CLHEP::cm^2] = 378.954 microgram/CLHEP::cm
// From Dan Levin: MDT
// wireTen=350 for most chambers,  285 gm for some NIKHEF chambers (BOL ?),

#define verbose_Bline false
// Typical b-line par  values
// m_bz = 0.01;   // 10 microns
// m_bp = 0.1;    // 100 microns
// m_bn = 0.1;    // 100 microns
// m_sp = 0.001;  // 1 micron
// m_sn = 0.001;  // 1 micron
// m_tw = 0.1;    // 100 microns
// m_pg = 0.1;    // 100 microns
// m_tr = 0.1;    // 100 microns
// m_eg = 1.0e-4; // 100 ppm
// m_ep = 1.0e-5; // 10 ppm
// m_en = 1.0e-5; // 10 ppm

namespace {
    // the tube number of a tube in a tubeLayer in encoded in the GeoSerialIdentifier (modulo maxNTubesPerLayer)
    static constexpr unsigned int const maxNTubesPerLayer = MdtIdHelper::maxNTubesPerLayer;
    static constexpr double linearDensity = 378.954;
}  // namespace

namespace MuonGM {

    MdtReadoutElement::MdtReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, MuonDetectorManager* mgr) :
        MuonReadoutElement(pv, mgr, Trk::DetectorElemType::Mdt) {
        // get the setting of the caching flag from the manager
        setCachingFlag(mgr->cachingFlag());

        m_inBarrel = stName[0]== 'B';
       
        setStationName(stName);
    }
      bool MdtReadoutElement::barrel() const {
        return m_inBarrel;
    }

    bool MdtReadoutElement::endcap() const { return !barrel(); }


    bool MdtReadoutElement::getWireFirstLocalCoordAlongZ(int tubeLayer, double& coord) const {
        coord = -9999.;
        if (tubeLayer > getNLayers() || tubeLayer < 1) return false;
        coord = m_firstwire_x[tubeLayer - 1];
        return true;
    }
    bool MdtReadoutElement::getWireFirstLocalCoordAlongR(int tubeLayer, double& coord) const {
        coord = -9999.;
        if (tubeLayer > getNLayers() || tubeLayer < 1) return false;
        coord = m_firstwire_y[tubeLayer - 1];
        return true;
    }

    double MdtReadoutElement::innerTubeRadius() const { return m_innerRadius; }
    double MdtReadoutElement::outerTubeRadius() const { return m_innerRadius + m_tubeWallThickness; }

    void MdtReadoutElement::geoInitDone() {
        m_tubeGeo.resize(m_nlayers * m_ntubesperlayer);
        m_deformTransf.resize(m_nlayers * m_ntubesperlayer);
        m_tubeSurfaces.resize(m_nlayers * m_ntubesperlayer);

        int ntot_steps = m_nsteps;
        if (hasCutouts() && manager()->MinimalGeoFlag() == 0) { ntot_steps = m_nlayers * m_ntubesperlayer; }
        m_tubeBounds.resize(ntot_steps);
    }

    double MdtReadoutElement::getTubeLengthForCaching(const int tubeLayer, const int tube) const {
        if (tube <= 0 || (unsigned int)(tube) > maxNTubesPerLayer)
            throw std::runtime_error(
                Form("File: %s, Line: %d\nMdtReadoutElement::getTubeLengthForCaching() - got called with tube=%d which is definitely out "
                     "of range",
                     __FILE__, __LINE__, tube));
        if (tubeLayer <= 0 || tubeLayer > 4)
            throw std::runtime_error(
                Form("File: %s, Line: %d\nMdtReadoutElement::getTubeLengthForCaching() - got called with tubeLayer=%d which is definitely "
                     "out of range",
                     __FILE__, __LINE__, tubeLayer));

        const MdtIdHelper* idh = manager()->mdtIdHelper();
        double nominalTubeLength = 0.;
        if (m_inBarrel)
            nominalTubeLength = m_Ssize;
        else {
            int istep = int((tube - 1) / m_ntubesinastep);
            if (istep < 0 || istep >= m_nsteps) {
                MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
                log << MSG::WARNING << "getTubeLenght for Element named " << getStationName() << " with tech. " << getTechnologyType()
                    << " DEid = " << idh->show_to_string(identify()) << " called with: tubeL, tube " << tubeLayer << " " << tube
                    << "; step " << istep << " out of range 0-" << m_nsteps - 1 << " m_ntubesinastep " << m_ntubesinastep << endmsg;
                istep = 0;
            }
            nominalTubeLength = m_tubelength[istep];
        }

        double tlength = nominalTubeLength;

        if (hasCutouts()) {
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
            if (manager()->MinimalGeoFlag() == 0) {
                if (log.level() <= MSG::VERBOSE)
                    log << MSG::VERBOSE << " MdtReadoutElement " << getStationName() << " stEta " << getStationEta() << " stPhi "
                        << getStationPhi() << "multilayer " << getMultilayer()
                        << " has cutouts, check for real tube length for tubeLayer, tube " << tubeLayer << " " << tube << endmsg;
                PVConstLink cv = getMaterialGeom();  // it is "Multilayer"
                int nGrandchildren = cv->getNChildVols();
                if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << " nchild = " << nGrandchildren << endmsg;
                if (nGrandchildren <= 0) return tlength;
                // child vol 0 is foam; 1 to (nGrandchildren-1) should be tubes
                int ii = (tubeLayer - 1) * m_ntubesperlayer + tube;
                const Identifier id = identify();
                // BIS78 only (the BIS7 of Run1/2 has no cutouts, thus, this block won't be reached)
                if ((idh->stationName(id) == 1 && std::abs(idh->stationEta(id)) == 7)) --ii;
                if (idh->isBMG(identify())) {
                    // usually the tube number corresponds to the child number, however for
                    // BMG chambers full tubes are skipped during the building process
                    // therefore the matching needs to be done via the volume ID
                    int packed_id = tube + maxNTubesPerLayer * tubeLayer;
                    int kk = 0;
                    bool found = false;
                    geoGetIds(
                        [&](int id) {
                            if (!found && id == packed_id) {
                                ii = kk;
                                found = true;
                            }
                            ++kk;
                        },
                        &*cv);
                    if (found && log.level() <= MSG::DEBUG) {
                        log << MSG::DEBUG << " MdtReadoutElement tube match found for BMG - input : tube(" << tube << "), layer("
                            << tubeLayer << ") - output match : tube(" << ii % maxNTubesPerLayer << "), layer(" << ii / maxNTubesPerLayer
                            << ")" << endmsg;
                    }
                }
                if (ii >= nGrandchildren) {
                    log << MSG::WARNING << " MdtReadoutElement " << getStationName() << " stEta " << getStationEta() << " stPhi "
                        << getStationPhi() << " multilayer " << getMultilayer() << " has cutouts, nChild = " << nGrandchildren
                        << " --- getTubeLength is looking for child with index ii=" << ii << " for tubeL and tube = " << tubeLayer << " "
                        << tube << endmsg;
                    log << MSG::WARNING << "returning nominalTubeLength " << endmsg;
                    return tlength;
                }
                if (ii < 0) {
                    log << MSG::WARNING << " MdtReadoutElement " << getStationName() << " stEta " << getStationEta() << " stPhi "
                        << getStationPhi() << " multilayer " << getMultilayer() << " has cutouts, nChild = " << nGrandchildren
                        << " --- getTubeLength is looking for child with index ii=" << ii << " for tubeL and tube = " << tubeLayer << " "
                        << tube << endmsg;
                    log << MSG::WARNING << "returning nominalTubeLength " << endmsg;
                    return tlength;
                }
                PVConstLink physChild = cv->getChildVol(ii);
                const GeoShape* shape = physChild->getLogVol()->getShape();
                if (shape == nullptr) return tlength;
                const GeoTube* theTube = dynamic_cast<const GeoTube*>(shape);
                if (theTube != nullptr)
                    tlength = 2. * theTube->getZHalfLength();
                else
                    log << MSG::WARNING << "PhysChild with index " << ii
                        << " out of (tubelayer-1)*m_ntubesperlayer+tube with tl=" << tubeLayer << " tubes/lay=" << m_ntubesperlayer
                        << " t=" << tube << " for  MdtReadoutElement " << getStationName() << " stEta " << getStationEta() << " stPhi "
                        << getStationPhi() << " ml = " << getMultilayer() << endmsg;
            }
            if (std::abs(tlength - nominalTubeLength) > 0.1) {
                if (log.level() <= MSG::VERBOSE)
                    log << MSG::VERBOSE << "Tube " << tube << " in tubeLayer = " << tubeLayer
                        << " is affected by a cutout: eff. length =  " << tlength << " while nominal = " << nominalTubeLength
                        << " in station " << getStationName() << " at Eta/Phi " << getStationEta() << "/" << getStationPhi() << " ml "
                        << getMultilayer() << endmsg;
            } else {
                if (log.level() <= MSG::VERBOSE)
                    log << MSG::VERBOSE << "Tube " << tube << " in tubeLayer = " << tubeLayer
                        << " is NOT affected by the cutout: eff. length =  " << tlength << " while nominal = " << nominalTubeLength
                        << endmsg;
            }
        }
        return tlength;
    }

    double MdtReadoutElement::tubeLength(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int layer = idh->tubeLayer(id);
        int tube = idh->tube(id);
        return getTubeLength(layer, tube);
    }

    double MdtReadoutElement::distanceFromRO(const Amg::Vector3D& x, const Identifier& id) const {
        // x is given in the global reference frame
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int tubelayer = idh->tubeLayer(id);
        int tube = idh->tube(id);
        Amg::Vector3D cPos = center(tubelayer, tube);
        Amg::Vector3D roPos = ROPos(id);

        Amg::Vector3D c_ro(cPos.x() - roPos.x(), cPos.y() - roPos.y(), cPos.z() - roPos.z());
        Amg::Vector3D x_ro(x.x() - roPos.x(), x.y() - roPos.y(), x.z() - roPos.z());

        double scalprod = c_ro.x() * x_ro.x() + c_ro.y() * x_ro.y() + c_ro.z() * x_ro.z();
        double wlen = getWireLength(tubelayer, tube);
        if (wlen > 10. * CLHEP::mm)
            scalprod = std::abs(2. * scalprod / getWireLength(tubelayer, tube));
        else {
            double xx = x.x();
            double xy = x.y();
            double xz = x.z();
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
            log << MSG::WARNING << " Distance of Point " << xx << " " << xy << " " << xz
                << " from RO side cannot be calculated (=0) since wirelength = " << wlen << endmsg;
            scalprod = 0.;
        }
        return scalprod;
    }

    double MdtReadoutElement::distanceFromRO(const Amg::Vector3D& x, int multilayer, int tubelayer, int tube) const {
        // x is given in the global reference frame
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        Identifier id = idh->channelID(idh->parentID(identify()), multilayer, tubelayer, tube);
        return distanceFromRO(x, id);
    }

    int MdtReadoutElement::isAtReadoutSide(const Amg::Vector3D& GlobalHitPosition, const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int tubel = idh->tubeLayer(id);
        int tube = idh->tube(id);
        double distance = distanceFromRO(GlobalHitPosition, id);
        if (distance < 0) {
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
            log << MSG::WARNING << "MdtReadoutElement::isAtReadoutSide GlobalHitPosition"
                << " appears to be outside the tube volume " << distance << endmsg;
            return 1;
        } else if (distance <= getWireLength(tubel, tube) / 2.)
            return 1;
        else if (distance < getWireLength(tubel, tube))
            return -1;
        else {
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
            log << MSG::WARNING << "MdtReadoutElement::isAtReadoutSide "
                << "GlobalHitPosition appears to be outside the tube volume " << distance << endmsg;
            return -1;
        }
    }
    int MdtReadoutElement::isAtReadoutSide(const Amg::Vector3D& GlobalHitPosition, int ml, int tubel, int tube) const {
        double distance = distanceFromRO(GlobalHitPosition, ml, tubel, tube);
        if (distance < 0) {
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
            log << MSG::WARNING << "isAtReadoutSide() - GlobalHitPosition appears to be outside the tube volume " << distance << endmsg;
            return 1;
        } else if (distance <= getWireLength(tubel, tube) / 2.)
            return 1;
        else if (distance < getWireLength(tubel, tube))
            return -1;
        else {
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
            log << MSG::WARNING << "isAtReadoutSide() - GlobalHitPosition appears to be outside the tube volume " << distance << endmsg;
            return -1;
        }
    }

    double MdtReadoutElement::RODistanceFromTubeCentre(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int ml = idh->multilayer(id);
        int layer = idh->tubeLayer(id);
        int tube = idh->tube(id);
        return RODistanceFromTubeCentre(ml, layer, tube);
    }
    double MdtReadoutElement::signedRODistanceFromTubeCentre(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int ml = idh->multilayer(id);
        int layer = idh->tubeLayer(id);
        int tube = idh->tube(id);
        return signedRODistanceFromTubeCentre(ml, layer, tube);
    }
    double MdtReadoutElement::RODistanceFromTubeCentre(const int multilayer, const int tubelayer, const int tube) const {
        // it is a un-signed quantity:
        if (multilayer != m_multilayer) {
            throw std::runtime_error(Form(
                "File: %s, Line: %d\nMdtReadoutElement::RODistanceFromTubeCentre() - inserted multilayer is not the multilayer of the RE.",
                __FILE__, __LINE__));
        }
        return getWireLength(tubelayer, tube) / 2.;
    }
    double MdtReadoutElement::signedRODistanceFromTubeCentre(const int multilayer, const int tubelayer, const int tube) const {
        // it is a signed quantity:
        // the sign corresponds to the sign of the z coordinate of the RO endplug in the tube
        // reference frame
        if (multilayer != m_multilayer) {
            throw std::runtime_error(
                Form("File: %s, Line: %d\nMdtReadoutElement::signedRODistanceFromTubeCentre() - inserted multilayer is not the multilayer "
                     "of the RE.",
                     __FILE__, __LINE__));
        }

        int amdb_plus_minus1 = 1;
        if (!m_zsignRO_tubeFrame.isValid()) {
            const MdtIdHelper* idh = manager()->mdtIdHelper();
            Identifier id = idh->channelID(idh->parentID(identify()), multilayer, tubelayer, tube);
            const MuonStation* ms = parentMuonStation();
            if (std::abs(ms->xAmdbCRO()) > 10.) {
                Amg::Vector3D tem = Amg::Vector3D(ms->xAmdbCRO(), 0., 0.);
                Amg::Transform3D amdbToGlobal = Amg::CLHEPTransformToEigen(ms->getAmdbLRSToGlobal());
                Amg::Vector3D temGlo = amdbToGlobal * tem;
                Amg::Vector3D ROtubeFrame = nodeform_globalToLocalCoords(temGlo, id);
                if (ROtubeFrame.z() < 0)
                    m_zsignRO_tubeFrame.set(-1);
                else
                    m_zsignRO_tubeFrame.set(1);
            }
        }
        if (!m_zsignRO_tubeFrame
                 .isValid()) {  // if no CRO in a chamber in AMDB (BIS in layout R), use the standard convention for RO-HV side
            int sign = 0;
            if (barrel()) {
                if (sideA()) {
                    if (largeSector())
                        sign = -1;
                    else
                        sign = 1;
                } else {
                    if (largeSector())
                        sign = 1;
                    else
                        sign = -1;
                }
                // a special case is BIS in sector 12
                if (getStationName().substr(0, 3) == "BIS" && getStationPhi() == 6) sign = -sign;
            } else {
                if (sideA()) {
                    if (largeSector())
                        sign = 1;
                    else
                        sign = -1;
                } else {
                    if (largeSector())
                        sign = -1;
                    else
                        sign = 1;
                }
            }
            m_zsignRO_tubeFrame.set(sign);
        }
        amdb_plus_minus1 = *m_zsignRO_tubeFrame.ptr();
        if (amdb_plus_minus1 == 0) {
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
            log << MSG::WARNING << "Unable to get the sign of RO side; signedRODistancefromTubeCenter returns 0" << endmsg;
        }

        return amdb_plus_minus1 * getWireLength(tubelayer, tube) / 2.;
    }

    Amg::Vector3D MdtReadoutElement::tubeFrame_localROPos(const int multilayer, const int tubelayer, const int tube) const {
        Amg::Vector3D Pro(0., 0., signedRODistanceFromTubeCentre(multilayer, tubelayer, tube));
        return Pro;
    }
    Amg::Vector3D MdtReadoutElement::tubeFrame_localROPos(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int ml = idh->multilayer(id);
        int layer = idh->tubeLayer(id);
        int tube = idh->tube(id);
        return tubeFrame_localROPos(ml, layer, tube);
    }
    Amg::Vector3D MdtReadoutElement::localROPos(const int multilayer, const int tubelayer, const int tube) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        Identifier id = idh->channelID(idh->parentID(identify()), multilayer, tubelayer, tube);
        return tubeToMultilayerCoords(tubeFrame_localROPos(multilayer, tubelayer, tube), id);
    }
    Amg::Vector3D MdtReadoutElement::localROPos(const Identifier& id) const { return tubeToMultilayerCoords(tubeFrame_localROPos(id), id); }

    Amg::Vector3D MdtReadoutElement::ROPos(const int multilayer, const int tubelayer, const int tube) const {
        return transform(tubelayer, tube) * tubeFrame_localROPos(multilayer, tubelayer, tube);
    }

    Amg::Vector3D MdtReadoutElement::ROPos(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int ml = idh->multilayer(id);
        int layer = idh->tubeLayer(id);
        int tube = idh->tube(id);
        return ROPos(ml, layer, tube);
    }

    Amg::Vector3D MdtReadoutElement::localTubePos(const int multilayer, const int tubelayer, const int tube) const {
        const Amg::Vector3D idealPos = nodeform_localTubePos(multilayer, tubelayer, tube);
        const Amg::Transform3D toDeform = fromIdealToDeformed(multilayer, tubelayer, tube);
        return toDeform * idealPos;
    }

    Amg::Vector3D MdtReadoutElement::nodeform_localTubePos(const int multilayer, const int tubelayer, const int tube) const {
        int theMultilayer = multilayer;
        if (theMultilayer != m_multilayer) {
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
            log << MSG::WARNING << "asking WRONG QUESTION: nodeform_localTubePos with args. ml/tl/t = " << theMultilayer << "/" << tubelayer
                << "/" << tube << " to MdtReadoutElement " << getStationName() << "/" << getTechnologyName() << " eta/phi "
                << getStationEta() << "/" << getStationPhi() << " multilayer " << getMultilayer() << endmsg;
            log << MSG::WARNING << "will answer for  args:                                  ml/tl/t = " << getMultilayer() << "/"
                << tubelayer << "/" << tube << endmsg;
            theMultilayer = m_multilayer;
        }
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (log.level() <= MSG::VERBOSE)
            log << MSG::VERBOSE << " Computing LocalTubePos for " << getStationName() << "/" << getTechnologyName() << " eta/phi "
                << getStationEta() << "/" << getStationPhi() << " ml/tl/t " << theMultilayer << "/" << tubelayer << "/" << tube << endmsg;
#endif

        double xtube{0.}, ytube{0.}, ztube{0.};
        if (m_inBarrel) {
            xtube = -m_Rsize / 2. + m_firstwire_y[tubelayer - 1];
            ztube = -m_Zsize / 2. + m_firstwire_x[tubelayer - 1] + (tube - 1) * m_tubepitch;
        } else {
            xtube = -m_Zsize / 2. + m_firstwire_y[tubelayer - 1];
            ztube = -m_Rsize / 2. + m_firstwire_x[tubelayer - 1] + (tube - 1) * m_tubepitch;
        }

        if (hasCutouts()) {
            if (manager()->MinimalGeoFlag() == 0) {
#ifndef NDEBUG
                if (log.level() <= MSG::DEBUG)
                    log << MSG::DEBUG << " MdtReadoutElement " << getStationName() << " stEta " << getStationEta() << " stPhi "
                        << getStationPhi() << " has cutouts, check for real position of tubes " << endmsg;
#endif
                PVConstLink cv = getMaterialGeom();  // it is "Multilayer"
                int nGrandchildren = cv->getNChildVols();
                // child vol 0 is foam; 1 to (nGrandchildren-1) should be tubes
                int ii = (tubelayer - 1) * m_ntubesperlayer + tube;
                const Identifier id = identify();
                const MdtIdHelper* idh = manager()->mdtIdHelper();
                // BIS78 only (the BIS7 of Run1/2 has no cutouts, thus, this block won't be reached)
                if ((idh->stationName(id) == 1 && std::abs(idh->stationEta(id)) == 7)) --ii;
                if (manager()->mdtIdHelper()->isBMG(identify())) {
                    // usually the tube number corresponds to the child number, however for
                    // BMG chambers full tubes are skipped during the building process
                    // therefore the matching needs to be done via the volume ID
                    int packed_id = tube + maxNTubesPerLayer * tubelayer;
                    int kk = 0;
                    bool found = false;
                    geoGetIds(
                        [&](int id) {
                            if (!found && id == packed_id) {
                                ii = kk;
                                found = true;
                            }
                            ++kk;
                        },
                        &*cv);
#ifndef NDEBUG
                    if (found && log.level() <= MSG::DEBUG) {
                        log << MSG::DEBUG << " MdtReadoutElement tube match found for BMG - input : tube(" << tube << "), layer("
                            << tubelayer << ") - output match : tube(" << ii % maxNTubesPerLayer << "), layer(" << ii / maxNTubesPerLayer
                            << ")" << endmsg;
                    }
#endif
                }
                GeoTrf::Transform3D tubeTrans = cv->getXToChildVol(ii);
                PVConstLink tv = cv->getChildVol(ii);
                constexpr double maxtol = 0.0000001;

                if (std::abs(xtube - tubeTrans(0, 3)) > maxtol || std::abs(ztube - tubeTrans(2, 3)) > maxtol) {
                    throw std::runtime_error(
                        Form("File: %s, Line: %d\nMdtReadoutElement::nodeform_localTubePos(%d,%d,%d) - mismatch between local from "
                             "tube-id/pitch/cutout tube position (x,y,z=%.3f,%.3f,%.3f) and GeoModel (x,y,z=%.3f,%.3f,%.3f)\nfor "
                             "MdtReadoutElement with stationName=%d (%s), stationEta=%d, stationPhi=%d, multilayer=%d, tubeLayer=%d, "
                             "tube=%d\nThere are %d child volumes & %d tubes expected. There should be %d layers and %d tubes per layer.",
                             __FILE__, __LINE__, theMultilayer, tubelayer, tube, xtube, ytube, ztube, tubeTrans(0, 3), tubeTrans(1, 3),
                             tubeTrans(2, 3), idh->stationName(id), idh->stationNameString(idh->stationName(id)).c_str(),
                             idh->stationEta(id), idh->stationPhi(id), idh->multilayer(id), idh->tubeLayer(id), idh->tube(id),
                             nGrandchildren, m_ntubesperlayer * m_nlayers, m_nlayers, m_ntubesperlayer));
                }
                if (tubeTrans(1, 3) > maxtol) {
#ifndef NDEBUG
                    if (log.level() <= MSG::DEBUG)
                        log << MSG::DEBUG << "This a tube with cutout stName/Eta/Phi/ml/tl/t = " << getStationName() << "/"
                            << getStationEta() << "/" << getStationPhi() << "/" << getMultilayer() << "/" << tubelayer << "/" << tube
                            << endmsg;
#endif
                    if (std::abs(m_cutoutShift - tubeTrans(1, 3)) > maxtol)  // check only for tubes actually shifted
                    {
                        throw std::runtime_error(Form(
                            "File: %s, Line: %d\nMdtReadoutElement::nodeform_localTubePos(%d,%d,%d) - mismatch between local from "
                            "tube-id/pitch/cutout tube position (x,y,z=%.3f,%.3f,%.3f) and GeoModel (x,y,z=%.3f,%.3f,%.3f)\nfor "
                            "MdtReadoutElement with stationName=%d (%s), stationEta=%d, stationPhi=%d, multilayer=%d, tubeLayer=%d, "
                            "tube=%d\nThere are %d child volumes & %d tubes expected. There should be %d layers and %d tubes per layer.",
                            __FILE__, __LINE__, theMultilayer, tubelayer, tube, xtube, m_cutoutShift, ztube, tubeTrans(0, 3),
                            tubeTrans(1, 3), tubeTrans(2, 3), idh->stationName(id), idh->stationNameString(idh->stationName(id)).c_str(),
                            idh->stationEta(id), idh->stationPhi(id), idh->multilayer(id), idh->tubeLayer(id), idh->tube(id),
                            nGrandchildren, m_ntubesperlayer * m_nlayers, m_nlayers, m_ntubesperlayer));
                    }
                }

                Amg::Vector3D x = tubeTrans.translation();
                if (tube > m_ntubesperlayer || tubelayer > m_nlayers) { x = Amg::Vector3D(xtube, ytube, ztube); }
                return x;
            }
        }
        return Amg::Vector3D(xtube, ytube, ztube);
    }

    Amg::Vector3D MdtReadoutElement::localTubePos(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int ml = idh->multilayer(id);
        int layer = idh->tubeLayer(id);
        int tube = idh->tube(id);

        return localTubePos(ml, layer, tube);
    }
    Amg::Vector3D MdtReadoutElement::nodeform_localTubePos(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int ml = idh->multilayer(id);
        int layer = idh->tubeLayer(id);
        int tube = idh->tube(id);

        return nodeform_localTubePos(ml, layer, tube);
    }
    Amg::Vector3D MdtReadoutElement::nodeform_tubePos(int multilayer, int tubelayer, int tube) const {
        if (multilayer != m_multilayer) {
            throw std::runtime_error(
                Form("File: %s, Line: %d\nMdtReadoutElement::nodeform_tubePos() - inserted multilayer is not the multilayer of the RE.",
                     __FILE__, __LINE__));
        }

        const Amg::Vector3D lp = nodeform_localTubePos(multilayer, tubelayer, tube);
        const Amg::Transform3D mdtTrans = transform();

        return mdtTrans * lp;
    }
    Amg::Vector3D MdtReadoutElement::tubePos(int multilayer, int tubelayer, int tube) const {
        if (multilayer != m_multilayer) {
            throw std::runtime_error(
                Form("File: %s, Line: %d\nMdtReadoutElement::tubePos() - inserted multilayer is not the multilayer of the RE.", __FILE__,
                     __LINE__));
        }
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (log.level() <= MSG::VERBOSE) {
            log << MSG::VERBOSE << "in tubePos-- id " << (manager()->mdtIdHelper())->show_to_string(identify())
                << " ml, tl, t = " << multilayer << ", " << tubelayer << ", " << tube << endmsg;
            log << MSG::VERBOSE << " MdtReadoutElement::tubePos(ml,tl,t) going to look for local coord.s" << endmsg;
        }
#endif
        const Amg::Vector3D lp = localTubePos(multilayer, tubelayer, tube);
#ifndef NDEBUG
        if (log.level() <= MSG::VERBOSE)
            log << MSG::VERBOSE << " MdtReadoutElement::tubePos(ml,tl,t) going to look for det transform" << endmsg;
#endif
        const Amg::Transform3D mdtTrans = absTransform();
#ifndef NDEBUG
        if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << " MdtReadoutElement::tubePos(ml,tl,t) got localP and trans " << endmsg;
#endif
        return mdtTrans * lp;
    }
    Amg::Vector3D MdtReadoutElement::nodeform_tubePos(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int ml = idh->multilayer(id);
        int layer = idh->tubeLayer(id);
        int tube = idh->tube(id);
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (log.level() <= MSG::VERBOSE) {
            log << MSG::VERBOSE << " Computing tubePos for     id  = " << idh->show_to_string(id) << endmsg;
            log << MSG::VERBOSE << " in MdtReadoutElement with id  = " << idh->show_to_string(identify()) << endmsg;
        }
#endif
        return nodeform_tubePos(ml, layer, tube);
    }
    Amg::Vector3D MdtReadoutElement::tubePos(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int ml = idh->multilayer(id);
        int layer = idh->tubeLayer(id);
        int tube = idh->tube(id);
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (log.level() <= MSG::VERBOSE) {
            log << MSG::VERBOSE << " Computing tubePos for     id  = " << idh->show_to_string(id) << endmsg;
            log << MSG::VERBOSE << " in MdtReadoutElement with id  = " << idh->show_to_string(identify()) << endmsg;
        }
#endif
        return tubePos(ml, layer, tube);
    }

    Amg::Vector3D MdtReadoutElement::tubeToMultilayerCoords(Amg::Vector3D x, const Identifier& id) const {
        const Amg::Vector3D tp = nodeform_localTubePos(id);
        const Amg::Translation3D xfp(tp.x(), tp.y(), tp.z());
        const Amg::Transform3D toDeform = fromIdealToDeformed(id);
        return toDeform * xfp * Amg::Vector3D(x.x(), -x.z(), x.y());
    }
    Amg::Vector3D MdtReadoutElement::nodeform_tubeToMultilayerCoords(Amg::Vector3D x, const Identifier& id) const {
        const Amg::Vector3D tp = nodeform_localTubePos(id);
        const Amg::Translation3D xfp(tp.x(), tp.y(), tp.z());
        return xfp * Amg::Vector3D(x.x(), -x.z(), x.y());
    }
    Amg::Transform3D MdtReadoutElement::tubeToMultilayerTransf(const Identifier& id) const {
        const Amg::Vector3D tp = nodeform_localTubePos(id);
        const Amg::Translation3D xfp(tp.x(), tp.y(), tp.z());
        const Amg::Transform3D toDeform = fromIdealToDeformed(id);
        return toDeform * xfp * Amg::AngleAxis3D(90. * CLHEP::deg, Amg::Vector3D(1., 0., 0.));
    }
    Amg::Transform3D MdtReadoutElement::nodeform_tubeToMultilayerTransf(const Identifier& id) const {
        const Amg::Vector3D tp = nodeform_localTubePos(id);
        const Amg::Translation3D xfp(tp.x(), tp.y(), tp.z());
        return xfp * Amg::AngleAxis3D(90. * CLHEP::deg, Amg::Vector3D(1., 0., 0.));
    }

    Amg::Vector3D MdtReadoutElement::multilayerToTubeCoords(const Amg::Vector3D& x, const Identifier& id) const {
        const Amg::Vector3D tp = nodeform_localTubePos(id);
        const Amg::Translation3D xfp(-tp.x(), -tp.y(), -tp.z());
        const Amg::Transform3D fromDeform = fromIdealToDeformed(id).inverse();
        return Amg::AngleAxis3D(-90. * CLHEP::deg, Amg::Vector3D(1., 0., 0.)) * xfp * fromDeform * x;
    }

    Amg::Vector3D MdtReadoutElement::nodeform_multilayerToTubeCoords(const Amg::Vector3D& x, const Identifier& id) const {
        const Amg::Vector3D tp = nodeform_localTubePos(id);
        const Amg::Translation3D xfp(-tp.x(), -tp.y(), -tp.z());
        return Amg::AngleAxis3D(-90. * CLHEP::deg, Amg::Vector3D(1., 0., 0.)) * xfp * x;
    }

    Amg::Transform3D MdtReadoutElement::multilayerToTubeTransf(const Identifier& id) const {
        const Amg::Vector3D tp = nodeform_localTubePos(id);
        const Amg::Translation3D xfp(-tp.x(), -tp.y(), -tp.z());
        const Amg::Transform3D fromDeform = fromIdealToDeformed(id).inverse();
        return Amg::AngleAxis3D(-90. * CLHEP::deg, Amg::Vector3D(1., 0., 0.)) * xfp * fromDeform;
    }
    Amg::Transform3D MdtReadoutElement::nodeform_multilayerToTubeTransf(const Identifier& id) const {
        const Amg::Vector3D tp = nodeform_localTubePos(id);
        const Amg::Translation3D xfp(-tp.x(), -tp.y(), -tp.z());
        return Amg::AngleAxis3D(-90. * CLHEP::deg, Amg::Vector3D(1., 0., 0.)) * xfp;
    }

    Amg::Vector3D MdtReadoutElement::localToGlobalCoords(const Amg::Vector3D& x, const Identifier& id) const { return transform(id) * x; }

#if defined(FLATTEN) && defined(__GNUC__)
    // We compile this package with optimization, even in debug builds; otherwise,
    // the heavy use of Eigen makes it too slow.  However, from here we may call
    // to out-of-line Eigen code that is linked from other DSOs; in that case,
    // it would not be optimized.  Avoid this by forcing all Eigen code
    // to be inlined here if possible.
    __attribute__((flatten))
#endif

    Amg::Transform3D
    MdtReadoutElement::globalTransform(const Amg::Vector3D& tubePos, const Amg::Transform3D& toDeform) const {
        const Amg::Translation3D xfp(tubePos.x(), tubePos.y(), tubePos.z());
        return transform() * toDeform * xfp * Amg::AngleAxis3D(90. * CLHEP::deg, Amg::Vector3D(1., 0., 0.));
    }

    Amg::Transform3D MdtReadoutElement::globalTransform(const Amg::Vector3D& tubePos) const {
        const Amg::Translation3D xfp(tubePos.x(), tubePos.y(), tubePos.z());
        return transform() * xfp * Amg::AngleAxis3D(90. * CLHEP::deg, Amg::Vector3D(1., 0., 0.));
    }

    Amg::Vector3D MdtReadoutElement::nodeform_localToGlobalCoords(const Amg::Vector3D& x, const Identifier& id) const {
        const Amg::Vector3D tp = nodeform_localTubePos(id);
        const Amg::Translation3D xfp(tp.x(), tp.y(), tp.z());
        return transform() * xfp * Amg::Vector3D(x.x(), -x.z(), x.y());
    }
    Amg::Transform3D MdtReadoutElement::localToGlobalTransf(const Identifier& id) const {
        // a point at z=-L/2 goes at y=+L/2
        return globalTransform(nodeform_localTubePos(id), fromIdealToDeformed(id));
    }
    Amg::Transform3D MdtReadoutElement::localToGlobalTransf(int tubeLayer, int tube) const {
        // a point at z=-L/2 goes at y=+L/2
        return globalTransform(nodeform_localTubePos(getMultilayer(), tubeLayer, tube),
                               fromIdealToDeformed(getMultilayer(), tubeLayer, tube));
    }
    Amg::Transform3D MdtReadoutElement::nodeform_localToGlobalTransf(const Identifier& id) const {
        return globalTransform(nodeform_localTubePos(id));
    }

    Amg::Transform3D MdtReadoutElement::globalToLocalTransf(const Identifier& id) const {
        Amg::Transform3D mytransf = transform(id).inverse();
        return mytransf;
    }

    Amg::Transform3D MdtReadoutElement::nodeform_globalToLocalTransf(const Identifier& id) const {
        Amg::Transform3D mytransf = nodeform_localToGlobalTransf(id).inverse();
        return mytransf;
    }

    Amg::Vector3D MdtReadoutElement::globalToLocalCoords(const Amg::Vector3D& x, const Identifier& id) const {
        const Amg::Transform3D mytransf = globalToLocalTransf(id);
        Amg::Vector3D xx = mytransf * x;
        return xx;
    }
    Amg::Vector3D MdtReadoutElement::nodeform_globalToLocalCoords(const Amg::Vector3D& x, const Identifier& id) const {
        const Amg::Transform3D mytransf = nodeform_globalToLocalTransf(id);
        Amg::Vector3D xx = mytransf * x;
        return xx;
    }
    Amg::Vector3D MdtReadoutElement::AmdbLRStubePos(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int ml = idh->multilayer(id);
        int layer = idh->tubeLayer(id);
        int tube = idh->tube(id);
        return AmdbLRStubePos(ml, layer, tube);
    }

    Amg::Vector3D MdtReadoutElement::AmdbLRStubePos(int multilayer, int tubelayer, int tube) const {
        if (multilayer != m_multilayer) {
            throw std::runtime_error(
                Form("File: %s, Line: %d\nMdtReadoutElement::AmdbLRStubePos() - inserted multilayer is not the multilayer of the RE.",
                     __FILE__, __LINE__));
        }
        const Amg::Vector3D tp = localTubePos(multilayer, tubelayer, tube);

        // Have the position in local(GM) MDT coords.
        // Need to translate to Amdb local (szt) Station Coord.s

        // go from local(GM) MDT  to local(GM) MuonStation
        const MuonStation* ms = parentMuonStation();
        // the method in the following 3 lines gives the same result of Amg::Vector3D tpNativeMuonStation = toParentStation()*tp;
        Amg::Vector3D tpNativeMuonStation = toParentStation() * tp;

        Amg::Transform3D xf = Amg::CLHEPTransformToEigen(*ms->getNativeToAmdbLRS());
        return xf * tpNativeMuonStation;
    }

    const Amg::Transform3D& MdtReadoutElement::fromIdealToDeformed(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int ml = idh->multilayer(id);
        int layer = idh->tubeLayer(id);
        int tube = idh->tube(id);
        return fromIdealToDeformed(ml, layer, tube);
    }

    double MdtReadoutElement::getNominalTubeLengthWoCutouts(const int tubeLayer, const int tube) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        if (m_inBarrel)
            return m_Ssize;
        else {
            int istep = int((tube - 1) / m_ntubesinastep);
            if (istep < 0 || istep >= m_nsteps) {
                MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
                log << MSG::WARNING << "getNominalTubeLengthWoCutouts for Element named " << getStationName() << " with tech. "
                    << getTechnologyType() << " DEid = " << idh->show_to_string(identify()) << " called with: tubeL, tube " << tubeLayer
                    << " " << tube << "; step " << istep << " out of range 0-" << m_nsteps - 1 << " m_ntubesinastep " << m_ntubesinastep
                    << endmsg;
                istep = 0;
            }
            return m_tubelength[istep];
        }
    }

    Amg::Vector3D MdtReadoutElement::localNominalTubePosWoCutouts(const int tubelayer, const int tube) const {
        double xtube = 0.;
        double ytube = 0.;
        double ztube = 0.;
        if (m_inBarrel) {
            xtube = -m_Rsize / 2. + m_firstwire_y[tubelayer - 1];
            ztube = -m_Zsize / 2. + m_firstwire_x[tubelayer - 1] + (tube - 1) * m_tubepitch;
        } else {
            xtube = -m_Zsize / 2. + m_firstwire_y[tubelayer - 1];
            ztube = -m_Rsize / 2. + m_firstwire_x[tubelayer - 1] + (tube - 1) * m_tubepitch;
        }
        return Amg::Vector3D(xtube, ytube, ztube);
    }

    const Amg::Transform3D& MdtReadoutElement::fromIdealToDeformed(const int multilayer, const int tubelayer, const int tube) const {
        size_t itube = (tubelayer - 1) * m_ntubesperlayer + tube - 1;
        if (itube >= m_deformTransf.size()) {
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
            log << MSG::WARNING << " geoInfo called with tubeLayer or tube out of range in chamber "
                << manager()->mdtIdHelper()->print_to_string(identify()) << " : layer " << tubelayer << " max " << m_nlayers << " tube " << tube
                << " max " << m_ntubesperlayer << " will compute deformation for first tube in this chamber" << endmsg;
            log << MSG::WARNING << "Please run in DEBUG mode to get extra diagnostic" << endmsg;
            itube = 0;
        }

        const CxxUtils::CachedUniquePtr<Amg::Transform3D>& ptr = m_deformTransf.at(itube);
        if (!ptr) {
            Amg::Transform3D trans = deformedTransform(multilayer, tubelayer, tube);
            ptr.set(std::make_unique<Amg::Transform3D>(trans));
            if (!m_haveDeformTransf) m_haveDeformTransf = true;
        }
        return *ptr;
    }

#if defined(FLATTEN) && defined(__GNUC__)
    // We compile this package with optimization, even in debug builds; otherwise,
    // the heavy use of Eigen makes it too slow.  However, from here we may call
    // to out-of-line Eigen code that is linked from other DSOs; in that case,
    // it would not be optimized.  Avoid this by forcing all Eigen code
    // to be inlined here if possible.
    __attribute__((flatten))
#endif
    Amg::Transform3D
    MdtReadoutElement::deformedTransform(int multilayer, int tubelayer, int tube) const {
        const MuonStation* ms = parentMuonStation();
        HepGeom::Point3D<double> fpp = ms->getUpdatedBlineFixedPointInAmdbLRS();
        Amg::Vector3D fixedPoint(fpp.x(), fpp.y(), fpp.z());
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        const MdtIdHelper* idh = manager()->mdtIdHelper();
#endif
        if (!ms->hasBLines() && !ms->hasMdtAsBuiltParams()) {
            return Amg::Transform3D::Identity();
        }

        int ntot_tubes = m_nlayers * m_ntubesperlayer;
        int itube = (tubelayer - 1) * m_ntubesperlayer + tube - 1;
        if (itube >= ntot_tubes) {
#ifdef NDEBUG
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
#endif
            log << MSG::WARNING << "global index for tubelayer/tube =  " << tubelayer << "/" << tube << " is " << itube
                << " >=ntot_tubes =" << ntot_tubes << " RESETTING global index to 0" << endmsg;
            itube = 0;
        }

        // Chamber parameters
        double width_narrow = m_Ssize;
        double width_wide = m_LongSsize;
        double height = m_inBarrel ? ms->ZsizeMdtStation() : ms->RsizeMdtStation();
        double thickness = m_inBarrel ? ms->RsizeMdtStation() : ms->ZsizeMdtStation();
#ifndef NDEBUG
        double heightML = m_inBarrel ? m_Zsize : m_Rsize;
        double thicknessML = m_inBarrel ? m_Rsize : m_Zsize;
        if (std::abs(height - heightML) > 10.) {
            if (log.level() <= MSG::DEBUG)
                log << MSG::DEBUG << "RE " << idh->show_to_string(identify())
                    << "Different ML and MDTStation length in the precision coord.direction  ---  MultiLayerHeight, MDTstationHeigh "
                    << heightML << " " << height << " MultiLayerThickness, MDTstationThickness " << thicknessML << " " << thickness
                    << endmsg;
        }
#endif
        // Chamber dimension in Z is defined with extraneous glue width. Correct for it
        double glue = (tubePitch() - 2. * outerTubeRadius());
        height -= glue;

        // Calculate transformation from native to AMDB.
        Amg::Transform3D toAMDB = Amg::CLHEPTransformToEigen(*ms->getNativeToAmdbLRS()) * toParentStation();
        Amg::Transform3D fromAMDB = toAMDB.inverse();

        // Get positions of the wire center and end without deformations
        Amg::Vector3D pt_center = localNominalTubePosWoCutouts(tubelayer, tube);
        double halftubelen = 0.5 * getNominalTubeLengthWoCutouts(tubelayer, tube);

        // Compute tube ends in AMDB coordinates
        Amg::Vector3D pt_end1 = toAMDB * pt_center + Amg::Vector3D(+halftubelen, 0., 0.);  // s>0 side
        Amg::Vector3D pt_end2 = toAMDB * pt_center + Amg::Vector3D(-halftubelen, 0., 0.);  // s>0 side

        Amg::Vector3D pt_end1_new = pt_end1;
        Amg::Vector3D pt_end2_new = pt_end2;

        // if there are as built parameters ... apply them here
        if (ms->hasMdtAsBuiltParams()) {
            wireEndpointsAsBuilt(pt_end1_new, pt_end2_new, multilayer, tubelayer, tube);
        }

        // if there are deformation parameters ... apply them here
        if (ms->hasBLines()) {
            using Parameter = BLinePar::Parameter;
            const double bz = m_BLinePar->getParameter(Parameter::bz);
            const double bp = m_BLinePar->getParameter(Parameter::bp);
            const double bn = m_BLinePar->getParameter(Parameter::bn);
            const double sp = m_BLinePar->getParameter(Parameter::sp);
            const double sn = m_BLinePar->getParameter(Parameter::sn);
            const double tw = m_BLinePar->getParameter(Parameter::tw);
            const double pg = m_BLinePar->getParameter(Parameter::pg);
            const double tr = m_BLinePar->getParameter(Parameter::tr);
            const double eg = m_BLinePar->getParameter(Parameter::eg);
            const double ep = m_BLinePar->getParameter(Parameter::ep);
            const double en = m_BLinePar->getParameter(Parameter::en);

            // Get positions after the deformations applied
            // first wire end point
            pt_end1_new = posOnDefChamWire(pt_end1_new, width_narrow, width_wide, height, thickness, bz, bp, bn, sp, sn, tw, pg, tr, eg, ep,
                                           en, fixedPoint);
            // pt_end1_new = fromAMDB * pt_end1_new;

            // second wire end point
            pt_end2_new = posOnDefChamWire(pt_end2_new, width_narrow, width_wide, height, thickness, bz, bp, bn, sp, sn, tw, pg, tr, eg, ep,
                                           en, fixedPoint);
            // pt_end2_new = fromAMDB * pt_end2_new;
        }

        // Switch tube ends back to MGM coordinates
        pt_end1 = fromAMDB * pt_end1;
        pt_end2 = fromAMDB * pt_end2;
        pt_end1_new = fromAMDB * pt_end1_new;
        pt_end2_new = fromAMDB * pt_end2_new;

        // Calculate deformation. Make sure that the wire length stays the same.
        // Code in positionOnDeformedChamber does not provide this by default.
        // Break transformation into translation of the wire center and the rotation of the wire
        // Move to the coordinate system originated at the wire center, then rotate the wire, then
        // move wire center to the new position
        const Amg::Vector3D pt_center_new = 0.5 * (pt_end1_new + pt_end2_new);
        const Amg::Translation3D to_center(-pt_center.x(), -pt_center.y(), -pt_center.z());
        const Amg::Translation3D from_center(pt_center_new.x(), pt_center_new.y(), pt_center_new.z());
        const Amg::Vector3D old_direction(pt_end2.x() - pt_end1.x(), pt_end2.y() - pt_end1.y(), pt_end2.z() - pt_end1.z());
        const Amg::Vector3D new_direction(pt_end2_new.x() - pt_end1_new.x(), pt_end2_new.y() - pt_end1_new.y(),
                                          pt_end2_new.z() - pt_end1_new.z());
        const Amg::Vector3D rotation_vector = old_direction.unit().cross(new_direction.unit());
        Amg::Vector3D rotation_vector_unit(1., 0., 0.);
        if (rotation_vector.mag() > 10. * std::numeric_limits<double>::epsilon()) rotation_vector_unit = rotation_vector.unit();
        const Amg::AngleAxis3D wire_rotation(asin(rotation_vector.mag()), rotation_vector_unit);

        return Amg::Transform3D(from_center * wire_rotation * to_center);
    }

    Amg::Vector3D MdtReadoutElement::posOnDefChamWire(const Amg::Vector3D& locAMDBPos, const BLinePar* bLine,
                                                      const Amg::Vector3D& fixedPoint) const {
        double height = m_inBarrel ? m_Zsize : m_Rsize;
        double thickness = m_inBarrel ? m_Rsize : m_Zsize;
        using Parameter = BLinePar::Parameter;
        return posOnDefChamWire(locAMDBPos, m_Ssize, m_LongSsize, height, thickness, 
                                bLine->getParameter(Parameter::bz), bLine->getParameter(Parameter::bp), 
                                bLine->getParameter(Parameter::bn), bLine->getParameter(Parameter::sp),
                                bLine->getParameter(Parameter::sn), bLine->getParameter(Parameter::tw), 
                                bLine->getParameter(Parameter::pg), bLine->getParameter(Parameter::tr), 
                                bLine->getParameter(Parameter::eg), bLine->getParameter(Parameter::ep), 
                                bLine->getParameter(Parameter::en), fixedPoint);
    }

    //   //Correspondence to AMDB parameters -TBM
    //   //cpl_x is tr "trapezoidal effect"
    //   //cpl_y is sy "Longbeam vertical sagitta"
    //   //cpl_z is sz "Longbeam horizontal sagitta"
    //   //sag_x is ?? "shearing deformation"
    //   //sag_y is so,sv "RO crossplate sag, HV crossplate sag"
    //   //sag_z is ?? "different long-beam bow for short/long side"
    //   //the_g is tw "common twist angle for HV and RO side"
    //   //the_c is tw "torsion along tube axis"
    //   //the_m is tw "torsion along tube axis"
    //   //tem_g is T, "temperature"
    //   //tem_c is ev "HV elongation"
    //   //tem_m is eo "RO elongation"

    //   /*
    //     CPM: mask-side cross plate (=readout side in endcap)
    //     CPC: CCD-side cross plate (=HV side in endcap)
    //     CPL: lens cross plate (=central cross plate)

    //     note that nearly all deformation parameter names are meaningless
    //   */

    Amg::Vector3D MdtReadoutElement::posOnDefChamWire(const Amg::Vector3D& locAMDBPos, double width_narrow, double width_wide,
                                                      double height, double thickness, double /*bz*/, double /*bp*/, double /*bn*/,
                                                      double sp, double sn, double tw, double /*pg*/, double /*tr*/, double eg, double ep,
                                                      double en, const Amg::Vector3D& fixedPoint) const {
        // S.Spagnolo Feb.6, 2011, modified by P.F Giraud, 2015-01-17
        // This version does not implement deformations modifying only the second
        // coordinate, or leaving the end-plug positions unchanged.
        // This version should be called only with the coordinates of the end-plugs
        // as argument, as is done in fromIdealToDeformed

        // MDT deformations like tube bow: bz,bp,bn bend the tube while the wire endpoints are not affected
        //                                 => the wire keeps it's nominal straight line trajectory but it is not concentric to the tube
        //                                 ==> in this function bz, bp and bn are ignored or set to 0
        // MDT deformations that extend/contract the wire longitudinally (while keeping it straight):
        //                                 delta_s from eg and tr are irrelevant for the tube geometry
        //                                 and for the wire trajectory => set to 0 here
        //                                 (should be applied as a correction to the
        //                                 tube lenght => tube surface bounds
        //                                 =++>>>> effect in tracking just through the gravitational sagging TOTALLY NEGLIGIBLE=> ignore)
        // pg is irrelevant for tracking purposes and (at least for the endcaps) is applies to the internal bars only, not to the tubes !!!
        //                                 =++>>>> IGNORE IT
        // ep,en: bend the tube by moving (differently) the endplugs ===> the wire straight trajectory is moved w.r.t. the nominal one
        //                                 in addition the tubes keep their nominal position at the center => the wire is not concentric to
        //                                 the tube delta_s from ep,en must also be considered for the implementation of the realistic tube
        //                                 trajectory induced by ep,en
        // tw,sp,sn,pg (for deltaT and deltaZ) are geometrical effects, that impact on tracking and keep the wire straight.

        Amg::Vector3D deformPos(locAMDBPos);

        // NOTE s0,z0,t0 are the coord. in the amdb frame of this point: the origin of the frame can be different than the fixed point for
        // deformations s0mdt,z0mdt,t0mdt
        //    (always equal to the point at lowest t,z and s=0 of the MDT stack)
        double s0 = locAMDBPos.x();
        double z0 = locAMDBPos.y();
        double t0 = locAMDBPos.z();
#ifdef TESTBLINES
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        log << MSG::WARNING << "** In posOnDefChamWire - width_narrow, width_wide, length, thickness, " << width_narrow << " " << width_wide
            << " " << height << " " << thickness << " " << endmsg;
        log << MSG::WARNING << "** In posOnDefChamWire - going to correct for B-line the position of Point at " << s0 << " " << z0 << " "
            << t0 << " in the amdb-szt frame" << endmsg;
#endif

        double s0mdt = s0;  // always I think !
        if (std::abs(fixedPoint.x()) > 0.01) s0mdt = s0 - fixedPoint.x();
        double z0mdt = z0;  // unless in the D section of this station there's a dy diff. from 0 for the innermost MDT multilayer (sometimes
                            // in the barrel)
        if (std::abs(fixedPoint.y()) > 0.01) z0mdt = z0 - fixedPoint.y();
        double t0mdt =
            t0;  // unless in the D section of this station there's a dz diff. from 0 for the innermost MDT multilayer (often in barrel)
        if (std::abs(fixedPoint.z()) > 0.01) t0mdt = t0 - fixedPoint.z();
        if (z0mdt < 0 || t0mdt < 0) {
#ifndef TESTBLINES
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
#endif
            log << MSG::WARNING
                << "posOnDefChamWire: correcting the local position of a point outside the mdt station (2 multilayers) volume -- RE "
                << manager()->mdtIdHelper()->show_to_string(identify()) << " local point: szt=" << s0 << " " << z0 << " " << t0
                << " fixedPoint " << fixedPoint << endmsg;
        }
#ifdef TESTBLINES
        log << MSG::WARNING << "** In posOnDefChamWire - correct for offset of B-line fixed point " << s0mdt << " " << z0mdt << " " << t0mdt
            << " " << endmsg;
#endif

        double ds = 0.;
        double dz = 0.;
        double dt = 0.;

        double width_actual = width_narrow + (width_wide - width_narrow) * (z0mdt / height);
        double s_rel = s0mdt / (width_actual / 2.);
        double z_rel = (z0mdt - height / 2.) / (height / 2.);
        double t_rel = (t0mdt - thickness / 2.) / (thickness / 2.);
#ifdef TESTBLINES
        log << MSG::WARNING << "** In posOnDefChamWire - width_actual, s_rel, z_rel, t_rel  " << width_actual << " " << s_rel << " "
            << z_rel << " " << t_rel << endmsg;
#endif

        // sp, sn - cross plate sag out of plane
        if ((sp != 0) || (sn != 0)) {
            double ztmp = z_rel * z_rel - 1;
            dt += 0.5 * (sp + sn) * ztmp + 0.5 * (sp - sn) * ztmp * s_rel;
        }

        // tw     - twist
        if (tw != 0) {
            dt -= tw * s_rel * z_rel;
            dz += tw * s_rel * t_rel * thickness / height;
#ifdef TESTBLINES
            log << MSG::WARNING << "** In posOnDefChamWire: tw=" << tw << " dt, dz " << dt << " " << dz << endmsg;
#endif
        }

        // eg     - global expansion
        if (eg != 0) {
            double egppm = eg / 1000.;
            ds += 0.;
            dz += z0mdt * egppm;
            dt += t0mdt * egppm;
        }

        // ep, en - local expansion
        //
        // Imporant note: the chamber height and length, as they denoted in Christoph's talk,
        // correspond to thickness and height parameters of this function;
        //

        if ((ep != 0) || (en != 0)) {
            ep = ep / 1000.;
            en = en / 1000.;
            double phi = 0.5 * (ep + en) * s_rel * s_rel + 0.5 * (ep - en) * s_rel;
            double localDt = phi * (t0mdt - thickness / 2.);
            double localDz = phi * (z0mdt - height / 2.);
            dt += localDt;
            dz += localDz;
        }

#ifdef TESTBLINES
        log << MSG::WARNING << "posOnDefChamStraighWire: ds,z,t = " << ds << " " << dz << " " << dt << endmsg;
#endif

        deformPos[0] = s0 + ds;
        deformPos[1] = z0 + dz;
        deformPos[2] = t0 + dt;

        return deformPos;
    }

// Function to apply AsBuilt parameter correction to wire center and end position
// For definitions of AsBuilt parameters see Florian Bauer's talk:
// http://atlas-muon-align.web.cern.ch/atlas-muon-align/endplug/asbuilt.pdf
#if defined(FLATTEN) && defined(__GNUC__)
    // We compile this package with optimization, even in debug builds; otherwise,
    // the heavy use of Eigen makes it too slow.  However, from here we may call
    // to out-of-line Eigen code that is linked from other DSOs; in that case,
    // it would not be optimized.  Avoid this by forcing all Eigen code
    // to be inlined here if possible.
    __attribute__((flatten))
#endif
    void
    MdtReadoutElement::wireEndpointsAsBuilt(Amg::Vector3D& locAMDBWireEndP, Amg::Vector3D& locAMDBWireEndN, const int multilayer,
                                            const int tubelayer, const int tube) const {
        const MdtAsBuiltPar* params = parentMuonStation()->getMdtAsBuiltParams();
        if (!params) {
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
            log << MSG::WARNING << "Cannot find Mdt AsBuilt parameters for chamber " << getStationType() << ", eta " << getStationEta()
                << ", phi " << getStationPhi() << endmsg;
            return;
        }
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (log.level() <= MSG::VERBOSE)
            log << MSG::VERBOSE << "Applying as-built parameters for chamber " << getStationType() << ", eta " << getStationEta()
                << ", phi " << getStationPhi() << " -- multilayer=" << multilayer << " tubelayer=" << tubelayer << " tube=" << tube
                << endmsg;
#endif

        static const int nsid = 2;
        MdtAsBuiltPar::tubeSide_t tubeSide[nsid] = {MdtAsBuiltPar::POS, MdtAsBuiltPar::NEG};
        Amg::Vector3D wireEnd[nsid] = {locAMDBWireEndP, locAMDBWireEndN};
        MdtAsBuiltPar::multilayer_t ml = (multilayer == 1) ? MdtAsBuiltPar::ML1 : MdtAsBuiltPar::ML2;

        for (int isid = 0; isid < nsid; ++isid) {  // first s>0 then s<0
            // Compute the reference for the as-built parameters
            double xref = 0.;
            double yref = 0.;
            double zref = 0.;
            int ref_layer = (ml == MdtAsBuiltPar::ML1) ? m_nlayers : 1;
            double y_offset = (ml == MdtAsBuiltPar::ML1) ? outerTubeRadius() : -outerTubeRadius();
            double xmin = *std::min_element(m_firstwire_x.begin(), m_firstwire_x.begin() + m_nlayers) - outerTubeRadius();
            if (m_inBarrel) {
                xref = -m_Rsize / 2. + m_firstwire_y[ref_layer - 1] + y_offset;
                zref = -m_Zsize / 2. + xmin;
            } else {
                xref = -m_Zsize / 2. + m_firstwire_y[ref_layer - 1] + y_offset;
                zref = -m_Rsize / 2. + xmin;
            }
            Amg::Vector3D reference_point(xref, yref, zref);
            Amg::Transform3D toAMDB = Amg::CLHEPTransformToEigen(*parentMuonStation()->getNativeToAmdbLRS()) * toParentStation();
            reference_point = toAMDB * reference_point;
            if (isid == 0)
                reference_point = reference_point + Amg::Vector3D(0.5 * getNominalTubeLengthWoCutouts(ref_layer, 1), 0., 0.);
            else
                reference_point = reference_point + Amg::Vector3D(-0.5 * getNominalTubeLengthWoCutouts(ref_layer, 1), 0., 0.);

            int layer_delta = tubelayer;
            if (ml == MdtAsBuiltPar::ML1) layer_delta = m_nlayers + 1 - tubelayer;

            // Get the As-Built parameters for this ML and side of the chamber
            double zpitch = params->zpitch(ml, tubeSide[isid]);
            double ypitch = params->ypitch(ml, tubeSide[isid]);
            double stagg = (double)params->stagg(ml, tubeSide[isid]);
            double alpha = params->alpha(ml, tubeSide[isid]);
            double y0 = params->y0(ml, tubeSide[isid]);
            double z0 = params->z0(ml, tubeSide[isid]);

            // Find the vector from the reference_point to the endplug
            int do_stagg = (layer_delta - 1) % 2;  // 0 for layer 1 and 3, 1 for layer 2 and 4
            double offset_stagg = ((double)do_stagg) * 0.5 * zpitch * stagg;
            Amg::Vector3D end_plug(0., (tube - 1.0) * zpitch + offset_stagg, (layer_delta - 1) * ypitch);

            Amg::Translation3D amgtranslation(0., 0., 0.);
            Amg::Transform3D amgTransf = amgtranslation * Amg::RotationMatrix3D::Identity();
            amgTransf *= Amg::AngleAxis3D(-alpha, Amg::Vector3D(1., 0., 0.));
            end_plug = amgTransf * end_plug;

            // Calculate x position, which varies for endcap chambers
            double xshift = 0.;
            if (isid == 0)
                xshift = 0.5 * getNominalTubeLengthWoCutouts(tubelayer, tube) - 0.5 * getNominalTubeLengthWoCutouts(ref_layer, 1);
            else
                xshift = -0.5 * getNominalTubeLengthWoCutouts(tubelayer, tube) + 0.5 * getNominalTubeLengthWoCutouts(ref_layer, 1);

            Amg::Vector3D ret(reference_point.x() + xshift, reference_point.y() + z0 + end_plug.y(),
                              reference_point.z() + y0 + end_plug.z());

            // Warn if result of calculation is too far off
            // BIL1A13 has as-built parameters up to 3mm off, giving the size of the cut
            if ((ret - wireEnd[isid]).mag() > 3. * CLHEP::mm) {
#ifdef NDEBUG
                MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
#endif
                log << MSG::WARNING << "Large as-built correction for chamber " << getStationType() << ", eta " << getStationEta()
                    << ", phi " << getStationPhi() << ", ML " << multilayer << ", layer " << tubelayer << ", tube " << tube << ", side "
                    << isid << ", Delta (" << ret.x() - wireEnd[isid].x() << ", " << ret.y() - wireEnd[isid].y() << ", "
                    << ret.z() - wireEnd[isid].z() << ")" << endmsg;
            }

            // Save the result
            if (tubeSide[isid] == MdtAsBuiltPar::POS)
                locAMDBWireEndP = ret;
            else
                locAMDBWireEndN = ret;
        }
    }
    // **************************** interfaces related to Tracking *****************************************************
    const Amg::Transform3D& MdtReadoutElement::transform(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();

#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (!idh->valid(id)) {  // is input id valid ?
            log << MSG::WARNING << "transform requested to RE " << idh->show_to_string(identify()) << " named " << getStationName()
                << " for invalid tube id " << idh->show_to_string(id) << " computing tranform for first tube of this RE" << endmsg;
            return transform(identify());
        }
        if (!containsId(id)) {
            log << MSG::WARNING << "transform requested to RE " << idh->show_to_string(identify()) << " named " << getStationName()
                << " for not contained tube id " << idh->show_to_string(id) << " computing tranform for first tube of this RE" << endmsg;
            return transform(identify());
        }
#endif
        int tubeLayer = idh->tubeLayer(id);
        int tube = idh->tube(id);
        return transform(tubeLayer, tube);
    }

    std::unique_ptr<MdtReadoutElement::GeoInfo> MdtReadoutElement::makeGeoInfo(const int tubelayer, const int tube) const {
        const Amg::Transform3D& toDeformed = fromIdealToDeformed(getMultilayer(), tubelayer, tube);
        const Amg::Transform3D transform = globalTransform(nodeform_localTubePos(getMultilayer(), tubelayer, tube), toDeformed);
        return std::make_unique<GeoInfo>(transform);
    }

    const MdtReadoutElement::GeoInfo& MdtReadoutElement::geoInfo(const int tubeLayer, const int tube) const {
        size_t itube = (tubeLayer - 1) * m_ntubesperlayer + tube - 1;
        if (itube >= m_tubeGeo.size()) {
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
            log << MSG::WARNING << " geoInfo called with tubeLayer or tube out of range in chamber "
                << manager()->mdtIdHelper()->print_to_string(identify()) << " : layer " << tubeLayer << " max " << m_nlayers << " tube " << tube
                << " max " << m_ntubesperlayer << " will compute transform for first tube in this chamber" << endmsg;
            log << MSG::WARNING << "Please run in DEBUG mode to get extra diagnostic" << endmsg;
            itube = 0;
        }

        const CxxUtils::CachedUniquePtr<GeoInfo>& ptr = m_tubeGeo.at(itube);
        if (!ptr) {
            ptr.set(makeGeoInfo(tubeLayer, tube));
            if (!m_haveTubeGeo) m_haveTubeGeo = true;
        }
        return *ptr;
    }

    const Amg::Transform3D& MdtReadoutElement::transform(const int tubeLayer, const int tube) const {
        return geoInfo(tubeLayer, tube).m_transform;
    }

    void MdtReadoutElement::restoreTubes() {
        if (m_backupTubeGeo.empty()) return;
        m_tubeGeo = std::move(m_backupTubeGeo);
        m_haveTubeGeo = true;
        m_deformTransf = std::move(m_backupDeformTransf);
        m_haveDeformTransf = true;
    }

    void MdtReadoutElement::shiftTube(const Identifier& id) {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        int mlayer = idh->multilayer(id);
        int tubeLayer = idh->tubeLayer(id);
        int tube = idh->tube(id);
        int ntot_tubes = m_nlayers * m_ntubesperlayer;

        // check for valid tube
        if (mlayer != getMultilayer() || tubeLayer < 1 || tubeLayer > getNLayers() || tube < 1 || tube > getNtubesperlayer()) return;

        int itube = (tubeLayer - 1) * m_ntubesperlayer + tube - 1;

        if (m_backupTubeGeo.empty()) { m_backupTubeGeo.resize(ntot_tubes); }

        if (!m_backupTubeGeo[itube]) {
            m_backupTubeGeo[itube].store(m_tubeGeo[itube].release());
            m_tubeGeo[itube].store(makeGeoInfo(tubeLayer, tube));
            m_haveTubeGeo = true;
        }

        if (m_backupDeformTransf.empty()) { m_backupDeformTransf.resize(ntot_tubes); }

        if (!m_backupDeformTransf[itube]) { m_backupDeformTransf[itube].store(m_deformTransf[itube].release()); }

        return;
    }

    const Trk::SaggedLineSurface& MdtReadoutElement::surface(const int tubeLayer, const int tube) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        Identifier id = idh->channelID(idh->parentID(identify()), getMultilayer(), tubeLayer, tube);

        int ntot_tubes = m_nlayers * m_ntubesperlayer;
        int itube = (tubeLayer - 1) * m_ntubesperlayer + tube - 1;
        // consistency checks
        if (itube >= ntot_tubes) {
            MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
            log << MSG::WARNING << "surface called with tubeLayer or tube out of range in chamber "
                << manager()->mdtIdHelper()->print_to_string(identify()) << " : layer " << tubeLayer << " max " << m_nlayers << " tube " << tube
                << " max " << m_ntubesperlayer << " will compute surface for first tube in this chamber" << endmsg;
            log << MSG::WARNING << "Please run in DEBUG mode to get extra diagnostic" << endmsg;
            itube = 0;
        }

        const CxxUtils::CachedUniquePtr<Trk::SaggedLineSurface>& ptr = m_tubeSurfaces.at(itube);
        if (!ptr) {
            double wireTension = 350.;
            if (getStationName().substr(0, 3) == "BOL") wireTension = 285.;
            ptr.set(std::make_unique<Trk::SaggedLineSurface>(*this, id, getWireLength(tubeLayer, tube), wireTension, linearDensity));
            if (!m_haveTubeSurfaces) m_haveTubeSurfaces = true;
        }
        return *ptr;
    }
    const Trk::SaggedLineSurface& MdtReadoutElement::surface(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (!idh->valid(id)) {  // is input id valid ?
            log << MSG::WARNING << "surface requested to RE " << idh->show_to_string(identify()) << " named " << getStationName()
                << " for invalid tube id " << idh->show_to_string(id) << " computing surface for first tube of this RE" << endmsg;
            return surface(identify());
        }
        if (!containsId(id)) {
            log << MSG::WARNING << "surface requested to RE " << idh->show_to_string(identify()) << " named " << getStationName()
                << " for not contained tube id " << idh->show_to_string(id) << " computing surface for first tube of this RE" << endmsg;
            return surface(identify());
        }
#endif
        return surface(idh->tubeLayer(id), idh->tube(id));
    }
    const Trk::CylinderBounds& MdtReadoutElement::bounds(const int tubeLayer, const int tube) const {
        int istep = 0;
        int ntot_steps = m_nsteps;

        if (hasCutouts() && manager()->MinimalGeoFlag() == 0) {
            ntot_steps = m_nlayers * m_ntubesperlayer;
            istep = (tubeLayer - 1) * m_ntubesperlayer + tube - 1;
        } else {
            if (!m_inBarrel) istep = int((tube - 1) / m_ntubesinastep);

            if (istep < 0 || istep >= ntot_steps) {
                const MdtIdHelper* idh = manager()->mdtIdHelper();
                MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
                log << MSG::WARNING << "bounds for Element named " << getStationName() << " with tech. " << getTechnologyType()
                    << " DEid = " << idh->show_to_string(identify()) << " called with: tubeL, tube " << tubeLayer << " " << tube
                    << "; step " << istep << " out of range 0-" << m_nsteps - 1 << " m_ntubesinastep " << m_ntubesinastep << endmsg;
                log << MSG::WARNING << "Please run in DEBUG mode to get extra diagnostic; setting istep = 0" << endmsg;
                istep = 0;
            }
        }
        if ((unsigned int)istep >= m_tubeBounds.size()) {
            const MdtIdHelper* idh = manager()->mdtIdHelper();
            const Identifier id = identify();
            throw std::runtime_error(Form(
                "File: %s, Line: %d\nMdtReadoutElement::bounds(%d,%d) - istep=%d but m_tubeBounds.size()=%lu for station %s, stationEta=%d",
                __FILE__, __LINE__, tubeLayer, tube, istep, m_tubeBounds.size(), idh->stationNameString(idh->stationName(id)).c_str(),
                idh->stationEta(id)));
        }
        const CxxUtils::CachedUniquePtr<Trk::CylinderBounds>& ptr = m_tubeBounds.at(istep);
        if (!ptr) {
            double tubelength = getTubeLengthForCaching(tubeLayer, tube);
            ptr.set(std::make_unique<Trk::CylinderBounds>(innerTubeRadius(), 0.5 * tubelength - m_deadlength));
            if (!m_haveTubeBounds) m_haveTubeBounds = true;
        }
        return *ptr;
    }
    const Trk::CylinderBounds& MdtReadoutElement::bounds(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (!idh->valid(id)) {  // is input id valid ?
            log << MSG::WARNING << "bounds requested to RE " << idh->show_to_string(identify()) << " named " << getStationName()
                << " for invalid tube id " << idh->show_to_string(id) << " computing bounds for first tube in this mdt RE" << endmsg;
            return bounds(identify());
        }
        if (!containsId(id)) {
            log << MSG::WARNING << "bounds requested to RE " << idh->show_to_string(identify()) << " named " << getStationName()
                << " for not contained tube id " << idh->show_to_string(id) << " computing bounds for first tube in this mdt RE" << endmsg;
            return bounds(identify());
        }
#endif
        return bounds(idh->tubeLayer(id), idh->tube(id));
    }
    const Amg::Vector3D& MdtReadoutElement::center(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (!idh->valid(id)) {  // is input id valid ?
            log << MSG::WARNING << "center requested to RE " << idh->show_to_string(identify()) << " named " << getStationName()
                << " for invalid tube id " << idh->show_to_string(id) << " computing center for first tube in this mdt RE" << endmsg;
            return center(identify());
        }
        if (!containsId(id)) {
            log << MSG::WARNING << "center requested to RE " << idh->show_to_string(identify()) << " named " << getStationName()
                << " for not contained tube id " << idh->show_to_string(id) << " computing center for first tube in this mdt RE" << endmsg;
            return center(identify());
        }
#endif
        return center(idh->tubeLayer(id), idh->tube(id));
    }
    const Amg::Vector3D& MdtReadoutElement::center(const int tubeLayer, const int tube) const { return geoInfo(tubeLayer, tube).m_center; }
    const Amg::Vector3D& MdtReadoutElement::normal() const {
        if (!m_elemNormal.isValid()) { m_elemNormal.set(Amg::Vector3D(transform().linear() * Amg::Vector3D(1., 0., 0.))); }
        return *m_elemNormal.ptr();
    }

    Amg::Vector3D MdtReadoutElement::tubeNormal(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();
        return tubeNormal(idh->tubeLayer(id), idh->tube(id));
    }
    Amg::Vector3D MdtReadoutElement::tubeNormal(int tubeLayer, int tube) const { return transform(tubeLayer, tube).rotation().col(2); }

    const Trk::Surface& MdtReadoutElement::surface() const {
        if (!m_associatedSurface) {
            Amg::RotationMatrix3D muonTRotation(transform().rotation());
            Amg::RotationMatrix3D surfaceTRotation;
            surfaceTRotation.col(0) = muonTRotation.col(1);
            surfaceTRotation.col(1) = muonTRotation.col(2);
            surfaceTRotation.col(2) = muonTRotation.col(0);

            Amg::Transform3D trans3D(surfaceTRotation);
            trans3D.pretranslate(transform().translation());

            if (barrel()) {
                m_associatedSurface.set(std::make_unique<Trk::PlaneSurface>(Amg::Transform3D(trans3D), MuonReadoutElement::getSsize() * 0.5,
                                                                            MuonReadoutElement::getZsize() * 0.5));
            } else {
                m_associatedSurface.set(std::make_unique<Trk::PlaneSurface>(Amg::Transform3D(trans3D), MuonReadoutElement::getSsize() * 0.5,
                                                                            MuonReadoutElement::getLongSsize() * 0.5,
                                                                            MuonReadoutElement::getRsize() * 0.5));
            }
        }
        return *m_associatedSurface;
    }

    const Amg::Vector3D& MdtReadoutElement::center() const { return surface().center(); }

    const Trk::SurfaceBounds& MdtReadoutElement::bounds() const {
        if (!m_associatedBounds) {
            if (barrel()) {
                m_associatedBounds.set(
                    std::make_unique<Trk::RectangleBounds>(MuonReadoutElement::getSsize() / 2., MuonReadoutElement::getZsize() / 2.));
            } else {
                m_associatedBounds.set(std::make_unique<Trk::TrapezoidBounds>(
                    MuonReadoutElement::getSsize() / 2., MuonReadoutElement::getLongSsize() / 2., MuonReadoutElement::getRsize() / 2.));
            }
        }
        return *m_associatedBounds;
    }

    void MdtReadoutElement::fillBLineCache() {
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (log.level() <= MSG::DEBUG)
            log << MSG::DEBUG << "Filling BLine cache for ReadoutElement " << getStationName() << "/" << getTechnologyName() << " eta/phi "
                << getStationEta() << "/" << getStationPhi() << " ml " << m_multilayer << endmsg;
#endif
        for (int tubeL = 1; tubeL <= m_nlayers; ++tubeL) {
            for (int tube = 1; tube <= m_ntubesperlayer; ++tube) { fromIdealToDeformed(getMultilayer(), tubeL, tube); }
        }
    }
    void MdtReadoutElement::clearBLineCache() {
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (log.level() <= MSG::VERBOSE)
            log << MSG::VERBOSE << "Clearing BLine cache for ReadoutElement " << getStationName() << "/" << getTechnologyName()
                << " eta/phi " << getStationEta() << "/" << getStationPhi() << " ml " << m_multilayer << endmsg;
#endif
        if (m_haveDeformTransf) {
            m_haveDeformTransf = false;
            for (auto& d : m_deformTransf) { d.release(); }
        }
    }

    void MdtReadoutElement::clearCache() {
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (log.level() <= MSG::DEBUG)
            log << MSG::DEBUG << "Clearing cache for ReadoutElement " << getStationName() << "/" << getTechnologyName() << " eta/phi "
                << getStationEta() << "/" << getStationPhi() << " ml " << m_multilayer << endmsg;
#endif
        if (m_associatedSurface) {
            m_associatedSurface.release();
        }
#ifndef NDEBUG
        else {
            if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "no associated surface to be deleted" << endmsg;
        }
#endif
        if (m_associatedBounds) {
            m_associatedBounds.release();
        }
#ifndef NDEBUG
        else {
            if (log.level() <= MSG::VERBOSE) log << MSG::VERBOSE << "no associated bounds to be deleted" << endmsg;
        }
#endif
        m_elemNormal.reset();
        if (m_haveTubeSurfaces) {
            m_haveTubeSurfaces = false;
            for (auto& s : m_tubeSurfaces) { s.release(); }
        }
        if (m_haveTubeGeo) {
            m_haveTubeGeo = false;
            for (auto& g : m_tubeGeo) { g.release(); }
        }
        if (m_haveTubeBounds) {
            m_haveTubeBounds = false;
            for (auto& b : m_tubeBounds) { b.release(); }
        }
        // reset here the deform-related transforms
        clearBLineCache();
    }

    void MdtReadoutElement::setBLinePar(const BLinePar* bLine) {
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (log.level() <= MSG::DEBUG)
            log << MSG::DEBUG << "Setting B-line for " << getStationName().substr(0, 3) << " at eta/phi " << getStationEta() << "/"
                << getStationPhi() << endmsg;
#endif
        m_BLinePar = bLine;
    }

    void MdtReadoutElement::fillCache() {
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MdtReadoutElement");
        if (log.level() <= MSG::DEBUG)
            log << MSG::DEBUG << "Filling cache for ReadoutElement " << getStationName() << "/" << getTechnologyName() << " eta/phi "
                << getStationEta() << "/" << getStationPhi() << " ml " << getMultilayer() << endmsg;
#endif

#ifndef NDEBUG
        const Trk::PlaneSurface* tmpSurface = dynamic_cast<const Trk::PlaneSurface*>(&surface());  //<! filling m_associatedSurface
        const Trk::SurfaceBounds* tmpBounds = nullptr;                         //<! filling m_associatedBounds
        if (MuonReadoutElement::barrel())
            tmpBounds = dynamic_cast<const Trk::RectangleBounds*>(&bounds());
        else
            tmpBounds = dynamic_cast<const Trk::TrapezoidBounds*>(&bounds());
        if (log.level() <= MSG::VERBOSE) {
            log << MSG::VERBOSE << "global Surface / Bounds pointers " << tmpSurface << " " << tmpBounds << endmsg;
            log << MSG::VERBOSE << "global Normal " << normal() << endmsg;
        }
#endif
        const Trk::CylinderBounds* tmpCil = nullptr;
        const Trk::SaggedLineSurface* tmpSaggL = nullptr;
        Amg::Vector3D myPoint;
        Amg::Transform3D myTransform;
        for (int tl = 1; tl <= getNLayers(); ++tl) {
            for (int tube = 1; tube <= getNtubesperlayer(); ++tube) {
                // in case of BMG chambers, do not check the 'dead' tubes
                // (the tubes are numbered from 1-54 for each multilayer, however there are cutouts for the
                // alignment system where no tubes are built-in, meaning, those tubes do not exist/are 'dead')
                if (manager()->mdtIdHelper()->isBMG(identify())) {
                    PVConstLink cv = getMaterialGeom();
                    // usually the tube number corresponds to the child number, however for
                    // BMG chambers full tubes are skipped during the building process
                    // therefore the matching needs to be done via the volume ID
                    int packed_id = tube + maxNTubesPerLayer * tl;
                    bool found = false;
                    geoGetIds(
                        [&](int id) {
                            if (!found && id == packed_id) {
                                myTransform = transform(tl, tube);                                           //<! filling m_tubeTransf
                                myPoint = center(tl, tube);                                                  //<! filling m_tubeCenter
                                tmpCil = dynamic_cast<const Trk::CylinderBounds*>(&bounds(tl, tube));        //<! filling m_tubeBounds
                                tmpSaggL = dynamic_cast<const Trk::SaggedLineSurface*>(&surface(tl, tube));  //<! filling m_tubeSurfaces
                                found = true;
                            }
                        },
                        &*cv);
#ifndef NDEBUG
                    if (found && log.level() <= MSG::VERBOSE) {
                        log << MSG::VERBOSE << "tubeLayer/tube " << tl << " " << tube << " transform at origin  "
                            << myTransform * Amg::Vector3D(0., 0., 0.) << endmsg;
                        log << MSG::VERBOSE << "tubeLayer/tube " << tl << " " << tube << " tube center          " << myPoint << endmsg;
                        log << MSG::VERBOSE << "tubeLayer/tube " << tl << " " << tube << " tube bounds pointer  " << tmpCil << endmsg;
                        log << MSG::VERBOSE << "tubeLayer/tube " << tl << " " << tube << " tube surface pointer " << tmpSaggL << endmsg;
                    }
#endif
                } else {
                    // print in order to compute !!!
                    myTransform = transform(tl, tube);                                           //<! filling m_tubeTransf
                    myPoint = center(tl, tube);                                                  //<! filling m_tubeCenter
                    tmpCil = dynamic_cast<const Trk::CylinderBounds*>(&bounds(tl, tube));        //<! filling m_tubeBounds
                    tmpSaggL = dynamic_cast<const Trk::SaggedLineSurface*>(&surface(tl, tube));  //<! filling m_tubeSurfaces
#ifndef NDEBUG
                    if (log.level() <= MSG::VERBOSE) {
                        log << MSG::VERBOSE << "tubeLayer/tube " << tl << " " << tube << " transform at origin  "
                            << myTransform * Amg::Vector3D(0., 0., 0.) << endmsg;
                        log << MSG::VERBOSE << "tubeLayer/tube " << tl << " " << tube << " tube center          " << myPoint << endmsg;
                        log << MSG::VERBOSE << "tubeLayer/tube " << tl << " " << tube << " tube bounds pointer  " << tmpCil << endmsg;
                        log << MSG::VERBOSE << "tubeLayer/tube " << tl << " " << tube << " tube surface pointer " << tmpSaggL << endmsg;
                    }
#endif
                }
            }
        }
    }

    bool MdtReadoutElement::containsId(const Identifier& id) const {
        const MdtIdHelper* idh = manager()->mdtIdHelper();

        int mlayer = idh->multilayer(id);
        if (mlayer != getMultilayer()) return false;

        int layer = idh->tubeLayer(id);
        if (layer < 1 || layer > getNLayers()) return false;

        int tube = idh->tube(id);
        if (tube < 1 || tube > getNtubesperlayer()) return false;

        return true;
    }

    // **************************** interfaces related to Tracking *****************************************************)

}  // namespace MuonGM
