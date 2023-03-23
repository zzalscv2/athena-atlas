/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JTOWERMAKERFROMSUPERCELLS_H
#define JTOWERMAKERFROMSUPERCELLS_H

// STL
#include <string>


// Athena/Gaudi
#include "StoreGate/WriteHandle.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloEvent/CaloCellContainer.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "L1CaloFEXSim/jTowerBuilder.h"
#include "L1CaloFEXSim/jSuperCellTowerMapper.h"

#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"

#include "TH1F.h"
#include "TH1I.h"
#include "TFile.h"

class CaloIdManager;

namespace LVL1 {

class jTowerMakerFromSuperCells : public AthAlgorithm
{
    public:

        jTowerMakerFromSuperCells(const std::string& name, ISvcLocator* svc);

        virtual StatusCode initialize();
        virtual StatusCode execute();

    private:

        SG::WriteHandleKey<LVL1::jTowerContainer> m_jTowerContainerSGKey {this, "MyJTowers", "jTowerContainer", "MyJTowers"};

        ToolHandle<IjTowerBuilder> m_jTowerBuilderTool {this, "jTowerBuilderTool", "LVL1::jTowerBuilder", "Tool that builds jTowers for simulation"};
        ToolHandle<IjSuperCellTowerMapper> m_jSuperCellTowerMapperTool {this, "jSuperCellTowerMapperTool", "LVL1::jSuperCellTowerMapper", "Tool that maps supercells to jTowers"};

        //property for jFEX mapping
        Gaudi::Property<std::string> m_PileupWeigthFile {this, "PileupWeigthFile", "Run3L1CaloSimulation/Noise/jTowerCorrection.20210308.r12406.root", "Root file for the pileup weight"};
        Gaudi::Property<std::string> m_PileupHelperFile {this, "PileupHelperFile", "Run3L1CaloSimulation/Calibrations/jFEX_MatchedMapping.2022Mar10.r12406.root", "Root file to set the jTower coordinated (float eta/phi)"};
        
        //histograms need to set coordinates and noise subtraction
        TH1F* m_jTowerArea_hist;
        TH1I* m_Firmware2BitwiseID;
        TH1I* m_BinLayer;
        TH1F* m_EtaCoords;
        TH1F* m_PhiCoords;


};

} // end of LVL1 namespace
#endif
