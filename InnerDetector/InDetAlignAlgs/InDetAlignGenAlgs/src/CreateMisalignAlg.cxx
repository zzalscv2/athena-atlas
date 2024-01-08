/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////
// ================================================
// CreateMisalignAlg
// ================================================
//
// CreateMisalignAlg.cxx
// Source file for CreateMisalignAlg
//
// Namespace LocalChi2Align
// Header include

// Gaudi & StoreGate
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/SmartDataPtr.h"  //NTupleFilePtr
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/IRndmGenSvc.h"

// Geometry Stuff
#include "Identifier/Identifier.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetIdentifier/TRT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "TRT_ReadoutGeometry/TRT_DetElementCollection.h"
#include "TRT_ReadoutGeometry/TRT_BaseElement.h"
#include "DetDescrConditions/AlignableTransform.h"
#include "StoreGate/ReadCondHandleKey.h"

// Alignment DB Stuff
#include "InDetAlignGenTools/IInDetAlignDBTool.h"
#include "TRT_ConditionsServices/ITRT_AlignDbSvc.h"
#include "AthenaKernel/IOVRange.h"
#include "CreateMisalignAlg.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include <cmath>
#include <tuple> //for tuple decomposition and std::ignore
#include <sstream>




#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"


namespace{
  std::string commonAlignmentOutput(const HepGeom::Transform3D & initialAlignment){
  std::ostringstream os;
  os << "\nAlignment x = ("  << initialAlignment.getTranslation().x() / CLHEP::micrometer << ") micron\n";
  os << "Alignment y = ("  << initialAlignment.getTranslation().y() / CLHEP::micrometer << ") micron\n";
  os << "Alignment z = ("  << initialAlignment.getTranslation().z() / CLHEP::micrometer << ") micron\n";
  os << "Alignment x phi   = ("  << initialAlignment.getRotation().phiX() / CLHEP::deg << ") degree\n";
  os << "Alignment x Theta = ("  << initialAlignment.getRotation().thetaX() / CLHEP::deg << ") degree\n";
  os << "Alignment y phi   = ("  << initialAlignment.getRotation().phiY() / CLHEP::deg << ") degree\n";
  os << "Alignment y Theta = ("  << initialAlignment.getRotation().thetaY() / CLHEP::deg << ") degree\n";
  os << "Alignment z phi   = ("  << initialAlignment.getRotation().phiZ() / CLHEP::deg << ") degree\n";
  os << "Alignment z Theta = ("  << initialAlignment.getRotation().thetaZ() / CLHEP::deg << ") degree\n";
  return os.str();
  }
}

namespace InDetAlignment
{
	
	// Constructor
	CreateMisalignAlg::CreateMisalignAlg(const std::string& name, ISvcLocator* pSvcLocator):
	AthAlgorithm(name,pSvcLocator),
    m_idHelper(nullptr),
    m_pixelIdHelper(nullptr),
    m_sctIdHelper(nullptr),
    m_trtIdHelper(nullptr),
	m_IDAlignDBTool("InDetAlignDBTool",this),
	m_trtaligndbservice("TRT_AlignDbSvc",name),
	m_asciiFileNameBase("MisalignmentSet"),
	m_SQLiteTag("test_tag"),
	m_firstEvent(true),
	m_createFreshDB(true),
	m_MisalignmentMode(0),
	m_nEvents(0),
	m_Misalign_maxShift(1*CLHEP::mm),
    m_Misalign_maxShift_Inner(50*CLHEP::micrometer),
    m_ScalePixelIBL(1.),
    m_ScalePixelDBM(1.),
	m_IBLBowingTshift(0.),
	m_ScalePixelBarrel(1.),
	m_ScalePixelEndcap(1.),
	m_ScaleSCTBarrel(1.),
	m_ScaleSCTEndcap(1.),
	m_ScaleTRTBarrel(1.),
	m_ScaleTRTEndcap(1.),
    m_VisualizationLookupTree(nullptr),
    m_AthenaHashedID(-1),
    m_HumanReadableID(-1),
    m_doPix(true),
    m_doStrip(true),
    m_doTRT(true)
	{
		declareProperty("ASCIIFilenameBase"             ,     m_asciiFileNameBase);
		declareProperty("SQLiteTag"                     ,     m_SQLiteTag);
		declareProperty("MisalignMode"                  ,     m_MisalignmentMode);
		declareProperty("MaxShift"                      ,     m_Misalign_maxShift);
        declareProperty("MaxShiftInner"                 ,     m_Misalign_maxShift_Inner);
		declareProperty("CreateFreshDB"                 ,     m_createFreshDB);
		declareProperty("IDAlignDBTool"                 ,     m_IDAlignDBTool);
		declareProperty("TRTAlignDBService"             ,     m_trtaligndbservice);
        declareProperty("ScalePixelIBL"                 ,     m_ScalePixelIBL);
        declareProperty("ScalePixelDBM"                 ,     m_ScalePixelDBM);
		declareProperty("IBLBowingTshift"               ,     m_IBLBowingTshift);
		declareProperty("ScalePixelBarrel"              ,     m_ScalePixelBarrel);
		declareProperty("ScalePixelEndcap"              ,     m_ScalePixelEndcap);
		declareProperty("ScaleSCTBarrel"                ,     m_ScaleSCTBarrel);
		declareProperty("ScaleSCTEndcap"                ,     m_ScaleSCTEndcap);
		declareProperty("ScaleTRTBarrel"                ,     m_ScaleTRTBarrel);
		declareProperty("ScaleTRTEndcap"                ,     m_ScaleTRTEndcap);
	}
	
	//__________________________________________________________________________
	// Destructor
	CreateMisalignAlg::~CreateMisalignAlg(void)
	{
		ATH_MSG_DEBUG( "CreateMisalignAlg destructor called" );
	}
	
