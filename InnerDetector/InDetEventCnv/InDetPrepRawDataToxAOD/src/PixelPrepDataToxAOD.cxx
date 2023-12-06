/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PixelPrepDataToxAOD.cxx
//   Implementation file for class PixelPrepDataToxAOD
///////////////////////////////////////////////////////////////////

#include "PixelPrepDataToxAOD.h"

#include "InDetPrepRawData/PixelClusterContainer.h"

#include "xAODTracking/TrackMeasurementValidation.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"
#include "xAODTracking/TrackMeasurementValidationAuxContainer.h"

#include "Identifier/Identifier.h"
#include "InDetIdentifier/PixelID.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "PixelConditionsData/ChargeCalibParameters.h" //for LegacyFitParameters


#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/SimpleVector.h"
#include "InDetSimEvent/SiHit.h"
#include "InDetSimData/InDetSimDataCollection.h"


#include "TMath.h" 
#include "CLHEP/Geometry/Point3D.h"

#include <map>

#define AUXDATA(OBJ, TYP, NAME) \
  static const SG::AuxElement::Accessor<TYP> acc_##NAME (#NAME);  acc_##NAME(*(OBJ))

namespace {
   unsigned int makeKey(short phi, char eta, char layer) {
      return phi | (eta << 16) |  (layer << 24);
   }
}

/////////////////////////////////////////////////////////////////////
//
//         Constructor with parameters:
//
/////////////////////////////////////////////////////////////////////
PixelPrepDataToxAOD::PixelPrepDataToxAOD(const std::string &name, ISvcLocator *pSvcLocator) :
  AthAlgorithm(name,pSvcLocator),
  m_PixelHelper(nullptr),
  m_useSiHitsGeometryMatching(true),
  m_firstEventWarnings(true),
  m_need_sihits{false}
{ 
  // --- Steering and configuration flags
 
  declareProperty("UseTruthInfo", m_useTruthInfo=false);
  declareProperty("UseSiHitsGeometryMatching", m_useSiHitsGeometryMatching=true);
  declareProperty("WriteSDOs", m_writeSDOs = false);
  declareProperty("WriteSiHits", m_writeSiHits = false);
  declareProperty("WriteNNinformation", m_writeNNinformation = true);
  declareProperty("WriteRDOinformation", m_writeRDOinformation = true);
  declareProperty("WriteExtendedPRDinformation", m_writeExtendedPRDinformation = false);

  // --- Configuration keys
  declareProperty("SiClusterContainer",  m_clustercontainer_key = "PixelClusters");
  declareProperty("MC_SDOs", m_SDOcontainer_key = "PixelSDO_Map");
  declareProperty("MC_Hits", m_sihitContainer_key = "PixelHits");
  declareProperty("PRD_MultiTruth", m_multiTruth_key = "PRD_MultiTruthPixel");
  //Keep this the same as input for now, for consistency with downstream assumptions
  declareProperty("OutputClusterContainer",  m_write_xaod_key = "PixelClusters");

  // --- Services and Tools
  declare(m_write_xaod_key);
  declare(m_write_offsets);

}

/////////////////////////////////////////////////////////////////////
//
//        Initialize method: 
//
/////////////////////////////////////////////////////////////////////
StatusCode PixelPrepDataToxAOD::initialize()
{
  ATH_CHECK( detStore()->retrieve(m_PixelHelper, "PixelID") );

  //make sure we don't write what we don't have
  if (not m_useTruthInfo) {
    m_writeSDOs = false;
    m_writeSiHits = false;
  }

  ATH_CHECK(m_pixelReadout.retrieve());
  ATH_CHECK(m_chargeDataKey.initialize( m_writeRDOinformation));

  ATH_CHECK(m_condDCSStateKey.initialize());
  ATH_CHECK(m_condDCSStatusKey.initialize());
  ATH_CHECK(m_readKeyTemp.initialize());
  ATH_CHECK(m_readKeyHV.initialize());

  ATH_CHECK(m_pixelSummary.retrieve());

  ATH_CHECK(m_lorentzAngleTool.retrieve());

  ATH_CHECK(m_clustercontainer_key.initialize());
  m_need_sihits = (m_writeNNinformation || m_writeSiHits) && m_useTruthInfo;
  ATH_CHECK(m_sihitContainer_key.initialize(m_need_sihits));
  ATH_CHECK(m_SDOcontainer_key.initialize(m_writeSDOs));
  ATH_CHECK(m_multiTruth_key.initialize(m_useTruthInfo));

  ATH_CHECK(m_write_xaod_key.initialize());
  m_write_offsets = m_clustercontainer_key.key() + "Offsets";
  ATH_CHECK(m_write_offsets.initialize());

  ATH_CHECK(m_clusterSplitProbContainer.initialize( !m_clusterSplitProbContainer.key().empty()));

  return StatusCode::SUCCESS;
}

