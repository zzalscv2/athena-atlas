/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DNNCaloSimSvc.cxx, (c) ATLAS Detector software             //
///////////////////////////////////////////////////////////////////

// class header include
#include "DNNCaloSimSvc.h"


// FastCaloSim includes
#include "ISF_FastCaloSimEvent/TFCSParametrizationBase.h"
#include "ISF_FastCaloSimEvent/TFCSSimulationState.h"
#include "ISF_FastCaloSimEvent/TFCSTruthState.h"
#include "ISF_FastCaloSimEvent/TFCSExtrapolationState.h"
#include "ISF_FastCaloSimParametrization/CaloGeometryFromCaloDDM.h"
#include "ISF_FastCaloSimEvent/FastCaloSim_CaloCell_ID.h"

//!
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "IdDictParser/IdDictParser.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/TileID.h"
//!

// StoreGate
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/StoreGate.h"

#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "LArReadoutGeometry/FCALDetectorManager.h"

#include "PathResolver/PathResolver.h"

#include "lwtnn/parse_json.hh"

#include "TFile.h"
#include <fstream>
#include "CLHEP/Random/RandGauss.h"

using std::abs;
using std::atan2;

/** Constructor **/
ISF::DNNCaloSimSvc::DNNCaloSimSvc(const std::string& name, ISvcLocator* svc) :
  BaseSimulationSvc(name, svc),
  m_graph(nullptr),
  m_theContainer(nullptr),
  m_rndGenSvc("AtRndmGenSvc", name),
  m_randomEngine(nullptr),
  m_caloDetDescrManager(nullptr),
  m_caloGeo(nullptr)
{
  declareProperty("ParamsInputFilename"            ,       m_paramsFilename,"DNNCaloSim/DNNCaloSim_GAN_nn_v0.json");
  declareProperty("ParamsInputArchitecture"        ,       m_paramsInputArchitecture,"GANv0");
  declareProperty("CaloCellsOutputName"            ,       m_caloCellsOutputName) ;
  declareProperty("CaloCellMakerTools_setup"       ,       m_caloCellMakerToolsSetup) ;
  declareProperty("CaloCellMakerTools_release"     ,       m_caloCellMakerToolsRelease) ;
  declareProperty("RandomSvc"                      ,       m_rndGenSvc                );
  declareProperty("RandomStream"                   ,       m_randomEngineName         );
  declareProperty("FastCaloSimCaloExtrapolation"   ,       m_FastCaloSimCaloExtrapolation );
}

ISF::DNNCaloSimSvc::~DNNCaloSimSvc()
{}

/** framework methods */
StatusCode ISF::DNNCaloSimSvc::initialize()
{
  ATH_MSG_INFO(m_screenOutputPrefix << "Initializing ...");

  ATH_CHECK(m_rndGenSvc.retrieve());
  m_randomEngine = m_rndGenSvc->GetEngine( m_randomEngineName);
  if(!m_randomEngine)
  {
   ATH_MSG_ERROR("Could not get random number engine from RandomNumberService. Abort.");
   return StatusCode::FAILURE;
  }
  
  m_caloDetDescrManager  = CaloDetDescrManager::instance();
  const FCALDetectorManager * fcalManager=NULL;
  ATH_CHECK(detStore()->retrieve(fcalManager));

  const CaloIdManager* caloId_mgr = m_caloDetDescrManager->getCalo_Mgr();
  m_emID = caloId_mgr->getEM_ID();
  
  m_caloGeo = new CaloGeometryFromCaloDDM();
  m_caloGeo->LoadGeometryFromCaloDDM(m_caloDetDescrManager);
  if(!m_caloGeo->LoadFCalChannelMapFromFCalDDM(fcalManager) )ATH_MSG_FATAL("Found inconsistency between FCal_Channel map and GEO file. Please, check if they are configured properly.");


  // initialize DNN 
  if (initializeNetwork().isFailure())
    {
      ATH_MSG_ERROR("Could not initialize network ");
      return StatusCode::FAILURE;

    }

  
  // Get FastCaloSimCaloExtrapolation
  if(m_FastCaloSimCaloExtrapolation.retrieve().isFailure())
  {
   ATH_MSG_ERROR("FastCaloSimCaloExtrapolation not found ");
   return StatusCode::FAILURE;
  }

  m_windowCells.reserve(m_numberOfCellsForDNN);
  
  return StatusCode::SUCCESS;
}