	//__________________________________________________________________________
	StatusCode CreateMisalignAlg::initialize()
	{
		ATH_MSG_DEBUG("CreateMisalignAlg initialize()");
		if(m_pixelDetEleCollKey.empty()) {m_doPix=false;ATH_MSG_DEBUG("Not creating misalignment for Pixel");}
		if(m_SCTDetEleCollKey.empty()) {m_doStrip=false;ATH_MSG_DEBUG("Not creating misalignment for Strip/SCT");}
		if(m_trtDetEleCollKey.empty()) {m_doTRT=false;ATH_MSG_DEBUG("Not creating misalignment for TRT");}

        ATH_CHECK(m_pixelDetEleCollKey.initialize(SG::AllowEmpty));
        ATH_CHECK(m_SCTDetEleCollKey.initialize(SG::AllowEmpty));
        ATH_CHECK(m_trtDetEleCollKey.initialize(SG::AllowEmpty));

        if (m_doPix || m_doStrip) ATH_CHECK(m_IDAlignDBTool.retrieve());
		if(m_doTRT) ATH_CHECK(m_trtaligndbservice.retrieve());
		//ID helpers
		// Pixel
		if(m_doPix){
            ATH_CHECK(detStore()->retrieve(m_pixelIdHelper, "PixelID"));
        }
		// SCT
        if(m_doStrip){
		    ATH_CHECK(detStore()->retrieve(m_sctIdHelper, "SCT_ID"));
        }
		// TRT
        if(m_doTRT){
		    ATH_CHECK(detStore()->retrieve(m_trtIdHelper, "TRT_ID"));
        }
		ATH_CHECK(detStore()->retrieve(m_idHelper, "AtlasID"));

		// Retrieve the Histo Service
		ITHistSvc* hist_svc;
		ATH_CHECK(service("THistSvc",hist_svc));
		//Registering TTree for Visualization Lookup
		m_VisualizationLookupTree = new TTree("IdentifierTree", "Visualization Identifier Lookup Tree");
		ATH_CHECK(hist_svc->regTree("/IDENTIFIERTREE/IdentifierTree", m_VisualizationLookupTree));
		m_VisualizationLookupTree->Branch ("AthenaHashedID", &m_AthenaHashedID, "AthenaID/i");
		m_VisualizationLookupTree->Branch ("HumanReadableID", &m_HumanReadableID, "HumanID/I");
		
		// initialize generated Initial Alignment NTuple
		NTupleFilePtr file1(ntupleSvc(), "/NTUPLES/CREATEMISALIGN");
		
		NTuplePtr nt(ntupleSvc(), "/NTUPLES/CREATEMISALIGN/InitialAlignment");
		if ( !nt ) {       // Check if already booked
			nt = ntupleSvc()->book("/NTUPLES/CREATEMISALIGN/InitialAlignment", CLID_ColumnWiseTuple, "InitialAlignment");
			if ( nt ) {
				ATH_MSG_INFO( "InitialAlignment ntuple booked." );
				ATH_CHECK(  nt->addItem("x"         ,m_AlignResults_x) );
				ATH_CHECK(  nt->addItem("y"         ,m_AlignResults_y) );
				ATH_CHECK(  nt->addItem("z"         ,m_AlignResults_z) );
				ATH_CHECK(  nt->addItem("alpha"     ,m_AlignResults_alpha) );
				ATH_CHECK(  nt->addItem("beta"      ,m_AlignResults_beta) );
				ATH_CHECK(  nt->addItem("gamma"     ,m_AlignResults_gamma) );
				ATH_CHECK(  nt->addItem("ID"        ,m_AlignResults_Identifier_ID) );
				ATH_CHECK(  nt->addItem("PixelSCT"  ,m_AlignResults_Identifier_PixelSCT) );
				ATH_CHECK(  nt->addItem("BarrelEC"  ,m_AlignResults_Identifier_BarrelEC) );
				ATH_CHECK(  nt->addItem("LayerDisc" ,m_AlignResults_Identifier_LayerDisc) );
				ATH_CHECK(  nt->addItem("Phi"       ,m_AlignResults_Identifier_Phi) );
				ATH_CHECK(  nt->addItem("Eta"       ,m_AlignResults_Identifier_Eta) );
				ATH_CHECK(  nt->addItem("center_x"         ,m_Initial_center_x ) );
				ATH_CHECK(  nt->addItem("center_y"         ,m_Initial_center_y ) );
                                ATH_CHECK(  nt->addItem("center_z"         ,m_Initial_center_z ) );
                                ATH_CHECK(  nt->addItem("misaligned_global_x"         ,m_Global_center_x ) );
                                ATH_CHECK(  nt->addItem("misaligned_global_y"         ,m_Global_center_y ) );
                                ATH_CHECK(  nt->addItem("misaligned_global_z"         ,m_Global_center_z ) );
			} else {  // did not manage to book the N tuple....
				msg(MSG::ERROR) << "Failed to book InitialAlignment ntuple." << endmsg;
			}
		}
		
		if (m_MisalignmentMode) {
			ATH_MSG_INFO( "Misalignment mode chosen: " << m_MisalignmentMode );
			if (m_MisalignmentMode == 1) {
				ATH_MSG_INFO( "MisalignmentX     : " << m_Misalign_x / CLHEP::micrometer << " micrometer" );
				ATH_MSG_INFO( "MisalignmentY     : " << m_Misalign_y / CLHEP::micrometer << " micrometer" );
				ATH_MSG_INFO( "MisalignmentZ     : " << m_Misalign_z / CLHEP::micrometer << " micrometer" );
				ATH_MSG_INFO( "MisalignmentAlpha : " << m_Misalign_alpha / CLHEP::mrad << " mrad" );
				ATH_MSG_INFO( "MisalignmentBeta  : " << m_Misalign_beta / CLHEP::mrad << " mrad" );
				ATH_MSG_INFO( "MisalignmentGamma : " << m_Misalign_gamma / CLHEP::mrad << " mrad" );
			} else {
				ATH_MSG_INFO( "with maximum shift of " << m_Misalign_maxShift / CLHEP::micrometer << " micrometer" );
			}
		} else {
			ATH_MSG_INFO( "Dry run, no misalignment will be generated." );
		}
		
		return StatusCode::SUCCESS;
	}
	
	//__________________________________________________________________________
	StatusCode CreateMisalignAlg::execute()
	{
    ATH_MSG_DEBUG( "AlignAlg execute()" );
		++m_nEvents;
		
		if (m_firstEvent) {
			int nSCT   = 0;
			int nPixel = 0;
			int nTRT   = 0;
			
			if (m_createFreshDB) {
				m_IDAlignDBTool->createDB();
				//m_trtaligndbservice->createAlignObjects(); //create DB for TRT? should be ok... //TODO
			}
			
			if(m_doPix) setupPixel_AlignModule(nPixel);
			if(m_doStrip) setupSCT_AlignModule(nSCT);
			if(m_doTRT) setupTRT_AlignModule(nTRT);
			
			ATH_MSG_INFO( "Back from AlignModuleObject Setup. " );
			ATH_MSG_INFO(  nPixel << " Pixel modules found." );
			ATH_MSG_INFO(  nSCT   << "  SCT  modules found," );
			ATH_MSG_INFO(  nTRT   << "  TRT  modules found." );
			
			ATH_MSG_INFO( m_ModuleList.size() << " entries in identifier list" );
			
			if (StatusCode::SUCCESS!=GenerateMisaligment()) {
        ATH_MSG_ERROR( "GenerateMisalignment failed!" );
			  return StatusCode::FAILURE;
			};
			
			m_firstEvent = false;
		}
		
		return StatusCode::SUCCESS;
	}
	
	//__________________________________________________________________________
	StatusCode CreateMisalignAlg::finalize()
	{
		ATH_MSG_DEBUG("CreateMisalignAlg finalize()" );
		
		m_ModuleList.clear();
		
		return StatusCode::SUCCESS;
	}
	
	//__________________________________________________________________________
	void CreateMisalignAlg::setupSCT_AlignModule(int& nSCT)
	{
                // SiDetectorElementCollection for SCT
                SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> sctDetEleHandle(m_SCTDetEleCollKey);
                const InDetDD::SiDetectorElementCollection* elements(*sctDetEleHandle);
                if (not sctDetEleHandle.isValid() or elements==nullptr) {
                        ATH_MSG_FATAL(m_SCTDetEleCollKey.fullKey() << " is not available.");
                        return;
                }
		for (const InDetDD::SiDetectorElement *element: *elements) {
			const Identifier SCT_ModuleID = m_sctIdHelper->module_id(element->identify()); //from wafer id to module id
                        const IdentifierHash SCT_ModuleHash = m_sctIdHelper->wafer_hash(SCT_ModuleID);
			
			if (m_ModuleList.find(SCT_ModuleID) == m_ModuleList.end())
			{
				const InDetDD::SiDetectorElement *module = elements->getDetectorElement(SCT_ModuleHash);
				m_ModuleList[SCT_ModuleID][0] = module->center()[0];
				m_ModuleList[SCT_ModuleID][1] = module->center()[1];
				m_ModuleList[SCT_ModuleID][2] = module->center()[2];
				++nSCT;
				ATH_MSG_VERBOSE( "SCT module " << nSCT );
			}
			
			if (m_sctIdHelper->side(element->identify()) == 0) { // inner side case
				// Write out Visualization Lookup Tree
				m_AthenaHashedID = SCT_ModuleID.get_identifier32().get_compact();
				m_HumanReadableID = 1000000*2 /*2 = SCT*/
				+ 100000*m_sctIdHelper->layer_disk(SCT_ModuleID)
				+ 1000*(10+m_sctIdHelper->eta_module(SCT_ModuleID))
				+ m_sctIdHelper->phi_module(SCT_ModuleID);
				if ( m_sctIdHelper->barrel_ec(SCT_ModuleID) != 0 ) {
					m_HumanReadableID = m_sctIdHelper->barrel_ec(SCT_ModuleID)*(m_HumanReadableID + 10000000);
				}
				
				ATH_MSG_VERBOSE( "Human Readable ID: " << m_HumanReadableID );
				
				m_VisualizationLookupTree->Fill();
				
				// Syntax is (ID, Level) where Level is from 1 to 3 (3 is single module level)
				if (msgLvl(MSG::VERBOSE)) {
          HepGeom::Transform3D InitialAlignment = Amg::EigenTransformToCLHEP(m_IDAlignDBTool->getTrans(SCT_ModuleID,3));
					msg() << "Initial Alignment of module " << m_idHelper->show_to_string(SCT_ModuleID,nullptr,'/') << endmsg;
					msg() << commonAlignmentOutput(InitialAlignment);
					msg() << endmsg;
				}
			} // end inner side case
		} //end loop over SCT elements
	}
	
