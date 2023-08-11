/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef gTowerMakerFromGfexTowers_H
#define gTowerMakerFromGfexTowers_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "L1CaloFEXSim/gTowerBuilder.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "xAODTrigL1Calo/gFexTowerContainer.h"
#include "L1CaloFEXSim/gSuperCellTowerMapper.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

class CaloIdManager;

namespace LVL1 {

class gTowerMakerFromGfexTowers : public AthAlgorithm
{
    public:

        gTowerMakerFromGfexTowers(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~gTowerMakerFromGfexTowers() = default;

        virtual StatusCode initialize() override;
        virtual StatusCode execute() override;

    private:
        
        // Decoded input data
        SG::ReadHandleKey<xAOD::gFexTowerContainer> m_gDataTowerKey {this, "InputDataTowers", "L1_gFexDataTowers", "gfexTowers (use L1_gFexEmulatedTowers for built from SC, or L1_gFexDataTowers for efex readout"};
        
        //Emulated input data
        // the use-case for this second input tower collection is in monitoring when running in prescaled readout mode
        // we want to use the input data readout when we have it, but otherwise fallback to the calo readout
        SG::ReadHandleKey<xAOD::gFexTowerContainer> m_gEmulTowerKey {this, "InputEmulatedTowers", "L1_gFexEmulatedTowers", "If specified, will fallback to this collection of towers if the first collection is incomplete/empty"};
        
        //Gaudi properties
        Gaudi::Property<bool> m_UseEmulated {this, "UseEmulated", false, "It switches off the CaloCell -> Jtower path. It uses instead L1_gFexDataTowers and L1_gFexEmulatedTowers "};
        Gaudi::Property<bool> m_isMC {this, "IsMC", false, "Is used to know when we run on data. So L1_gFexDataTowers can be present"};

        
        // SG object for the gFEX simulation input
        SG::WriteHandleKey<LVL1::gTowerContainer> m_gTowerContainerSGKey {this, "MyGTowers", "gTowerContainer", "MyGTowers"};
        
        ToolHandle<IgTowerBuilder> m_gTowerBuilderTool {this, "gTowerBuilderTool", "LVL1::gTowerBuilder", "Tool that builds jTowers for simulation"};
        ToolHandle<IgSuperCellTowerMapper> m_gSuperCellTowerMapperTool {this, "gSuperCellTowerMapperTool", "LVL1::gSuperCellTowerMapper", "Tool that maps supercells to gTowers"};
};

} // end of LVL1 namespace
#endif