/////////////////////////////////////////////////////////////////////
//
//        Execute method: 
//
/////////////////////////////////////////////////////////////////////
StatusCode PixelPrepDataToxAOD::execute() 
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  //Mandatory. Require if the algorithm is scheduled.
  SG::ReadHandle<InDet::PixelClusterContainer> PixelClusterContainer(m_clustercontainer_key,ctx);
  
  if ( !PixelClusterContainer.isValid() )
  {
      ATH_MSG_ERROR("Failed to retrieve PixelClusterContainer with key" << PixelClusterContainer.key() );
      return StatusCode::FAILURE;
  }

  const PRD_MultiTruthCollection* prdmtColl(nullptr);
  if (m_useTruthInfo) {
     SG::ReadHandle<PRD_MultiTruthCollection> prdmtCollHandle(m_multiTruth_key,ctx);
    if (prdmtCollHandle.isValid()) {
      prdmtColl = &*prdmtCollHandle;
    }
  }

  const InDetSimDataCollection* sdoCollection(nullptr);
  if (m_writeSDOs) {
    SG::ReadHandle<InDetSimDataCollection> sdoCollectionHandle(m_SDOcontainer_key,ctx);
    if (sdoCollectionHandle.isValid()) {
      sdoCollection = &*sdoCollectionHandle;
    } else if (m_firstEventWarnings) {
      ATH_MSG_WARNING("SDO information requested, but SDO collection not available!");
    }
  }

  SG::ReadHandle<Trk::ClusterSplitProbabilityContainer> splitProbContainer;
  if (!m_clusterSplitProbContainer.key().empty()) {
     splitProbContainer=SG::ReadHandle<Trk::ClusterSplitProbabilityContainer>(m_clusterSplitProbContainer, ctx);
     if (!splitProbContainer.isValid()) {
        ATH_MSG_FATAL("Failed to get cluster splitting probability container " << m_clusterSplitProbContainer);
     }
  }

  std::vector<std::vector<const SiHit*>> siHits(m_PixelHelper->wafer_hash_max());
  if (m_need_sihits) {
    SG::ReadHandle<SiHitCollection> siHitCollectionHandle(m_sihitContainer_key, ctx);
    if (siHitCollectionHandle.isValid()) {
      for (const SiHit& siHit: *siHitCollectionHandle) {
        // Check if it is a Pixel hit
        if (!siHit.isPixel()) continue;

        Identifier wafer_id(m_PixelHelper->wafer_id(siHit.getBarrelEndcap(),
                                                    siHit.getLayerDisk(),
                                                    siHit.getPhiModule(),
                                                    siHit.getEtaModule()));
        IdentifierHash wafer_hash(m_PixelHelper->wafer_hash(wafer_id));
        if (wafer_hash>=m_PixelHelper->wafer_hash_max()) continue;
        siHits[wafer_hash].push_back(&siHit);
      }
    } else if (m_firstEventWarnings) {
      ATH_MSG_WARNING("SiHit information requested, but SiHit collection not available!");
    }
  }

  const PixelChargeCalibCondData *calibData=nullptr;
  if (m_writeRDOinformation) {
     SG::ReadCondHandle<PixelChargeCalibCondData> calibData_handle(m_chargeDataKey,ctx);
     if (!calibData_handle.isValid()) {
        ATH_MSG_FATAL("Failed to get PixelChargeCalibCondData with key " << m_chargeDataKey);
     }
     calibData=calibData_handle.cptr();
  }

  // Create the xAOD container and its auxiliary store:
  SG::WriteHandle<xAOD::TrackMeasurementValidationContainer> xaod(m_write_xaod_key,ctx);
  ATH_CHECK(xaod.record(std::make_unique<xAOD::TrackMeasurementValidationContainer>(),
                        std::make_unique<xAOD::TrackMeasurementValidationAuxContainer>()));

  SG::WriteHandle<std::vector<unsigned int>> offsets(m_write_offsets,ctx);
  ATH_CHECK(offsets.record(std::make_unique<std::vector<unsigned int>>(m_PixelHelper->wafer_hash_max(), 0)));
  
  // Loop over the container
  unsigned int counter(0);
 
  SG::ReadCondHandle<PixelDCSStateData> dcsState(m_condDCSStateKey,ctx);
  SG::ReadCondHandle<PixelDCSHVData> dcsHV(m_readKeyHV,ctx);
  SG::ReadCondHandle<PixelDCSTempData> dcsTemp(m_readKeyTemp,ctx);

  std::unordered_map< unsigned int , std::vector<unsigned int> > cluster_map;
  for( const auto clusterCollection : * PixelClusterContainer ){

    //Fill Offset container
    (*offsets)[clusterCollection->identifyHash()] = counter;

    // skip empty collections
    if( clusterCollection->empty() ) continue;
    
    // loop over collection and convert to xAOD
    for( const InDet::PixelCluster* prd : *clusterCollection ){    
      ++counter;

      Identifier clusterId = prd->identify();
      if ( !clusterId.is_valid() ) {
        ATH_MSG_WARNING("Pixel cluster identifier is not valid");
      }
            
      // create and add xAOD object
      xAOD::TrackMeasurementValidation* xprd = new xAOD::TrackMeasurementValidation();
      unsigned int cluster_idx = xaod->size();
      xaod->push_back(xprd);
      
      //Set Identifier
      xprd->setIdentifier( clusterId.get_compact() );

      //Set Global Position
      Amg::Vector3D gpos = prd->globalPosition();
      xprd->setGlobalPosition(gpos.x(),gpos.y(),gpos.z());

      //Set Local Position
      const Amg::Vector2D& locpos = prd->localPosition();

      // Set local error matrix
      xprd->setLocalPosition( locpos.x(),  locpos.y() ); 
      
      const Amg::MatrixX& localCov = prd->localCovariance();
      //std::cout << localCov <<  std::endl;
      if(localCov.size() == 1){
        //std::cout << "Size  == 1" << std::endl;
        xprd->setLocalPositionError( localCov(0,0), 0., 0. ); 
      } else if(localCov.size() == 4){
        //std::cout << "Size  == 2" << std::endl;
        xprd->setLocalPositionError( localCov(0,0), localCov(1,1), localCov(0,1) );     
      } else {
        //std::cout << "Size  == "<< localCov.size() << std::endl;
        xprd->setLocalPositionError(0.,0.,0.);
      }

      // Set vector of hit identifiers
      std::vector< uint64_t > rdoIdentifierList;
      rdoIdentifierList.reserve(prd->rdoList().size());
      int rowmin=9999; int rowmax=-9999;
      int colmin=9999; int colmax=-9999;
      for( const auto &hitIdentifier : prd->rdoList() ){
        rdoIdentifierList.push_back( hitIdentifier.get_compact() );
        //May want to addinformation about the individual hits here
	int row = m_PixelHelper->phi_index(hitIdentifier);
	int col = m_PixelHelper->eta_index(hitIdentifier);
	if(rowmin > row) rowmin = row;
	if(rowmax < row) rowmax = row;
	if(colmin > col) colmin = col;
	if(colmax < col) colmax = col;
      }
      xprd->setRdoIdentifierList(rdoIdentifierList);

      //Add pixel cluster properties
      AUXDATA(xprd,int,bec)          =   m_PixelHelper->barrel_ec(clusterId)   ;
      char the_layer                 =   m_PixelHelper->layer_disk(clusterId)  ;
      char the_eta                   =   m_PixelHelper->eta_module(clusterId)  ;
      short the_phi                  =   m_PixelHelper->phi_module(clusterId)  ;
      AUXDATA(xprd,int,layer)        =   the_layer ;
      AUXDATA(xprd,int,phi_module)   =   the_phi ;
      AUXDATA(xprd,int,eta_module)   =   the_eta ;
      AUXDATA(xprd,int,eta_pixel_index)         =  m_PixelHelper->eta_index(clusterId);
      AUXDATA(xprd,int,phi_pixel_index)         =  m_PixelHelper->phi_index(clusterId);

      cluster_map[ makeKey(the_phi, the_eta, the_layer)].push_back(cluster_idx);

      const InDet::SiWidth cw = prd->width();
      AUXDATA(xprd,int,sizePhi) = (int)cw.colRow()[0];
      AUXDATA(xprd,int,sizeZ)   = (int)cw.colRow()[1];
      AUXDATA(xprd,int,nRDO)    = (int)prd->rdoList().size();
   
      AUXDATA(xprd,float,charge)  =  prd->totalCharge(); 
      AUXDATA(xprd,int,ToT)       =  prd->totalToT(); 
      AUXDATA(xprd,int,LVL1A)     =  prd->LVL1A(); 
   
      AUXDATA(xprd,char,isFake)      =  (char)prd->isFake(); 
      AUXDATA(xprd,char,gangedPixel) =  (char)prd->gangedPixel();
      const Trk::ClusterSplitProbabilityContainer::ProbabilityInfo &
         splitProb = splitProbContainer.isValid() ? splitProbContainer->splitProbability(prd) : Trk::ClusterSplitProbabilityContainer::getNoSplitProbability();
      AUXDATA(xprd,int,isSplit)      =  static_cast<int>(splitProb.isSplit());
      AUXDATA(xprd,float,splitProbability1)  =  splitProb.splitProbability1();
      AUXDATA(xprd,float,splitProbability2)  =  splitProb.splitProbability2();

      // Need to add something to Add the NN splitting information
      if(m_writeNNinformation) addNNInformation( xprd,  prd, 7, 7);
      
      // Add information for each contributing hit
      if(m_writeRDOinformation) {
        IdentifierHash moduleHash = clusterCollection->identifyHash();
        AUXDATA(xprd,int,hasBSError) = (int)m_pixelSummary->hasBSError(moduleHash, ctx);
        AUXDATA(xprd,int,DCSState) = dcsState->getModuleStatus(moduleHash);

        float deplVoltage = 0.0;
        AUXDATA(xprd,float,BiasVoltage) = dcsHV->getBiasVoltage(moduleHash);
        AUXDATA(xprd,float,Temperature) = dcsTemp->getTemperature(moduleHash);
        AUXDATA(xprd,float,DepletionVoltage) = deplVoltage;

        AUXDATA(xprd,float,LorentzShift) = (float)m_lorentzAngleTool->getLorentzShift(moduleHash);

        assert (calibData);
        addRdoInformation(xprd,  prd, calibData);
      } 
  
  
      // Add the Detector element ID  --  not sure if needed as we have the informations above
      const InDetDD::SiDetectorElement* de = prd->detectorElement();
      uint64_t detElementId(0);
      if(de){
        Identifier detId = de->identify();
        if ( detId.is_valid() ) {
          detElementId = detId.get_compact();
        }
      }
      AUXDATA(xprd,uint64_t,detectorElementID) = detElementId;

      if(m_writeExtendedPRDinformation){
	AUXDATA(xprd,int,waferID) = m_PixelHelper->wafer_hash(de->identify());

	const InDetDD::PixelModuleDesign* design = dynamic_cast<const InDetDD::PixelModuleDesign*>(&de->design());
	InDetDD::SiLocalPosition pos1 = design->positionFromColumnRow(colmin,rowmin);
	InDetDD::SiLocalPosition pos2 = design->positionFromColumnRow(colmax,rowmin);
	InDetDD::SiLocalPosition pos3 = design->positionFromColumnRow(colmin,rowmax);
	InDetDD::SiLocalPosition pos4 = design->positionFromColumnRow(colmax,rowmax);
	InDetDD::SiLocalPosition centroid = 0.25*(pos1+pos2+pos3+pos4);

	AUXDATA(xprd,float,centroid_xphi) = centroid.xPhi();
	AUXDATA(xprd,float,centroid_xeta) = centroid.xEta();

	AUXDATA(xprd,float,omegax) = prd->omegax();
	AUXDATA(xprd,float,omegay) = prd->omegay();
      }
      
      // Use the MultiTruth Collection to get a list of all true particle contributing to the cluster
      if (prdmtColl) {
        std::vector<int> barcodes;
        auto range = prdmtColl->equal_range(clusterId);
        for (auto i = range.first; i != range.second; ++i) {
          barcodes.push_back( i->second.barcode() );
        }
        AUXDATA(xprd,std::vector<int>, truth_barcode) = barcodes;
      }
      
      std::vector< std::vector< int > > sdo_tracks;
      // Use the SDO Collection to get a list of all true particle contributing to the cluster per readout element
      //  Also get the energy deposited by each true particle per readout element   
      if (sdoCollection) {
        sdo_tracks = addSDOInformation(xprd, prd, *sdoCollection);
      }
    
      // Now Get the most detailed truth from the SiHits
      // Note that this could get really slow if there are a lot of hits and clusters
      if (m_need_sihits) {
        const std::vector<SiHit> matched_hits = findAllHitsCompatibleWithCluster(prd, &siHits[prd->detectorElement()->identifyHash()], sdo_tracks);
        if (m_writeSiHits) {
          addSiHitInformation(xprd, prd, matched_hits); 
        }
	    
        if (m_writeNNinformation) {
          addNNTruthInfo(xprd, prd, matched_hits);
        }
      }
    }
  }

  for ( auto clusItr = xaod->begin(); clusItr != xaod->end(); ++clusItr ) {
      AUXDATA(*clusItr,char,broken) = false;
  }

  static const SG::AuxElement::Accessor<int> acc_layer ("layer");
  static const SG::AuxElement::Accessor<int> acc_phi_module ("phi_module");
  static const SG::AuxElement::Accessor<int> acc_eta_module ("eta_module");
  static const SG::AuxElement::Accessor<std::vector<int> > acc_sihit_barcode ("sihit_barcode");
  for ( auto clusItr = xaod->begin(); clusItr != xaod->end(); ++clusItr)
  {
      auto pixelCluster = *clusItr;
      int layer = acc_layer(*pixelCluster);
      std::vector<int> barcodes = acc_sihit_barcode(*pixelCluster);

      const std::vector< unsigned int> &cluster_idx_list = cluster_map.at( makeKey(acc_phi_module(*pixelCluster), acc_eta_module(*pixelCluster), acc_layer(*pixelCluster) ));
      for (unsigned int cluster_idx : cluster_idx_list) {
          auto pixelCluster2 = xaod->at(cluster_idx);
	  if ( acc_layer(*pixelCluster2) != layer )
	      continue;
	  if ( acc_eta_module(*pixelCluster) != acc_eta_module(*pixelCluster2) )
	      continue;
	  if ( acc_phi_module(*pixelCluster) != acc_phi_module(*pixelCluster2) )
	      continue;

	  std::vector<int> barcodes2 = acc_sihit_barcode(*pixelCluster2);
	  
	  for ( auto bc : barcodes ) {
              if (std::find(barcodes2.begin(), barcodes2.end(), bc ) == barcodes2.end()) continue;
              static const SG::AuxElement::Accessor<char> acc_broken ("broken");
              acc_broken(*pixelCluster)  = true;
              acc_broken(*pixelCluster2) = true;
              break;
          }
      }
  }

  ATH_MSG_DEBUG( " recorded PixelPrepData objects: size " << xaod->size() );

  m_firstEventWarnings = false;

  return StatusCode::SUCCESS;
}


