/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 The sTgc detector = an assembly module = STGC in amdb
 ----------------------------------------------------
***************************************************************************/

#include "MuonReadoutGeometry/sTgcReadoutElement.h"

#include <GaudiKernel/IMessageSvc.h>
#include <GeoModelKernel/GeoLogVol.h>
#include <GeoModelKernel/GeoVFullPhysVol.h>
#include <GeoModelKernel/GeoVPhysVol.h>
#include <TString.h>

#include <cmath>
#include <ext/alloc_traits.h>
#include <map>
#include <memory>
#include <stdexcept>
#include <utility>

#include "AthenaKernel/getMessageSvc.h"
#include "EventPrimitives/AmgMatrixBasePlugin.h"
#include "GaudiKernel/MsgStream.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "Identifier/IdentifierHash.h"
#include "MuonAGDDDescription/sTGCDetectorDescription.h"
#include "MuonAGDDDescription/sTGCDetectorHelper.h"
#include "MuonAGDDDescription/sTGC_Technology.h"
#include "MuonAlignmentData/ALinePar.h"
#include "MuonAlignmentData/CorrContainer.h"
#include "TrkSurfaces/DiamondBounds.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "TrkSurfaces/RotatedDiamondBounds.h"
#include "TrkSurfaces/RotatedTrapezoidBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"

#include "GaudiKernel/ISvcLocator.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "GeoModelInterfaces/IGeoDbTagSvc.h"

namespace MuonGM {

    //============================================================================
    sTgcReadoutElement::sTgcReadoutElement(GeoVFullPhysVol* pv, const std::string& stName, int zi, int fi, int mL, MuonDetectorManager* mgr) 
    : MuonClusterReadoutElement(pv, mgr, Trk::DetectorElemType::sTgc)
    , m_ml(mL) {

        // get the setting of the caching flag from the manager
        setCachingFlag(mgr->cachingFlag());

        std::string fixName = (stName[1] == 'L') ? "STL" : "STS";
        Identifier id = mgr->stgcIdHelper()->channelID(fixName, zi, fi, mL, 1, 2, 1);

        setStationName(fixName);       
        setChamberLayer(mL);
        setIdentifier(id); // representative identifier, with stName, stEta, stPhi, mL 

#ifndef NDEBUG
        sTGCDetectorHelper sTGC_helper;
        std::string sTGCname = std::string("sTG1-") + stName;
        MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
        if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "sTGCname: " << sTGCname << endmsg;
        sTGCDetectorDescription* sTGC = sTGC_helper.Get_sTGCDetectorType(sTGCname);
        if (not sTGC){
          log <<  MSG::FATAL << "sTGC helper could not be retrieved in sTgcReadoutElement constructor "<<endmsg;
          return;
        }
        if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Found sTGC detector: " << sTGCname << " " << sTGC << endmsg;
        static const int nLayers = 4;
        for (int layer = 0; layer < nLayers; layer++) {
            double length  = sTGC->Length();   // Distance between parallel sides of the trapezoid
            double ysFrame = sTGC->ysFrame();  // Frame thickness on short parallel edge
            double ylFrame = sTGC->ylFrame();  // Frame thickness on long parallel edge
            if (log.level() <= MSG::DEBUG)
                log << MSG::DEBUG << "length: " << length << " ysFrame: " << ysFrame << " ylFrame: " << ylFrame << endmsg;
        }
#endif

        if (mgr->MinimalGeoFlag() == 0) {
            if (GeoFullPhysVol* pvc = dynamic_cast<GeoFullPhysVol*>(pv)) {
                unsigned int nchildvol = pvc->getNChildVols();
                int llay = 0;
                std::string::size_type npos;
                for (unsigned ich = 0; ich < nchildvol; ++ich) {
                    PVConstLink pc = pvc->getChildVol(ich);
                    std::string childname = (pc->getLogVol())->getName();
#ifndef NDEBUG
                    if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "Volume Type: " << pc->getLogVol()->getShape()->type() << endmsg;
#endif
                    if ((npos = childname.find("Sensitive")) != std::string::npos) {
                        llay++;
                        if (llay > 4) {
#ifndef NDEBUG
                            if (log.level() <= MSG::DEBUG)
                                log << MSG::DEBUG << "number of sTGC layers > 4: increase transform array size" << endmsg;
#endif
                            continue;
                        }
                        m_Xlg[llay - 1] = pvc->getXToChildVol(ich);
                    }
                }
                m_nlayers = llay;
            } else {
                throw std::runtime_error(Form(
                    "File: %s, Line: %d\nsTgcReadoutElement::sTgcReadoutElement() - Cannot perform a dynamic cast !", __FILE__, __LINE__));
            }
        }
    }


    //============================================================================
    sTgcReadoutElement::~sTgcReadoutElement() { clearCache(); }

