/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"

#include "xAODTrigL1Calo/TriggerTowerContainer.h"

#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerBuilder.h"

#include "L1CaloFEXSim/jSuperCellTowerMapper.h"
#include "L1CaloFEXSim/jTowerMakerFromSuperCells.h"

#include "SGTools/TestStore.h"
#include "PathResolver/PathResolver.h"

#include <fstream>

#define DEBUG_VHB 1


namespace LVL1 {

jTowerMakerFromSuperCells::jTowerMakerFromSuperCells(const std::string& name, ISvcLocator* svc):  AthAlgorithm(name, svc) {}

StatusCode jTowerMakerFromSuperCells::initialize()
{

    ATH_CHECK( m_jTowerBuilderTool.retrieve() );
    ATH_CHECK( m_jSuperCellTowerMapperTool.retrieve() );
    ATH_CHECK( m_jTowerContainerSGKey.initialize() );

    std::unique_ptr<TFile> jTowerFile(TFile::Open(PathResolver::find_calib_file(m_PileupWeigthFile).c_str()));
    std::unique_ptr<TFile> jTowerMapFile(TFile::Open(PathResolver::find_calib_file(m_PileupHelperFile).c_str()));
    if (!jTowerFile || jTowerFile->IsZombie()) {
        ATH_MSG_ERROR("Failed to open cell timing file " << m_PileupWeigthFile);
        return StatusCode::FAILURE;
    }
    if (!jTowerMapFile || jTowerMapFile->IsZombie()) {
        ATH_MSG_ERROR("Failed to open cell timing file " << m_PileupHelperFile);
        return StatusCode::FAILURE;
    }

    m_jTowerArea_hist    = (TH1F*) jTowerFile->Get("jTowerArea_final_hist");
    m_Firmware2BitwiseID = (TH1I*) jTowerMapFile->Get("Firmware2BitwiseID");
    m_BinLayer           = (TH1I*) jTowerMapFile->Get("BinLayer");
    m_EtaCoords          = (TH1F*) jTowerMapFile->Get("EtaCoords");
    m_PhiCoords          = (TH1F*) jTowerMapFile->Get("PhiCoords");

    //detach the Histograms from the TFiles
    m_jTowerArea_hist->SetDirectory(0);
    m_Firmware2BitwiseID->SetDirectory(0);
    m_BinLayer->SetDirectory(0);
    m_EtaCoords->SetDirectory(0);
    m_PhiCoords->SetDirectory(0);

    jTowerFile->Close();
    jTowerMapFile->Close();


    return StatusCode::SUCCESS;

}


StatusCode jTowerMakerFromSuperCells::execute() {


    // STEP 0 - Make a fresh local jTowerContainer
    std::unique_ptr<jTowerContainer> local_jTowerContainerRaw = std::make_unique<jTowerContainer>();

    // STEP 1 - Make some jTowers and fill the local container
    m_jTowerBuilderTool->init(local_jTowerContainerRaw);
    local_jTowerContainerRaw->clearContainerMap();
    local_jTowerContainerRaw->fillContainerMap();

    // STEP 2 - Do the supercell-tower mapping - put this information into the jTowerContainer
    ATH_CHECK(m_jSuperCellTowerMapperTool->AssignSuperCellsToTowers(local_jTowerContainerRaw));
    ATH_CHECK(m_jSuperCellTowerMapperTool->AssignTriggerTowerMapper(local_jTowerContainerRaw));
    ATH_CHECK(m_jSuperCellTowerMapperTool->AssignPileupAndNoiseValues(local_jTowerContainerRaw,m_jTowerArea_hist,m_Firmware2BitwiseID,m_BinLayer,m_EtaCoords,m_PhiCoords));

    //STEP 2.5 - Set up a file mapping if necessary (should only need to be done if the mapping changes, which should never happen unless major changes to the simulation are required)
    // Only used for simulation experts. Contact one of us first
    // With just one event should be enough to generate the file
    if(false) {

        ATH_MSG_INFO("Writting the mapping for jFEX");
        std::ofstream sc_tower_map;
        sc_tower_map.open("./new_jfex_SCID.txt");
        sc_tower_map << "# Simulation ID, 12 Scells (EMB or EMEC or FCAL1 layer) + 1 Scell (HEC or FCAL2/3 layer)" << "\n";

        for(const auto & jtower : *local_jTowerContainerRaw) {
            sc_tower_map << jtower->id() << " ";

            std::vector<Identifier> vEM = jtower->getEMSCIDs();
            for(const auto& SCellID : vEM) {
                sc_tower_map << SCellID << " ";
            }
            for(unsigned int i=0; i<(12-vEM.size()); i++) {
                sc_tower_map << "0xffffffffffffffff" << " ";
            }

            std::vector<Identifier> vHAD = jtower->getHADSCIDs();
            for(const auto& SCellID : vHAD) {
                sc_tower_map << SCellID << " ";
            }
            if(vHAD.size()==0) {
                sc_tower_map << "0xffffffffffffffff" << " ";
            }
            sc_tower_map << "\n";
        }

        sc_tower_map.close();
    }

    // STEP 3 - Write the completed jTowerContainer into StoreGate (move the local copy in memory)
    SG::WriteHandle<LVL1::jTowerContainer> jTowerContainerSG(m_jTowerContainerSGKey /*, ctx*/);
    ATH_CHECK(jTowerContainerSG.record(std::move( local_jTowerContainerRaw ) ) );

    // STEP 4 - Close and clean the event
    m_jSuperCellTowerMapperTool->reset();
    m_jTowerBuilderTool->reset();

    return StatusCode::SUCCESS;
}

} // end of LVL1 namespace
