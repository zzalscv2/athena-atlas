/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 The MM detector = an assembly module = STGC in amdb
 ----------------------------------------------------
***************************************************************************/

#include "MuonReadoutGeometry/MMReadoutElement.h"

#include <GaudiKernel/IMessageSvc.h>
#include <GeoModelKernel/GeoLogVol.h>
#include <GeoModelKernel/GeoPVConstLink.h>
#include <GeoModelKernel/GeoShape.h>
#include <GeoModelKernel/GeoVFullPhysVol.h>
#include <GeoModelKernel/GeoVPhysVol.h>
#include <stdlib.h>

#include <cmath>
#include <map>
#include <memory>
#include <utility>

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoTrd.h"
#include "Identifier/IdentifierHash.h"
#include "MuonAGDDDescription/MMDetectorDescription.h"
#include "MuonAGDDDescription/MMDetectorHelper.h"
#include "MuonAlignmentData/ALinePar.h"
#include "MuonAlignmentData/CorrContainer.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RotatedTrapezoidBounds.h"

namespace Trk {
    class SurfaceBounds;
}
namespace {
    static const Amg::Vector3D x_axis{1., 0., 0.};
    static const Amg::Vector3D y_axis{0., 1., 0.};
    static const Amg::Vector3D z_axis{0., 0., 1.};
}
namespace MuonGM {