    //============================================================================
  void sTgcReadoutElement::initDesign(double /*largeX*/, double /*smallX*/, double /*lengthY*/, double /*stripPitch*/,
                      double /*wirePitch*/, double /*stripWidth*/, double /*wireWidth*/, double thickness) {
    
    
    MsgStream log(Athena::getMessageSvc(), "sTGCReadoutElement");
    ISvcLocator* svcLocator = Gaudi::svcLocator(); // from Bootstrap
    IGeoDbTagSvc* geoDbTag{nullptr};
    StatusCode sc = svcLocator->service("GeoDbTagSvc",geoDbTag);
    if (sc.isFailure()) log << MSG::FATAL << "Could not locate GeoDbTagSvc" << endmsg;
    GeoModelIO::ReadGeoModel* sqliteReader = geoDbTag->getSqliteReader();
    if (sqliteReader) {
      IRDBAccessSvc *accessSvc{nullptr};
      StatusCode sc=svcLocator->service(geoDbTag->getParamSvcName(),accessSvc);
      if (sc.isFailure()) log << MSG::FATAL << "Could not locate " << geoDbTag->getParamSvcName() << endmsg;
      IRDBRecordset_ptr nswdimRec = accessSvc->getRecordsetPtr("NSWDIM","","");
      IRDBRecordset_ptr wstgcRec  = accessSvc->getRecordsetPtr("WSTGC","","");
      IRDBRecordset_ptr nswPars   = accessSvc->getRecordsetPtr("NSWPARS","","");

      PVConstLink parent = getMaterialGeom()->getParent();
      unsigned int index=parent->indexOf(getMaterialGeom());
      std::string pVName=parent->getNameOfChildVol(index);
      float yCutoutCathode(0);
      if (nswPars->size()==0) {
    throw std::runtime_error("Error, cannot access NSWPARS record!");
      }
      else {
    yCutoutCathode=(*nswPars)[0]->getFloat("NSW_sTGC_yCutoutCathode");
      }

      for (unsigned int ind = 0; ind < wstgcRec->size(); ind++) {
    std::string WSTGC_TYPE       = (*wstgcRec)[ind]->getString("WSTGC_TYPE");               
    
    if (getStationName()[2] != WSTGC_TYPE[6])            continue;
    if (abs(getStationEta())!=(int) (WSTGC_TYPE[7]-'0')) continue;
    if (m_ml != (int) (pVName[7]-'0'))                   continue;
    const IRDBRecord *nswdim{nullptr};
    for (size_t w=0;w<nswdimRec->size();w++) {
      nswdim = (*nswdimRec)[w];
      break;
    }
    
    m_sWidthChamber = nswdim->getDouble("BASE_WIDTH");;         // bottom base length (full chamber)
    m_lWidthChamber = nswdim->getDouble("TOP_WIDTH");           // top base length (full chamber)
    m_lengthChamber = nswdim->getDouble("LENGTH");              // height of the trapezoid (full chamber)
    
    double      gasTck                = (*wstgcRec)[ind]->getDouble("gasTck");                 
    double      Tck                   = (*wstgcRec)[ind]->getDouble("Tck");                    
    double      xFrame                = (*wstgcRec)[ind]->getDouble("xFrame");                 
    double      ylFrame               = (*wstgcRec)[ind]->getDouble("ylFrame");                
    double      ysFrame               = (*wstgcRec)[ind]->getDouble("ysFrame");                
    double      wirePitch             = (*wstgcRec)[ind]->getDouble("wirePitch");              
    double      stripWidth            = (*wstgcRec)[ind]->getDouble("stripWidth");             
    double      sPadWidth             = (*wstgcRec)[ind]->getDouble("sPadWidth");              
    double      lPadWidth             = (*wstgcRec)[ind]->getDouble("lPadWidth");              
    double      anglePadPhi           = (*wstgcRec)[ind]->getDouble("anglePadPhi");            
    double      sStripWidth           = (*wstgcRec)[ind]->getDouble("sStripWidth");            
    double      lStripWidth           = (*wstgcRec)[ind]->getDouble("lStripWidth");            
    int         wireGroupWidth        = (*wstgcRec)[ind]->getInt("wireGroupWidth");            
    int         nStrips               = (*wstgcRec)[ind]->getInt("nStrips");                   
    std::string padH                  = (*wstgcRec)[ind]->getString("padH");                   
    std::string rankPadPhi            = (*wstgcRec)[ind]->getString("rankPadPhi");             
    std::string nPadPhi               = (*wstgcRec)[ind]->getString("nPadPhi");                
    std::string firstPadPhiDivision_C = (*wstgcRec)[ind]->getString("firstPadPhiDivision_C");  
    std::string PadPhiShift_C         = (*wstgcRec)[ind]->getString("PadPhiShift_C");          
    std::string firstPadPhiDivision_A = (*wstgcRec)[ind]->getString("firstPadPhiDivision_A");  
    std::string PadPhiShift_A         = (*wstgcRec)[ind]->getString("PadPhiShift_A");          
    std::string rankPadH              = (*wstgcRec)[ind]->getString("rankPadH");               
    std::string nPadH                 = (*wstgcRec)[ind]->getString("nPadH");                  
    std::string firstPadH             = (*wstgcRec)[ind]->getString("firstPadH");              
    std::string firstPadRow           = (*wstgcRec)[ind]->getString("firstPadRow");            
    std::string wireCutout            = (*wstgcRec)[ind]->getString("wireCutout");             
    std::string nWires                = (*wstgcRec)[ind]->getString("nWires");                 
    std::string firstWire             = (*wstgcRec)[ind]->getString("firstWire");              
    std::string firstTriggerBand      = (*wstgcRec)[ind]->getString("firstTriggerBand");       
    std::string nTriggerBands         = (*wstgcRec)[ind]->getString("nTriggerBands");          
    std::string firstStripInTrigger   = (*wstgcRec)[ind]->getString("firstStripInTrigger");    
    std::string firstStripWidth       = (*wstgcRec)[ind]->getString("firstStripWidth");        
    std::string StripsInBandsLayer1   = (*wstgcRec)[ind]->getString("StripsInBandsLayer1");    
    std::string StripsInBandsLayer2   = (*wstgcRec)[ind]->getString("StripsInBandsLayer2");    
    std::string StripsInBandsLayer3   = (*wstgcRec)[ind]->getString("StripsInBandsLayer3");    
    std::string StripsInBandsLayer4   = (*wstgcRec)[ind]->getString("StripsInBandsLayer4");    
    std::string nWireGroups           = (*wstgcRec)[ind]->getString("nWireGroups");            
    std::string firstWireGroup        = (*wstgcRec)[ind]->getString("firstWireGroup");         
    
    for (std::string * s : {
             &padH                 ,
          &rankPadPhi           ,
          &nPadPhi              ,
          &firstPadPhiDivision_C,
          &PadPhiShift_C        ,
          &firstPadPhiDivision_A,
          &PadPhiShift_A        ,
          &rankPadH             ,
          &nPadH                ,
          &firstPadH            ,
          &firstPadRow          ,
          &wireCutout           ,
          &nWires               ,
          &firstWire            ,
          &firstTriggerBand     ,
          &nTriggerBands        ,
          &firstStripInTrigger  ,
          &firstStripWidth      ,
          &StripsInBandsLayer1  ,
          &StripsInBandsLayer2  ,
          &StripsInBandsLayer3  ,
          &StripsInBandsLayer4  ,
          &nWireGroups          ,
          &firstWireGroup       
        }) {
      std::replace(s->begin(),s->end(),';',' ');
    }

    
    char sector_l  = getStationName().substr(2, 1) == "L" ? 'L' : 'S';
    int  stEta     = std::abs(getStationEta());
    int  Etasign   = getStationEta() / stEta;
    std::string side = (Etasign > 0) ? "A" : "C";
    m_diamondShape = sector_l == 'L' && stEta == 3;
    
    m_phiDesign = std::vector<MuonChannelDesign>(m_nlayers);
    m_etaDesign = std::vector<MuonChannelDesign>(m_nlayers);
    m_padDesign = std::vector<MuonPadDesign>(m_nlayers);
    
    // Get frame widths
    m_tckChamber    = Tck;            // thickness (full chamber)

    double yCutout  = getStationName().substr(0,3)=="QL3" ? yCutoutCathode: 0.0; // y of cutout of trapezoid (only in outermost detectors)

    // For strips:
    m_halfX        = std::vector<double>(m_nlayers);
    m_minHalfY     = std::vector<double>(m_nlayers);
    m_maxHalfY     = std::vector<double>(m_nlayers);
    // For pads and wires:
    m_PadhalfX     = std::vector<double>(m_nlayers);
    m_PadminHalfY  = std::vector<double>(m_nlayers);
    m_PadmaxHalfY  = std::vector<double>(m_nlayers);
    
    // Radial shift of the local frame origin w.r.t. the center of the quadruplet.
    // For diamond shape (QL3) the origin is on the cutout base. For the rest, the it is at the center 
    // of the active area, therefore the shift is half the difference of the top and bottom frame widths.
    m_offset = (m_diamondShape) ? 0.5*m_lengthChamber - (yCutout + ylFrame) : -0.5*(ylFrame - ysFrame); 
    
    //-------------------
    // Strips
    //-------------------
    std::istringstream firstStripWidthStream(firstStripWidth);
    for (int il = 0; il < m_nlayers; il++) {
      // identifier of the first channel - strip plane - to retrieve max number of strips
      /*Identifier id = manager()->stgcIdHelper()->channelID(getStationName(),getStationEta(),getStationPhi(),m_ml, il+1, 1, 1);
        int chMax =  manager()->stgcIdHelper()->channelMax(id);
        if (chMax<0) chMax = 350;*/
      
      m_etaDesign[il].type        = MuonChannelDesign::ChannelType::etaStrip;
      m_etaDesign[il].detType     = MuonChannelDesign::DetType::STGC;
      if (yCutout == 0.)
        m_etaDesign[il].defineTrapezoid(0.5 * sStripWidth, 0.5 * lStripWidth, 0.5 * (m_lengthChamber - ysFrame - ylFrame));
      else 
        m_etaDesign[il].defineDiamond(0.5 * sStripWidth, 0.5 * lStripWidth, 0.5 * (m_lengthChamber - ysFrame - ylFrame), yCutout);
      
      m_etaDesign[il].inputPitch  = stripWidth;
      m_etaDesign[il].inputWidth  = stripWidth;
      m_etaDesign[il].thickness   = gasTck;
      firstStripWidthStream >> m_etaDesign[il].firstPitch;
      m_etaDesign[il].setFirstPos((m_diamondShape) ? -(m_etaDesign[il].xSize()- yCutout) + m_etaDesign[il].firstPitch
                      : -0.5 * m_etaDesign[il].xSize()+ m_etaDesign[il].firstPitch);
      m_etaDesign[il].nch         = nStrips;
      
      m_nStrips.push_back(m_etaDesign[il].nch);
          
      m_halfX[il]    = 0.5*m_etaDesign[il].xSize();
      m_minHalfY[il] = 0.5*sStripWidth;
      m_maxHalfY[il] = 0.5*lStripWidth;
      
    }
    
    //-------------------
    // Wires
    //-------------------
    std::istringstream firstWireStream      (firstWire);
    std::istringstream firstWireGroupStream (firstWireGroup);
    std::istringstream nWireGroupsStream    (nWireGroups);
    std::istringstream wireCutoutStream     (wireCutout);
    std::istringstream nWiresStream         (nWires);
    
    for (int il = 0; il < m_nlayers; il++) {
      m_phiDesign[il].type        = MuonChannelDesign::ChannelType::phiStrip;
      m_phiDesign[il].detType     = MuonChannelDesign::DetType::STGC;
      if (yCutout == 0.)
        m_phiDesign[il].defineTrapezoid(0.5 * sPadWidth, 0.5 * lPadWidth, 0.5 * (m_lengthChamber - ysFrame - ylFrame) );
      else 
        m_phiDesign[il].defineDiamond(0.5 * sPadWidth, 0.5 * lPadWidth, 0.5 * (m_lengthChamber - ysFrame - ylFrame), yCutout);
      m_phiDesign[il].inputPitch  = wirePitch;
      m_phiDesign[il].inputWidth  = 0.015;
      m_phiDesign[il].thickness   = m_tckChamber;
      {int fw; firstWireStream >> fw; m_phiDesign[il].setFirstPos(fw);}       // Position of 1st wire, accounts for staggering
      firstWireGroupStream >> m_phiDesign[il].firstPitch;                     // Number of Wires in 1st group, group staggering
      m_phiDesign[il].groupWidth  = wireGroupWidth;                           // Number of Wires normal group
      nWireGroupsStream >> m_phiDesign[il].nGroups;                           // Number of Wire Groups
      wireCutoutStream >> m_phiDesign[il].wireCutout;                         // Size of "active" wire region for digits
      nWiresStream >> m_phiDesign[il].nch;
      
      m_nWires.push_back(m_phiDesign[il].nGroups);                            // number of nWireGroups
      
        }
    
        //-------------------
        // Pads
        //-------------------
    std::istringstream nPadPhiStream(nPadPhi);
    std::istringstream firstPadPhiDivision_AStream(firstPadPhiDivision_A);
    std::istringstream padPhiShift_AStream(PadPhiShift_A);
    std::istringstream firstPadRowStream(firstPadRow);
    std::istringstream nPadHStream(nPadH);
    std::istringstream firstPadHStream(firstPadH);
    std::istringstream padHStream(padH);

        double radius = absTransform().translation().perp() + m_offset;
        for (int il = 0; il < m_nlayers; il++) {
      m_padDesign[il].Length  = m_lengthChamber;
      m_padDesign[il].sWidth  = m_sWidthChamber;
      m_padDesign[il].lWidth  = m_lWidthChamber;
      m_padDesign[il].Size    = m_lengthChamber - ylFrame - ysFrame;
      m_padDesign[il].xFrame  = xFrame;
      m_padDesign[il].ysFrame = ysFrame;
      m_padDesign[il].ylFrame = ylFrame;
      m_padDesign[il].yCutout = yCutout;
      m_padDesign[il].etasign = Etasign;
      m_padDesign[il].setR(radius);
      m_padDesign[il].sPadWidth = sPadWidth;
      m_padDesign[il].lPadWidth = lPadWidth;
      nPadPhiStream >>m_padDesign[il].nPadColumns;
      
      m_PadhalfX[il]    = 0.5*m_padDesign[il].Size;
      m_PadminHalfY[il] = 0.5*sPadWidth;
      m_PadmaxHalfY[il] = 0.5*lPadWidth;
      
      // The C side of the NSW is mirrored instead of rotated
      // We should be using the same values for the pads for both A and C
      // It is easier for us to simply read the same correct value once
      // whereas changing the XML and the reading functions will make this incompatible with past versions
      // Alexandre Laurier 12 Sept 2018
      firstPadPhiDivision_AStream >> m_padDesign[il].firstPhiPos;
      m_padDesign[il].inputPhiPitch = anglePadPhi;                                            // stEta<2 ?  PAD_PHI_DIVISION/PAD_PHI_SUBDIVISION : PAD_PHI_DIVISION ;
      padPhiShift_AStream >> m_padDesign[il].PadPhiShift;
      firstPadRowStream >> m_padDesign[il].padEtaMin;                                         // FIRST_PAD_ROW_DIVISION[2*sector+(m_ml-1)][stEta-1][il];
      nPadHStream >> m_padDesign[il].nPadH;
      m_padDesign[il].padEtaMax     = m_padDesign[il].padEtaMin + m_padDesign[il].nPadH;      // PAD_ROWS[2*sector+(m_ml-1)][stEta-1][il];
      firstPadHStream >> m_padDesign[il].firstRowPos;                                         // H_PAD_ROW_0[2*sector+(m_ml-1)][il];
      padHStream >> m_padDesign[il].inputRowPitch;                                            // PAD_HEIGHT[2*sector+(m_ml-1)][il];
      
      if (sector_l == 'L') {
        m_padDesign[il].isLargeSector = 1;
        m_padDesign[il].sectorOpeningAngle = m_padDesign[il].largeSectorOpeningAngle;
      } else {
        m_padDesign[il].isLargeSector = 0;
        m_padDesign[il].sectorOpeningAngle = m_padDesign[il].smallSectorOpeningAngle;
      }
      
      m_padDesign[il].thickness = thickness;
        }      
      }
    }
    // AGDD STYLE STARTS HERE:
    else {
    char sector_l  = getStationName().substr(2, 1) == "L" ? 'L' : 'S';
    int  stEta     = std::abs(getStationEta());
    int  Etasign   = getStationEta() / stEta;
    std::string side = (Etasign > 0) ? "A" : "C";
    m_diamondShape = sector_l == 'L' && stEta == 3;
    
#ifndef NDEBUG
     MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
     if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "station name" << getStationName() << endmsg;
#endif
     
     sTGCDetectorHelper aHelper;
     sTGCDetectorDescription* stgc = aHelper.Get_sTGCDetector(sector_l, stEta, getStationPhi(), m_ml, side.back());
     
#ifndef NDEBUG
     log << MSG::DEBUG << "Found sTGC Detector " << stgc->GetName() << endmsg;
#endif
     
     auto *tech = stgc->GetTechnology();
     if (!tech)
       throw std::runtime_error(
                    Form("File: %s, Line: %d\nsTgcReadoutElement::initDesign() - Failed To get Technology for stgc element: %s", __FILE__,  __LINE__, stgc->GetName().c_str()));
     
     m_phiDesign = std::vector<MuonChannelDesign>(m_nlayers);
     m_etaDesign = std::vector<MuonChannelDesign>(m_nlayers);
     m_padDesign = std::vector<MuonPadDesign>(m_nlayers);
     
     // Get Chamber length, width and frame widths
     m_sWidthChamber = stgc->sWidth();         // bottom base length (full chamber)
     m_lWidthChamber = stgc->lWidth();         // top base length (full chamber)
     m_lengthChamber = stgc->Length();         // height of the trapezoid (full chamber)
     m_tckChamber    = stgc->Tck();            // thickness (full chamber)
     double ysFrame  = stgc->ysFrame();        // Frame thickness on short parallel edge
     double ylFrame  = stgc->ylFrame();        // Frame thickness on long parallel edge
     double xFrame   = stgc->xFrame();         // Frame thickness of non parallel edges
     double yCutout  = stgc->yCutoutCathode(); // y of cutout of trapezoid (only in outermost detectors)
     sTGCReadoutParameters roParam = stgc->GetReadoutParameters();
     
     // For strips:
     m_halfX        = std::vector<double>(m_nlayers);
     m_minHalfY     = std::vector<double>(m_nlayers);
     m_maxHalfY     = std::vector<double>(m_nlayers);
     // For pads and wires:
     m_PadhalfX     = std::vector<double>(m_nlayers);
     m_PadminHalfY  = std::vector<double>(m_nlayers);
     m_PadmaxHalfY  = std::vector<double>(m_nlayers);
     
     // Radial shift of the local frame origin w.r.t. the center of the quadruplet.
     // For diamond shape (QL3) the origin is on the cutout base. For the rest, the it is at the center 
     // of the active area, therefore the shift is half the difference of the top and bottom frame widths.
     m_offset = (m_diamondShape) ? 0.5*m_lengthChamber - (yCutout + ylFrame) : -0.5*(ylFrame - ysFrame); 
     
     //-------------------
     // Strips
     //-------------------
     
     for (int il = 0; il < m_nlayers; il++) {
       // identifier of the first channel - strip plane - to retrieve max number of strips
       /*Identifier id = manager()->stgcIdHelper()->channelID(getStationName(),getStationEta(),getStationPhi(),m_ml, il+1, 1, 1);
         int chMax =  manager()->stgcIdHelper()->channelMax(id);
         if (chMax<0) chMax = 350;*/
       
       m_etaDesign[il].type        = MuonChannelDesign::ChannelType::etaStrip;
       m_etaDesign[il].detType     = MuonChannelDesign::DetType::STGC;
       if (yCutout == 0.)
         m_etaDesign[il].defineTrapezoid(0.5 * roParam.sStripWidth, 0.5 * roParam.lStripWidth, 0.5 * (m_lengthChamber - ysFrame - ylFrame));
       else 
         m_etaDesign[il].defineDiamond(0.5 * roParam.sStripWidth, 0.5 * roParam.lStripWidth, 0.5 * (m_lengthChamber - ysFrame - ylFrame), yCutout);
       
       m_etaDesign[il].inputPitch  = stgc->stripPitch();
       m_etaDesign[il].inputWidth  = stgc->stripWidth();
       m_etaDesign[il].thickness   = tech->gasThickness;
       m_etaDesign[il].firstPitch  = roParam.firstStripWidth[il];
       m_etaDesign[il].setFirstPos((m_diamondShape) ? -(m_etaDesign[il].xSize()- yCutout) + m_etaDesign[il].firstPitch
                       : -0.5 * m_etaDesign[il].xSize()+ m_etaDesign[il].firstPitch);
       m_etaDesign[il].nch         = roParam.nStrips;
       
       m_nStrips.push_back(m_etaDesign[il].nch);
           
       m_halfX[il]    = 0.5*m_etaDesign[il].xSize();
       m_minHalfY[il] = 0.5*roParam.sStripWidth;
       m_maxHalfY[il] = 0.5*roParam.lStripWidth;
       
#ifndef NDEBUG
       if (log.level() <= MSG::DEBUG)
         log << MSG::DEBUG << "initDesign:" << getStationName() << " layer " << il << ", strip pitch " << m_etaDesign[il].inputPitch
         << ", nstrips " << m_etaDesign[il].nch << ", firstPos: " << m_etaDesign[il].firstPos() << endmsg;
#endif
     }
     
     //-------------------
     // Wires
     //-------------------
     
     for (int il = 0; il < m_nlayers; il++) {
       m_phiDesign[il].type        = MuonChannelDesign::ChannelType::phiStrip;
       m_phiDesign[il].detType     = MuonChannelDesign::DetType::STGC;
       if (yCutout == 0.)
         m_phiDesign[il].defineTrapezoid(0.5 * roParam.sPadWidth, 0.5 * roParam.lPadWidth, 0.5 * (m_lengthChamber - ysFrame - ylFrame) );
       else 
         m_phiDesign[il].defineDiamond(0.5 * roParam.sPadWidth, 0.5 * roParam.lPadWidth, 0.5 * (m_lengthChamber - ysFrame - ylFrame), yCutout);
       m_phiDesign[il].inputPitch  = stgc->wirePitch();
       m_phiDesign[il].inputWidth  = 0.015;
       m_phiDesign[il].thickness   = m_tckChamber;
       m_phiDesign[il].setFirstPos(roParam.firstWire[il]);      // Position of 1st wire, accounts for staggering
       m_phiDesign[il].firstPitch  = roParam.firstWireGroup[il]; // Number of Wires in 1st group, group staggering
       m_phiDesign[il].groupWidth  = roParam.wireGroupWidth;     // Number of Wires normal group
       m_phiDesign[il].nGroups     = roParam.nWireGroups[il];    // Number of Wire Groups
       m_phiDesign[il].wireCutout  = roParam.wireCutout[il];     // Size of "active" wire region for digits
       m_phiDesign[il].nch         = roParam.nWires[il];

            m_nWires.push_back(m_phiDesign[il].nGroups);  // number of nWireGroups

#ifndef NDEBUG
            if (log.level() <= MSG::DEBUG)
                log << MSG::DEBUG << "initDesign:" << getStationName() << " layer " << il << ", wireGang pitch "
                    << m_phiDesign[il].inputPitch << ", nWireGangs " << m_phiDesign[il].nch << endmsg;
#endif
        }

        //-------------------
        // Pads
        //-------------------
        double radius = absTransform().translation().perp() + m_offset;
        for (int il = 0; il < m_nlayers; il++) {
            m_padDesign[il].Length  = m_lengthChamber;
            m_padDesign[il].sWidth  = m_sWidthChamber;
            m_padDesign[il].lWidth  = m_lWidthChamber;
            m_padDesign[il].Size    = m_lengthChamber - ylFrame - ysFrame;
            m_padDesign[il].xFrame  = xFrame;
            m_padDesign[il].ysFrame = ysFrame;
            m_padDesign[il].ylFrame = ylFrame;
            m_padDesign[il].yCutout = yCutout;
            m_padDesign[il].etasign = Etasign;
            m_padDesign[il].setR(radius);
            m_padDesign[il].sPadWidth = roParam.sPadWidth;
            m_padDesign[il].lPadWidth = roParam.lPadWidth;
            m_padDesign[il].nPadColumns = roParam.nPadPhi[il];

            m_PadhalfX[il]    = 0.5*m_padDesign[il].Size;
            m_PadminHalfY[il] = 0.5*roParam.sPadWidth;
            m_PadmaxHalfY[il] = 0.5*roParam.lPadWidth;

            // The C side of the NSW is mirrored instead of rotated
            // We should be using the same values for the pads for both A and C
            // It is easier for us to simply read the same correct value once
            // whereas changing the XML and the reading functions will make this incompatible with past versions
            // Alexandre Laurier 12 Sept 2018
            m_padDesign[il].firstPhiPos   = roParam.firstPadPhiDivision_A[il];
            m_padDesign[il].inputPhiPitch = roParam.anglePadPhi;       // stEta<2 ?  PAD_PHI_DIVISION/PAD_PHI_SUBDIVISION : PAD_PHI_DIVISION ;
            m_padDesign[il].PadPhiShift   = roParam.PadPhiShift_A[il];
            m_padDesign[il].padEtaMin     = roParam.firstPadRow[il];   // FIRST_PAD_ROW_DIVISION[2*sector+(m_ml-1)][stEta-1][il];
            m_padDesign[il].nPadH         = roParam.nPadH[il];
            m_padDesign[il].padEtaMax     = m_padDesign[il].padEtaMin + roParam.nPadH[il];  // PAD_ROWS[2*sector+(m_ml-1)][stEta-1][il];
            m_padDesign[il].firstRowPos   = roParam.firstPadH[il];     // H_PAD_ROW_0[2*sector+(m_ml-1)][il];
            m_padDesign[il].inputRowPitch = roParam.padH[il];          // PAD_HEIGHT[2*sector+(m_ml-1)][il];

            if (sector_l == 'L') {
                m_padDesign[il].isLargeSector = 1;
                m_padDesign[il].sectorOpeningAngle = m_padDesign[il].largeSectorOpeningAngle;
            } else {
                m_padDesign[il].isLargeSector = 0;
                m_padDesign[il].sectorOpeningAngle = m_padDesign[il].smallSectorOpeningAngle;
            }

            m_padDesign[il].thickness = thickness;

#ifndef NDEBUG
            if (log.level() <= MSG::DEBUG)
                log << MSG::DEBUG << "initDesign stationname " << getStationName() << " layer " << il << ",pad phi angular width "
                    << m_padDesign[il].inputPhiPitch << ", eta pad size " << m_padDesign[il].inputRowPitch
                    << "  Length: " << m_padDesign[il].Length << " sWidth: " << m_padDesign[il].sWidth
                    << " lWidth: " << m_padDesign[il].lWidth << " firstPhiPos:" << m_padDesign[il].firstPhiPos
                    << " padEtaMin:" << m_padDesign[il].padEtaMin << " padEtaMax:" << m_padDesign[il].padEtaMax
                    << " firstRowPos:" << m_padDesign[il].firstRowPos << " inputRowPitch:" << m_padDesign[il].inputRowPitch
                    << " thickness:" << m_padDesign[il].thickness << " sPadWidth: " << m_padDesign[il].sPadWidth
                    << " lPadWidth: " << m_padDesign[il].lPadWidth << " xFrame: " << m_padDesign[il].xFrame
                    << " ysFrame: " << m_padDesign[il].ysFrame << " ylFrame: " << m_padDesign[il].ylFrame
                    << " yCutout: " << m_padDesign[il].yCutout << endmsg;
#endif
        }       
      }
    }