std::vector< std::vector< int > > PixelPrepDataToxAOD::addSDOInformation( xAOD::TrackMeasurementValidation* xprd,
									  const InDet::PixelCluster* prd,
									  const InDetSimDataCollection& sdoCollection ) const
{
  std::vector<int> sdo_word;
  std::vector< std::vector< int > > sdo_depositsBarcode;
  std::vector< std::vector< float > > sdo_depositsEnergy;
  // find hit
  for( const auto &hitIdentifier : prd->rdoList() ){
    auto pos = sdoCollection.find(hitIdentifier);
    if( pos == sdoCollection.end() ) continue;
    sdo_word.push_back( pos->second.word() ) ;
      
    size_t toreserve = pos->second.getdeposits().size();
    std::vector< int > sdoDepBC; sdoDepBC.reserve(toreserve);
    std::vector< float > sdoDepEnergy; sdoDepEnergy.reserve(toreserve);
    for( const auto& deposit : pos->second.getdeposits() ){
        sdoDepBC.push_back(deposit.first?deposit.first.barcode():-1);
        sdoDepEnergy.push_back( deposit.second  );
        ATH_MSG_DEBUG(" SDO Energy Deposit " << deposit.second  ) ;
    }
    sdo_depositsBarcode.push_back( sdoDepBC );
    sdo_depositsEnergy.push_back( sdoDepEnergy );
  }
  AUXDATA(xprd,std::vector<int>,sdo_words)  = sdo_word;
  AUXDATA(xprd,std::vector< std::vector<int> >,sdo_depositsBarcode)  = sdo_depositsBarcode;
  AUXDATA(xprd,std::vector< std::vector<float> >,sdo_depositsEnergy) = sdo_depositsEnergy;
  
  return sdo_depositsBarcode;
}