// initialize lwtnn network 
StatusCode ISF::DNNCaloSimSvc::initializeNetwork()
{



  // get neural net JSON file as an std::istream object
  std::string inputFile=PathResolverFindCalibFile(m_paramsFilename);
  if (inputFile==""){
    ATH_MSG_ERROR("Could not find json file " << m_paramsFilename );
    return StatusCode::FAILURE;
  } 

  // initialize all necessary constants
  // FIXME eventually all these could be stored in the .json file

  if (m_paramsInputArchitecture=="GANv0") // GAN then VAE etc...
    {
      m_GANLatentSize = 300;
      m_logTrueEnergyMean = 9.70406053;
      m_logTrueEnergyScale = 1.76099569;
      m_riImpactEtaMean = 3.47603256e-05;
      m_riImpactEtaScale = 0.00722316;
      m_riImpactPhiMean = -5.42153684e-05;
      m_riImpactPhiScale = 0.00708241;
    }



  ATH_MSG_DEBUG("Using json file " << m_paramsFilename );
  std::ifstream input(inputFile);
  // build the graph
  m_graph=new lwt::LightweightGraph(lwt::parse_json_graph(input));
  if (m_graph==nullptr){
    ATH_MSG_ERROR("Could not create LightWeightGraph from  " << m_paramsFilename );
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

/** framework methods */
StatusCode ISF::DNNCaloSimSvc::finalize()
{
  ATH_MSG_INFO(m_screenOutputPrefix << "Finalizing ...");
  return StatusCode::SUCCESS;
}

StatusCode ISF::DNNCaloSimSvc::setupEvent()
{
  ATH_MSG_INFO(m_screenOutputPrefix << "setupEvent NEW EVENT! DRDR");
  
  m_theContainer = new CaloCellContainer(SG::VIEW_ELEMENTS);

  StatusCode sc = evtStore()->record(m_theContainer, m_caloCellsOutputName);
  if (sc.isFailure())
  {
    ATH_MSG_FATAL( m_screenOutputPrefix << "cannot record CaloCellContainer " << m_caloCellsOutputName );
    return StatusCode::FAILURE;
  }

  CHECK( m_caloCellMakerToolsSetup.retrieve() );
  ATH_MSG_DEBUG( "Successfully retrieve CaloCellMakerTools: " << m_caloCellMakerToolsSetup );
  ToolHandleArray<ICaloCellMakerTool>::iterator itrTool = m_caloCellMakerToolsSetup.begin();
  ToolHandleArray<ICaloCellMakerTool>::iterator endTool = m_caloCellMakerToolsSetup.end();
  for (; itrTool != endTool; ++itrTool)
  {
    std::string chronoName=this->name()+"_"+ itrTool->name();
    if (m_chrono) m_chrono->chronoStart(chronoName);
    StatusCode sc = (*itrTool)->process(m_theContainer);
    if (m_chrono) {
      m_chrono->chronoStop(chronoName);
      ATH_MSG_DEBUG( m_screenOutputPrefix << "Chrono stop : delta " << m_chrono->chronoDelta (chronoName,IChronoStatSvc::USER) * CLHEP::microsecond / CLHEP::second << " second " );
    }

    if (sc.isFailure())
    {
      ATH_MSG_ERROR( m_screenOutputPrefix << "Error executing tool " << itrTool->name() );
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode ISF::DNNCaloSimSvc::releaseEvent()
{
 
 ATH_MSG_VERBOSE(m_screenOutputPrefix << "release Event");
 
 CHECK( m_caloCellMakerToolsRelease.retrieve() );
 
 //run release tools in a loop
 ToolHandleArray<ICaloCellMakerTool>::iterator itrTool = m_caloCellMakerToolsRelease.begin();
 ToolHandleArray<ICaloCellMakerTool>::iterator endTool = m_caloCellMakerToolsRelease.end();
 for (; itrTool != endTool; ++itrTool)
 {
  ATH_MSG_VERBOSE( m_screenOutputPrefix << "Calling tool " << itrTool->name() );
  
  StatusCode sc = (*itrTool)->process(m_theContainer);
  
  if (sc.isFailure())
  {
   ATH_MSG_ERROR( m_screenOutputPrefix << "Error executing tool " << itrTool->name() );
  }
 }
 
 return StatusCode::SUCCESS;
 
}
bool compCellsForDNNSortMirror(const CaloCell* a, const CaloCell* b)
{
    if ((a->caloDDE()->getSampling()) < (b->caloDDE()->getSampling()))
        return true;
    else if ((a->caloDDE()->getSampling()) > (b->caloDDE()->getSampling()))
        return false;
      // reverse sort in eta for left half of detector
    if ((a->caloDDE()->eta_raw()) < (b->caloDDE()->eta_raw()))
        return false;
    else if ((a->caloDDE()->eta_raw()) > (b->caloDDE()->eta_raw()))
        return true;

    if (((a->caloDDE()->phi_raw()) > (b->caloDDE()->phi_raw()))){
    	// check for pi -pi discontinuity
    	if ((((a->caloDDE()->phi_raw()) - (b->caloDDE()->phi_raw()))) > CLHEP::pi )
    		return true;
    	else
        	return false;
    }
    // check for pi -pi discontinuity
    else if ((((b->caloDDE()->phi_raw()) - (a->caloDDE()->phi_raw()))) > CLHEP::pi )
    	return false;
        

    return true;
}

bool compCellsForDNNSort(const CaloCell* a, const CaloCell* b)
{
    if ((a->caloDDE()->getSampling()) < (b->caloDDE()->getSampling()))
        return true;
    else if ((a->caloDDE()->getSampling()) > (b->caloDDE()->getSampling()))
        return false;

    if ((a->caloDDE()->eta_raw()) < (b->caloDDE()->eta_raw()))
        return true;
    else if ((a->caloDDE()->eta_raw()) > (b->caloDDE()->eta_raw()))
        return false;

    if (((a->caloDDE()->phi_raw()) > (b->caloDDE()->phi_raw()))){
    	// check for pi -pi discontinuity
    	if ((((a->caloDDE()->phi_raw()) - (b->caloDDE()->phi_raw()))) > CLHEP::pi )
    		return true;
    	else
        	return false;
    }
    // check for pi -pi discontinuity
    else if ((((b->caloDDE()->phi_raw()) - (a->caloDDE()->phi_raw()))) > CLHEP::pi )
    	return false;
        

    return true;
}

/** Simulation Call */
StatusCode ISF::DNNCaloSimSvc::simulate(const ISF::ISFParticle& isfp)
{

  ATH_MSG_VERBOSE("NEW PARTICLE! DNNCaloSimSvc called with ISFParticle: " << isfp);
 

  //Don't simulate particles with total energy below 10 MeV
  if(isfp.ekin() < 10) {
    ATH_MSG_VERBOSE("Skipping particle with Ekin: " << isfp.ekin() <<" MeV. Below the 10 MeV threshold.");
    return StatusCode::SUCCESS;
  }


  //Compute all inputs to the network
  NetworkInputs inputs;
  double trueEnergy;
  if (fillNetworkInputs(isfp,inputs,trueEnergy).isFailure())
    {
      ATH_MSG_WARNING("Could not initialize network ");
      return StatusCode::SUCCESS;
    }
 

  // compute the network output values
  NetworkOutputs outputs = m_graph->compute(inputs);
  ATH_MSG_VERBOSE("neural network output = "<<outputs);


  // add network output energy in the right place in the calorimeter
  //NOT NEEDED std::vector<CaloCell*>::iterator windowCell;
  int itr = 0;

  // switch to cpp 11 style
  //  for ( windowCell = m_windowCells.begin(); windowCell != m_windowCells.end(); ++windowCell ) {
  for ( auto & windowCell : m_windowCells) {
    windowCell->addEnergy(trueEnergy * outputs[std::to_string(itr)]);
    itr++;

    ATH_MSG_VERBOSE(" cell eta_raw " << windowCell->caloDDE()->eta_raw() 
		    << " phi_raw " << windowCell->caloDDE()->phi_raw() 
		    << " sampling " << windowCell->caloDDE()->getSampling() 
		    << " energy " << windowCell->energy());
  }




  
  return StatusCode::SUCCESS;
}



// compute all the necessary inputs to the network
StatusCode ISF::DNNCaloSimSvc::fillNetworkInputs(const ISF::ISFParticle& isfp, NetworkInputs & inputs, double & trueEnergy)
{
  Amg::Vector3D particle_position =  isfp.position();  
  Amg::Vector3D particle_direction(isfp.momentum().x(),isfp.momentum().y(),isfp.momentum().z());
  
   //int barcode=isfp.barcode(); // isfp barcode, eta and phi: in case we need them
  // float eta_isfp = particle_position.eta();  
  // float phi_isfp = particle_position.phi(); 

  TFCSTruthState truth(isfp.momentum().x(),isfp.momentum().y(),isfp.momentum().z(),sqrt(isfp.momentum().mag2()+pow(isfp.mass(),2)),isfp.pdgCode());
  truth.set_vertex(particle_position[Amg::x], particle_position[Amg::y], particle_position[Amg::z]);

  TFCSExtrapolationState extrapol;
  //FIXME this is extrapolating to many layers, when we only need middle layer middle surface
  //FIXME could have dedicated extrapolation to save time 
  m_FastCaloSimCaloExtrapolation->extrapolate(extrapol,&truth);
  //  extrapol.Print();
 

  if (false)
    {
      for (int isam=0; isam< CaloCell_ID_FCS::MaxSample ; isam++){
	//enum SUBPOS { SUBPOS_MID = 0, SUBPOS_ENT = 1, SUBPOS_EXT = 2}; //MID=middle, ENT=entrance, EXT=exit of cal layer

	for (int isubpos=0; isubpos< 3 ; isubpos++){
      
	  ATH_MSG_VERBOSE("EXTRAPO isam=" << isam <<
			" isubpos=" << isubpos <<
			" OK="    << extrapol.OK(isam,isubpos) <<
			" eta="  << extrapol.eta(isam,isubpos) <<
			" phi="  << extrapol.phi(isam,isubpos) <<
			" r="  << extrapol.r(isam,isubpos) );

	}
      }
    }

  //FIXME deal with endcap as well 
  int isam=CaloCell_ID_FCS::EMB2;
  int isubpos=SUBPOS_ENT;
  double etaExtrap=-999.;
  double phiExtrap=-999.;
  if (extrapol.eta(isam,isubpos)) {
    etaExtrap=extrapol.eta(isam,isubpos);
    phiExtrap=extrapol.phi(isam,isubpos);
  }

  ATH_MSG_VERBOSE("Will use isam=" << isam <<
		" isubpos=" << isubpos <<
		" eta="  << etaExtrap << 
		" phi="  << phiExtrap );

  //now find the cell it corresponds to  
  //FIXME this is barrel should also look in endcap 
  // (note that is really looking up eta, phi, not raw eta phi
  const CaloDetDescrElement* impactCellDDE=m_caloDetDescrManager->get_element(CaloCell_ID::EMB2,etaExtrap,phiExtrap);
  double caloHashImpactCell=-999;
  double etaImpactCell=-999;
  double phiImpactCell=-999;
  double etaRawImpactCell=-999;
  double phiRawImpactCell=-999;


  trueEnergy = isfp.ekin();

  if (impactCellDDE!=nullptr){
    caloHashImpactCell=impactCellDDE->calo_hash();
    etaImpactCell=impactCellDDE->eta();
    phiImpactCell=impactCellDDE->phi();
    etaRawImpactCell=impactCellDDE->eta_raw();
    phiRawImpactCell=impactCellDDE->phi_raw();

  }

  ATH_MSG_VERBOSE("impact cell calohash=" << caloHashImpactCell <<
		" eta="  << etaImpactCell << 
		" phi="  << phiImpactCell <<
		" eta raw="  << etaRawImpactCell << 
		" phi raw="  << phiRawImpactCell <<
    " true energy=" << trueEnergy  );

  int impactEtaIndex = m_emID->eta(impactCellDDE->identify());
  int impactPhiIndex = m_emID->phi(impactCellDDE->identify());

  ATH_MSG_VERBOSE("impact eta_index " << m_emID->eta(impactCellDDE->identify()) 
		  << " phi_index " << m_emID->phi(impactCellDDE->identify()) 
		  << " sampling " << m_emID->sampling(impactCellDDE->identify()));


  //FIXME move to initialize?
  TFCSSimulationState simulstate(m_randomEngine);

  int nSqCuts = 0;


  // select the cells DNN will simulate 
  // note that m_theCellContainer has all the calorimeter cells
  // this will hold the list of cells to simulate
  //FIXME this can be sped up certainly
  m_windowCells.clear();
  CaloCell_ID::CaloSample sampling;
  for(const auto& theCell : * m_theContainer) {
    sampling = theCell->caloDDE()->getSampling();
    if ((theCell->caloDDE()->eta_raw() < etaRawImpactCell + m_etaRawBackCut) && (theCell->caloDDE()->eta_raw() > etaRawImpactCell - m_etaRawBackCut)) {
      if ((theCell->caloDDE()->phi_raw() < phiRawImpactCell + m_phiRawStripCut) && (theCell->caloDDE()->phi_raw() > phiRawImpactCell - m_phiRawStripCut)) {

      }
      else{
        continue;
      }
    }
    else{
      continue;
    }

    if ((sampling == 0) || (sampling == 1) ){
      if ((theCell->caloDDE()->eta_raw() < etaRawImpactCell + m_etaRawMiddleCut) && (theCell->caloDDE()->eta_raw() > etaRawImpactCell - m_etaRawMiddleCut)) {
	nSqCuts ++;
	// add to vector
	m_windowCells.push_back(theCell);
	
      }
    }
    else if((sampling == 2)) {
      if ((theCell->caloDDE()->phi_raw() < phiRawImpactCell + m_phiRawMiddleCut) && (theCell->caloDDE()->phi_raw() > phiRawImpactCell - m_phiRawMiddleCut)) {
	if ((theCell->caloDDE()->eta_raw() < etaRawImpactCell + m_etaRawMiddleCut) && (theCell->caloDDE()->eta_raw() > etaRawImpactCell - m_etaRawMiddleCut)) {
	  nSqCuts ++;
	  m_windowCells.push_back(theCell);
	}
      }
    }

    else if(sampling == 3){
      if ((theCell->caloDDE()->phi_raw() < phiRawImpactCell + m_phiRawMiddleCut) && (theCell->caloDDE()->phi_raw() > phiRawImpactCell - m_phiRawMiddleCut)) {
	nSqCuts ++;
	m_windowCells.push_back(theCell);
      }

    }
  }

  if (nSqCuts != m_numberOfCellsForDNN){
  	ATH_MSG_WARNING("Total cells passing DNN selection is " << nSqCuts << " but should be " << m_numberOfCellsForDNN );
  	// bail out, but do not stop the job
  	return StatusCode::SUCCESS;

  }

 // sort cells within the cluster like they are fed to DNN
  if (etaRawImpactCell < 0){
    std::sort(m_windowCells.begin(), m_windowCells.end(), &compCellsForDNNSortMirror);
  }
  else{
    std::sort(m_windowCells.begin(), m_windowCells.end(), &compCellsForDNNSort);
  }


  // start neural network part
  // fill a map of input nodes
  // this is for GAN
  // most likely it should be specialised as a function of m_ParamsInputArchitecture

  double riImpactEta;
  double riImpactPhi;
  double randGaussz = 0.;

  int pconf = impactPhiIndex % 4 ;
  int econf = (impactEtaIndex + 1) % 2 ; // ofset corresponds to difference in index calculated for neural net preprocessing
    
  riImpactEta = ((etaExtrap - etaRawImpactCell) - m_riImpactEtaMean)/m_riImpactEtaScale; // ??? or imact - extrap?
  riImpactPhi = ((phiExtrap - phiRawImpactCell) - m_riImpactPhiMean);
  // keep phi in -pi to pi
  if (riImpactPhi > CLHEP::pi){
    riImpactPhi -= 2 * CLHEP::pi;
  }
  else if (riImpactPhi < - CLHEP::pi){
    riImpactPhi += 2 * CLHEP::pi;
  }
  riImpactPhi = riImpactPhi/m_riImpactPhiScale;

  // fill randomize latent space
  //FIXME generate in one go
  for (int i = 0; i< m_GANLatentSize; i ++)
    {
      randGaussz = CLHEP::RandGauss::shoot(simulstate.randomEngine(), 0., 1.);
      inputs["Z"].insert ( std::pair<std::string,double>(std::to_string(i), randGaussz) );

    }

  // fill preprocessed true energy
  //FIXME this is a loop of 1
  for (int i = 0; i< 1; i ++)
    {
      inputs["E_true"].insert ( std::pair<std::string,double>(std::to_string(i), (std::log(trueEnergy) - m_logTrueEnergyMean)/m_logTrueEnergyScale) );
    }
  // fill p,e configurations multi-hot vector
  for (int i = 0; i< 4; i ++)
    {
      if (i == pconf){
	inputs["pconfig"].insert ( std::pair<std::string,double>(std::to_string(i),1.) );
      }
      else{
	inputs["pconfig"].insert ( std::pair<std::string,double>(std::to_string(i),0.) );
      }
    }
  for (int i = 0; i< 2; i ++){
    if (i == econf){
      inputs["econfig"].insert ( std::pair<std::string,double>(std::to_string(i),1.) );
    }
    else{
      inputs["econfig"].insert ( std::pair<std::string,double>(std::to_string(i),0.) );
    }
  }
  // fill position of extrap particle in impact cell
  inputs["ripos"].insert ( std::pair<std::string,double>("0", riImpactEta) ); 
  inputs["ripos"].insert ( std::pair<std::string,double>("1", riImpactPhi ) );

return StatusCode::SUCCESS;
}
