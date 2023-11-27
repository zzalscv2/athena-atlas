/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonReadoutGeometry/CscReadoutElement.h"

#include <GaudiKernel/IMessageSvc.h>
#include <GeoModelKernel/GeoDefinitions.h>
#include <GeoModelKernel/GeoLogVol.h>
#include <GeoModelKernel/GeoVFullPhysVol.h>
#include <GeoModelKernel/GeoVPhysVol.h>
#include <TString.h>

#include <cmath>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "AthenaKernel/getMessageSvc.h"
#include "EventPrimitives/AmgMatrixBasePlugin.h"
#include "GaudiKernel/MsgStream.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "Identifier/IdContext.h"
#include "MuonAlignmentData/CorrContainer.h"
#include "MuonIdHelpers/CscIdHelper.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RotatedTrapezoidBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"

namespace Trk {
    class SurfaceBounds;
}

namespace MuonGM {

    CscReadoutElement::CscReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, MuonDetectorManager* mgr) :
        MuonClusterReadoutElement(pv, mgr, Trk::DetectorElemType::Csc) {
        // Set a few parameters here.  The rest are set in MuonChamber::setCscReadoutGeometry
        // get the setting of the caching flag from the manager
        setCachingFlag(mgr->cachingFlag());

        // st name
        setStationName(stName);
        for (unsigned int i = 0; i < 4; i++) m_wireplanez[i] = 0.;

        if (mgr->MinimalGeoFlag() == 0) {
            if (GeoFullPhysVol* pvc = dynamic_cast<GeoFullPhysVol*>(pv)) {
                unsigned int nchildvol = pvc->getNChildVols();
                int lgg = 0;
                std::string::size_type npos;
                for (unsigned ich = 0; ich < nchildvol; ich++) {
                    PVConstLink pc = pvc->getChildVol(ich);
                    std::string childname = (pc->getLogVol())->getName();
                    int nch1 = pc->getNChildVols();
                    lgg = 3;
                    for (int ngv = 0; ngv < nch1; ngv++) {
                        PVConstLink pcgv = pc->getChildVol(ngv);
                        std::string childname1 = (pcgv->getLogVol())->getName();

                        if ((npos = childname1.find("CscArCO2")) != std::string::npos) {
                            const GeoTrf::Vector3D trans = (pvc->getXToChildVol(ich) * pc->getXToChildVol(ngv)).translation();
                            m_wireplanez[lgg] = trans.x();
                            lgg--;
                        }
                    }
                }
            } else {
                std::stringstream error_str{};
                error_str<<__FILE__<<":"<<__LINE__<<" - Cannot performa dynamic cast!";
                throw std::runtime_error(error_str.str());
            }
        } else {
            // hard wire for the moment
            double pitch = 25.69;
            m_wireplanez[0] = -38.51;
            for (int i = 1; i < 4; ++i) { m_wireplanez[i] = m_wireplanez[i - 1] + pitch; }
        }

        for (unsigned int i = 0; i < 4; ++i) {
            for (unsigned int j = 0; j < 3; ++j) {
                m_cscIntTransl[i][j] = 0.;  // first index is wireLayer, second = 0,1,2 for    s,z,t
            }
        }
        for (unsigned int i = 0; i < 4; ++i) {
            for (unsigned int j = 0; j < 3; ++j) {
                m_cscIntRot[i][j] = 0.;  // first index is wireLayer, second = 0,1,2 for rots,z,t
            }
        }

        // doTests();
    }

    CscReadoutElement::~CscReadoutElement() { clearCache(); }

    Amg::Vector3D CscReadoutElement::localToGlobalCoords(const Amg::Vector3D& x, const Identifier& id) const {
        const Amg::Vector3D gasgapP = localWireLayerPos(id);
        const Amg::Translation3D xfp(gasgapP.x(), gasgapP.y(), gasgapP.z());
        return absTransform() * xfp * x;
    }

    Amg::Transform3D CscReadoutElement::localToGlobalTransf(int gasGap) const {
        const Amg::Vector3D gasgapP = localWireLayerPos(gasGap);
        const Amg::Translation3D xfp(gasgapP.x(), gasgapP.y(), gasgapP.z());
        return absTransform() * xfp;
    }
    Amg::Transform3D CscReadoutElement::localToGlobalTransf(const Identifier& id) const {
        const Amg::Vector3D gasgapP = localWireLayerPos(id);
        const Amg::Translation3D xfp(gasgapP.x(), gasgapP.y(), gasgapP.z());
        return absTransform() * xfp;
    }

    Amg::Vector3D CscReadoutElement::globalToLocalCoords(const Amg::Vector3D& x, const Identifier& id) const {
        return localToGlobalTransf(id).inverse() * x;
    }

    Amg::Transform3D CscReadoutElement::globalToLocalTransf(const Identifier& id) const { return localToGlobalTransf(id).inverse(); }

    Amg::Vector3D CscReadoutElement::localWireLayerPos(const Identifier& id) const {
        const CscIdHelper* idh = manager()->cscIdHelper();
        int gasgap = idh->wireLayer(id);
        return localWireLayerPos(gasgap);
    }

    Amg::Vector3D CscReadoutElement::localWireLayerPos(int gg) const {
        Amg::Vector3D localP(m_wireplanez[gg - 1], 0., 0.);
        return localP;
    }

    Amg::Vector3D CscReadoutElement::wireLayerPos(const Identifier& id) const {
        const CscIdHelper* idh = manager()->cscIdHelper();
        int gasgap = idh->wireLayer(id);
        return wireLayerPos(gasgap);
    }

    Amg::Vector3D CscReadoutElement::wireLayerPos(int gg) const {
        const Amg::Vector3D localP = localWireLayerPos(gg);
        const Amg::Transform3D cscTrans = absTransform();
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "CscReadoutElement");
        if (log.level() <= MSG::VERBOSE) log << "CscReadoutElement::wireLayerPos got localWireLayerPos " << localP << endmsg;