void  PixelPrepDataToxAOD::addSiHitInformation( xAOD::TrackMeasurementValidation* xprd, 
						const InDet::PixelCluster* prd,
						const std::vector<SiHit> & matchingHits ) const
{

  int numHits = matchingHits.size();

  std::vector<float> sihit_energyDeposit(numHits,0);
  std::vector<float> sihit_meanTime(numHits,0);
  std::vector<int>   sihit_barcode(numHits,0);
  std::vector<int>   sihit_pdgid(numHits,0);
  
  std::vector<float> sihit_startPosX(numHits,0);
  std::vector<float> sihit_startPosY(numHits,0);
  std::vector<float> sihit_startPosZ(numHits,0);

  std::vector<float> sihit_endPosX(numHits,0);
  std::vector<float> sihit_endPosY(numHits,0);
  std::vector<float> sihit_endPosZ(numHits,0);

  int hitNumber(0);
  const InDetDD::SiDetectorElement* de = prd->detectorElement();
  if(de){
    for ( const auto& sihit : matchingHits ) {          
      sihit_energyDeposit[hitNumber] =  sihit.energyLoss() ;
      sihit_meanTime[hitNumber] =  sihit.meanTime() ;
      const HepMcParticleLink& HMPL = sihit.particleLink();
      sihit_barcode[hitNumber] =  HMPL.barcode() ;
      if(HMPL.isValid()){
        sihit_pdgid[hitNumber]   = HMPL->pdg_id();
      }
    
      // Convert Simulation frame into reco frame
      const HepGeom::Point3D<double>& startPos=sihit.localStartPosition();

      Amg::Vector2D pos= de->hitLocalToLocal( startPos.z(), startPos.y() );
      sihit_startPosX[hitNumber] =  pos[0];
      sihit_startPosY[hitNumber] =  pos[1];
      sihit_startPosZ[hitNumber] =  startPos.x();
 

      const HepGeom::Point3D<double>& endPos=sihit.localEndPosition();
      pos= de->hitLocalToLocal( endPos.z(), endPos.y() );
      sihit_endPosX[hitNumber] =  pos[0];
      sihit_endPosY[hitNumber] =  pos[1];
      sihit_endPosZ[hitNumber] =  endPos.x();
      ++hitNumber;
    }
  }

  AUXDATA(xprd,std::vector<float>,sihit_energyDeposit) = sihit_energyDeposit;
  AUXDATA(xprd,std::vector<float>,sihit_meanTime) = sihit_meanTime;
  AUXDATA(xprd,std::vector<int>,sihit_barcode) = sihit_barcode;
  AUXDATA(xprd,std::vector<int>,sihit_pdgid) = sihit_pdgid;
  
  AUXDATA(xprd,std::vector<float>,sihit_startPosX) = sihit_startPosX;
  AUXDATA(xprd,std::vector<float>,sihit_startPosY) = sihit_startPosY;
  AUXDATA(xprd,std::vector<float>,sihit_startPosZ) = sihit_startPosZ;

  AUXDATA(xprd,std::vector<float>,sihit_endPosX) = sihit_endPosX;
  AUXDATA(xprd,std::vector<float>,sihit_endPosY) = sihit_endPosY;
  AUXDATA(xprd,std::vector<float>,sihit_endPosZ) = sihit_endPosZ;


}






std::vector<SiHit> PixelPrepDataToxAOD::findAllHitsCompatibleWithCluster( const InDet::PixelCluster* prd, 
                                                                          const std::vector<const SiHit*>* sihits,
									  std::vector< std::vector< int > > & trkBCs ) const
{
  ATH_MSG_VERBOSE( "Got " << sihits->size() << " SiHits to look through" );
  std::vector<SiHit>  matchingHits;
    
  // Check if we have detector element  --  needed to find the local position of the SiHits
  const InDetDD::SiDetectorElement* de = prd->detectorElement();
  if(!de)
    return matchingHits;

  std::vector<const SiHit* >  multiMatchingHits;
  
  for ( const SiHit* siHit : *sihits) {
    // Now we have all hits in the module that match lets check to see if they match the cluster
    // Must be within +/- 1 hits of any hit in the cluster to be included
    
    if ( m_useSiHitsGeometryMatching )
    {
	HepGeom::Point3D<double>  averagePosition =  siHit->localStartPosition() + siHit->localEndPosition();
	averagePosition *= 0.5;
	Amg::Vector2D pos = de->hitLocalToLocal( averagePosition.z(), averagePosition.y() );
	InDetDD::SiCellId diode = de->cellIdOfPosition(pos);
	
	for( const auto &hitIdentifier : prd->rdoList() ){
	    ATH_MSG_DEBUG("Truth Phi " <<  diode.phiIndex() << " Cluster Phi " <<   m_PixelHelper->phi_index( hitIdentifier ) );
	    ATH_MSG_DEBUG("Truth Eta " <<  diode.etaIndex() << " Cluster Eta " <<   m_PixelHelper->eta_index( hitIdentifier ) );
	    if( abs( int(diode.etaIndex()) - m_PixelHelper->eta_index( hitIdentifier ) ) <=1  
		&& abs( int(diode.phiIndex()) - m_PixelHelper->phi_index( hitIdentifier ) ) <=1 ) 
	    {
		multiMatchingHits.push_back(siHit);
		break;
	    }
	}
    }
    else
    {
    auto bc = siHit->particleLink().barcode();
    for ( const auto& barcodeSDOColl : trkBCs ) {
           if (std::find(barcodeSDOColl.begin(),barcodeSDOColl.end(),bc) == barcodeSDOColl.end() ) continue;
           multiMatchingHits.push_back(siHit);
           break;   
    }
    }
  }
  //Now we will now make 1 SiHit for each true particle if the SiHits "touch" other 
  std::vector<const SiHit* >::iterator siHitIter  = multiMatchingHits.begin();
  std::vector<const SiHit* >::iterator siHitIter2 = multiMatchingHits.begin();
  ATH_MSG_DEBUG( "Found " << multiMatchingHits.size() << " SiHit " );
  for ( ; siHitIter != multiMatchingHits.end(); ++siHitIter) {
    const SiHit* lowestXPos  = *siHitIter;
    const SiHit* highestXPos = *siHitIter;


    // We will merge these hits
    std::vector<const SiHit* > ajoiningHits;
    ajoiningHits.push_back( *siHitIter );
    
    siHitIter2 = siHitIter+1;    
    auto bc = (*siHitIter)->particleLink().barcode();
    while ( siHitIter2 != multiMatchingHits.end() ) {
      // Need to come from the same truth particle 
            
      if( bc != (*siHitIter2)->particleLink().barcode() ){
        ++siHitIter2;
        continue;
      }
      
      // Check to see if the SiHits are compatible with each other.
      if (std::abs((highestXPos->localEndPosition().x()-(*siHitIter2)->localStartPosition().x()))<0.00005 &&
          std::abs((highestXPos->localEndPosition().y()-(*siHitIter2)->localStartPosition().y()))<0.00005 &&
          std::abs((highestXPos->localEndPosition().z()-(*siHitIter2)->localStartPosition().z()))<0.00005 )
      {
        highestXPos = *siHitIter2;
        ajoiningHits.push_back( *siHitIter2 );
        // Dont use hit  more than once
        // @TODO could invalidate siHitIter
        siHitIter2 = multiMatchingHits.erase( siHitIter2 );
      }else if (std::abs((lowestXPos->localStartPosition().x()-(*siHitIter2)->localEndPosition().x()))<0.00005 &&
                std::abs((lowestXPos->localStartPosition().y()-(*siHitIter2)->localEndPosition().y()))<0.00005 &&
                std::abs((lowestXPos->localStartPosition().z()-(*siHitIter2)->localEndPosition().z()))<0.00005)
      {
        lowestXPos = *siHitIter2;
        ajoiningHits.push_back( *siHitIter2 );
        // Dont use hit  more than once
        // @TODO could invalidate siHitIter
        siHitIter2 = multiMatchingHits.erase( siHitIter2 );
      } else {
        ++siHitIter2;
      }
    }
    
    if( ajoiningHits.size() == 0){
      ATH_MSG_WARNING("This should really never happen");
      continue;
    }else if(ajoiningHits.size() == 1){
      // Copy Si Hit ready to return
      matchingHits.push_back( *ajoiningHits[0] );
      continue;
    } else {
    //  Build new SiHit and merge information together.  
      ATH_MSG_DEBUG("Merging " << ajoiningHits.size() << " SiHits together." );
      
      
      float energyDep(0);
      float time(0);
      for( const auto& siHit :  ajoiningHits){
        energyDep += siHit->energyLoss();
        time += siHit->meanTime();    
      }
      time /= (float)ajoiningHits.size();
       
      matchingHits.emplace_back(lowestXPos->localStartPosition(), 
                                     highestXPos->localEndPosition(),
                                     energyDep,
                                     time,
                                     (*siHitIter)->particleLink().barcode(),
                                     0, // 0 for pixel 1 for Pixel
                                     (*siHitIter)->getBarrelEndcap(),
                                     (*siHitIter)->getLayerDisk(),
                                     (*siHitIter)->getEtaModule(),
                                     (*siHitIter)->getPhiModule(),
                                     (*siHitIter)->getSide() );
     ATH_MSG_DEBUG("Finished Merging " << ajoiningHits.size() << " SiHits together." );

    }
  } 
  

  return matchingHits;
  
}

