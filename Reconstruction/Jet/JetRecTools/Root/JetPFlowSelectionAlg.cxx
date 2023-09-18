/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "JetRecTools/JetPFlowSelectionAlg.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/WriteHandle.h"
#include "AsgDataHandles/ReadDecorHandle.h"

#include "xAODEgamma/Electron.h"
#include "xAODMuon/Muon.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODPFlow/FlowElementAuxContainer.h"
#include "xAODPFlow/FEHelpers.h"

StatusCode JetPFlowSelectionAlg::initialize() {
  ATH_MSG_DEBUG("Initializing  " );

  ATH_CHECK(m_ChargedPFlowContainerKey.initialize());
  ATH_CHECK(m_NeutralPFlowContainerKey.initialize());
  ATH_CHECK(m_outputChargedPFlowHandleKey.initialize());
  ATH_CHECK(m_outputNeutralPFlowHandleKey.initialize());
  ATH_CHECK(m_chargedFEElectronsReadDecorKey.initialize());
  ATH_CHECK(m_chargedFEMuonsReadDecorKey.initialize());

  // it is enough to initialise the keys, ReadDecorHandles won't be needed
  ATH_CHECK(m_chargedFETausReadDecorKey.initialize());
  ATH_CHECK(m_neutralFETausReadDecorKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode JetPFlowSelectionAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG(" execute() ... ");

  SG::ReadHandle<xAOD::FlowElementContainer> ChargedPFlowObjects(m_ChargedPFlowContainerKey, ctx);
  if (! ChargedPFlowObjects.isValid()){
    ATH_MSG_ERROR("Can't retrieve input container "<< m_ChargedPFlowContainerKey);                  
    return StatusCode::FAILURE;
  }

  SG::ReadHandle<xAOD::FlowElementContainer> NeutralPFlowObjects(m_NeutralPFlowContainerKey, ctx);
  if (! NeutralPFlowObjects.isValid()){
    ATH_MSG_ERROR("Can't retrieve input container "<< m_NeutralPFlowContainerKey);                  
    return StatusCode::FAILURE;
  }

  SG::ReadDecorHandle<xAOD::FlowElementContainer, std::vector< ElementLink<xAOD::ElectronContainer> > > chargedFE_ElectronLinks(m_chargedFEElectronsReadDecorKey,ctx);
  if (!chargedFE_ElectronLinks.isValid()){
    ATH_MSG_ERROR("Can't retrieve input decoration " << chargedFE_ElectronLinks.key());                  
    return StatusCode::FAILURE;
  }

  SG::ReadDecorHandle<xAOD::FlowElementContainer, std::vector< ElementLink<xAOD::MuonContainer> > > chargedFE_MuonLinks(m_chargedFEMuonsReadDecorKey,ctx);
  if (!chargedFE_MuonLinks.isValid()){
    ATH_MSG_ERROR("Can't retrieve input decoration "<< chargedFE_MuonLinks.key());                  
    return StatusCode::FAILURE;
  }

  auto selectedChargedPFlowObjects = std::make_unique<xAOD::FlowElementContainer>(); // SG::VIEW_ELEMENTS
  auto selectedChargedPFlowObjectsAux = std::make_unique<xAOD::FlowElementAuxContainer>();
  selectedChargedPFlowObjects->setStore(selectedChargedPFlowObjectsAux.get());

  auto selectedNeutralPFlowObjects = std::make_unique<xAOD::FlowElementContainer>();
  auto selectedNeutralPFlowObjectsAux = std::make_unique<xAOD::FlowElementAuxContainer>();
  selectedNeutralPFlowObjects->setStore(selectedNeutralPFlowObjectsAux.get());

  // To store the charged FE objects matched to an electron/muon
  std::vector< const xAOD::FlowElement* > ChargedPFlowObjects_matched;

  // Loop over Charged FE objects
  for ( const xAOD::FlowElement* fe : *ChargedPFlowObjects ) {

    // Select FE object if not matched to an electron or muon via links
    if ( !checkLeptonLinks(chargedFE_ElectronLinks(*fe), chargedFE_MuonLinks(*fe)) ){
      xAOD::FlowElement* selectedFE = new xAOD::FlowElement();
      selectedChargedPFlowObjects->push_back(selectedFE);
      *selectedFE = *fe; // copies auxdata
    }
    else { // Use the matched object to put back its energy later
      ChargedPFlowObjects_matched.push_back(fe);
    }

  } // End loop over Charged FE Objects


  // Loop over Neutral FE objects
  for ( const xAOD::FlowElement* fe : *NeutralPFlowObjects ) {

    xAOD::FlowElement* selectedFE = new xAOD::FlowElement();
    selectedNeutralPFlowObjects->push_back(selectedFE);
    *selectedFE = *fe;

  } // End loop over Neutral FE Objects


  // Add the energy from removed charged FE clusters to neutral FE object 
  // if shared clusters exist, create the new neutral FE object otherwise
  for ( const xAOD::FlowElement* chargedFE : ChargedPFlowObjects_matched ){
      
    // Get charged FE topoclusters and weights
    std::vector<std::pair<const xAOD::IParticle*,float> > theOtherPairs_charged = chargedFE->otherObjectsAndWeights();
    std::vector<ElementLink<xAOD::IParticleContainer>> theOtherLinks_charged = chargedFE->otherObjectLinks();

    // Loop over charged FE topoclusters
    for (unsigned int iCluster = 0; iCluster < chargedFE->nOtherObjects(); ++iCluster){

      bool thisCluster_matched = false;

      std::pair<const xAOD::IParticle*,float> theOtherPair_charged = theOtherPairs_charged[iCluster];
      const xAOD::IParticle* theCluster_charged = theOtherPair_charged.first;
      float theClusterWeight_charged = theOtherPair_charged.second;

      // Loop over neutral FE objects
      for ( xAOD::FlowElement* neutralFE : *selectedNeutralPFlowObjects ) {
        if (thisCluster_matched) continue;

        // Loop over neutral FE topoclusters
        std::vector<std::pair<const xAOD::IParticle*,float> > theOtherPairs_neutral = neutralFE->otherObjectsAndWeights();
        for (auto& [theCluster_neutral, theClusterWeight_neutral] : theOtherPairs_neutral){

          // If topoclusters are matched, add the energy to the neutral FE object
          if (theCluster_charged == theCluster_neutral){

            // Add the energy to the neutral FE object
            float newEnergy = neutralFE->e() + theClusterWeight_charged;
            neutralFE->setP4(newEnergy/cosh(neutralFE->eta()), 
                            neutralFE->eta(),
                            neutralFE->phi(),
                            neutralFE->m());

            if (m_addCPData){
              //compare the list of calorimeter cells in the calorimeter cluster linked to the neutral FE object
              //to the list of calorimeter cells in the calorimeter clusters linked to the charged FE object

              //get the list of calorimeter cells in the charged cluster
              const xAOD::CaloCluster* chargedCluster = dynamic_cast<const xAOD::CaloCluster*>(theCluster_charged);
              const CaloClusterCellLink* chargedClusterCellLinks = chargedCluster->getCellLinks();
              CaloClusterCellLink::const_iterator chargedClusterCellLinksItr = chargedClusterCellLinks->begin();

              //get the list of calorimeter cells in the neutral cluster
              const xAOD::CaloCluster* neutralCluster = dynamic_cast<const xAOD::CaloCluster*>(theCluster_neutral);
              const CaloClusterCellLink* neutralClusterCellLinks = neutralCluster->getCellLinks();
              CaloClusterCellLink::const_iterator neutralClusterCellLinksItr = neutralClusterCellLinks->begin();

              //list of CaloCell that we will decorate the neutral FE object with
              std::vector<ElementLink<CaloCellContainer> > cellsToDecorate;

              //loop over the cells in the charged cluster
              for ( ; chargedClusterCellLinksItr != chargedClusterCellLinks->end(); ++chargedClusterCellLinksItr ) {
                //get the cell
                const CaloCell* chargedCell = *chargedClusterCellLinksItr;

                //loop over the cells in the neutral cluster
                //flag whether the charged cell is in the neutral cluster
                bool cellInNeutralCluster = false;                  
                for ( ; neutralClusterCellLinksItr != neutralClusterCellLinks->end(); ++neutralClusterCellLinksItr ) {
                  //get the cell
                  const CaloCell* neutralCell = *neutralClusterCellLinksItr;

                  //if the cells are the same, set the flag to true
                  if (chargedCell == neutralCell) cellInNeutralCluster = true;

                }//loop over the cells in the neutral cluster

                //if the cell is not in the neutral cluster, store the cell in the list of cells to decorate the neutral FE object with
                if (!cellInNeutralCluster) cellsToDecorate.push_back(ElementLink<CaloCellContainer>("AllCalo", chargedClusterCellLinksItr.index()));
              }//loop over the cells in the charged cluster

              SG::AuxElement::Decorator< std::vector<ElementLink<CaloCellContainer> > > dec_cellsToDecorate("cellsRemovedFromNeutralFE");
              dec_cellsToDecorate(*neutralFE) = cellsToDecorate;

            }//if we are adding CP data to the neutral FE object

            ATH_MSG_DEBUG("Updated neutral FlowElement with E, pt, eta and phi: "
                    << neutralFE->e() << ", " << neutralFE->pt() << ", "
                    << neutralFE->eta() << " and " << neutralFE->phi());

            thisCluster_matched = true;
          }

        } // End loop over neutral FE clusters
      } // End loop over neutral FE objects

      // If a topocluster is left unmatched, create a neutral FE object.
      // Ignore topoclusters with nullptr
      if ( !thisCluster_matched && theCluster_charged ){

        xAOD::FlowElement* newFE = new xAOD::FlowElement();
        selectedNeutralPFlowObjects->push_back(newFE);

        newFE->setP4(theClusterWeight_charged / cosh(theCluster_charged->eta()),  // using energy from charged FE weight, not cluster->e()
                    theCluster_charged->eta(),
                    theCluster_charged->phi(),
                    theCluster_charged->m());
        newFE->setCharge(0);
        newFE->setSignalType(xAOD::FlowElement::SignalType::NeutralPFlow);

        ATH_MSG_DEBUG("Created neutral FlowElement with E, pt, eta and phi: "
                  << newFE->e() << ", " << newFE->pt() << ", "
                  << newFE->eta() << " and " << newFE->phi());

        std::vector<ElementLink<xAOD::IParticleContainer>> theClusters;
        ElementLink< xAOD::IParticleContainer > theIParticleLink;
        theIParticleLink.resetWithKeyAndIndex(theOtherLinks_charged[iCluster].persKey(), theOtherLinks_charged[iCluster].persIndex()); 

        theClusters.push_back(theIParticleLink);
        newFE->setOtherObjectLinks(theClusters);

        //Add Standard data to these new FlowElements
        FEHelpers::FillNeutralFlowElements FEFiller;
        const xAOD::CaloCluster* castCluster_charged = dynamic_cast<const xAOD::CaloCluster*>(theCluster_charged);
        FEFiller.addStandardMoments(*newFE,*castCluster_charged);        
        FEFiller.addStandardSamplingEnergies(*newFE,*castCluster_charged);    

        float layerEnergy_TileBar0 = castCluster_charged->eSample(xAOD::CaloCluster::CaloSample::TileBar0);
        float layerEnergy_TileExt0 = castCluster_charged->eSample(xAOD::CaloCluster::CaloSample::TileExt0);
        const static SG::AuxElement::Accessor<float> accFloatTIle0E("LAYERENERGY_TILE0");
        accFloatTIle0E(*newFE) = layerEnergy_TileBar0 + layerEnergy_TileExt0;

        const static SG::AuxElement::Accessor<float> accFloatTiming("TIMING");
        accFloatTiming(*newFE) = castCluster_charged->time();
      }
        
    } // End loop over topoclusters of removed charged FE objects
  } // End loop over removed charged FE objects


  auto handle_ChargedPFlow_out =  SG::makeHandle(m_outputChargedPFlowHandleKey, ctx);
  if (!handle_ChargedPFlow_out.record(std::move(selectedChargedPFlowObjects), std::move(selectedChargedPFlowObjectsAux)) ){
    ATH_MSG_ERROR("Can't record output PFlow container "<< m_outputChargedPFlowHandleKey);
    return StatusCode::FAILURE;
  }

  auto handle_NeutralPFlow_out =  SG::makeHandle(m_outputNeutralPFlowHandleKey, ctx);
  if (!handle_NeutralPFlow_out.record(std::move(selectedNeutralPFlowObjects), std::move(selectedNeutralPFlowObjectsAux)) ){
    ATH_MSG_ERROR("Can't record output PFlow container "<< m_outputNeutralPFlowHandleKey);
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

bool JetPFlowSelectionAlg::checkLeptonLinks(const std::vector < ElementLink< xAOD::ElectronContainer > >& chargedFE_ElectronLinks,
					    const std::vector < ElementLink< xAOD::MuonContainer > >& chargedFE_MuonLinks) const {

  // Links to electrons
  for (const ElementLink<xAOD::ElectronContainer>& ElectronLink: chargedFE_ElectronLinks){
    if (!ElectronLink.isValid()){
      ATH_MSG_WARNING("JetPFlowSelectionAlg encountered an invalid electron element link. Skipping. ");
      continue; 
    }

    const xAOD::Electron* electron = *ElectronLink;
    bool passElectronID = false;
    bool gotID = electron->passSelection(passElectronID, m_electronID);
    if (!gotID) {
      ATH_MSG_WARNING("Could not get Electron ID");
      continue;
    }
    
    if( electron->pt() > 10000 && passElectronID){
      return true;
    }
  }
  
  // Links to muons
  for (const ElementLink<xAOD::MuonContainer>& MuonLink: chargedFE_MuonLinks){
    if (!MuonLink.isValid()){
      ATH_MSG_WARNING("JetPFlowSelectionAlg encountered an invalid muon element link. Skipping. ");
      continue; 
    }
    
    //Details of medium muons are here:
    //https://twiki.cern.ch/twiki/bin/view/Atlas/MuonSelectionTool
    const xAOD::Muon* muon = *MuonLink;
    if ( muon->quality() <= xAOD::Muon::Medium && muon->muonType() == xAOD::Muon::Combined ){
      return true;
    }    
  }

  return false;
}