	//__________________________________________________________________________
	void CreateMisalignAlg::setupPixel_AlignModule(int& nPixel)
	{
                // SiDetectorElementCollection for Pixel
                SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(m_pixelDetEleCollKey);
                const InDetDD::SiDetectorElementCollection* elements(*pixelDetEleHandle);
                if (not pixelDetEleHandle.isValid() or elements==nullptr) {
                  ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
                  return;
                }
                for (const InDetDD::SiDetectorElement *element: *elements) {			
			// get the ID
                        const Identifier Pixel_ModuleID = element->identify();
                        const IdentifierHash Pixel_ModuleHash = m_pixelIdHelper->wafer_hash(Pixel_ModuleID);
			// check the validity
			if (Pixel_ModuleID.is_valid()) {
				if (m_ModuleList.find(Pixel_ModuleID) == m_ModuleList.end()) {
                                        const InDetDD::SiDetectorElement *module = elements->getDetectorElement(Pixel_ModuleHash);
					m_ModuleList[Pixel_ModuleID][0] = module->center()[0];
					m_ModuleList[Pixel_ModuleID][1] = module->center()[1];
					m_ModuleList[Pixel_ModuleID][2] = module->center()[2];
					
					++nPixel;
					ATH_MSG_VERBOSE( "Pixel module " << nPixel );
					
					// Write out Visualization Lookup Tree
					m_AthenaHashedID = Pixel_ModuleID.get_identifier32().get_compact();
					m_HumanReadableID = 1000000*1 /*1 = Pixel*/
					+ 100000*m_pixelIdHelper->layer_disk(Pixel_ModuleID)
					+ 1000*(10+m_pixelIdHelper->eta_module(Pixel_ModuleID))
					+ m_pixelIdHelper->phi_module(Pixel_ModuleID);
					if ( m_pixelIdHelper->barrel_ec(Pixel_ModuleID) != 0 ) {
						m_HumanReadableID = m_pixelIdHelper->barrel_ec(Pixel_ModuleID)*(m_HumanReadableID + 10000000);
					}
					
					ATH_MSG_VERBOSE( "Human Readable ID: " << m_HumanReadableID );
					
					m_VisualizationLookupTree->Fill();
					
					if (msgLvl(MSG::VERBOSE)) {
            HepGeom::Transform3D InitialAlignment = Amg::EigenTransformToCLHEP(m_IDAlignDBTool->getTrans(Pixel_ModuleID,3));
						msg() << "Initial Alignment of module " << m_idHelper->show_to_string(Pixel_ModuleID,nullptr,'/') << endmsg;
						msg() << commonAlignmentOutput(InitialAlignment);
						msg() << endmsg;
					}
				}
			} else {
				ATH_MSG_INFO( "not a valid PIXEL Module ID (setup)" );
			}
		}
	}
	
	//__________________________________________________________________________
	void CreateMisalignAlg::setupTRT_AlignModule(int& nTRT)
	{
		//TODO: writing into the Identifier tree is undone for TRT (AthenaHashedID and HumanReadableID)
		
		std::map< Identifier, std::vector<double> > trtModulesWithCOG;

                // TRT_DetElementContainer->TRT_DetElementCollection for TRT                                                                                                                            
		SG::ReadCondHandle<InDetDD::TRT_DetElementContainer> trtDetEleHandle(m_trtDetEleCollKey);
                const InDetDD::TRT_DetElementCollection* elements(trtDetEleHandle->getElements());
                if (not trtDetEleHandle.isValid() or elements==nullptr) {
		  ATH_MSG_FATAL(m_trtDetEleCollKey.fullKey() << " is not available.");
		  return;
                }
		
		//step through all detector elements (=strawlayers) and accumulate strawcenters per
		// element (with DB granularity, i.e. phi sectors in endcap, bi-modules in barrel)
		for (const InDetDD::TRT_BaseElement *element: *elements) {
			const Identifier TRTID_orig = element->identify();
			const Identifier TRTID      = reduceTRTID(TRTID_orig);
			bool insertSuccess{};
			std::tie(std::ignore, insertSuccess) = trtModulesWithCOG.insert({TRTID,std::vector<double>(4,0.)}); //create fresh vector for module center
			if (not insertSuccess){
			  ATH_MSG_VERBOSE("No insert was performed, identifier was already in the trtModulesWithCOG map");
			}
			
			unsigned int nStraws = element->nStraws();
			for (unsigned int l = 0; l<nStraws; l++) {
				const Amg::Vector3D strawcenter = element->strawCenter(l);
				trtModulesWithCOG[TRTID].at(0) += strawcenter.x(); /*sumx*/
				trtModulesWithCOG[TRTID].at(1) += strawcenter.y(); /*sumy*/
				trtModulesWithCOG[TRTID].at(2) += strawcenter.z(); /*sumz*/
				trtModulesWithCOG[TRTID].at(3) += 1.;              /*nStraws*/
				
			}
			
				ATH_MSG_DEBUG( "this strawlayer has " << nStraws << " straws." );
				ATH_MSG_DEBUG( "strawcount of this module: " << trtModulesWithCOG[TRTID].at(3) );
			
		}
		
		//go through cog list and create one COG per TRT module (at DB granularity)
		std::map< Identifier, std::vector<double> >::const_iterator iter2;
		for (iter2 = trtModulesWithCOG.begin(); iter2!=trtModulesWithCOG.end(); ++iter2) {
			const Identifier TRTID = iter2->first;
			double nStraws = iter2->second.at(3);
			nTRT++;
			ATH_MSG_VERBOSE( "TRT module " << nTRT );
			m_ModuleList[TRTID] = HepGeom::Point3D<double>(iter2->second.at(0)/nStraws, iter2->second.at(1)/nStraws,iter2->second.at(2)/nStraws);
			
			HepGeom::Transform3D InitialAlignment ;
			
			const Amg::Transform3D* p = m_trtaligndbservice->getAlignmentTransformPtr(TRTID,2) ;
			if ( p ) {
				if (msgLvl(MSG::VERBOSE)) {
  				InitialAlignment = Amg::EigenTransformToCLHEP(*p) ;
					msg() << "Initial Alignment of module " << m_idHelper->show_to_string(TRTID,nullptr,'/') << endmsg;
					msg() << commonAlignmentOutput(InitialAlignment);
					msg() << endmsg;
				}
			} else {
				
					ATH_MSG_VERBOSE("No initial alignment for TRT module " << m_idHelper->show_to_string(TRTID,nullptr,'/') );
			}
			
			
		}
	}
	