#endif
        return cscTrans * localP;
    }

    double CscReadoutElement::cathodeReadoutPitch(int /*chLayer*/, int measuresPhi) const {
        if (measuresPhi == 0)
            return m_Etastrippitch;
        else
            return m_Phistrippitch;
    }
    double CscReadoutElement::cathodeReadoutPitch(int measuresPhi) const {
        if (measuresPhi == 0)
            return m_Etastrippitch;
        else
            return m_Phistrippitch;
    }

    int CscReadoutElement::maxNumberOfStrips(int measuresPhi, double /*width*/) const { return maxNumberOfStrips(measuresPhi); }

    int CscReadoutElement::maxNumberOfStrips(int measuresPhi) const {
        if (measuresPhi == 0)
            return m_nEtastripsperlayer;
        else
            return m_nPhistripsperlayer;
    }

    double CscReadoutElement::activeWidth(int measuresPhi) const {
        double midWidth = 0;
        // we need to know the geometry version
        // if P or earlier, we have pointing phi strips
        // if Q or later, we have parallel phi strips
        // R-strips are alwas parallel in all the geometry layouts

        if (0 == measuresPhi) {
            midWidth = length() - 2. * roxacellWidth();
        } else {
            double beta = std::atan((longWidth() - shortWidth()) / (2. * lengthUpToMaxWidth()));
            double lWidth = longWidth() - 2 * roxacellWidth() * (1 + std::sin(beta)) / std::cos(beta);
            midWidth = lWidth;
        }

        return midWidth;
    }

    double CscReadoutElement::z0() const {
        double beta = std::atan((longWidth() - shortWidth()) / (2. * lengthUpToMaxWidth()));
        double sWidth = shortWidth() - 2 * roxacellWidth() * (1 - std::sin(beta)) / std::cos(beta);
        double lWidth = longWidth() - 2 * roxacellWidth() * (1 + std::sin(beta)) / std::cos(beta);
        if (lengthUpToMaxWidth() == length()) {
            double realLength = lengthUpToMaxWidth() - 2 * roxacellWidth();
            return (realLength * sWidth / (lWidth - sWidth));
        } else {
            double bigLength = length() - 2 * roxacellWidth();
            double alpha = std::atan((excent() - lengthUpToMaxWidth()) / (longWidth() / 2.));
            double shortLongWidth = longWidth() * (excent() - length()) / (excent() - lengthUpToMaxWidth());
            double gslWidth = shortLongWidth - 2 * roxacellWidth() * (1 - std::cos(alpha)) / std::sin(alpha);
            lWidth = 2 * (bigLength + 0.5 * sWidth / std::tan(beta) + 0.5 * gslWidth * std::tan(alpha)) /
                     (std::tan(alpha) + 1.0 / std::tan(beta));
            double shortLength = bigLength - (lWidth - gslWidth) * std::tan(alpha) / 2.;
            return (shortLength * sWidth / (lWidth - sWidth));
        }
    }

    Amg::Vector3D CscReadoutElement::localStripPos(const Identifier& id) const {
        // uses localStripPos(int chamberLayer, int wireLayer, int measPhi, int channel) const;
        const CscIdHelper* idh = manager()->cscIdHelper();
        int eta = idh->stationEta(id);
        int chamberLayer = idh->chamberLayer(id);
        int wireLayer = idh->wireLayer(id);
        int measPhi = idh->measuresPhi(id);
        int channel = idh->strip(id);
        return localStripPos(eta, chamberLayer, wireLayer, measPhi, channel);
    }
    Amg::Vector3D CscReadoutElement::nominalLocalStripPos(const Identifier& id) const {
        // uses localStripPos(int chamberLayer, int wireLayer, int measPhi, int channel) const;
        const CscIdHelper* idh = manager()->cscIdHelper();
        int eta = idh->stationEta(id);
        int chamberLayer = idh->chamberLayer(id);
        int wireLayer = idh->wireLayer(id);
        int measPhi = idh->measuresPhi(id);
        int channel = idh->strip(id);
        return nominalLocalStripPos(eta, chamberLayer, wireLayer, measPhi, channel);
    }

    Amg::Vector3D CscReadoutElement::stripPos(const Identifier& id) const {
        const CscIdHelper* idh = manager()->cscIdHelper();
        int eta = idh->stationEta(id);
        int chamberLayer = idh->chamberLayer(id);
        int wireLayer = idh->wireLayer(id);
        int measPhi = idh->measuresPhi(id);
        int channel = idh->strip(id);
        return stripPos(eta, chamberLayer, wireLayer, measPhi, channel);
    }
    Amg::Vector3D CscReadoutElement::nominalStripPos(const Identifier& id) const {
        const CscIdHelper* idh = manager()->cscIdHelper();
        int eta = idh->stationEta(id);
        int chamberLayer = idh->chamberLayer(id);
        int wireLayer = idh->wireLayer(id);
        int measPhi = idh->measuresPhi(id);
        int channel = idh->strip(id);
        return nominalStripPos(eta, chamberLayer, wireLayer, measPhi, channel);
    }

    Amg::Vector3D CscReadoutElement::stripPos(int eta, int chamberLayer, int wireLayer, int measPhi, int channel) const {
        // const Amg::Vector3D localP = nominalLocalStripPos(eta, chamberLayer, wireLayer,
        //                                         measPhi, channel);
        const Amg::Vector3D localP = localStripPos(eta, chamberLayer, wireLayer, measPhi, channel);
        const Amg::Transform3D cscTrans = absTransform();
        return cscTrans * localP;
    }

    Amg::Vector3D CscReadoutElement::nominalStripPos(int eta, int chamberLayer, int wireLayer, int measPhi, int channel) const {
        const Amg::Vector3D localP = nominalLocalStripPos(eta, chamberLayer, wireLayer, measPhi, channel);
        const Amg::Transform3D cscTrans = absTransform();
        return cscTrans * localP;
    }

    double CscReadoutElement::StripWidth(int chlayer, int measphi) const {
        // assume StripWidth = StripPitch
        double width = cathodeReadoutPitch(chlayer, measphi);
        return width;
    }
    double CscReadoutElement::StripPitch(int chlayer, int measphi) const {
        // assume StripWidth = StripPitch
        double width = cathodeReadoutPitch(chlayer, measphi);
        return width;
    }
    double CscReadoutElement::StripWidth(int measphi) const {
        // assume StripWidth = StripPitch
        double width = cathodeReadoutPitch(measphi);
        return width;
    }
    double CscReadoutElement::StripPitch(int measphi) const {
        // assume StripWidth = StripPitch
        double width = cathodeReadoutPitch(measphi);
        return width;
    }

    //****************************************************************************
    double CscReadoutElement::stripLength(const Identifier& id) const {
        const CscIdHelper* idh = manager()->cscIdHelper();
        int chamberLayer = idh->chamberLayer(id);
        int measPhi = idh->measuresPhi(id);
        int channel = idh->strip(id);
        double epsilon;
        return stripLength(chamberLayer, measPhi, channel, epsilon);
    }

    //****************************************************************************
    double CscReadoutElement::stripLength(int chamberLayer, int measuresPhi, int stripNumber, double& epsilon) const {
        double stripWidth = cathodeReadoutPitch(chamberLayer, measuresPhi);
        int numberOfStrips = maxNumberOfStrips(measuresPhi, stripWidth);

        double chamberLength = length() - 2 * roxacellWidth();
        double beta = std::atan((longWidth() - shortWidth()) / (2. * lengthUpToMaxWidth()));

        double smallWidth = shortWidth() - 2 * roxacellWidth() * (1.0 - std::sin(beta)) / std::cos(beta);
        double bigWidth = longWidth() - 2 * roxacellWidth() * (1.0 + std::sin(beta)) / std::cos(beta);
        double gslWidth{0.}, sLength{0.};

        double alpha = std::atan((excent() - lengthUpToMaxWidth()) / (longWidth() / 2.));
        if (length() != lengthUpToMaxWidth()) {
            double shortLongWidth = longWidth() * (excent() - length()) / (excent() - lengthUpToMaxWidth());
            gslWidth = shortLongWidth - 2 * roxacellWidth() * (1 - std::cos(alpha)) / std::sin(alpha);
            bigWidth = 2 * (chamberLength + 0.5 * smallWidth / std::tan(beta) + 0.5 * gslWidth * std::tan(alpha)) /
                       (std::tan(alpha) + 1.0 / std::tan(beta));
        }

        double pos = stripWidth * (stripNumber - 0.5 - numberOfStrips / 2.0);
        epsilon = lengthCorrection(measuresPhi, pos);
        double stripPos = std::abs(pos);

        if (measuresPhi == 0) {
            double effectiveLength = stripWidth * numberOfStrips;
            sLength = 2.0 * (effectiveLength / 2.0 + pos) * std::tan(beta) + smallWidth;
        } else {
            if (stripPos <= (smallWidth / 2.0))
                sLength = chamberLength;
            else {
                double diff = stripPos - smallWidth / 2.0;
                if (length() != lengthUpToMaxWidth()) {
                    double shortLength = chamberLength - (bigWidth - gslWidth) * std::tan(alpha) / 2.;
                    sLength = chamberLength - 2.0 * diff * shortLength / (bigWidth - smallWidth);
                } else
                    sLength = chamberLength * (1.0 - 2.0 * diff / (bigWidth - smallWidth));
            }
        }
        return (sLength - epsilon);
    }

    double CscReadoutElement::lengthCorrection(int measuresPhi, double stripPos) const {
        if (lengthUpToMaxWidth() == length()) return 0.0;  // CSS

        double epsilon = 0.0;
        double alpha = std::atan((excent() - lengthUpToMaxWidth()) / (longWidth() / 2.));
        double beta = std::atan((longWidth() - shortWidth()) / (2. * lengthUpToMaxWidth()));

        double bigLength = length() - 2 * roxacellWidth();
        double shortLongWidth = longWidth() * (excent() - length()) / (excent() - lengthUpToMaxWidth());
        double gslWidth = shortLongWidth - 2 * roxacellWidth() * (1 - std::cos(alpha)) / std::sin(alpha);
        double smallWidth = shortWidth() - 2 * roxacellWidth() * (1 - std::sin(beta)) / std::cos(beta);
        double bigWidth = 2 * (bigLength + 0.5 * smallWidth / std::tan(beta) + 0.5 * gslWidth * std::tan(alpha)) /
                          (std::tan(alpha) + 1.0 / std::tan(beta));
        double shortLength = bigLength - (bigWidth - gslWidth) * std::tan(alpha) / 2.;

        if (measuresPhi == 1) {
            if (std::abs(stripPos) > (gslWidth / 2.)) epsilon = (std::abs(stripPos) - gslWidth / 2.) * std::tan(alpha);
        } else {
            double z0 = shortLength - bigLength / 2;
            if (stripPos > z0) {
                double diff = stripPos - z0;
                double corr1 = diff / std::tan(alpha);
                double corr2 = diff * std::tan(beta);
                epsilon = 2.0 * (corr1 + corr2);
            }
        }
        if (epsilon < 0.0) epsilon = 0.0;
        return epsilon;
    }

    //****************************************************************************
    Amg::Vector3D CscReadoutElement::localStripPos(int eta, int chamberLayer, int wireLayer, int measPhi, int strip) const {
        Amg::Vector3D nominalLP = nominalLocalStripPos(eta, chamberLayer, wireLayer, measPhi, strip);
        // const Amg::Transform3D cscTrans = absTransform();
        Amg::Transform3D transfPtr_internalgeo(
            Amg::Translation3D(m_cscIntTransl[wireLayer - 1][2], m_cscIntTransl[wireLayer - 1][0], m_cscIntTransl[wireLayer - 1][1]) *
            Amg::AngleAxis3D(m_cscIntRot[wireLayer - 1][0], Amg::Vector3D::UnitY()) *
            Amg::AngleAxis3D(m_cscIntRot[wireLayer - 1][1], Amg::Vector3D::UnitZ()) *
            Amg::AngleAxis3D(m_cscIntRot[wireLayer - 1][2], Amg::Vector3D::UnitX()));
        return transfPtr_internalgeo * nominalLP;
    }
    //****************************************************************************
    Amg::Vector3D CscReadoutElement::nominalLocalStripPos(int eta, int chamberLayer, int wireLayer, int measPhi, int strip) const {
        // get the coordinate of the strip plane
        Amg::Vector3D stripPlane = localStripLayerPos(chamberLayer, wireLayer, measPhi, strip);

        // some initializations
        double x = stripPlane.x();
        double y = stripPlane.y();
        double z = stripPlane.z();

        // we need to know the geometry version
        // if P or earlier, we have pointing phi strips
        // if Q or later, we have parallel phi strips
        // R-strips are alwas parallel in all the geometry layouts

        double epsilon = 0.0;
        double lengthOfStrip = stripLength(chamberLayer, measPhi, strip, epsilon);

        // get some necessary parameters
        double stripWidth = cathodeReadoutPitch(chamberLayer, measPhi);
        int nStrips = maxNumberOfStrips(measPhi, stripWidth);

        if (0 == measPhi) {
            z = stripWidth * (strip - 0.5 - nStrips / 2.0);
        } else {
            z = (length() - 2 * roxacellWidth() - lengthOfStrip) / 2.0 - epsilon;
            if (eta < 0)
                y = stripWidth * (strip - 0.5 - nStrips / 2.0);
            else
                y = -stripWidth * (strip - 0.5 - nStrips / 2.0);
        }

        return Amg::Vector3D(x, y, z);
    }

    //****************************************************************************
    Amg::Vector3D CscReadoutElement::nominalLocalClusterPos(int eta, int wireLayer, int measPhi, double p) const {
        // get the coordinates of the wire plane
        Amg::Vector3D wireLayerPosition = localWireLayerPos(wireLayer);

        // some initializations
        double x = wireLayerPosition.x();
        double y = wireLayerPosition.y();
        double z = wireLayerPosition.z();

        // local position of cluster
        if (0 == measPhi)
            z = p;
        else {
            if (eta < 0)
                y = p;
            else
                y = -p;
        }
        return Amg::Vector3D(x, y, z);
    }

    //****************************************************************************
    Amg::Vector3D CscReadoutElement::localClusterPos(int eta, int wireLayer, int measPhi, double p) const {
        Amg::Vector3D nominalLCP = nominalLocalClusterPos(eta, wireLayer, measPhi, p);

        Amg::Transform3D transfPtr_internalgeo = Amg::Transform3D::Identity();
        transfPtr_internalgeo *=
            Amg::Translation3D(m_cscIntTransl[wireLayer - 1][2], m_cscIntTransl[wireLayer - 1][0], m_cscIntTransl[wireLayer - 1][1]);
        transfPtr_internalgeo *= Amg::AngleAxis3D(m_cscIntRot[wireLayer - 1][0], Amg::Vector3D::UnitY());
        transfPtr_internalgeo *= Amg::AngleAxis3D(m_cscIntRot[wireLayer - 1][1], Amg::Vector3D::UnitZ());
        transfPtr_internalgeo *= Amg::AngleAxis3D(m_cscIntRot[wireLayer - 1][2], Amg::Vector3D::UnitX());

        return transfPtr_internalgeo * nominalLCP;
    }

    //****************************************************************************
    Amg::Vector3D CscReadoutElement::localPos(const Amg::Vector3D& globalP) const {
        // localP is a local position
        const Amg::Transform3D cscTrans = absTransform();
        return cscTrans.inverse() * globalP;
    }

    //****************************************************************************
    Amg::Vector3D CscReadoutElement::nominalGlobalPos(const Amg::Vector3D& localP) const {
        // globalP is a global position
        const Amg::Transform3D cscTrans = absTransform();
        return cscTrans * localP;
    }

    //****************************************************************************
    Amg::Vector3D CscReadoutElement::globalPos(const Amg::Vector3D& localP) const { return nominalGlobalPos(localP); }

    double CscReadoutElement::xCoordinateInTrackingFrame(const Identifier& id) const {
        const CscIdHelper* idh = manager()->cscIdHelper();
        int eta = idh->stationEta(id);
        int chamberLayer = idh->chamberLayer(id);
        bool measPhi = idh->measuresPhi(id);
        int strip = idh->strip(id);
        // get some necessary parameters
        double stripWidth = cathodeReadoutPitch(chamberLayer, measPhi);
        int nStrips = maxNumberOfStrips(measPhi, stripWidth);
        double pos = stripWidth * (strip - 0.5 - nStrips / 2.0);
        if (eta > 0 && measPhi) pos *= -1;
        return pos;
    }

    //****************************************************************************
    Amg::Vector3D CscReadoutElement::localStripLayerPos(int /*chamberLayer*/, int wireLayer, int measPhi, int /*strip*/) const {
        if (!(measPhi == 0 || measPhi == 1)) throw std::runtime_error ("CscReadoutElement::localStripLayerPos bad measPhi");
        Amg::Vector3D wireLayerPosition = localWireLayerPos(wireLayer);
        double anodeCathodeDis = anodeCathodeDistance();
        double x = wireLayerPosition.x();
        double y = wireLayerPosition.y();
        double z = wireLayerPosition.z();

        if (wireLayer == 1 || wireLayer == 3) {
            if (measPhi == 0)
                x = wireLayerPosition.x() + anodeCathodeDis;
            else
                x = wireLayerPosition.x() - anodeCathodeDis;
        } else {
            if (measPhi == 0)
                x = wireLayerPosition.x() - anodeCathodeDis;
            else
                x = wireLayerPosition.x() + anodeCathodeDis;
        }

        return Amg::Vector3D(x, y, z);
    }

    // ******************************* sin stereo*************************************
    double CscReadoutElement::sinStereo(const Identifier& id) const {
        // sin stero for R-strips
        Amg::Vector3D posStrip = stripPos(id);
        const CscIdHelper* idh = manager()->cscIdHelper();
        int measPhi = idh->measuresPhi(id);
        if (measPhi == 0) return 1.0;

        // return zero for phi strips in layout P
        // std::string gVersion = manager()->geometryVersion();

        // sin stero for phi strips in layout Q and above
        int eta = idh->stationEta(id);
        int chamberLayer = idh->chamberLayer(id);
        int wireLayer = idh->wireLayer(id);
        int strip = maxNumberOfStrips(measPhi) / 2;
        Amg::Vector3D etaAxis = stripPos(eta, chamberLayer, wireLayer, measPhi, strip);
        double sinstero = (posStrip.y() * etaAxis.x() - posStrip.x() * etaAxis.y()) / (posStrip.perp() * etaAxis.perp());
        return sinstero;
    }

    void CscReadoutElement::doTests() {
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "CscReadoutElement");
        if (log.level() <= MSG::VERBOSE) {
            constexpr std::array<int, 2> eta{1, -1};
            constexpr std::array<int, 2> maxStrips{192, 48};
            int chamberLayer = 1;
            int wireLayer = 4;
            for (int measPhi = 0; measPhi < 2; ++measPhi) {
                for (int ieta = 0; ieta < 2; ++ieta) {
                    for (int ilayer = 1; ilayer <= wireLayer; ++ilayer) {
                        int strips[3] = {1, maxStrips[measPhi] / 2, maxStrips[measPhi]};
                        for (int i = 0; i < 3; i++) {
                            int istrip = strips[i];
                            Amg::Vector3D npos = nominalLocalStripPos(eta[ieta], chamberLayer, ilayer, measPhi, istrip);
                            Amg::Vector3D pos = localStripPos(eta[ieta], chamberLayer, ilayer, measPhi, istrip);
                            log << MSG::VERBOSE << "the nominal positions = " << npos.x() << " " << npos.y() << " " << npos.z() << endmsg;
                            log << MSG::VERBOSE << "the positions = " << pos.x() << " " << pos.y() << " " << pos.z() << endmsg;
                        }
                    }
                }
            }
        }