    //============================================================================
    void sTgcReadoutElement::fillCache() {

        if (!m_surfaceData)
            m_surfaceData = std::make_unique<SurfaceData>();
        else {
            MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
            log << MSG::WARNING << "calling fillCache on an already filled cache" << endmsg;
            return;
        }

        for (int layer{0}; layer < m_nlayers; ++layer) {
        
            // Define the geometry for the strips, pads and wires of this readout element. 
            // For QL3 (cutoff trapezoid), diamondBounds are used, while trapezoid bounds are used for the rest. 
            // The assigned coordinate along the layer normal is at the center of the gas gap; 
            // wires are considered at x=0, while strips and pads are shifted by +10/-10 microns.

            //-------------------
            // Layer boundaries
            //-------------------

            if (m_diamondShape) {
                m_surfaceData->m_surfBounds.push_back(std::make_unique<Trk::RotatedDiamondBounds>(
                    m_minHalfY[layer], m_maxHalfY[layer], m_maxHalfY[layer], m_halfX[layer] - m_etaDesign[layer].yCutout() / 2, m_etaDesign[layer].yCutout() / 2));  // strips
                m_surfaceData->m_surfBounds.push_back(std::make_unique<Trk::DiamondBounds>(
                    m_PadminHalfY[layer], m_PadmaxHalfY[layer], m_PadmaxHalfY[layer], m_PadhalfX[layer] - m_padDesign[layer].yCutout / 2, m_padDesign[layer].yCutout / 2));  // pad and wires
                    
            } else {
                m_surfaceData->m_surfBounds.push_back(
                    std::make_unique<Trk::RotatedTrapezoidBounds>(m_halfX[layer], m_minHalfY[layer], m_maxHalfY[layer]));  // strips
                m_surfaceData->m_surfBounds.push_back(
                    std::make_unique<Trk::TrapezoidBounds>(m_PadminHalfY[layer], m_PadmaxHalfY[layer], m_PadhalfX[layer]));
            }

            //-------------------
            // Wires
            //-------------------

            // identifier of the first channel - wire plane - locX along phi, locY max->min R
            Identifier id = manager()->stgcIdHelper()->channelID(getStationName(), getStationEta(), getStationPhi(), m_ml, layer + 1, 2, 1);

            m_surfaceData->m_layerSurfaces.push_back(std::make_unique<Trk::PlaneSurface>(*this, id));

            m_surfaceData->m_layerTransforms.push_back(
               absTransform()                        // transformation from chamber to ATLAS frame
               * m_delta                             // transformations from the alignment group
               * m_Xlg[layer]                        // x-shift of the gas-gap center w.r.t. quadruplet center
               * Amg::Translation3D(0, 0., m_offset) // z-shift to volume center (after m_delta!)
               * Amg::AngleAxis3D(-90 * CLHEP::deg, Amg::Vector3D(0., 1., 0.))   // x<->z because of GeoTrd definition
               * Amg::AngleAxis3D(-90 * CLHEP::deg, Amg::Vector3D(0., 0., 1.))); // x<->y for wires

            m_surfaceData->m_layerCenters.emplace_back(m_surfaceData->m_layerTransforms.back().translation());
            m_surfaceData->m_layerNormals.emplace_back(m_surfaceData->m_layerTransforms.back().linear() * Amg::Vector3D(0., 0., -1.));

            //-------------------
            // Strips
            //-------------------

            const double shift{layer%2 == 0 ? 0.01 : -0.01}; // 1st layer gets +0.01; layer numbering starts from 0 here!

            // identifier of the first channel - strip plane
            id = manager()->stgcIdHelper()->channelID(getStationName(), getStationEta(), getStationPhi(), m_ml, layer + 1, 1, 1);

            m_surfaceData->m_layerSurfaces.push_back(std::make_unique<Trk::PlaneSurface>(*this, id));

            m_surfaceData->m_layerTransforms.push_back(absTransform() * m_delta * m_Xlg[layer] *Amg::Translation3D(shift, 0., m_offset)
                                                      *Amg::AngleAxis3D(-90 * CLHEP::deg, Amg::Vector3D(0., 1., 0.))); // x<->z because of GeoTrd definition

            m_surfaceData->m_layerCenters.emplace_back(m_surfaceData->m_layerTransforms.back().translation());
            m_surfaceData->m_layerNormals.emplace_back(m_surfaceData->m_layerTransforms.back().linear() * Amg::Vector3D(0., 0., -1.));

            //-------------------
            // Trigger Pads
            //-------------------
            
            // identifier of the first channel - pad plane
            id = manager()->stgcIdHelper()->channelID(getStationName(), getStationEta(), getStationPhi(), m_ml, layer + 1, 0, 1);

            m_surfaceData->m_layerSurfaces.push_back(std::make_unique<Trk::PlaneSurface>(*this, id));

            m_surfaceData->m_layerTransforms.push_back(absTransform() * m_delta * m_Xlg[layer] * Amg::Translation3D(-shift, 0., m_offset)
                                                       * Amg::AngleAxis3D(-90 * CLHEP::deg, Amg::Vector3D(0., 1., 0.))   // x<->z because of GeoTrd definition
                                                       * Amg::AngleAxis3D(-90 * CLHEP::deg, Amg::Vector3D(0., 0., 1.))); // x<->y for pads

            m_surfaceData->m_layerCenters.emplace_back(m_surfaceData->m_layerTransforms.back().translation());
            m_surfaceData->m_layerNormals.emplace_back(m_surfaceData->m_layerTransforms.back().linear() * Amg::Vector3D(0., 0., -1.));
        }
    }