	//__________________________________________________________________________
	StatusCode  CreateMisalignAlg::GenerateMisaligment()
	{

		IRndmGenSvc* randsvc;
		if (StatusCode::SUCCESS!=service("RndmGenSvc",randsvc,true)) {
			ATH_MSG_WARNING( "Cannot find RndmGenSvc" );
		}
		else {
      ATH_MSG_DEBUG( "Got RndmGenSvc" );
		}
		
		int i = 0;
		
		/*
		 ===================================
		 Documentation of misalignment modes
		 ===================================
		 
		 MisalignMode =
		 0: nothing, no misalignments are generated
		 1: Misalignment of whole InDet by 6 parameters
		 2: random misalignment
                 3: IBL-stave temperature dependent bowing		 

		 ====================================================
		 Global Distortions according to David Brown (LHC Detector Alignment Workshop 2006-09-04, slides page 11)
		 ====================================================
		 11: R delta R:     Radial expansion linearly with r
		 12: Phi delta R:   radial expansion sinuisoidally with phi
		 13: Z delta R:     radial expansion linearly with z
		 21: R delta Phi:   rotation linearly with r
		 22: Phi delta Phi: rotation sinusoidally with phi
		 23: Z delta Phi:   rotation linearly with z
		 31: R delta Z:     z-shift linearly with r
		 32: Phi delta Z:   z-shift sinusoidally with phi
		 33: Z delta Z:     z-shift linearly with z
		 */
		
		const double maxRadius=51.4*CLHEP::cm;   // maximum radius of Silicon Detector (=outermost SCT radius)
		const double minRadius=50.5*CLHEP::mm;   // minimum radius of Silicon Detector (=innermost PIX radius)
		const double maxLength=158.*CLHEP::cm;   // maximum length of Silicon Detector barrel (=length of SCT barrel)
		
		const double maxDeltaR = m_Misalign_maxShift;
		const double maxAngle = 2 * asin( m_Misalign_maxShift / (2*maxRadius));
		const double maxAngleInner = 2 * asin ( m_Misalign_maxShift_Inner / (2*minRadius));
		const double maxDeltaZ = m_Misalign_maxShift;
		ATH_MSG_DEBUG( "maximum deltaPhi              = " << maxAngle/CLHEP::mrad << " mrad" );
		ATH_MSG_DEBUG( "maximum deltaPhi for 1/r term = " << maxAngleInner/CLHEP::mrad << " mrad" );
                const InDetDD::SiDetectorElementCollection* pixelElements=nullptr;
                const InDetDD::SiDetectorElementCollection* sctElements=nullptr;
                if(m_doPix){
                    // SiDetectorElementCollection for Pixel
                    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> pixelDetEleHandle(m_pixelDetEleCollKey);
                    pixelElements = *pixelDetEleHandle;
                    if (not pixelDetEleHandle.isValid() or pixelElements==nullptr) {
                        ATH_MSG_FATAL(m_pixelDetEleCollKey.fullKey() << " is not available.");
                        return StatusCode::FAILURE;
                    }
                }
                if(m_doStrip){
                    // SiDetectorElementCollection for SCT
                    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> sctDetEleHandle(m_SCTDetEleCollKey);
                    sctElements = *sctDetEleHandle;
                    if (not sctDetEleHandle.isValid() or sctElements==nullptr) {
                        ATH_MSG_FATAL(m_SCTDetEleCollKey.fullKey() << " is not available.");
                        return StatusCode::FAILURE;
                    }
                }
		
		for (std::map<Identifier, HepGeom::Point3D<double> >::const_iterator iter = m_ModuleList.begin(); iter != m_ModuleList.end(); ++iter) {
			++i;
			const Identifier& ModuleID = iter->first;
			
			const InDetDD::SiDetectorElement * SiModule = nullptr; //dummy to get moduleTransform() for silicon
			
			if (m_idHelper->is_pixel(ModuleID)) {
                                const IdentifierHash Pixel_ModuleHash = m_pixelIdHelper->wafer_hash(ModuleID);
				if(pixelElements) SiModule = pixelElements->getDetectorElement(Pixel_ModuleHash);
                else ATH_MSG_WARNING("Trying to access a Pixel module when running with no Pixel!");
				//module = SiModule;
			} else if (m_idHelper->is_sct(ModuleID)) {
                                const IdentifierHash SCT_ModuleHash = m_sctIdHelper->wafer_hash(ModuleID);
				if(sctElements) SiModule = sctElements->getDetectorElement(SCT_ModuleHash);
                else ATH_MSG_WARNING("Trying to access an SCT/Strop module when running with no SCT/Strip!");
				//module = SiModule;OB
			} else if (m_idHelper->is_trt(ModuleID)) {
				//module = m_TRT_Manager->getElement(ModuleID);
				//const InDetDD::TRT_BaseElement *p_TRT_Module = m_TRT_Manager->getElement(iter->second.moduleID());
			} else {
				ATH_MSG_WARNING( "Something fishy, identifier is neither Pixel, nor SCT or TRT!" );
			}
			
			//TRT alignment transformations are given in global frame in DB,
			// that's not fully correct, since the level2 transform can rotate the system in which level1 transforms
			// are applied ...
			
			//Si have a local coordinate system
			// Take care: For SCT we have to ensure that module's
			// system is taken, not the system of one of the wafers!
			HepGeom::Transform3D localToGlobal = HepGeom::Transform3D();
			if ((not m_idHelper->is_trt(ModuleID))){
			  if (SiModule){
			    localToGlobal=Amg::EigenTransformToCLHEP(SiModule->moduleTransform());
			  } else {
			    ATH_MSG_WARNING("Apparently in a silicon detector, but SiModule is a null pointer");
			  }
			}
			const HepGeom::Point3D<double> center = iter->second;
			
			//center of module in global coordinates
			double r = center.rho(); //distance from beampipe
			double phi = center.phi();
			double z = center.z();
			
			HepGeom::Transform3D parameterizedTrafo;
			HepGeom::Transform3D alignmentTrafo;
			

			// prepare scale factor for different subsystems:
      double ScaleFactor = 1.;

      if (m_idHelper->is_pixel(ModuleID))
        {
          if (m_pixelIdHelper->is_barrel(ModuleID))   {
            ScaleFactor=m_ScalePixelBarrel;
          }
          else {
            ScaleFactor=m_ScalePixelEndcap;
          }
          if (m_pixelIdHelper->is_blayer(ModuleID))   {  // IBL
            ScaleFactor=m_ScalePixelIBL;
          }
          if (m_pixelIdHelper->is_dbm(ModuleID))   {    // DBM
            ScaleFactor=m_ScalePixelDBM;
          }

        } else if (m_idHelper->is_sct(ModuleID))
        {
          if (m_sctIdHelper->is_barrel(ModuleID)) {
            ScaleFactor=m_ScaleSCTBarrel;
          }
          else {
            ScaleFactor=m_ScaleSCTEndcap;
          }

        } else if (m_idHelper->is_trt(ModuleID))
        {
          if (m_trtIdHelper->is_barrel(ModuleID)) {
            ScaleFactor=m_ScaleTRTBarrel;
          }
          else {
            ScaleFactor=m_ScaleTRTEndcap;
          }
        } else {
        ATH_MSG_WARNING( "Something fishy, identifier is neither Pixel, nor SCT or TRT!" );
      }



			ATH_MSG_INFO(  "ID Module " << i << " with ID " << m_idHelper->show_to_string(ModuleID,nullptr,'/') );
			if (msgLvl(MSG::DEBUG)) {
				msg() << "radius "  << r / CLHEP::cm << " centimeter" << endmsg;
				msg() << "phi "  << phi << endmsg;
				msg() << "z "  << z / CLHEP::cm << " centimeter" << endmsg;
				if (msgLvl(MSG::VERBOSE)) {
					msg() << "localToGlobal transformation:" << endmsg;
					msg() << "translation: "  << localToGlobal.dx() / CLHEP::cm << ";" << localToGlobal.dy() / CLHEP::cm << ";" << localToGlobal.dz() / CLHEP::cm << endmsg;
					msg() << "rotation: "  << endmsg;
					msg() << localToGlobal.xx() << " " << localToGlobal.xy() << " " << localToGlobal.xz() << endmsg;
					msg() << localToGlobal.yx() << " " << localToGlobal.yy() << " " << localToGlobal.yz() << endmsg;
					msg() << localToGlobal.zx() << " " << localToGlobal.zy() << " " << localToGlobal.zz() << endmsg;
				}
			}
			
			if (!m_MisalignmentMode) {
				//no misalignment mode set
				parameterizedTrafo = HepGeom::Transform3D(); // initialized as identity transformation
			}
			
			else if (m_MisalignmentMode==1) {
				//shift whole detector
				HepGeom::Vector3D<double> shift(ScaleFactor*m_Misalign_x, ScaleFactor*m_Misalign_y, ScaleFactor*m_Misalign_z);

				CLHEP::HepRotation rot;
        rot = CLHEP::HepRotationX(ScaleFactor*m_Misalign_alpha) * CLHEP::HepRotationY(ScaleFactor*m_Misalign_beta) * CLHEP::HepRotationZ(ScaleFactor*m_Misalign_gamma);

        if (ScaleFactor == 0.0)  {
          parameterizedTrafo = HepGeom::Transform3D(); // initialized as identity transformation
        } else {
          parameterizedTrafo = HepGeom::Transform3D(rot, shift);
        }

			}
			
			else if (m_MisalignmentMode == 2) {
				
                                // randomly misalign modules at L3
                Rndm::Numbers RandMisX(randsvc, Rndm::Gauss(m_Misalign_x,m_RndmMisalignWidth_x*ScaleFactor));
                Rndm::Numbers RandMisY(randsvc, Rndm::Gauss(m_Misalign_y,m_RndmMisalignWidth_y*ScaleFactor));
                Rndm::Numbers RandMisZ(randsvc, Rndm::Gauss(m_Misalign_z,m_RndmMisalignWidth_z*ScaleFactor));
                Rndm::Numbers RandMisalpha(randsvc, Rndm::Gauss(m_Misalign_alpha,m_RndmMisalignWidth_alpha*ScaleFactor));
                Rndm::Numbers RandMisbeta(randsvc, Rndm::Gauss(m_Misalign_beta,m_RndmMisalignWidth_beta*ScaleFactor));
                Rndm::Numbers RandMisgamma(randsvc, Rndm::Gauss(m_Misalign_gamma,m_RndmMisalignWidth_gamma*ScaleFactor));
                
                double randMisX = RandMisX(); //assign to variables to allow the values to be queried
                double randMisY = RandMisY();
                double randMisZ = RandMisZ();
                
                HepGeom::Vector3D<double> shift(randMisX, randMisY, randMisZ);
                double randMisaplha = RandMisalpha();
                double randMisbeta = RandMisbeta();
                double randMisgamma = RandMisgamma();
                
                CLHEP::HepRotation rot;
				rot = CLHEP::HepRotationX(randMisaplha) * CLHEP::HepRotationY(randMisbeta) * CLHEP::HepRotationZ(randMisgamma);
				

				if (ScaleFactor == 0.0)  {
                                  parameterizedTrafo = HepGeom::Transform3D(); // initialized as identity transformation
                                } else {
                                  parameterizedTrafo = HepGeom::Transform3D(rot, shift);
                                }

			}

                        else if (m_MisalignmentMode==3) {
                          //shift whole detector                                                                                                        
                          double deltaX;
                          if ( m_idHelper->is_pixel(ModuleID) && m_pixelIdHelper->is_blayer(ModuleID) )   {
                            //function is parameterized in global z                                                                                     
			    deltaX = getBowingTx( getBowingMagParam(m_IBLBowingTshift), z);
                                                  
                          } else {
                            //IBL-stave temperature distortion not applied to anything but IBL                             
                            deltaX = 0.;
                            ATH_MSG_DEBUG( "will not move this module for IBL temp distortion " );
                          }

                          ATH_MSG_DEBUG( "deltaX for this module: "  << deltaX/CLHEP::micrometer << " um" );
                          parameterizedTrafo = HepGeom::Translate3D(deltaX,0,0); // translation in x direction                                          
                        }
			
			else { // systematic misalignments
				if (m_MisalignmentMode/10==1) {
					//radial misalignments
					double deltaR;
					if (m_MisalignmentMode==11) {
						//R deltaR = radial expansion
						if (m_idHelper->is_trt(ModuleID) && abs(m_trtIdHelper->barrel_ec(ModuleID))==2) {
							//radial mode cannot handle TRT endcap, sorry
							deltaR = 0.;
							ATH_MSG_DEBUG( "will not move TRT endcap for radial distortion " );
						} else {
							//deltaR = 0.5 * cos ( 2*phi ) * r/maxRadius * maxDeltaR;
							deltaR = r/maxRadius * maxDeltaR; //scale linearly in r
						}
					} else if (m_MisalignmentMode==12) {
						//Phi deltaR = elliptical (egg-shape)
						if (m_idHelper->is_trt(ModuleID) && abs(m_trtIdHelper->barrel_ec(ModuleID))==2) {
							//elliptical mode cannot handle TRT endcap, sorry
							deltaR = 0.;
							ATH_MSG_DEBUG( "will not move TRT endcap for elliptical distortion " );
						} else {
							// deltaR = 0.5 * cos ( 2*phi ) * r/maxRadius * maxDeltaR;
							deltaR = cos ( 2*phi ) * r/maxRadius * maxDeltaR;
						}
					} else if (m_MisalignmentMode==13) {
						//Z deltaR = funnel
						if (m_idHelper->is_trt(ModuleID) && abs(m_trtIdHelper->barrel_ec(ModuleID))==2) {
							//funnel mode cannot handle TRT endcap, sorry
							deltaR = 0.;
							ATH_MSG_DEBUG( "will not move TRT endcap for funnel distortion " );
						} else {
							//deltaR = z/maxLength * maxDeltaR; // linearly in z
							deltaR = 2. * z/maxLength * maxDeltaR; // linearly in z
						}
					} else {
						ATH_MSG_DEBUG( "Wrong misalignment mode entered, doing nothing." );
						deltaR=0;
					}
					
					ATH_MSG_DEBUG( "deltaR for this module: "  << deltaR / CLHEP::micrometer << " um" );
					parameterizedTrafo = HepGeom::Translate3D(deltaR*cos(phi),deltaR * sin(phi),0.); // translation along R vector
				}
				
				else if (m_MisalignmentMode/10==2) {
					//azimuthal misalignments
					double deltaPhi;
					if (m_MisalignmentMode==21) {
						
					        deltaPhi = r/maxRadius * maxAngle + minRadius/r * maxAngleInner; //linearly + reciprocal term in r
					} else if (m_MisalignmentMode==22) {
						//Phi deltaPhi = clamshell
						//                     deltaPhi = std::abs( sin ( phi )) * maxAngle;
						if (m_idHelper->is_trt(ModuleID) && abs(m_trtIdHelper->barrel_ec(ModuleID))==2) {
							//clamshell mode cannot handle TRT endcap, sorry
							deltaPhi = 0.;
							ATH_MSG_DEBUG( "will not move TRT endcap for clamshell distortion " );
						} else {
							//                        deltaPhi = 0.5 * cos ( 2*phi ) * maxAngle;
							deltaPhi =  cos ( 2*phi ) * maxAngle;
						}
					} else if (m_MisalignmentMode==23) {
						//Z deltaPhi = Twist
						deltaPhi = 2*z/maxLength * maxAngle;
						//deltaPhi = z/maxLength * maxAngle;
					} else {
						ATH_MSG_WARNING( "Wrong misalignment mode entered, doing nothing." );
						deltaPhi=0;
					}
					
					ATH_MSG_DEBUG( "deltaPhi for this module: "  << deltaPhi/CLHEP::mrad << " mrad" );
					parameterizedTrafo = HepGeom::RotateZ3D(deltaPhi); // rotation around z axis => in phi
				}
				
				else if (m_MisalignmentMode/10==3) {
					//z misalignments
					double deltaZ;
					if (m_MisalignmentMode==31) {
						//R deltaZ = Telescope
						deltaZ = r/maxRadius * maxDeltaZ; //scale linearly in r
					} else if (m_MisalignmentMode==32) {
						
						if (m_idHelper->is_trt(ModuleID) && abs(m_trtIdHelper->barrel_ec(ModuleID))==2) {
							//clamshell mode cannot handle TRT endcap, sorry
							deltaZ = 0.;
							ATH_MSG_DEBUG( "will not move TRT endcap for skew distortion " );
						} else {
							
							deltaZ =  cos ( 2*phi ) * maxDeltaZ;
						}
					} else if (m_MisalignmentMode==33) {
						//Z deltaZ = Z expansion
						//                  deltaZ = z/maxLength * maxDeltaZ;
						deltaZ = 2. * z/maxLength * maxDeltaZ;
					} else {
						ATH_MSG_WARNING( "Wrong misalignment mode entered, doing nothing." );
						deltaZ=0;
					}
					
					ATH_MSG_DEBUG( "deltaZ for this module: "  << deltaZ/CLHEP::micrometer << " um" );
					parameterizedTrafo = HepGeom::Translate3D(0,0,deltaZ); // translation in z direction
				}
				
				else {
					//no or wrong misalignment selected
					ATH_MSG_WARNING( "Wrong misalignment mode entered, doing nothing." );
					
					parameterizedTrafo = HepGeom::Transform3D(); // initialized as identity transformation
				}
			} //end of misalignment
			
			if ( m_MisalignmentMode==21 && m_idHelper->is_trt(ModuleID) && m_trtIdHelper->is_barrel(ModuleID) ) {
				//curl for TRT barrel
				ATH_MSG_DEBUG( "additional rotation for TRT barrel module!" );
				HepGeom::Transform3D realLocalToGlobalTRT = HepGeom::Translate3D(center.x(),center.y(),center.z());
				//rotate a TRT barrel module by the same angle again, but around its local z axis
				//this is an approximation to accomodate the impossible curling of TRT segments
				alignmentTrafo = parameterizedTrafo * realLocalToGlobalTRT * parameterizedTrafo * realLocalToGlobalTRT.inverse();
			} else if (m_MisalignmentMode==23 && m_idHelper->is_trt(ModuleID) && m_trtIdHelper->is_barrel(ModuleID) ) {
				//do the twist! (for TRT barrel)
				HepGeom::Transform3D realLocalToGlobalTRT = HepGeom::Translate3D(center.x(),center.y(),center.z());
				double deltaAlpha = (-2.) * r * maxAngle/maxLength;
				ATH_MSG_DEBUG( "TRT barrel module alpha for twist: "  << deltaAlpha/CLHEP::mrad << " mrad" );
				
				CLHEP::HepRotation twistForTRTRotation(HepGeom::Vector3D<double>(center.x(),center.y(),center.z()), deltaAlpha );
				HepGeom::Transform3D twistForTRT= HepGeom::Transform3D(twistForTRTRotation,HepGeom::Vector3D<double>(0.,0.,0.));
				//             HepGeom::Transform3D twistForTRTRotation = HepGeom::RotateZ3D( r * maxAngle/maxLength );
				
				alignmentTrafo = realLocalToGlobalTRT * twistForTRT * realLocalToGlobalTRT.inverse();
			} else if (m_MisalignmentMode==13 && m_idHelper->is_trt(ModuleID) && m_trtIdHelper->is_barrel(ModuleID) ) {
				// funneling for TRT barrel
				HepGeom::Transform3D realLocalToGlobalTRT = HepGeom::Translate3D(center.x(),center.y(),center.z());
				double deltaAlpha = (-2.) * maxDeltaR/maxLength;
				//double deltaAlpha = maxDeltaR/maxLength;
				ATH_MSG_DEBUG( "TRT barrel module alpha for funnel: "  << deltaAlpha/CLHEP::mrad << " mrad" );
				
				HepGeom::Vector3D<double> normalVector(center.x(),center.y(),center.z());
				HepGeom::Vector3D<double> beamVector(0.,0.,1.);
				HepGeom::Vector3D<double> rotationAxis = normalVector.cross(beamVector);
				CLHEP::HepRotation twistForTRTRotation(rotationAxis, deltaAlpha );
				HepGeom::Transform3D twistForTRT= HepGeom::Transform3D(twistForTRTRotation,HepGeom::Vector3D<double>(0.,0.,0.));
				
				alignmentTrafo = realLocalToGlobalTRT * twistForTRT * realLocalToGlobalTRT.inverse();
				
				
				
			} else if (m_MisalignmentMode==2 || m_MisalignmentMode==3) //random misalignment in local frame
			{
				alignmentTrafo = parameterizedTrafo;
			}
			else {
				// final transformation executed in global coordinates, converted to local coordinates
				alignmentTrafo = localToGlobal.inverse() * parameterizedTrafo * localToGlobal; 
			}
			
			if (msgLvl(MSG::INFO)) {
				msg() << "Align Transformation x = ("  << alignmentTrafo.getTranslation().x() / CLHEP::micrometer << " um)" << endmsg;
				msg() << "Align Transformation y = ("  << alignmentTrafo.getTranslation().y() / CLHEP::micrometer << " um)" << endmsg;
				msg() << "Align Transformation z = ("  << alignmentTrafo.getTranslation().z() / CLHEP::micrometer << " um)" << endmsg;
				msg() << "Align Transformation x phi   = ("  << alignmentTrafo.getRotation().phiX() / CLHEP::deg << ")" << endmsg;
				msg() << "Align Transformation x Theta = ("  << alignmentTrafo.getRotation().thetaX() / CLHEP::deg << ")" << endmsg;
				msg() << "Align Transformation y phi   = ("  << alignmentTrafo.getRotation().phiY() / CLHEP::deg << ")" << endmsg;
				msg() << "Align Transformation y Theta = ("  << alignmentTrafo.getRotation().thetaY() / CLHEP::deg << ")" << endmsg;
				msg() << "Align Transformation z phi   = ("  << alignmentTrafo.getRotation().phiZ() / CLHEP::deg << ")" << endmsg;
				msg() << "Align Transformation z Theta = ("  << alignmentTrafo.getRotation().thetaZ() / CLHEP::deg << ")" << endmsg;
			}
			
			// suppress tiny translations that occur due to trafo.inverse*trafo numerics
			if ( std::abs(alignmentTrafo.getTranslation().x()) < 1e-10) {
				HepGeom::Vector3D<double>
				zeroSuppressedTranslation(0,alignmentTrafo.getTranslation().y(),alignmentTrafo.
										  getTranslation().z());
				alignmentTrafo =
                HepGeom::Transform3D(alignmentTrafo.getRotation(),zeroSuppressedTranslation);
			}
			if ( std::abs(alignmentTrafo.getTranslation().y()) < 1e-10) {
				HepGeom::Vector3D<double>
				zeroSuppressedTranslation(alignmentTrafo.getTranslation().x(),0,alignmentTrafo.
										  getTranslation().z());
				alignmentTrafo =
                HepGeom::Transform3D(alignmentTrafo.getRotation(),zeroSuppressedTranslation);
			}
			if ( std::abs(alignmentTrafo.getTranslation().z()) < 1e-10) {
				HepGeom::Vector3D<double>
				zeroSuppressedTranslation(alignmentTrafo.getTranslation().x(),alignmentTrafo.getTranslation().y(),0);
				alignmentTrafo =
                HepGeom::Transform3D(alignmentTrafo.getRotation(),zeroSuppressedTranslation);
			}
			if ( std::abs(alignmentTrafo.getRotation().getDelta()) < 1e-10) {
				CLHEP::HepRotation zeroSuppressedRotation(alignmentTrafo.getRotation());
				zeroSuppressedRotation.setDelta(0.);
				alignmentTrafo =
                HepGeom::Transform3D(zeroSuppressedRotation,alignmentTrafo.getTranslation());
			}
			
			
			Amg::Transform3D alignmentTrafoAmg = Amg::CLHEPTransformToEigen(alignmentTrafo);
			
			if (m_idHelper->is_sct(ModuleID) || m_idHelper->is_pixel(ModuleID)) {
				if (m_IDAlignDBTool->tweakTrans(ModuleID,3, alignmentTrafoAmg)) {
					ATH_MSG_INFO( "Update of alignment constants for module " << m_idHelper->show_to_string(ModuleID,nullptr,'/') << " successful" );
				} else {
					ATH_MSG_ERROR( "Update of alignment constants for module " << m_idHelper->show_to_string(ModuleID,nullptr,'/') << " not successful" );
				}
			} else if (m_idHelper->is_trt(ModuleID)) {
				if (!m_trtIdHelper->is_barrel(ModuleID) && m_trtIdHelper->phi_module(ModuleID)!=0) {
					//don't align - there's no trans in the DB for phi sectors other than 0
					ATH_MSG_DEBUG( "TRT endcap phi sector " << m_trtIdHelper->phi_module(ModuleID) << " not aligned" );
				} else {
					//if (m_trtaligndbservice->tweakTrans(ModuleID,alignmentTrafo).isFailure()) {
					if (m_trtaligndbservice->tweakAlignTransform(ModuleID,alignmentTrafoAmg,2).isFailure()) { 
						ATH_MSG_ERROR( "Update of alignment constants for module " << m_idHelper->show_to_string(ModuleID,nullptr,'/') << " not successful" );
					} else {
						ATH_MSG_INFO( "Update of alignment constants for module " << m_idHelper->show_to_string(ModuleID,nullptr,'/') << " successful" );
					}
				}
			} else {
				ATH_MSG_WARNING( "Something fishy, identifier is neither Pixel, nor SCT or TRT!" );
			}
			
			double alpha, beta, gamma;
			m_IDAlignDBTool->extractAlphaBetaGamma(alignmentTrafoAmg, alpha, beta, gamma);

			m_AlignResults_x = alignmentTrafo.getTranslation().x();
			m_AlignResults_y = alignmentTrafo.getTranslation().y();
			m_AlignResults_z = alignmentTrafo.getTranslation().z();
			m_AlignResults_alpha = alpha;
			m_AlignResults_beta = beta;
			m_AlignResults_gamma = gamma;
			

                        HepGeom::Transform3D LocalaGlobal = HepGeom::Transform3D();
                        LocalaGlobal = Amg::EigenTransformToCLHEP(SiModule->moduleTransform());
                        HepGeom::Point3D<double> alignedPosLocal(m_AlignResults_x,m_AlignResults_y,m_AlignResults_z);



              
                        m_Initial_center_x = center.x() ;
                        m_Initial_center_y = center.y() ;
                        m_Initial_center_z = center.z() ;
                        
                        HepGeom::Point3D<double> alignedPosGlobal = LocalaGlobal * alignedPosLocal;
                        
                        // Global Misalignment HERE
                        if (m_idHelper->is_sct(ModuleID)) {
                              // non-zero local center position gives additional radial shift of SCT endcap
                              const InDetDD::StripStereoAnnulusDesign *p_design_check = dynamic_cast<const InDetDD::StripStereoAnnulusDesign*>(&(SiModule->design()));
                              if (p_design_check){
                                     Amg::Vector3D SCT_Center = p_design_check->sensorCenter();
                                     double radialShift_x = SCT_Center[0];     // in sensor frame, x direction
                                     double radialShift_y = SCT_Center[1];     // in sensor frame, y direction
                                     HepGeom::Transform3D radial_shift = HepGeom::Translate3D(radialShift_x,radialShift_y,0);       // the additional radial shift applied as translation
                                     HepGeom::Transform3D LocalaaGlobal = LocalaGlobal * radial_shift;                              // apply additional radial shift
                                     HepGeom::Point3D<double> SCT_endcap_alignedPosGlobal = LocalaaGlobal * alignedPosLocal;       // corrected global transformation 
                                     m_Global_center_x = SCT_endcap_alignedPosGlobal.x();
                                     m_Global_center_z = SCT_endcap_alignedPosGlobal.z();
                                     m_Global_center_y = SCT_endcap_alignedPosGlobal.y();
                              }

                              else { // no additional radial shift for SCT barrel 
                                     m_Global_center_x =  alignedPosGlobal.x();
                                     m_Global_center_y =  alignedPosGlobal.y();
                                     m_Global_center_z =  alignedPosGlobal.z();
                                                                                                                                                                 
                              }
                              
                        }

                        else { // no additional radial shift for non-SCT elements
                               m_Global_center_x =  alignedPosGlobal.x();
                               m_Global_center_y =  alignedPosGlobal.y();
                               m_Global_center_z =  alignedPosGlobal.z();
                        }


                         if (m_idHelper->is_sct(ModuleID)) {
				m_AlignResults_Identifier_ID = 2;
				m_AlignResults_Identifier_PixelSCT = 2;
				m_AlignResults_Identifier_BarrelEC = m_sctIdHelper->barrel_ec(ModuleID);
				m_AlignResults_Identifier_LayerDisc = m_sctIdHelper->layer_disk(ModuleID);
				m_AlignResults_Identifier_Phi = m_sctIdHelper->phi_module(ModuleID);
				m_AlignResults_Identifier_Eta = m_sctIdHelper->eta_module(ModuleID);
			} else if (m_idHelper->is_pixel(ModuleID)) {
				m_AlignResults_Identifier_ID = 2;
				m_AlignResults_Identifier_PixelSCT = 1;
				m_AlignResults_Identifier_BarrelEC = m_pixelIdHelper->barrel_ec(ModuleID);
				m_AlignResults_Identifier_LayerDisc = m_pixelIdHelper->layer_disk(ModuleID);
				m_AlignResults_Identifier_Phi = m_pixelIdHelper->phi_module(ModuleID);
				m_AlignResults_Identifier_Eta = m_pixelIdHelper->eta_module(ModuleID);
			} else if (m_idHelper->is_trt(ModuleID)) {
				m_AlignResults_Identifier_ID = 2;
				m_AlignResults_Identifier_PixelSCT = 3;
				m_AlignResults_Identifier_BarrelEC = m_trtIdHelper->barrel_ec(ModuleID);
				m_AlignResults_Identifier_LayerDisc = m_trtIdHelper->layer_or_wheel(ModuleID);
				m_AlignResults_Identifier_Phi = m_trtIdHelper->phi_module(ModuleID);
				m_AlignResults_Identifier_Eta = m_trtIdHelper->straw_layer(ModuleID);
				
			} else {
				ATH_MSG_WARNING( "Something fishy, identifier is neither Pixel, nor SCT or TRT!" );
			}
			
			// Write out AlignResults ntuple
			if (StatusCode::SUCCESS!=ntupleSvc()->writeRecord("NTUPLES/CREATEMISALIGN/InitialAlignment")) {
				ATH_MSG_ERROR( "Could not write InitialAlignment ntuple." );
			}
			
		} // end of module loop
		
		// 	i = 0;
		
		//m_IDAlignDBTool->printDB(2);
		if(m_doPix || m_doStrip){
		 if (StatusCode::SUCCESS!=m_IDAlignDBTool->outputObjs()) {
			ATH_MSG_ERROR( "Writing of AlignableTransforms failed" );
		 } else {
			ATH_MSG_INFO( "AlignableTransforms were written" );
			ATH_MSG_INFO( "Writing database to textfile" );
			m_IDAlignDBTool->writeFile(false,m_asciiFileNameBase+"_Si.txt");
			ATH_MSG_INFO( "Writing IoV information to mysql file" );
			m_IDAlignDBTool->fillDB("InDetSi_"+m_SQLiteTag,IOVTime::MINRUN,IOVTime::MINEVENT,IOVTime::MAXRUN,IOVTime::MAXEVENT);
		} 
        }
		
		if(m_doTRT){
         if (StatusCode::SUCCESS!=m_trtaligndbservice->streamOutAlignObjects()) {
			ATH_MSG_ERROR( "Write of AlignableTransforms (TRT) failed" );
		 } else {
			ATH_MSG_INFO( "AlignableTransforms for TRT were written" );
			ATH_MSG_INFO( "Writing TRT database to textfile" );
			if ( StatusCode::SUCCESS != m_trtaligndbservice->writeAlignTextFile(m_asciiFileNameBase+"_TRT.txt") ) {
                          ATH_MSG_ERROR( "Failed to write AlignableTransforms (TRT) to txt file " << m_asciiFileNameBase+"_TRT.txt" );
                        }
			ATH_MSG_INFO( "Writing IoV information for TRT to mysql file" );
			if ( StatusCode::SUCCESS
                             != m_trtaligndbservice->registerAlignObjects(m_SQLiteTag+"_TRT",IOVTime::MINRUN,IOVTime::MINEVENT,IOVTime::MAXRUN,IOVTime::MAXEVENT)) {
                          ATH_MSG_ERROR( "Write of AIoV information (TRT) to mysql failed (tag=" << m_SQLiteTag << "_TRT)");
                        }
		 }
        }

		return StatusCode::SUCCESS;               
		
	}
	
