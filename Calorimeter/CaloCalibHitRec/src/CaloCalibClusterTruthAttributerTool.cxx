/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloCalibHitRec/CaloCalibClusterTruthAttributerTool.h"

CaloCalibClusterTruthAttributerTool::CaloCalibClusterTruthAttributerTool(const std::string& type, const std::string& name,  const IInterface* parent) : base_class(type,name,parent) {
}

CaloCalibClusterTruthAttributerTool::~CaloCalibClusterTruthAttributerTool()= default;

StatusCode CaloCalibClusterTruthAttributerTool::calculateTruthEnergies(const xAOD::CaloCluster& theCaloCluster, unsigned int numTruthParticles, const std::map<Identifier,std::vector<const CaloCalibrationHit*> >& identifierToCaloHitMap, std::vector<std::pair<unsigned int, double > >& barcodeTrueCalHitEnergy) const{

  ATH_MSG_DEBUG("In calculateTruthEnergies");

  const CaloClusterCellLink* theCellLinks = theCaloCluster.getCellLinks();

  if (!theCellLinks) {
    ATH_MSG_ERROR("A CaloCluster has no CaloClusterCellLinks");
    return StatusCode::FAILURE;
  }  
  
  std::map<unsigned int, double> barcodeTruePtMap;

  //Loop on calorimeter cells to sum up the truth energies of the truth particles.    
  for (const auto *thisCaloCell : *theCellLinks){
    
    if (!thisCaloCell){
      ATH_MSG_WARNING("Have invalid pointer to CaloCell");
      continue;
    }

    //get the unique calorimeter cell identifier
    Identifier cellID = thisCaloCell->ID();
    
    //look up the calibration hit that corresponds to this calorimeter cell - we use find because not all calorimeter cells will have calibration hits
    std::map<Identifier,std::vector<const CaloCalibrationHit*> >::const_iterator identifierToCaloHitMapIterator = identifierToCaloHitMap.find(cellID);
    if (identifierToCaloHitMap.end() == identifierToCaloHitMapIterator) continue;
    std::vector<const CaloCalibrationHit*> theseCalibrationHits = (*identifierToCaloHitMapIterator).second;

    for (const auto *thisCalibrationHit : theseCalibrationHits){
      unsigned int barcode = thisCalibrationHit->particleID();
      double thisCalHitTruthEnergy = thisCalibrationHit->energyEM() + thisCalibrationHit->energyNonEM();
      if (true == m_fullTruthEnergy) thisCalHitTruthEnergy += (thisCalibrationHit->energyEscaped() + thisCalibrationHit->energyInvisible());

      auto iterator = barcodeTruePtMap.find(barcode);
      if (iterator != barcodeTruePtMap.end()) barcodeTruePtMap[barcode] += thisCalHitTruthEnergy;
      else barcodeTruePtMap[barcode] = thisCalHitTruthEnergy;
      
    }//calibration hit loop
    
  }//loop on calorimeter cells to sum up truth energies

  //now create a vector with the same information as the map, which we can then sort
  std::vector<std::pair<unsigned int, double > > barcodeTruePtPairs;

  barcodeTruePtPairs.reserve(barcodeTruePtMap.size());
  for (const auto& thisEntry : barcodeTruePtMap) barcodeTruePtPairs.emplace_back(thisEntry);

  //sort vector by calibration hit truth energy
  std::sort(barcodeTruePtPairs.begin(),barcodeTruePtPairs.end(),[]( std::pair<unsigned int, double> a, std::pair<unsigned int, double> b) -> bool {return a.second > b.second;} );

  //store the barcode and truth energy of the top numTruthParticles truth particles
  if (numTruthParticles > barcodeTruePtPairs.size()) numTruthParticles = barcodeTruePtPairs.size();
  for ( unsigned int counter = 0; counter < numTruthParticles; counter++) barcodeTrueCalHitEnergy.push_back(barcodeTruePtPairs[counter]);

  for (const auto& thisPair : barcodeTrueCalHitEnergy) ATH_MSG_DEBUG("Truncated loop 2: barcode and true energy are " << thisPair.first << " and " << thisPair.second << " for cluster with e, eta of " << theCaloCluster.e() << " and " << theCaloCluster.eta() );

  return StatusCode::SUCCESS;
  
}