void PixelPrepDataToxAOD::addRdoInformation(xAOD::TrackMeasurementValidation* xprd, 
                                            const InDet::PixelCluster* pixelCluster,
                                            const PixelChargeCalibCondData *calibData) const
{
  ATH_MSG_VERBOSE( " Starting creating input from cluster "   );


  const std::vector<Identifier>& rdos  = pixelCluster->rdoList();  

  const std::vector<float> &chList     = pixelCluster->chargeList();
  const std::vector<int>  &totList     = pixelCluster->totList();

  // std::vector<int>  rowList;
  // std::vector<int>  colList;
  std::vector<int>  etaIndexList;
  std::vector<int>  phiIndexList;
  std::vector<float> CTerm;
  std::vector<float> ATerm;
  std::vector<float> ETerm;

  ATH_MSG_VERBOSE( "Number of RDOs: " << rdos.size() );
  
  //Itererate over all elements hits in the cluster and fill the charge and tot matricies 
  std::vector<Identifier>::const_iterator rdosBegin = rdos.begin();
  std::vector<Identifier>::const_iterator rdosEnd = rdos.end();

  ATH_MSG_VERBOSE(" Putting together the n. " << rdos.size() << " rdos into a matrix.");

  phiIndexList.reserve( rdos.size());
  etaIndexList.reserve( rdos.size());
  CTerm.reserve( rdos.size());
  ATerm.reserve( rdos.size());
  ETerm.reserve( rdos.size());
  for (; rdosBegin!= rdosEnd; ++rdosBegin)
  {
    Identifier rId =  *rdosBegin;
    phiIndexList.push_back( m_PixelHelper->phi_index(rId) );
    etaIndexList.push_back( m_PixelHelper->eta_index(rId) );  

    // charge calibration parameters
    Identifier moduleID = m_PixelHelper->wafer_id(rId);
    IdentifierHash moduleHash = m_PixelHelper->wafer_hash(moduleID); // wafer hash
    unsigned int FE = m_pixelReadout->getFE(rId, moduleID);
    InDetDD::PixelDiodeType type = m_pixelReadout->getDiodeType(rId);
    const auto & parameters = calibData->getLegacyFitParameters(type, moduleHash, FE);
    CTerm.emplace_back(parameters.C);
    ATerm.emplace_back(parameters.A);
    ETerm.emplace_back(parameters.E);

  }//end iteration on rdos


  AUXDATA(xprd, std::vector<int>,rdo_phi_pixel_index)  = phiIndexList;
  AUXDATA(xprd, std::vector<int>,rdo_eta_pixel_index)  = etaIndexList;
  AUXDATA(xprd, std::vector<float>,rdo_charge)  = chList;
  AUXDATA(xprd, std::vector<int>,rdo_tot)  = totList;
  
  AUXDATA(xprd, std::vector<float>,rdo_Cterm) = CTerm;
  AUXDATA(xprd, std::vector<float>,rdo_Aterm) = ATerm;
  AUXDATA(xprd, std::vector<float>,rdo_Eterm) = ETerm;

}