    //============================================================================
    bool sTgcReadoutElement::containsId(const Identifier& id) const {
        if (manager()->stgcIdHelper()->stationEta(id) != getStationEta()) return false;
        if (manager()->stgcIdHelper()->stationPhi(id) != getStationPhi()) return false;

        if (manager()->stgcIdHelper()->multilayerID(id) != m_ml) return false;

        int gasgap = manager()->stgcIdHelper()->gasGap(id);
        if (gasgap < 1 || gasgap > m_nlayers) return false;

        int strip = manager()->stgcIdHelper()->channel(id);
        if (manager()->stgcIdHelper()->channelType(id) == 1 && (strip < 1 || strip > m_nStrips[gasgap - 1])) return false;
        if (manager()->stgcIdHelper()->channelType(id) == 2 && (strip < 1 || strip > m_nWires[gasgap - 1])) return false;
        if (manager()->stgcIdHelper()->channelType(id) == 0 && (strip < 1 || strip > m_nPads[gasgap - 1])) return false;

        return true;
    }


    //============================================================================
    double sTgcReadoutElement::channelPitch(const Identifier& id) const {
        if (manager()->stgcIdHelper()->channelType(id) == 0) {
            const MuonPadDesign* design = getPadDesign(id);
            if (!design) {
                MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
                log << MSG::WARNING << "no pad Design" << endmsg;
                return -1;
            }
            return design->channelWidth(Amg::Vector2D(0, 0), 0);
        }

        const MuonChannelDesign* design = getDesign(id);
        if (!design) return -1;

        if (manager()->stgcIdHelper()->channelType(id) == 1)  // sTGC strips
            return design->inputPitch;
        else if (manager()->stgcIdHelper()->channelType(id) == 2)  // sTGC wires
            return design->inputPitch * design->groupWidth;        // wire Pitch * number of wires in a group
        else
            return -1;
    }