	//__________________________________________________________________________
	const HepGeom::Transform3D CreateMisalignAlg::BuildAlignTransform(const CLHEP::HepVector & AlignParams)
	{
		HepGeom::Vector3D<double> AlignShift(AlignParams[0],AlignParams[1],AlignParams[2]);
		CLHEP::HepRotation AlignRot;
		
		AlignRot = CLHEP::HepRotationX(AlignParams[3]) * CLHEP::HepRotationY(AlignParams[4]) * CLHEP::HepRotationZ(AlignParams[5]);
		
		HepGeom::Transform3D AlignTransform = HepGeom::Transform3D(AlignRot,AlignShift);
		return AlignTransform;
	}
	
	//__________________________________________________________________________
	const Identifier CreateMisalignAlg::reduceTRTID(Identifier id)
	{
		//     msg(MSG::DEBUG)  << "in CreateMisalignAlg::reduceTRTID" << endmsg;
		ATH_MSG_DEBUG( "reduceTRTID got Id " << m_idHelper->show_to_string(id,nullptr,'/'));
		
		int barrel_ec= m_trtIdHelper->barrel_ec(id);
		// attention: TRT DB only has one alignment correction per barrel module (+1/-1) pair
		// which is stored in -1 identifier
		if (barrel_ec==1) barrel_ec=-1; //only regard -1 barrel modules, +1 modules will belong to them
		
		//if (abs(barrel_ec)==2) phi_module=0; 
		//  only regard phi sector 0, the only one having an alignmentTrafo
		//does not work, since the center-of-mass of all phi sectors is on the beamline,so
		//  transformations would become zero -> this has to be handled later
		int phi_module=m_trtIdHelper->phi_module(id);
		
		int layer_or_wheel=m_trtIdHelper->layer_or_wheel(id);
		
		int strawlayer=0;
		if (!m_trtIdHelper->is_barrel(id)) {
			strawlayer = m_trtIdHelper->straw_layer(id) / 4 * 4;
			// only strawlayers 0,4,8,12 are fed into DB for endcap
		}
		
		//     if (msgLvl(MSG::DEBUG)) msg()  << "    and returns Id " << m_idHelper->show_to_string(m_trtIdHelper->module_id(barrel_ec,phi_module,layer_or_wheel),0,'/') << endmsg;
		ATH_MSG_DEBUG(  "    and returns Id " << m_idHelper->show_to_string(m_trtIdHelper->layer_id(barrel_ec,phi_module,layer_or_wheel,strawlayer),nullptr,'/'));
		//     return  m_trtIdHelper->module_id(barrel_ec,phi_module,layer_or_wheel);
		return  m_trtIdHelper->layer_id(barrel_ec,phi_module,layer_or_wheel,strawlayer);
	}
	
