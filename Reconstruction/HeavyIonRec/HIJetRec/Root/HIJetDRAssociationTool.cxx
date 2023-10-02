/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "HIJetDRAssociationTool.h"
#include <iomanip>

//**********************************************************************

HIJetDRAssociationTool::HIJetDRAssociationTool(const std::string& n)
   :  JetModifierBase(n)
{
}

//**********************************************************************

StatusCode HIJetDRAssociationTool::initialize()
{
   ATH_MSG_VERBOSE("HIJetDRAssociationTool initialize");
   ATH_CHECK( m_containerKey.initialize( !m_containerKey.key().empty() ) );
   return StatusCode::SUCCESS;
}



StatusCode HIJetDRAssociationTool::modify(xAOD::JetContainer& jets) const
{

  const xAOD::IParticleContainer* ppars=0;

  ATH_MSG_DEBUG("Retrieving xAOD container " << m_containerKey.key() );
  SG::ReadHandle<xAOD::IParticleContainer>  readHandlePcontainer ( m_containerKey );
  ppars = readHandlePcontainer.get();

  using Grid=std::vector<std::vector<std::vector<const xAOD::IParticle*>>>;
  const double etaMin = -5;
  const double etaMax = 5;
  const double phiMin = -3.1416;
  const double phiMax = 3.1416;

  // build square grid
  Grid grid;
  grid.resize(1+ (etaMax-etaMin)/m_DR);
  for ( auto& phiVec: grid) {
    phiVec.resize(1+ (phiMax-phiMin)/m_DR);
  }

  auto etaToIndex = [etaMin, this](double eta) -> size_t { return (eta-etaMin)/m_DR; };
  auto phiToIndex = [phiMin, this](double phi) -> size_t { return (phi-phiMin)/m_DR; };
  
  auto neighborsInEta = [this, &grid, etaMin, etaToIndex](double eta) -> std::vector<size_t> {
    const size_t etaIndex = etaToIndex(eta);
    if ( etaIndex == 0){      
      return {etaIndex, etaIndex+1};
    }    
    if ( etaIndex == grid.size()-1){
      return {etaIndex-1, etaIndex};
    }
    return {etaIndex-1, etaIndex, etaIndex+1};
  };

  auto neighborsInPhi = [this, &grid, phiMin, phiToIndex](double phi) -> std::vector<size_t> {
    const size_t phiIndex = phiToIndex(phi);
    if ( phiIndex == 0){      
      return  {grid[0].size()-1, phiIndex, phiIndex+1};
    }    
    if ( phiIndex == grid[0].size()-1){
      return  {phiIndex-1, phiIndex, 0ul}; // wrapping condition
    }
    return  {phiIndex-1, phiIndex, phiIndex+1};
  };

  for(const auto* ap : *ppars) {
    grid[etaToIndex(ap->eta())][phiToIndex(ap->phi())].push_back(ap);
  }

  for (xAOD::JetContainer::iterator ijet=jets.begin(); ijet!=jets.end(); ++ijet)
  {
    std::vector<const xAOD::IParticle*> ParticleVector;
    xAOD::Jet* theJet=(*ijet);
    auto etaN = neighborsInEta(theJet->eta());
    auto phiN = neighborsInPhi(theJet->phi());
     
    for ( size_t etaI :  etaN) { 
      for ( size_t phiI : phiN ) {
        for ( auto ap: grid[etaI][phiI]) {
          if(theJet->p4().DeltaR( ap->p4()) < m_DR) ParticleVector.push_back(ap);
        }
      }
    }
    theJet->setAssociatedObjects(m_assocName,ParticleVector);
  }
  return StatusCode::SUCCESS;
}