    //============================================================================
    int sTgcReadoutElement::padNumber(const Amg::Vector2D& pos, const Identifier& id) const {
        const MuonPadDesign* design = getPadDesign(id);
        if (!design) {
            MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
            log << MSG::WARNING << "no pad Design" << endmsg;
            return -1;
        }
        std::pair<int, int> pad(design->channelNumber(pos));
        const sTgcIdHelper& id_helper{*manager()->stgcIdHelper()};
        if (pad.first > 0 && pad.second > 0) {
#ifndef NDEBUG
            bool is_valid {true};
#endif
            const Identifier padID = id_helper.padID(id, id_helper.multilayer(id),
                                                        id_helper.gasGap(id), sTgcIdHelper::Pad, pad.first, pad.second
#ifndef NDEBUG
                                                        , is_valid
#endif
                                                        
                                                        );
            int channel = id_helper.channel(padID);
            int padEta = id_helper.padEta(padID);
            int padPhi = id_helper.padPhi(padID);
            if (
#ifndef NDEBUG                
                !is_valid ||
#endif                
                 padEta != pad.first || padPhi != pad.second) {
                MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
                log << MSG::WARNING << " bad pad indices: input " << pad.first << " " << pad.second << " from ID " << padEta << " "
                    << padPhi << endmsg;
                return -1;
            }
            return channel;
        }
        MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
        log << MSG::WARNING <<__LINE__<< " bad channelNumber" <<pad.first<<" "<<pad.second << endmsg;

        return -1;
    }