void PixelPrepDataToxAOD::addNNInformation(xAOD::TrackMeasurementValidation* xprd, 
                                           const InDet::PixelCluster* pixelCluster, 
                                           const unsigned int sizeX, const unsigned int sizeY ) const
{
  ATH_MSG_VERBOSE( " Starting creating input from cluster "   );

  const InDetDD::SiDetectorElement* de = pixelCluster->detectorElement();
  if (de==nullptr) {
    ATH_MSG_ERROR("Could not get detector element");
    return;
  }


  const InDetDD::PixelModuleDesign* design(dynamic_cast<const InDetDD::PixelModuleDesign*>(&de->design()));
	if (not design) {
		ATH_MSG_WARNING("PixelModuleDesign was not retrieved in function 'addNNInformation'");
		return;
	}
  const std::vector<Identifier>& rdos  = pixelCluster->rdoList();  

  const std::vector<float>& chList     = pixelCluster->chargeList();
  const std::vector<int>&  totList     = pixelCluster->totList();

  ATH_MSG_VERBOSE( "Number of RDOs: " << rdos.size() );
  ATH_MSG_VERBOSE( "Number of charges: " << chList.size() );
  ATH_MSG_VERBOSE( "Number of TOT: " << totList.size() );

 
  //Calculate the centre of the cluster
  int phiPixelIndexMin, phiPixelIndexMax, etaPixelIndexMin, etaPixelIndexMax;
  InDetDD::SiCellId cellIdWeightedPosition= getCellIdWeightedPosition( pixelCluster, &phiPixelIndexMin, &phiPixelIndexMax, &etaPixelIndexMin, &etaPixelIndexMax);

  if (!cellIdWeightedPosition.isValid())
  {
    ATH_MSG_WARNING( "Weighted position is on invalid CellID." );
  }

  int etaPixelIndexWeightedPosition=cellIdWeightedPosition.etaIndex();
  int phiPixelIndexWeightedPosition=cellIdWeightedPosition.phiIndex();


  ATH_MSG_DEBUG(" weighted pos phiPixelIndex: " << phiPixelIndexWeightedPosition << " etaPixelIndex: " << etaPixelIndexWeightedPosition ); 

  // SiLocalPosition PixelModuleDesign::positionFromColumnRow(const int column, const int row) const;
  //
  // Given row and column index of diode, returns position of diode center
  // ALTERNATIVE/PREFERED way is to use localPositionOfCell(const SiCellId & cellId) or 
  // rawLocalPositionOfCell method in SiDetectorElement.
  // DEPRECATED (but used in numerous places)
  //
  // Comment by Hide (referring the original comment in the code) : 2015-02-04
  // I automatically replaced column to etaPixelIndex and row to phiPixelIndex here. It was bofore:
  // InDetDD::SiLocalPosition siLocalPosition( design->positionFromColumnRow(columnWeightedPosition,rowWeightedPosition) ); 
  //
  // Then I assume the argument of column/row in this function is in offline manner, not the real hardware column/row.
  // 
  InDetDD::SiLocalPosition w = design->positionFromColumnRow(etaPixelIndexWeightedPosition,phiPixelIndexWeightedPosition);


  double localEtaPixelIndexWeightedPosition = w.xEta();
  double localPhiPixelIndexWeightedPosition    = w.xPhi();

  int centralIndexX=(sizeX-1)/2;
  int centralIndexY=(sizeY-1)/2;



  // Check to see if the cluster is too big for the NN

  if (abs(phiPixelIndexWeightedPosition-phiPixelIndexMin)>centralIndexX ||
      abs(phiPixelIndexWeightedPosition-phiPixelIndexMax)>centralIndexX)
  {
    ATH_MSG_DEBUG(" Cluster too large phiPixelIndexMin " << phiPixelIndexMin << " phiPixelIndexMax " << phiPixelIndexMax << " centralX " << centralIndexX);
    //return;
  }

  if (abs(etaPixelIndexWeightedPosition-etaPixelIndexMin)>centralIndexY ||
      abs(etaPixelIndexWeightedPosition-etaPixelIndexMax)>centralIndexY)
  {
    ATH_MSG_DEBUG(" Cluster too large etaPixelIndexMin" << etaPixelIndexMin << " etaPixelIndexMax " << etaPixelIndexMax << " centralY " << centralIndexY);
    //return;
  }

  std::vector< std::vector<float> > matrixOfToT (sizeX, std::vector<float>(sizeY,0) );
  std::vector< std::vector<float> > matrixOfCharge(sizeX, std::vector<float>(sizeY,0));
  std::vector<float> vectorOfPitchesY(sizeY,0.4);


  //Itererate over all elements hits in the cluster and fill the charge and tot matrices 
  std::vector<Identifier>::const_iterator rdosBegin = rdos.begin();
  std::vector<Identifier>::const_iterator rdosEnd = rdos.end();
  auto charge = chList.begin();    
  auto tot = totList.begin();    

  ATH_MSG_VERBOSE(" Putting together the n. " << rdos.size() << " rdos into a matrix.");

  for (; rdosBegin!= rdosEnd; ++rdosBegin)
  {
    
    Identifier rId =  *rdosBegin;
    int absphiPixelIndex = m_PixelHelper->phi_index(rId)-phiPixelIndexWeightedPosition    + centralIndexX;
    int absetaPixelIndex = m_PixelHelper->eta_index(rId)-etaPixelIndexWeightedPosition + centralIndexY;
    if (charge != chList.end()){
      ATH_MSG_VERBOSE( " Phi Index: " << m_PixelHelper->phi_index(rId) << " absphiPixelIndex: " << absphiPixelIndex << " eta Idx: " << m_PixelHelper->eta_index(rId) << " absetaPixelIndex: " << absetaPixelIndex << " charge " << *charge );
    }
    if (absphiPixelIndex <0 || absphiPixelIndex >= (int)sizeX)
    {
      ATH_MSG_DEBUG(" problem with index: " << absphiPixelIndex << " min: " << 0 << " max: " << sizeX);
      continue;
    }
    
    if (absetaPixelIndex <0 || absetaPixelIndex >= (int)sizeY)
    {
      ATH_MSG_DEBUG(" problem with index: " << absetaPixelIndex << " min: " << 0 << " max: " << sizeY);
      continue;
    }

    InDetDD::SiCellId  cellId = de->cellIdFromIdentifier(*rdosBegin);
    InDetDD::SiDiodesParameters diodeParameters = design->parameters(cellId);
    float pitchY = diodeParameters.width().xEta();
  
    if ( (not totList.empty()) && tot    != totList.end()) {
      matrixOfToT[absphiPixelIndex][absetaPixelIndex]   =*tot;
      ++tot;
    } else matrixOfToT[absphiPixelIndex][absetaPixelIndex]   = -1;

    if ( (not chList.empty()) && charge != chList.end()){
     matrixOfCharge[absphiPixelIndex][absetaPixelIndex]=*charge;
     ++charge;
    } else matrixOfCharge[absphiPixelIndex][absetaPixelIndex] = -1;
  
    if (pitchY > 0.1)
    {
      vectorOfPitchesY[absetaPixelIndex]=pitchY;
    }
  }//end iteration on rdos
  

  ATH_MSG_VERBOSE( " End RDO LOOP " );

  // Using the centre of the module and beam spot calculate
  // the incidence angles of the tracks
  const Amg::Vector2D& prdLocPos = pixelCluster->localPosition();
  InDetDD::SiLocalPosition centroid(prdLocPos);

  Amg::Vector3D globalPos = de->globalPosition(centroid);
  Amg::Vector3D trackDir = globalPos; // - beamSpotPosition; 
  trackDir.normalize();

  Amg::Vector3D module_normal = de->normal();
  Amg::Vector3D module_phiax  = de->phiAxis();
  Amg::Vector3D module_etaax  = de->etaAxis();

  // Calculate the phi incidence angle
  float trkphicomp  = trackDir.dot(module_phiax);
  float trketacomp  = trackDir.dot(module_etaax);
  float trknormcomp = trackDir.dot(module_normal);
  double bowphi     = atan2(trkphicomp,trknormcomp);
  double boweta     = atan2(trketacomp,trknormcomp);
  double tanl = m_lorentzAngleTool->getTanLorentzAngle(de->identifyHash());
  if(bowphi > TMath::Pi()/2) bowphi -= TMath::Pi();
  if(bowphi < -TMath::Pi()/2) bowphi += TMath::Pi();
  int readoutside = design->readoutSide();
  double angle = atan(tan(bowphi)-readoutside*tanl);


  // Calculate the theta incidence angle
  ATH_MSG_VERBOSE( " Angle theta bef corr: " << boweta );
  if (boweta>TMath::Pi()/2.) boweta-=TMath::Pi();
  if (boweta<-TMath::Pi()/2.) boweta+=TMath::Pi();


  ATH_MSG_VERBOSE(" Angle phi: " << angle << " theta: " << boweta );
  ATH_MSG_VERBOSE(" PhiPixelIndexWeightedPosition: " << phiPixelIndexWeightedPosition << " EtaPixelIndexWeightedPosition: " << etaPixelIndexWeightedPosition );

  // store the matrixOfToT in a vector  
  std::vector<float> vectorOfCharge(sizeX*sizeY,0);
  std::vector<float> vectorOfToT(sizeX*sizeY,0);
  int counter(0);
  for (unsigned int u=0;u<sizeX;u++)
  {
    for (unsigned int s=0;s<sizeY;s++)
    {    
      vectorOfToT[counter]    = matrixOfToT[u][s];
      vectorOfCharge[counter] = matrixOfCharge[u][s];
      ++counter;
    }
  }

  ATH_MSG_VERBOSE( "matrixOfToT converted in a std::vector<float>  " );

  ATH_MSG_VERBOSE( "... and saved  " );
  // Add information to xAOD
  AUXDATA(xprd, int, NN_sizeX) = sizeX;
  AUXDATA(xprd, int, NN_sizeY) = sizeY;

  AUXDATA(xprd, float, NN_phiBS) = angle;
  AUXDATA(xprd, float, NN_thetaBS) = boweta;

  AUXDATA(xprd, std::vector<float>, NN_matrixOfToT)      = vectorOfToT;
  AUXDATA(xprd, std::vector<float>, NN_matrixOfCharge)   = vectorOfCharge;
  AUXDATA(xprd, std::vector<float>, NN_vectorOfPitchesY) = vectorOfPitchesY;
  
  
  AUXDATA(xprd, int, NN_etaPixelIndexWeightedPosition) = etaPixelIndexWeightedPosition;
  AUXDATA(xprd, int, NN_phiPixelIndexWeightedPosition) = phiPixelIndexWeightedPosition;

  AUXDATA(xprd, float, NN_localEtaPixelIndexWeightedPosition) = localEtaPixelIndexWeightedPosition;
  AUXDATA(xprd, float, NN_localPhiPixelIndexWeightedPosition) = localPhiPixelIndexWeightedPosition;

  ATH_MSG_VERBOSE( "NN training Written" );
}