#endif
    }
    
    Amg::Vector3D CscReadoutElement::stripLayerPos(const Identifier& id) const {
        const CscIdHelper* idh = manager()->cscIdHelper();
        int chamberLayer = idh->chamberLayer(id);
        int wireLayer = idh->wireLayer(id);
        int measuresPhi = idh->measuresPhi(id);
        int strip = idh->strip(id);
        return stripLayerPos(chamberLayer, wireLayer, measuresPhi, strip);
    }

    Amg::Vector3D CscReadoutElement::stripLayerPos(const IdentifierHash& hash) const {
        Identifier id;
        const CscIdHelper* idh = manager()->cscIdHelper();
        IdContext context = idh->channel_context();
        if (!idh->get_id(hash, id, &context))
            return stripLayerPos(id);
        else
            return Amg::Vector3D(0, 0, 0);
    }

    Amg::Vector3D CscReadoutElement::stripLayerPos(int chamberLayer, int wireLayer, int measPhi, int channel) const {
        Amg::Vector3D localP = localStripLayerPos(chamberLayer, wireLayer, measPhi, channel);
        const Amg::Transform3D cscTrans = absTransform();
        return cscTrans * localP;
    }
    
    void CscReadoutElement::setCscInternalAlignmentPar(const ALinePar& x) {
        // get id helper
        const CscIdHelper& idh{idHelperSvc()->cscIdHelper()};
        const int wlayer = idh.wireLayer(x.identify());
        const bool notAllowedLayer = (wlayer > 4 || wlayer <1);


        if (idh.elementID(x.identify()) != idh.elementID(identify()) || notAllowedLayer) {
            ATH_MSG_WARNING("Trying to set the following CSC internal A-line " << x << " for Csc readout Element " << idHelperSvc()->toString(identify()) 
                                << "Inconsistent CSC int. Aline assignment - Internal alignment will not be applied ");
            return;
        }
        ATH_MSG_DEBUG("Set internal alignment parameter "<<x);
        using Parameter = ALinePar::Parameter;
        m_cscIntTransl[wlayer - 1][0] = x.getParameter(Parameter::transS);
        m_cscIntTransl[wlayer - 1][1] = x.getParameter(Parameter::transZ);
        m_cscIntTransl[wlayer - 1][2] = x.getParameter(Parameter::transT);
        m_cscIntRot[wlayer - 1][0] =x.getParameter(Parameter::rotS);
        m_cscIntRot[wlayer - 1][1] =x.getParameter(Parameter::rotZ);
        m_cscIntRot[wlayer - 1][2] =x.getParameter(Parameter::rotT);
       
        for (unsigned int j = 0; j < 3; ++j) {
            ATH_MSG_DEBUG("<CscReadoutElement::setCscInternalAlignmentPar()>: m_cscIntTransl[" << (wlayer - 1) << "][" << j
                << "]: " << m_cscIntTransl[(wlayer - 1)][j]);
            ATH_MSG_DEBUG("<CscReadoutElement::setCscInternalAlignmentPar()>: m_cscIntRot[" << (wlayer - 1) << "][" << j
                << "]: " << m_cscIntRot[(wlayer - 1)][j]);
        }
    }


    double CscReadoutElement::getGasGapIntAlign_s(int gasGap) const { return m_cscIntTransl[gasGap - 1][0]; }

    double CscReadoutElement::getGasGapIntAlign_z(int gasGap) const { return m_cscIntTransl[gasGap - 1][1]; }

    double CscReadoutElement::getGasGapIntAlign_t(int gasGap) const { return m_cscIntTransl[gasGap - 1][2]; }

    double CscReadoutElement::getGasGapIntAlign_rots(int gasGap) const { return m_cscIntRot[gasGap - 1][0]; }

    double CscReadoutElement::getGasGapIntAlign_rotz(int gasGap) const { return m_cscIntRot[gasGap - 1][1]; }

    double CscReadoutElement::getGasGapIntAlign_rott(int gasGap) const { return m_cscIntRot[gasGap - 1][2]; }

    Amg::Transform3D CscReadoutElement::nominalTransform(const Identifier& id) const {
        const CscIdHelper* idh = manager()->cscIdHelper();
        return nominalTransform(idh->wireLayer(id), idh->measuresPhi(id));
    }
    Amg::Transform3D CscReadoutElement::nominalTransform(int gasGap, int measPhi) const {
        Amg::RotationMatrix3D muonTRotation(localToGlobalTransf(gasGap).rotation());
        Amg::RotationMatrix3D surfaceTRotation;
        surfaceTRotation.col(0) = muonTRotation.col(1);
        surfaceTRotation.col(1) = muonTRotation.col(2);
        surfaceTRotation.col(2) = muonTRotation.col(0);
        if (measPhi == 0) surfaceTRotation = surfaceTRotation * Amg::AngleAxis3D(M_PI / 2., Amg::Vector3D::UnitZ());

        Amg::Transform3D transfPtr_orig(surfaceTRotation);
        transfPtr_orig *= Amg::Translation3D(localToGlobalTransf(gasGap).translation());
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "CscReadoutElement");
        if (log.level() <= MSG::DEBUG) {
            log << MSG::DEBUG << "nominalTransform+++++++++++Original Tranformation ++++++++++++++++++++++" << endmsg;
            log << MSG::DEBUG << (transfPtr_orig)(0, 0) << " " << (transfPtr_orig)(0, 1) << " " << (transfPtr_orig)(0, 2) << " "
                << (transfPtr_orig)(0, 3) << endmsg;
            log << MSG::DEBUG << (transfPtr_orig)(1, 0) << " " << (transfPtr_orig)(1, 1) << " " << (transfPtr_orig)(1, 2) << " "
                << (transfPtr_orig)(1, 3) << endmsg;
            log << MSG::DEBUG << (transfPtr_orig)(2, 0) << " " << (transfPtr_orig)(2, 1) << " " << (transfPtr_orig)(2, 2) << " "
                << (transfPtr_orig)(2, 3) << endmsg;
            log << MSG::DEBUG << "+++++ transf ends " << endmsg;
        }