    //============================================================================
    int sTgcReadoutElement::wireNumber(const Amg::Vector2D& pos, const Identifier& id) const {
        const MuonChannelDesign* design = getDesign(id);
        if (!design) {
            MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
            log << MSG::WARNING << "no wire design when trying to get the wire number" << endmsg;
            return -1;
        }
        return design->wireNumber(pos);
    }


    //============================================================================
    double sTgcReadoutElement::wirePitch(int gas_gap) const {
        if (m_phiDesign.empty()) {
            MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
            log << MSG::WARNING << "no wire design when trying to get the wire pitch" << endmsg;
            return -1.0;
        }
        return (m_phiDesign[gas_gap - 1]).inputPitch;
    }


    //============================================================================
    double sTgcReadoutElement::positionFirstWire(const Identifier& id) const {
        double pos_wire = -9999.9;
        if (manager()->stgcIdHelper()->channelType(id) == sTgcIdHelper::sTgcChannelTypes::Wire) {
            const MuonChannelDesign* design = getDesign(id);
            if (!design) {
                MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
                log << MSG::WARNING << "no wire design when trying to get the 1st wire position" << endmsg;
                return pos_wire;
            }
            pos_wire = design->firstPos();
        } else {
            MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
            log << MSG::WARNING << "attempt to retrieve the 1st wire position with a wrong identifier" << endmsg;
        }
        return pos_wire;
    }