void  PixelPrepDataToxAOD::addNNTruthInfo(  xAOD::TrackMeasurementValidation* xprd,
                                            const InDet::PixelCluster* pixelCluster, 
                                            const std::vector<SiHit> & matchingHits ) const
{


  unsigned int numberOfSiHits = matchingHits.size();
  
  std::vector<float> positionsX(numberOfSiHits,0);
  std::vector<float> positionsY(numberOfSiHits,0);

  std::vector<float> positions_indexX(numberOfSiHits,0);
  std::vector<float> positions_indexY(numberOfSiHits,0);

  std::vector<float> theta(numberOfSiHits,0);
  std::vector<float> phi(numberOfSiHits,0);

  std::vector<int>   barcode(numberOfSiHits,0);
  std::vector<int>   pdgid(numberOfSiHits,0);
  std::vector<float> chargeDep(numberOfSiHits,0);
  std::vector<float> truep(numberOfSiHits,0);

  std::vector<float> pathlengthX(numberOfSiHits,0);
  std::vector<float> pathlengthY(numberOfSiHits,0);
  std::vector<float> pathlengthZ(numberOfSiHits,0);

  std::vector<int>   motherBarcode(numberOfSiHits,0);
  std::vector<int>   motherPdgid(numberOfSiHits,0);



  // Check if we have detector element  --  needed to find the local position of the SiHits
  const InDetDD::SiDetectorElement* de = pixelCluster->detectorElement();
  if(!de)
    return;

  InDetDD::SiCellId cellIdWeightedPosition = getCellIdWeightedPosition( pixelCluster );

  const InDetDD::PixelModuleDesign* design(dynamic_cast<const InDetDD::PixelModuleDesign*>(&de->design()));
  if (not design) {
		ATH_MSG_WARNING("PixelModuleDesign was not retrieved in function 'addNNTruthInfo'");
		return;
	}
  // lorentz shift correction    
  double shift = m_lorentzAngleTool->getLorentzShift(de->identifyHash());
  unsigned hitNumber(0);
  for( const auto& siHit : matchingHits ){
    
    HepGeom::Point3D<double> averagePosition = (siHit.localStartPosition() + siHit.localEndPosition()) * 0.5;
    
    ATH_MSG_VERBOSE("Truth Part X: " << averagePosition.y() << " shift " << shift << " Y: " << averagePosition.z() );

    // position lorentz shift corrected
    float YposC = averagePosition.y()-shift;

    if (std::abs(YposC)>design->width()/2 && 
        std::abs(averagePosition.y())<design->width()/2)
   { 
      if (YposC>design->width()/2)
      {
        YposC=design->width()/2-1e-6;
      } else if (YposC<-design->width()/2)
      {
        YposC=-design->width()/2+1e-6;
      }
    }
        
    positionsX[hitNumber] = YposC;
    positionsY[hitNumber] = averagePosition.z();

    HepGeom::Point3D<double> deltaPosition = siHit.localEndPosition() - siHit.localStartPosition();
    
    pathlengthX[hitNumber] = deltaPosition.y();
    pathlengthY[hitNumber] = deltaPosition.z();
    pathlengthZ[hitNumber] = deltaPosition.x();


    // Here we convert the hit position to the right frame
    Amg::Vector2D siLocalTruthPosition = de->hitLocalToLocal(averagePosition.z(), YposC);
    InDetDD::SiCellId cellIdOfTruthPosition = design->cellIdOfPosition(siLocalTruthPosition);

    
//    InDetDD::SiLocalPosition siLocalTruthPosition(averagePosition.z(),YposC ) ;
//    InDetDD::SiCellId cellIdOfTruthPosition =design->cellIdOfPosition(siLocalTruthPosition);

    int truthEtaIndex =  cellIdOfTruthPosition.etaIndex();
    int truthPhiIndex =  cellIdOfTruthPosition.phiIndex();

    InDetDD::SiDiodesParameters diodeParameters = design->parameters(cellIdOfTruthPosition);
    double pitchY = diodeParameters.width().xEta();
    double pitchX = diodeParameters.width().xPhi();

    // pixel center
    // SiLocalPosition PixelModuleDesign::positionFromColumnRow(const int column, const int row) const;
    //
    // Given row and column index of diode, returns position of diode center
    // ALTERNATIVE/PREFERED way is to use localPositionOfCell(const SiCellId & cellId) or 
    // rawLocalPositionOfCell method in SiDetectorElement.
    // DEPRECATED (but used in numerous places)
    //
    // Comment by Hide (referring the original comment in the code) : 2015-02-04
    // I automatically replaced column to etaPixelIndex and row to phiPixelIndex here. It was bofore:
    // InDetDD::SiLocalPosition siLocalPosition( design->positionFromColumnRow(truthColumn,truthRow) ); 
    //
    // Then I assume the argument of column/row in this function is in offline manner, not the real hardware column/row.
    // 
    InDetDD::SiLocalPosition siLocalPositionCenter(design->positionFromColumnRow(truthEtaIndex,truthPhiIndex)); 
    double pixelCenterY = siLocalPositionCenter.xEta();
    double pixelCenterX = siLocalPositionCenter.xPhi();
    

    // truth index
//    double truthIndexY =  truthEtaIndex + (averagePosition.z() - pixelCenterY)/pitchY;
//    double truthIndexX =  truthPhiIndex + (YposC               - pixelCenterX)/pitchX;
    double truthIndexY =  truthEtaIndex + (siLocalTruthPosition[Trk::distEta] - pixelCenterY)/pitchY;
    double truthIndexX =  truthPhiIndex + (siLocalTruthPosition[Trk::distPhi] - pixelCenterX)/pitchX;


    positions_indexX[hitNumber] = truthIndexX - cellIdWeightedPosition.phiIndex();
    positions_indexY[hitNumber] = truthIndexY - cellIdWeightedPosition.etaIndex();

    HepGeom::Point3D<double> diffPositions = (siHit.localEndPosition() - siHit.localStartPosition());
    double bowphi = std::atan2( diffPositions.y(), diffPositions.x() );
   

    //Truth Track incident angle theta
    theta[hitNumber] = std::atan2(diffPositions.z() ,diffPositions.x());
    //Truth track incident angle phi -- correct for lorentz angle
    float tanlorentz = m_lorentzAngleTool->getTanLorentzAngle(de->identifyHash());
  
    int readoutside = design->readoutSide();
    phi[hitNumber] = std::atan(std::tan(bowphi)-readoutside*tanlorentz);
    const HepMcParticleLink& HMPL = siHit.particleLink();
    if (HMPL.isValid()){
      barcode[hitNumber] = HMPL.barcode(); 
      const auto particle = HMPL.cptr();
      pdgid[hitNumber]   = particle->pdg_id();
      HepMC::FourVector mom=particle->momentum();
      truep[hitNumber]  = std::sqrt(mom.x()*mom.x()+mom.y()*mom.y()+mom.z()*mom.z());
      const auto vertex =  particle->production_vertex();
//AV Please note that taking the first particle as a mother is ambiguous.
#ifdef HEPMC3
      if ( vertex && !vertex->particles_in().empty()){
        const auto& mother_of_particle=vertex->particles_in().front();             
        motherBarcode[hitNumber] =  HepMC::barcode(mother_of_particle);
        motherPdgid[hitNumber]    = mother_of_particle->pdg_id();
      }
#else
      if ( vertex ){
        if( vertex->particles_in_const_begin() !=  vertex->particles_in_const_end() ){
          motherBarcode[hitNumber] =  (*vertex->particles_in_const_begin())->barcode();
          motherPdgid[hitNumber]    =  (*vertex->particles_in_const_begin())->pdg_id();
        }
      }
#endif
    }
    chargeDep[hitNumber] = siHit.energyLoss() ;
    
    ++hitNumber;
  }


  AUXDATA(xprd, std::vector<float>, NN_positionsX) = positionsX;
  AUXDATA(xprd, std::vector<float>, NN_positionsY) = positionsY;

  AUXDATA(xprd, std::vector<float>, NN_positions_indexX) = positions_indexX;
  AUXDATA(xprd, std::vector<float>, NN_positions_indexY) = positions_indexY;

  AUXDATA(xprd, std::vector<float>, NN_theta)     = theta;
  AUXDATA(xprd, std::vector<float>, NN_phi)       = phi;

  AUXDATA(xprd, std::vector<int>, NN_barcode)     = barcode;
  AUXDATA(xprd, std::vector<int>, NN_pdgid)       = pdgid;
  AUXDATA(xprd, std::vector<float>, NN_energyDep) = chargeDep;
  AUXDATA(xprd, std::vector<float>, NN_trueP)     = truep;

  AUXDATA(xprd, std::vector<int>, NN_motherBarcode) = motherBarcode;
  AUXDATA(xprd, std::vector<int>, NN_motherPdgid)   = motherPdgid;
 

  AUXDATA(xprd, std::vector<float>, NN_pathlengthX) = pathlengthX;
  AUXDATA(xprd, std::vector<float>, NN_pathlengthY) = pathlengthY;
  AUXDATA(xprd, std::vector<float>, NN_pathlengthZ) = pathlengthZ;


}