    //============================================================================
    MMReadoutElement::MMReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, int zi, int fi, int mL, bool is_mirrored, MuonDetectorManager* mgr, const NswPassivationDbData* passivData) 
    : MuonClusterReadoutElement(pv, stName, zi, fi, is_mirrored, mgr),
      m_passivData(passivData),
      m_ml(mL) {

        // get the setting of the caching flag from the manager
        setCachingFlag(mgr->cachingFlag());

        std::string vName = pv->getLogVol()->getName();
        std::string sName = vName.substr(vName.find('-') + 1);
        std::string fixName = (sName[2] == 'L') ? "MML" : "MMS";

        setStationName(fixName);
        setStationEta(zi);
        setStationPhi(fi);
        setChamberLayer(mL);
        Identifier id = mgr->mmIdHelper()->channelID(fixName, zi, fi, mL, 1, 1);
        setIdentifier(id);

        bool foundShape = false;

        if (!mgr->MinimalGeoFlag()) {
            if (GeoFullPhysVol* pvc = dynamic_cast<GeoFullPhysVol*>(pv)) {
                unsigned int nchildvol = pvc->getNChildVols();
                int llay = 0;
                std::string::size_type npos;
                for (unsigned ich = 0; ich < nchildvol; ++ich) {
                    PVConstLink pc = pvc->getChildVol(ich);
                    std::string childname = (pc->getLogVol())->getName();
                    if ((npos = childname.find("Sensitive")) != std::string::npos) {
                        ++llay;
                        if (llay > 4) {
                            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
                            log << MSG::WARNING << "number of MM layers > 4: increase transform array size" << endmsg;
                            continue;
                        }
                        m_Xlg[llay - 1] = pvc->getXToChildVol(ich);
                        // save layer dimensions
                        if (llay == 1) {
                            if (pc->getLogVol()->getShape()->type() == "Trd") {
                                const GeoTrd* trd = dynamic_cast<const GeoTrd*>(pc->getLogVol()->getShape());
                                m_halfX = trd->getZHalfLength();
                                // adjust phi dimensions according to the active area
                                m_minHalfY = trd->getYHalfLength1();
                                m_maxHalfY = trd->getYHalfLength2();
                                foundShape = true;
                            } else if (pc->getLogVol()->getShape()->type() == "Subtraction") {
                                const GeoShapeSubtraction* sh = dynamic_cast<const GeoShapeSubtraction*>(pc->getLogVol()->getShape());
                                const GeoShape* sub = sh->getOpA();

                                while (sub->type() == "Subtraction") {
                                    sh = dynamic_cast<const GeoShapeSubtraction*>(sub);
                                    sub = sh->getOpA();
                                }
                                const GeoTrd* trd = dynamic_cast<const GeoTrd*>(sub);

                                if (!trd) {
                                    MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
                                    log << MSG::WARNING << "MM layer base shape not trapezoid? " << sub->type() << endmsg;
                                } else {
                                    m_halfX = trd->getZHalfLength();
                                    m_minHalfY = trd->getYHalfLength1();
                                    m_maxHalfY = trd->getYHalfLength2();
                                    foundShape = true;
                                }
                            } else {
                                MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
                                log << MSG::WARNING << "MM layer shape not recognized:" << pc->getLogVol()->getShape()->type() << endmsg;
                            }
                        }
                    }
                }
                m_nlayers = llay;
            } else {
                MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
                log << MSG::WARNING << "Cannot perform a dynamic cast ! " << endmsg;
            }
        }
        if (!foundShape) {
            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
            log << MSG::WARNING << " failed to initialize dimensions of this chamber " << endmsg;
        }
    }


    //============================================================================
    MMReadoutElement::~MMReadoutElement() { clearCache(); }


    //============================================================================
    void MMReadoutElement::setIdentifier(const Identifier& id) {
        m_id = id;
        IdentifierHash collIdhash = 0;
        IdentifierHash detIdhash = 0;
        // set parent data collection hash id
        if (manager()->mmIdHelper()->get_module_hash(id, collIdhash) != 0) {
            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
            log << MSG::WARNING
                << "MMReadoutElement --  collection hash Id NOT computed for id = " << manager()->mmIdHelper()->show_to_string(id)
                << endmsg;
        }
        m_idhash = collIdhash;
        // // set RE hash id
        if (manager()->mmIdHelper()->get_detectorElement_hash(id, detIdhash) != 0) {
            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
            log << MSG::WARNING
                << "MMReadoutElement --  detectorElement hash Id NOT computed for id = " << manager()->mmIdHelper()->show_to_string(id)
                << endmsg;
        }
        m_detectorElIdhash = detIdhash;
    }


    //============================================================================
    void MMReadoutElement::initDesign(double /*maxY*/, double /*minY*/, double /*xS*/, double /*pitch*/, double /*thickness*/) {
       m_etaDesign.clear();
       m_etaDesign.resize(m_nlayers);

        if (m_ml < 1 || m_ml > 2) {
            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
            log << MSG::WARNING << "MMReadoutElement -- Unexpected Multilayer: m_ml= " << m_ml << endmsg;
            return;
        }
        
        char side     = getStationEta() < 0 ? 'C' : 'A';
        char sector_l = getStationName().substr(2, 1) == "L" ? 'L' : 'S';
        MMDetectorHelper aHelper;
        MMDetectorDescription* mm   = aHelper.Get_MMDetector(sector_l, std::abs(getStationEta()), getStationPhi(), m_ml, side);
        MMReadoutParameters roParam = mm->GetReadoutParameters();

        double ylFrame  = mm->ylFrame();
        double ysFrame  = mm->ysFrame();
        double pitch    = roParam.stripPitch;
        m_sWidthChamber = mm->sWidth();                   // bottom base length (full chamber)
        m_lWidthChamber = mm->lWidth();                   // top base length (full chamber)
        m_lengthChamber = mm->Length();                   // height of the trapezoid (full chamber)
        m_tckChamber    = mm->Tck();                      // thickness (full chamber)
        m_halfX         = roParam.activeH / 2;            // 0.5*radial_size (active area)
        m_minHalfY      = roParam.activeBottomLength / 2; // 0.5*bottom length (active area)
        m_maxHalfY      = roParam.activeTopLength / 2;    // 0.5*top length (active area)
        m_offset        = -0.5*(ylFrame - ysFrame);       // radial dist. of active area center w.r.t. chamber center
       
        for (int il = 0; il < m_nlayers; il++) {
            // identifier of the first channel to retrieve max number of strips
            Identifier id = manager()->mmIdHelper()->channelID(getStationName(), getStationEta(), getStationPhi(), m_ml, il + 1, 1);
            int chMax = manager()->mmIdHelper()->channelMax(id);
            if (chMax < 0) {
                chMax = 2500;
                MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
                log << MSG::WARNING << "MMReadoutElement -- Max number of strips not a valid value" << endmsg;
            }
            MuonChannelDesign& design = m_etaDesign[il];

            design.type                = MuonChannelDesign::ChannelType::etaStrip;
            design.detType             = MuonChannelDesign::DetType::MM;
            design.inputPitch          = pitch;
            design.thickness           = roParam.gasThickness; 
            design.nMissedTopEta       = roParam.nMissedTopEta;  // #of eta strips that are not connected to any FE board
            design.nMissedBottomEta    = roParam.nMissedBottomEta;
            design.nMissedTopStereo    = roParam.nMissedTopStereo;  // #of stereo strips that are not connected to any FE board
            design.nMissedBottomStereo = roParam.nMissedBottomStereo;
            design.totalStrips         = roParam.tStrips;   
            /// The stereo angle is defined clock-wise from the y-axis
            design.defineTrapezoid(m_minHalfY, m_maxHalfY,m_halfX, - roParam.stereoAngle.at(il));
            /// Input width is defined as the distance between two channels
            design.inputWidth          = pitch * std::cos(design.stereoAngle());
           
            m_nStrips.push_back(design.totalStrips);
            if (!design.hasStereoAngle()) {  // eta layers
                design.nch      = design.totalStrips - design.nMissedBottomEta - design.nMissedTopEta;
                design.setFirstPos(-0.5 * design.xSize() + pitch);
            } else {  // stereo layers
                design.nch      = design.totalStrips - design.nMissedBottomStereo - design.nMissedTopStereo;
                design.setFirstPos( -0.5 * design.xSize() + (1 + design.nMissedBottomStereo - design.nMissedBottomEta) * pitch);
            }

            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
            if (log.level() <= MSG::DEBUG)
                log << MSG::DEBUG << "initDesign:" << getStationName() << " layer " << il << ", strip pitch " << design.inputPitch
                    << ", nstrips " << design.nch << " stereo " << design.stereoAngle() / Gaudi::Units::degree << endmsg;
        }
    }


    //============================================================================
    void MMReadoutElement::fillCache() {
        if (!m_surfaceData)
            m_surfaceData = std::make_unique<SurfaceData>();
        else {
            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
            log << MSG::WARNING << "calling fillCache on an already filled cache" << endmsg;
            return;
        }

        

#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
#endif
        for (int layer = 0; layer < m_nlayers; ++layer) {
            // identifier of the first channel
            Identifier id = manager()->mmIdHelper()->channelID(getStationName(), getStationEta(), getStationPhi(), m_ml, layer + 1, 1);
            const double sAngle = m_etaDesign[layer].stereoAngle();
            m_surfaceData->m_layerSurfaces.emplace_back(std::make_unique<Trk::PlaneSurface>(*this, id));
            m_surfaceData->m_surfBounds.emplace_back(std::make_unique<Trk::RotatedTrapezoidBounds>(m_halfX, m_minHalfY, m_maxHalfY, sAngle));
         
            m_surfaceData->m_layerTransforms.push_back(
                absTransform()                         // transformation from chamber to ATLAS frame
                * m_delta                              // rotations (a-lines) from the alignment group
                * m_Xlg[layer]                         // x-shift of the gas-gap center w.r.t. quadruplet center
                * Amg::Translation3D(0., 0., m_offset) // z-shift to volume center
                * Amg::AngleAxis3D(-90. * CLHEP::deg, y_axis) // x<->z because of GeoTrd definition
                * Amg::AngleAxis3D(sAngle, z_axis)); 

            // surface info (center, normal)
            m_surfaceData->m_layerCenters.push_back(m_surfaceData->m_layerTransforms.back().translation());
            m_surfaceData->m_layerNormals.push_back(m_surfaceData->m_layerTransforms.back().linear() * (-z_axis));

#ifndef NDEBUG
           
            if (log.level() <= MSG::DEBUG)
                log << MSG::DEBUG << "MMReadoutElement layer " << layer << " sAngle " << sAngle << " phi direction MM eta strip "
                    << (m_surfaceData->m_layerTransforms.back().linear() * y_axis).phi() << endmsg;
#endif
        }
    }


    //============================================================================
    bool MMReadoutElement::containsId(const Identifier& id) const {
        if (manager()->mmIdHelper()->stationEta(id) != getStationEta()) return false;
        if (manager()->mmIdHelper()->stationPhi(id) != getStationPhi()) return false;

        if (manager()->mmIdHelper()->multilayerID(id) != m_ml) return false;

        int gasgap = manager()->mmIdHelper()->gasGap(id);
        if (gasgap < 1 || gasgap > m_nlayers) return false;

        int strip = manager()->mmIdHelper()->channel(id);
        if (strip < 1 || strip > m_nStrips[gasgap - 1]) return false;

        return true;
    }


    //============================================================================
    Amg::Vector3D MMReadoutElement::localToGlobalCoords(const Amg::Vector3D& locPos, const Identifier& id) const {
        int gg = manager()->mmIdHelper()->gasGap(id);
        //const MuonChannelDesign* design = getDesign(id);
        Amg::Vector3D locPos_ML = (m_Xlg[gg - 1]) * Amg::Translation3D(0., 0., m_offset) * 
                                 //   (design->hasStereoAngle() ?  
                                 //    Amg::AngleAxis3D(90. * CLHEP::deg, y_axis) * Amg::AngleAxis3D(design->stereoAngle(), z_axis)  *
                                 //    Amg::AngleAxis3D(-90. * CLHEP::deg, y_axis)  : AmgSymMatrix(3)::Identity())*
        locPos;
       
#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
        if (log.level() <= MSG::DEBUG) {
            log << MSG::DEBUG << "position coordinates in the gas-gap r.f.:    "  << locPos << endmsg;
            log << MSG::DEBUG << "position coordinates in the multilayer r.f.: " << locPos_ML << endmsg;
        }
#endif
        Amg::Vector3D gVec = absTransform() * m_delta * locPos_ML;
        return gVec;
    }


    //============================================================================
    void MMReadoutElement::setDelta(const ALinePar& aline) {

        // amdb frame (s, z, t) = chamber frame (y, z, x)
        float tras{0.}, traz{0.}, trat{0.}, rots{0.}, rotz{0.}, rott{0.};
        aline.getParameters(tras, traz, trat, rots, rotz, rott);
        if (std::abs(tras) + std::abs(traz) + std::abs(trat) + std::abs(rots) + std::abs(rotz) + std::abs(rott) > 1e-5) {
            m_delta = Amg::Translation3D(0., tras, 0.) // translations (applied after rotations)
                    * Amg::Translation3D(0., 0., traz)  
                    * Amg::Translation3D(trat, 0., 0.) 
                    * Amg::AngleAxis3D(rots, y_axis)  // rotation about Y (applied 3rd)
                    * Amg::AngleAxis3D(rotz, z_axis)  // rotation about Z (applied 2nd)
                    * Amg::AngleAxis3D(rott, x_axis); // rotation about X (applied 1st)

            // The origin of the rotation axes is at the center of the active area 
            // in the z (radial) direction. Account for this shift in the definition 
            // of m_delta so that it can be applied on chamber frame coordinates.
            Amg::Translation3D t(0., 0., m_offset);
            m_ALinePar  = &aline;
            m_delta     = t*m_delta*t.inverse();
            refreshCache();
        } else {  
            clearALinePar();
        }
    }


    //============================================================================
    void MMReadoutElement::setDelta(MuonDetectorManager* mgr) {

        const ALineMapContainer* alineMap = mgr->ALineContainer();
        Identifier id = mgr->mmIdHelper()->elementID(getStationName(), getStationEta(), getStationPhi());
        Identifier idMult = mgr->mmIdHelper()->multilayerID(id, m_ml);
        ALineMapContainer::const_iterator it = alineMap->find(idMult);

        if (it == alineMap->end()) {
            clearALinePar();
            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
            if (log.level() <= MSG::DEBUG) { log << MSG::DEBUG << "m_aLineMapContainer does not contain ALine for MM" << endmsg; }
        } else {
            setDelta(it->second);
        }
    }

    //============================================================================
    void MMReadoutElement::clearALinePar() {
        if (has_ALines()) {
            m_ALinePar = nullptr; 
            m_delta = Amg::Transform3D::Identity(); 
            refreshCache();
        }
    }

    //============================================================================
    void MMReadoutElement::setBLinePar(const BLinePar& bLine) {
        MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
        if (log.level() <= MSG::DEBUG)
            log << MSG::DEBUG << "Setting B-line for " << getStationName().substr(0, 3) << " at eta/phi " << getStationEta() << "/" << getStationPhi() << endmsg;
        m_BLinePar = &bLine;
    }


    //============================================================================
    void MMReadoutElement::setBLinePar(MuonDetectorManager* mgr) {
        const BLineMapContainer* blineMap = mgr->BLineContainer();
        Identifier id     = mgr->mmIdHelper()->elementID(getStationName(), getStationEta(), getStationPhi());
        Identifier idMult = mgr->mmIdHelper()->multilayerID(id, m_ml);
        BLineMapContainer::const_iterator it = blineMap->find(idMult);

        if (it == blineMap->end()) {
            clearBLinePar();
            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
            if (log.level() <= MSG::DEBUG) { log << MSG::DEBUG << "m_bLineMapContainer does not contain BLine for MM" << endmsg; }
        } else {
            setBLinePar(it->second);
        }
    }


    //============================================================================
    void MMReadoutElement::posOnDefChamber(Amg::Vector3D& locPosML) const {

        // note: amdb frame (s, z, t) = chamber frame (y, z, x)
        if (!has_BLines()) return;

        double t0    = locPosML.x();
        double s0    = locPosML.y();
        double z0    = locPosML.z();
        double width = m_sWidthChamber + (m_lWidthChamber - m_sWidthChamber)*(z0/m_lengthChamber + 0.5); // because z0 is in [-length/2, length/2]

        double s_rel = s0/(width/2.);           // in [-1, 1]
        double z_rel = z0/(m_lengthChamber/2.); // in [-1, 1]
        double t_rel = t0/(m_tckChamber/2.);    // in [-1, 1]

        // b-line parameters
        double bp    = m_BLinePar->bp();
        double bn    = m_BLinePar->bn();
        double sp    = m_BLinePar->sp();
        double sn    = m_BLinePar->sn();
        double tw    = m_BLinePar->tw();
        double eg    = m_BLinePar->eg()*1.e-3;
        double ep    = m_BLinePar->ep()*1.e-3;
        double en    = m_BLinePar->en()*1.e-3;

        double ds{0.}, dz{0.}, dt{0.};

        if (bp != 0 || bn != 0)
            dt += 0.5*(s_rel*s_rel - 1)*((bp + bn) + (bp - bn)*z_rel);

        if (sp != 0 || sn != 0)
            dt += 0.5*(z_rel*z_rel - 1)*((sp + sn) + (sp - sn)*s_rel);

        if (tw != 0) {
            dt -= tw*s_rel*z_rel;
            dz += tw*s_rel*t_rel*m_tckChamber/m_lengthChamber;
        }

        if (eg != 0) {
            dt += t0*eg;
            ds += s0*eg;
            dz += z0*eg;
        }

        if (ep != 0 || en != 0) {
            // the formulas below differ from those in Christoph's talk
            // because are origin for NSW is at the center of the chamber, 
            // whereas in the talk (i.e. MDTs), it is at the bottom!
            double delta = s_rel*s_rel * ((ep + en)*s_rel/6 + (ep - en)/4);
            double phi   = s_rel * ((ep + en)*s_rel + (ep - en)) / 2;
            dt += phi*t0;
            ds += delta*width/2;
            dz += phi*z0;
        }

        locPosML[0] += dt;
        locPosML[1] += ds;
        locPosML[2] += dz;
    }

    void MMReadoutElement::refreshCache() {
        clearCache();
        fillCache();
    }
    

    //============================================================================
    bool MMReadoutElement::spacePointPosition(const Identifier& layerId, const Amg::Vector2D& lpos, Amg::Vector3D& pos) const {

        pos = Amg::Vector3D(lpos.x(), lpos.y(), 0.);

        const MuonChannelDesign* design = getDesign(layerId);
        if (!design) {
            MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
            log << MSG::WARNING << "Unable to get MuonChannelDesign, therefore cannot provide position corrections. Returning." << endmsg;
            return false;
        }

        bool conditionsApplied{false};
        Amg::Transform3D trfToML{Amg::Transform3D::Identity()};

#ifndef SIMULATIONBASE
        //*********************
        // As-Built (MuonNswAsBuilt is not included in AthSimulation)
        //*********************
        const NswAsBuilt::StripCalculator* sc = manager()->getMMAsBuiltCalculator();
        if (sc) {

            // express the local position w.r.t. the nearest active strip
            Amg::Vector2D rel_pos;
            int istrip = design->positionRelativeToStrip(lpos, rel_pos); 
            if (istrip < 0) {
                MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
                log << MSG::WARNING << "As-built corrections are provided only within the active area. Returning." << endmsg;                
                return false;
            } 

            // setup strip calculator
            NswAsBuilt::stripIdentifier_t strip_id;
            strip_id.quadruplet = { (largeSector() ? NswAsBuilt::quadrupletIdentifier_t::MML : NswAsBuilt::quadrupletIdentifier_t::MMS), getStationEta(), getStationPhi(), m_ml };
            strip_id.ilayer     = manager()->mmIdHelper()->gasGap(layerId);
            strip_id.istrip     = istrip;

            // get the position coordinates, in the chamber frame, from NswAsBuilt.
            // Applying a 2.75mm correction along the layer normal, since NswAsBuilt considers the layer 
            // on the readout strips, whereas Athena wants it at the middle of the drift gap.
            NswAsBuilt::StripCalculator::position_t calcPos = sc->getPositionAlongStrip(NswAsBuilt::Element::ParameterClass::CORRECTION, strip_id, rel_pos.y(), rel_pos.x());
            
            if (calcPos.isvalid == NswAsBuilt::StripCalculator::IsValid::VALID) {
                pos     = calcPos.pos;
                pos[0] += strip_id.ilayer%2 ? -2.75 : 2.75;
            
                // signal that pos is now in the chamber reference frame
                // (don't go back to the layer frame yet, since we may apply b-lines later on)
                trfToML = m_delta.inverse()*absTransform().inverse()*transform(layerId);
                conditionsApplied = true;
            } 
#ifndef NDEBUG
            else {
                MsgStream log(Athena::getMessageSvc(), "MMReadoutElement");
                if (log.level() <= MSG::DEBUG) {    
                    log << MSG::DEBUG << "No as-built corrections provided for stEta: "<<getStationEta() << " stPhi: "<<getStationPhi()<<" ml: "<<m_ml<<" layer: "<<strip_id.ilayer<< endmsg;
                }
            }
#endif
        }
#endif 

        //*********************
        // B-Lines
        //*********************
        if (has_BLines()) {
          // go to the multilayer reference frame if we are not already there
          if (!conditionsApplied) {
             trfToML = m_delta.inverse()*absTransform().inverse()*transform(layerId);
             pos = trfToML*pos;
             
             // signal that pos is now in the multilayer reference frame
             conditionsApplied = true; 
          }
          posOnDefChamber(pos);
       }

        // back to the layer reference frame from where we started
        if (conditionsApplied) pos = trfToML.inverse()*pos;
        
        return true;
    }
    
}  // namespace MuonGM