	//__________________________________________________________________________
	const Identifier CreateMisalignAlg::reduceTRTID(IdentifierHash &hash)
	{
		Identifier id= m_trtIdHelper->module_id(hash);
		return reduceTRTID(id);
	}

        //__________________________________________________________________________                                                            
        double CreateMisalignAlg::getBowingMagParam(double temp_shift)
	{
	  // IBL staves are straight at a set point of 15 degrees.                                                                                            
	  // Get set point value to use for magnitude parameter from temp_shift starting at 15 degrees                                                        
	  double T = 15 + temp_shift;
	  return 1.53e-12 - 1.02e-13*T;
	}

        //__________________________________________________________________________                                                                          
        double CreateMisalignAlg::getBowingTx(double p1, double z)
	{
	  // Bowing fit function has the following form                                                                                                       
	  // [0]-[1]*(x+[2]) * (4.0*[2]*(x+[2])**2 - (x+[2])**3 - (2.0*[2])**3)                                                                               
	  // param 0 : is the baseline shift (fixed at 0 for MC)                                                                                              
	  // param 1 : is the magnitude fit param (temp dependent input param)                                                                                
	  // param 2 : is the stave fix pointat both ends (fixed at 366.5)                                                                                    
	  double p0 = 0;
	  double p2 = 366.5;
	  double Tx = p0 - p1*(z+p2) * (4.*p2*pow((z+p2),2)  - pow((z+p2),3) - pow((2.*p2),3));
	  return Tx;
	}
		
} // end of namespace bracket
