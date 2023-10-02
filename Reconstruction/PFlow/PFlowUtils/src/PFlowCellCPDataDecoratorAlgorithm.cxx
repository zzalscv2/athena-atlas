/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PFlowCellCPDataDecoratorAlgorithm.h"

#include "CaloEvent/CaloCellContainer.h"
#include "xAODCaloEvent/CaloCluster.h"

#include "vector"

//initialize
StatusCode PFlowCellCPDataDecoratorAlgorithm::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_CHECK(m_cellListWriteDecorHandleKey.initialize());
    ATH_CHECK(m_neutralPFOReadHandleKey.initialize());

    return StatusCode::SUCCESS;   
}

//execute
StatusCode PFlowCellCPDataDecoratorAlgorithm::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing " << name() << "...");

    SG::WriteDecorHandle<xAOD::FlowElementContainer, std::vector<ElementLink<CaloCellContainer> > > cellListWriteDecorHandle(m_cellListWriteDecorHandleKey,ctx);

    SG::ReadHandle<xAOD::FlowElementContainer> neutralPFOReadHandle(m_neutralPFOReadHandleKey,ctx);

    for (auto thisFlowElement : *cellListWriteDecorHandle){
    
        std::vector<std::pair<const xAOD::IParticle*,float> > theOtherPairs_charged = thisFlowElement->otherObjectsAndWeights();
        //list of CaloCell that we will decorate the charged FE object with
        std::vector<ElementLink<CaloCellContainer> > cellsToDecorate;

        for (auto thisPair : theOtherPairs_charged){
            const xAOD::IParticle* theCluster_charged = thisPair.first;

            for (auto thisNeutralFE : *neutralPFOReadHandle){
                std::vector<std::pair<const xAOD::IParticle*,float> > theOtherPairs_neutral = thisNeutralFE->otherObjectsAndWeights();
                if (1 != theOtherPairs_neutral.size()) ATH_MSG_WARNING("Expected neutral FE to have only 1 topocluster link");

                const xAOD::IParticle* theCluster_neutral = theOtherPairs_neutral.at(0).first;

                if (theCluster_charged == theCluster_neutral){
                    //get the list of calorimeter cells in the charged cluster
                    const xAOD::CaloCluster* chargedCluster = dynamic_cast<const xAOD::CaloCluster*>(theCluster_charged);
                    const CaloClusterCellLink* chargedClusterCellLinks = chargedCluster->getCellLinks();
                    CaloClusterCellLink::const_iterator chargedClusterCellLinksItr = chargedClusterCellLinks->begin();

                    //get the list of calorimeter cells in the neutral cluster
                   const xAOD::CaloCluster* neutralCluster = dynamic_cast<const xAOD::CaloCluster*>(theCluster_neutral);
                   const CaloClusterCellLink* neutralClusterCellLinks = neutralCluster->getCellLinks();

                   //loop over the cells in the charged cluster
                   for ( ; chargedClusterCellLinksItr != chargedClusterCellLinks->end(); ++chargedClusterCellLinksItr ) {
                        //get the cell
                        const CaloCell* chargedCell = *chargedClusterCellLinksItr;

                       //loop over the cells in the neutral cluster
                       //flag whether the charged cell is in the neutral cluster
                       bool cellInNeutralCluster = false;      
                       for (auto neutralCell : *neutralClusterCellLinks){          
                            
                            //if the cells are the same, set the flag to true
                            if (chargedCell == neutralCell) cellInNeutralCluster = true;

                        }//loop over the cells in the neutral cluster

                        //if the cell is not in the neutral cluster, store the cell in the list of cells to decorate the neutral FE object with
                        if (!cellInNeutralCluster) cellsToDecorate.push_back(ElementLink<CaloCellContainer>("AllCalo", chargedClusterCellLinksItr.index()));

                    }//loop over the cells in the neutral cluster
                }//if the clusters are the same
            }//loop over the neutral FE objects
        }//loop over the topoclusters linked to the charged FE objects

        cellListWriteDecorHandle(*thisFlowElement) = cellsToDecorate;

    }//loop over the charged FE objects

    return StatusCode::SUCCESS;

}

//finalize
StatusCode PFlowCellCPDataDecoratorAlgorithm::finalize() {
    ATH_MSG_DEBUG("Finalizing " << name() << "...");

    return StatusCode::SUCCESS;
}