#endif
        return transfPtr_orig;
    }
    Amg::Vector3D CscReadoutElement::stripPosOnTrackingSurface(const Identifier& id) const {
        const CscIdHelper* idh = manager()->cscIdHelper();
        return stripPosOnTrackingSurface(idh->stationEta(id), idh->chamberLayer(id), idh->wireLayer(id), idh->measuresPhi(id),
                                         idh->strip(id));
    }
    Amg::Vector3D CscReadoutElement::stripPosOnTrackingSurface(int eta, int chamberLayer, int wireLayer, int measPhi, int channel) const {
        Amg::Vector3D nP = nominalLocalStripPos(eta, chamberLayer, wireLayer, measPhi, channel);

        if (measPhi == 1)
            return Amg::Vector3D(nP.y(), 0., 0.);
        else
            return Amg::Vector3D(nP.z(), 0., 0.);
    }
    Amg::Vector3D CscReadoutElement::nominalCenter(int gasGap) const { return nominalTransform(gasGap, 1) * Amg::Vector3D(0., 0., 0.); }
    Amg::Vector3D CscReadoutElement::originForInternalALines(int gasGap) const { return nominalCenter(gasGap); }

    void CscReadoutElement::fillCache() {
        if (!m_surfaceData)
            m_surfaceData = std::make_unique<SurfaceData>();
        else {
            MsgStream log(Athena::getMessageSvc(), "CscReadoutElement");
            if (log.level() <= MSG::WARNING) log << MSG::WARNING << "calling fillCache on an already filled cache" << endmsg;
            return;
        }
        const CscIdHelper* idh = manager()->cscIdHelper();
        Identifier parentID = idh->parentID(identify());

        // loop over all gas gaps
        for (int gp = 1; gp <= m_ngasgaps; ++gp) {
            // loop over phi/eta projections
            for (int mp = 1; mp >= 0; --mp) {
                Identifier id = idh->channelID(parentID, ChamberLayer(), gp, mp, 1);

                const Amg::Vector3D gasgapP = localWireLayerPos(gp);
                const Amg::Translation3D xfp(gasgapP.x(), gasgapP.y(), gasgapP.z());
                Amg::Transform3D trans3D = absTransform() * xfp;
                Amg::RotationMatrix3D muonTRotation(trans3D.rotation());
                Amg::RotationMatrix3D surfaceTRotation;
                surfaceTRotation.col(0) = muonTRotation.col(1);
                surfaceTRotation.col(1) = muonTRotation.col(2);
                surfaceTRotation.col(2) = muonTRotation.col(0);
                if (mp == 0) surfaceTRotation = surfaceTRotation * Amg::AngleAxis3D(M_PI / 2., Amg::Vector3D::UnitZ());

                Amg::Transform3D transfPtr_orig(surfaceTRotation);
                transfPtr_orig.pretranslate(trans3D.translation());
                Amg::Transform3D transfPtr_internalgeo{Amg::Transform3D::Identity()};
                if (mp == 1) {
                    transfPtr_internalgeo *=
                        Amg::Translation3D(m_cscIntTransl[gp - 1][0], m_cscIntTransl[gp - 1][1], m_cscIntTransl[gp - 1][2]);
                    transfPtr_internalgeo *= Amg::AngleAxis3D(m_cscIntRot[gp - 1][0], Amg::Vector3D::UnitX());
                    transfPtr_internalgeo *= Amg::AngleAxis3D(m_cscIntRot[gp - 1][1], Amg::Vector3D::UnitY());
                    transfPtr_internalgeo *= Amg::AngleAxis3D(m_cscIntRot[gp - 1][2], Amg::Vector3D::UnitZ());
                } else {
                    transfPtr_internalgeo *=
                        Amg::Translation3D(m_cscIntTransl[gp - 1][1], -m_cscIntTransl[gp - 1][0], m_cscIntTransl[gp - 1][2]);
                    transfPtr_internalgeo *= Amg::AngleAxis3D(-m_cscIntRot[gp - 1][0], Amg::Vector3D::UnitY());
                    transfPtr_internalgeo *= Amg::AngleAxis3D(m_cscIntRot[gp - 1][1], Amg::Vector3D::UnitX());
                    transfPtr_internalgeo *= Amg::AngleAxis3D(m_cscIntRot[gp - 1][2], Amg::Vector3D::UnitZ());
                }

                m_surfaceData->m_layerTransforms.push_back(Amg::Transform3D(transfPtr_orig * transfPtr_internalgeo));
                m_surfaceData->m_layerSurfaces.emplace_back(std::make_unique<Trk::PlaneSurface>(*this, id));
                if (mp == 1) {
                    m_surfaceData->m_layerCenters.push_back(m_surfaceData->m_layerTransforms.back() * Amg::Vector3D(0., 0., 0.));
                    m_surfaceData->m_layerNormals.push_back(m_surfaceData->m_layerTransforms.back().linear() * Amg::Vector3D::UnitZ());
                }
            }
        }

        m_surfaceData->m_surfBounds.emplace_back(
            std::make_unique<Trk::TrapezoidBounds>(m_Ssize / 2., m_LongSsize / 2., m_Rsize / 2.));  // phi measurement
        m_surfaceData->m_surfBounds.emplace_back(
            std::make_unique<Trk::RotatedTrapezoidBounds>(m_Rsize / 2., m_Ssize / 2., m_LongSsize / 2.));  // eta measurement
    }

    bool CscReadoutElement::containsId(const Identifier& id) const {
        const CscIdHelper* idh = manager()->cscIdHelper();

        int chamberLayer = idh->chamberLayer(id);
        if (chamberLayer != ChamberLayer()) return false;

        int wireLayer = idh->wireLayer(id);
        if (wireLayer < 1 || wireLayer > Ngasgaps()) return false;

        int measPhi = idh->measuresPhi(id);
        int channel = idh->strip(id);
        if (measPhi == 0) {
            if (channel < 1 || channel > NetaStrips(wireLayer)) return false;
        } else if (measPhi == 1) {
            if (channel < 1 || channel > NphiStrips(wireLayer)) return false;
        } else
            return false;

        return true;
    }

    double CscReadoutElement::distanceToReadout(const Amg::Vector2D&, const Identifier&) const {
        MsgStream log(Athena::getMessageSvc(), "CscReadoutElement");
        if (log.level() <= MSG::WARNING) log << MSG::WARNING << " distanceToReadout::dummy routine " << endmsg;
        return 0.;
    }

    int CscReadoutElement::stripNumber(const Amg::Vector2D&, const Identifier&) const {
        MsgStream log(Athena::getMessageSvc(), "CscReadoutElement");
        if (log.level() <= MSG::WARNING) log << MSG::WARNING << " stripNumber::dummy routine " << endmsg;
        return 1;
    }

    bool CscReadoutElement::stripPosition(const Identifier& id, Amg::Vector2D& pos) const {
        /** please don't copy the inefficient code below!! Look at the RpcReadoutElement for a proper implementation */
        Amg::Vector3D gpos = stripPos(id);
        if (!surface(id).globalToLocal(gpos, gpos, pos)) {
            MsgStream log(Athena::getMessageSvc(), "CscReadoutElement");
            if (log.level() <= MSG::WARNING)
                log << MSG::WARNING << " stripPosition:: globalToLocal failed " << surface(id).transform().inverse() * gpos << endmsg;
            return false;
        }
        return true;
    }

    bool CscReadoutElement::spacePointPosition(const Identifier& phiId, const Identifier& etaId, Amg::Vector2D& pos) const {
        Amg::Vector2D phiPos;
        Amg::Vector2D etaPos;
        if (!stripPosition(phiId, phiPos) || !stripPosition(etaId, etaPos)) return false;
        spacePointPosition(phiPos, etaPos, pos);
        return true;
    }

}  // namespace MuonGM