    //============================================================================
    int sTgcReadoutElement::numberOfWires(const Identifier& id) const {
        int nWires = -1;
        if (manager()->stgcIdHelper()->channelType(id) == sTgcIdHelper::sTgcChannelTypes::Wire) {
            const MuonChannelDesign* design = getDesign(id);
            if (!design) {
                MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
                log << MSG::WARNING << "no wire design when trying to get the total number of wires" << endmsg;
                return nWires;
            }
            nWires = design->nch;
        } else {
            MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
            log << MSG::WARNING << "attempt to retrieve the number of wires with a wrong identifier" << endmsg;
        }
        return nWires;
    }


    //============================================================================
    Amg::Vector3D sTgcReadoutElement::localToGlobalCoords(const Amg::Vector3D& locPos, Identifier id) const {
        int gg = manager()->stgcIdHelper()->gasGap(id);
        int channelType = manager()->stgcIdHelper()->channelType(id);

        // The assigned coordinate along the layer normal is at the center of the gas gap; 
        // wires are considered at x=0, while:
        // for layers 1, 3 strips (pads) are shifted by +10 (-10) microns
        // for layers 2, 4 strips (pads) are shifted by -10 (+10) microns 
        Amg::Vector3D locPos_ML(0, 0, 0);
        double shift{0.};
        if (channelType != 2) shift = ((gg % 2) ^ (channelType==0)) ? 0.01 : -0.01;
        locPos_ML = m_Xlg[gg - 1] * Amg::Translation3D(shift, 0., m_offset) * locPos;

#ifndef NDEBUG
        MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
        if (log.level() <= MSG::DEBUG) {
            log << MSG::DEBUG << "position coordinates in the gas-gap r.f.:    "  << locPos << endmsg;
            log << MSG::DEBUG << "position coordinates in the multilayer r.f.: " << locPos_ML << endmsg;
        }
#endif
        Amg::Vector3D gVec = absTransform() * m_delta * locPos_ML;
        return gVec;
    }


