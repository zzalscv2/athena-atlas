/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFexEmulatedTowers  -  description:
//       This reentrant algorithm is meant to build the jFEX Towers from LAr and Tile containers
//                              -------------------
//     begin                : 01 11 2022
//     email                : sergi.rodriguez@cern.ch
//***************************************************************************/


#ifndef JFEXEMULATEDTOWERS_H
#define JFEXEMULATEDTOWERS_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AsgTools/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "PathResolver/PathResolver.h"

#include "xAODTrigL1Calo/jFexTowerContainer.h"
#include "xAODTrigL1Calo/jFexTowerAuxContainer.h"

#include "CaloEvent/CaloCellContainer.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"



namespace LVL1 {
    
class jFexEmulatedTowers : public AthReentrantAlgorithm{
    public:
        jFexEmulatedTowers(const std::string& name, ISvcLocator* svc);
        
        /// Function initialising the algorithm
        virtual StatusCode initialize() override;
        /// Function executing the algorithm
        virtual StatusCode execute( const EventContext& ) const override;
        
    private:
        // ------------------------- Properties --------------------------------------
        //Readhandle for Scell container
        SG::ReadHandleKey<CaloCellContainer> m_SCellKey {this, "SCell", "SCell", "SCell container"};
        
        //Readhandle for TriggerTower container
        SG::ReadHandleKey<xAOD::TriggerTowerContainer> m_triggerTowerKey {this, "xODTriggerTowers", "xAODTriggerTowers", "xAODTriggerTowers container"};
        
        //Writehhanlde for EmulatedTowers container
        SG::WriteHandleKey < xAOD::jFexTowerContainer > m_jTowersWriteKey    {this,"jTowersWriteKey"   ,"L1_jFexEmulatedTowers", "Write jFexEDM Trigger Tower container"};
        
        // FiberMapping property required by the interface
        Gaudi::Property<std::string> m_FiberMapping {this, "jFexTowerMapping", PathResolver::find_calib_file("L1CaloFEXByteStream/2022-10-19/jFexTowerMap.txt"), "Text file to convert from hardware fiber to eta-phi location"};
        
        //property for jFEX mapping
        Gaudi::Property<bool> m_apply_masking {this, "SCellMasking", true, "Applies masking. Only use for data"};
        
        //property for jFEX mapping
        Gaudi::Property<std::string> m_jFEX2Scellmapping {this, "jFEX2SCmapping"  , PathResolver::find_calib_file("L1CaloFEXByteStream/2022-10-19/jfex_SCID.txt")  , "Text file to convert from simulation ID to SuperCell Identifier"};
        Gaudi::Property<std::string> m_jFEX2Tilemapping  {this, "jFEX2Tilemapping", PathResolver::find_calib_file("L1CaloFEXByteStream/2022-10-19/jfex_TileID.txt"), "Text file to convert from simulation ID to Tile Identifier"};
        
        
        // Read mapping fucntions
        StatusCode ReadFibersfromFile(const std::string& );
        StatusCode ReadSCfromFile    (const std::string& );
        StatusCode ReadTilefromFile  (const std::string& );
        bool isBadSCellID(const std::string&) const;
        
        std::unordered_map< uint32_t, std::vector<uint64_t> > m_map_TTower2SCellsEM;
        std::unordered_map< uint32_t, std::vector<uint64_t> > m_map_TTower2SCellsHAD;
        std::unordered_map< uint32_t, std::tuple<uint32_t,float,float> > m_map_TTower2Tile;

        // hash the index into one integer in the format 0xJFCCT (hexadecimal)
        constexpr static unsigned int mapIndex(unsigned int jfex, unsigned int fpga, unsigned int channel, unsigned int tower);
        std::unordered_map<unsigned int, std::array<float,6> > m_Firm2Tower_map; /// {map index, {IDsimulation,eta,phi,source,iEta,iPhi}}


};
}
#endif