InDetDD::SiCellId PixelPrepDataToxAOD::getCellIdWeightedPosition(  const InDet::PixelCluster* pixelCluster,
                                                                   int *rphiPixelIndexMin,
                                                                   int *rphiPixelIndexMax,
                                                                   int *retaPixelIndexMin,
                                                                   int *retaPixelIndexMax   ) const
{

  const InDetDD::SiDetectorElement* de = pixelCluster->detectorElement();
  if (de==nullptr) {
    ATH_MSG_ERROR("Could not get detector element");
    return {};
  }

  const InDetDD::PixelModuleDesign* design(dynamic_cast<const InDetDD::PixelModuleDesign*>(&de->design()));
  if (not design) {
    ATH_MSG_WARNING("PixelModuleDesign was not retrieved in function 'getCellIdWeightedPosition'");
    return {};
  }
  const std::vector<Identifier>& rdos  = pixelCluster->rdoList();  

  ATH_MSG_VERBOSE( "Number of RDOs: " << rdos.size() );
  const std::vector<float>& chList     = pixelCluster->chargeList();

  ATH_MSG_VERBOSE( "Number of charges: " << chList.size() );
  std::vector<Identifier>::const_iterator rdosBegin = rdos.begin();
  std::vector<Identifier>::const_iterator rdosEnd = rdos.end();

  auto charge = chList.begin();    

  InDetDD::SiLocalPosition sumOfWeightedPositions(0,0,0);
  double sumOfCharge=0;

  int phiPixelIndexMin =  99999;
  int phiPixelIndexMax = -99999;
  int etaPixelIndexMin =  99999;
  int etaPixelIndexMax = -99999;

  for (; rdosBegin!= rdosEnd; ++rdosBegin, ++charge)
  {
  
    Identifier rId =  *rdosBegin;
    int phiPixelIndex = m_PixelHelper->phi_index(rId);
    int etaPixelIndex = m_PixelHelper->eta_index(rId);
  
    ATH_MSG_VERBOSE(" Adding pixel phiPixelIndex: " << phiPixelIndex << " etaPixelIndex: " << etaPixelIndex << " charge: " << *charge );
    
    // SiLocalPosition PixelModuleDesign::positionFromColumnRow(const int column, const int row) const;
    //
    // Given row and column index of diode, returns position of diode center
    // ALTERNATIVE/PREFERED way is to use localPositionOfCell(const SiCellId & cellId) or 
    // rawLocalPositionOfCell method in SiDetectorElement.
    // DEPRECATED (but used in numerous places)
    //
    // Comment by Hide (referring the original comment in the code): 2015-02-04
    // I automatically replaced column to etaPixelIndex and row to phiPixelIndex here. It was bofore:
    // InDetDD::SiLocalPosition siLocalPosition( design->positionFromColumnRow(column,row) ); 
    //
    // Then I assume the argument of column/row in this function is in offline manner, not the real hardware column/row.
    // 
    InDetDD::SiLocalPosition siLocalPosition( design->positionFromColumnRow(etaPixelIndex,phiPixelIndex) ); 
    ATH_MSG_VERBOSE ( "Local Position: Row = " << siLocalPosition.xRow() << ", Col = " << siLocalPosition.xColumn() );
  
    sumOfWeightedPositions += (*charge)*siLocalPosition;
    sumOfCharge += (*charge);

    if (phiPixelIndex < phiPixelIndexMin)
      phiPixelIndexMin = phiPixelIndex; 
     
    if (phiPixelIndex > phiPixelIndexMax)
      phiPixelIndexMax = phiPixelIndex;
    
    if (etaPixelIndex < etaPixelIndexMin)
      etaPixelIndexMin = etaPixelIndex;
      
    if (etaPixelIndex > etaPixelIndexMax)
      etaPixelIndexMax = etaPixelIndex;

  }
  sumOfWeightedPositions /= sumOfCharge;

  ATH_MSG_VERBOSE ( "Wighted position: Row = " << sumOfWeightedPositions.xRow() << ", Col = " << sumOfWeightedPositions.xColumn() );

  if(rphiPixelIndexMin) *rphiPixelIndexMin = phiPixelIndexMin;
  if(rphiPixelIndexMax) *rphiPixelIndexMax = phiPixelIndexMax;
  if(retaPixelIndexMin) *retaPixelIndexMin = etaPixelIndexMin;
  if(retaPixelIndexMax) *retaPixelIndexMax = etaPixelIndexMax;

  //what you want to know is simple:
  //just the phiPixelIndex and etaPixelIndex of this average position!

  InDetDD::SiCellId cellIdWeightedPosition=design->cellIdOfPosition(sumOfWeightedPositions);


  return cellIdWeightedPosition;
  
}



/////////////////////////////////////////////////////////////////////
//
//        Finalize method: 
//
/////////////////////////////////////////////////////////////////////
StatusCode PixelPrepDataToxAOD::finalize()
{
  return StatusCode::SUCCESS;
}