    //============================================================================
    void sTgcReadoutElement::setDelta(const ALinePar& aline) {
        // amdb frame (s, z, t) = chamber frame (y, z, x)        
        if (aline) {
            m_delta = aline.delta();                    
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
    void sTgcReadoutElement::clearALinePar() {
        if (has_ALines()) {
            m_ALinePar = nullptr; 
            m_delta = Amg::Transform3D::Identity(); 
            refreshCache();
        }
    }

    //============================================================================
    void sTgcReadoutElement::setBLinePar(const BLinePar& bLine) {
        ATH_MSG_DEBUG("Setting B-line for " <<idHelperSvc()->toStringDetEl(identify())<<" "<<bLine);
        m_BLinePar = &bLine;
    }

    //============================================================================
    void sTgcReadoutElement::posOnDefChamber(Amg::Vector3D& locPosML) const {

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
        using Parameter = BLinePar::Parameter;
        const double bp = m_BLinePar->getParameter(Parameter::bp);
        const double bn = m_BLinePar->getParameter(Parameter::bn);
        const double sp = m_BLinePar->getParameter(Parameter::sp);
        const double sn = m_BLinePar->getParameter(Parameter::sn);
        const double tw = m_BLinePar->getParameter(Parameter::tw);
        const double eg = m_BLinePar->getParameter(Parameter::eg)*1.e-3;
        const double ep = m_BLinePar->getParameter(Parameter::ep)*1.e-3;
        const double en = m_BLinePar->getParameter(Parameter::en)*1.e-3;

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


    //============================================================================
    void sTgcReadoutElement::spacePointPosition(const Identifier& layerId, double locXpos, double locYpos, Amg::Vector3D& pos) const {

        pos = Amg::Vector3D(locXpos, locYpos, 0.);

        const MuonChannelDesign* design = getDesign(layerId);
        if (!design) {
            MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
            log << MSG::WARNING << "Unable to get MuonChannelDesign, therefore cannot provide position corrections. Returning." << endmsg;
            return;
        }

        bool conditionsApplied{false};
        Amg::Transform3D trfToML{Amg::Transform3D::Identity()};

#ifndef SIMULATIONBASE
        //*********************
        // As-Built (MuonNswAsBuilt is not included in AthSimulation)
        //*********************
        const NswAsBuilt::StgcStripCalculator* sc = manager()->getStgcAsBuiltCalculator();
        if (sc && design->type == MuonChannelDesign::ChannelType::etaStrip) {

            Amg::Vector2D lpos(locXpos, locYpos);
            
            // express the local position w.r.t. the nearest active strip
            Amg::Vector2D rel_pos;
            int istrip = design->positionRelativeToStrip(lpos, rel_pos);
            if (istrip < 0) {
                MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
                log << MSG::WARNING << "As-built corrections are provided only for eta strips within the active area. Returning." << endmsg;
                return;
            }

            // setup strip calculator
            NswAsBuilt::stripIdentifier_t strip_id;
            strip_id.quadruplet = { (largeSector() ? NswAsBuilt::quadrupletIdentifier_t::STL : NswAsBuilt::quadrupletIdentifier_t::STS), getStationEta(), getStationPhi(), m_ml };
            strip_id.ilayer     = manager()->stgcIdHelper()->gasGap(layerId);
            strip_id.istrip     = istrip;

            // get the position coordinates, in the chamber frame, from NswAsBuilt.
            // applying the 10um shift along the beam axis for strips (see fillCache()).
            NswAsBuilt::StgcStripCalculator::position_t calcPos = sc->getPositionAlongStgcStrip(NswAsBuilt::Element::ParameterClass::CORRECTION, strip_id, rel_pos.y(), rel_pos.x());
            
            if (calcPos.isvalid == NswAsBuilt::StgcStripCalculator::IsValid::VALID) {
                pos = calcPos.pos;
                pos[0] += (strip_id.ilayer%2) ? 0.01 : -0.01; // 1st layer gets +0.01; layer numbering starts from 1

                // signal that pos is now in the chamber reference frame
                // (don't go back to the layer frame yet, since we may apply b-lines later on)
                trfToML = m_delta.inverse()*absTransform().inverse()*transform(layerId);   
                conditionsApplied = true;
            }
#ifndef NDEBUG
            else {
                MsgStream log(Athena::getMessageSvc(), "sTgcReadoutElement");
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
    }

}  // namespace MuonGM
