/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFexTower2SCellDecorator  -  description:
//       This reentrant algorithm is meant to decorate the FEX Towers (input data and simulation) with the corresponding matching set of SuperCell from LAr
//                              -------------------
//     begin                : 01 09 2022
//     email                : sergi.rodriguez@cern.ch
//***************************************************************************/
// EXAMPLE https://gitlab.cern.ch/atlas/athena/-/blob/release/22.0.91/Trigger/TrigAlgorithms/TrigLongLivedParticles/src/MuonCluster.h

#ifndef L1CALO2SCELLDECORATORTOOL_H
#define L1CALO2SCELLDECORATORTOOL_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AsgTools/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
#include "PathResolver/PathResolver.h"

#include "xAODTrigL1Calo/jFexTowerContainer.h"
#include "xAODTrigL1Calo/jFexTowerAuxContainer.h"

#include "CaloEvent/CaloCellContainer.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"



namespace LVL1 {
    
class jFexTower2SCellDecorator : public AthReentrantAlgorithm{
    public:
        jFexTower2SCellDecorator(const std::string& name, ISvcLocator* svc);
        
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
        
        const std::string m_ReadKey_name = "L1_jFexDataTowers";
        
        //Readhandle for Scell container
        SG::ReadHandleKey < xAOD::jFexTowerContainer > m_jTowersReadKey    {this,"jTowersReadKey"   ,m_ReadKey_name, "Read jFexEDM Trigger Tower container"};
        
        //WriteDecorHandle
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_SCellEtdecorKey    { this, "SCellEtdecorKey"    , m_ReadKey_name+".SCellEt"    , "SCell Et information of the jTower in MEV"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_SCellEtadecorKey   { this, "SCellEtadecorKey"   , m_ReadKey_name+".SCellEta"   , "SCell Eta information of the jTower"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_SCellPhidecorKey   { this, "SCellPhidecorKey"   , m_ReadKey_name+".SCellPhi"   , "SCell Phi information of the jTower"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_SCellIDdecorKey    { this, "SCellIDdecorKey"    , m_ReadKey_name+".SCellID"    , "SCell IDs information of the jTower"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_TileEtdecorKey     { this, "TileEtdecorKey"     , m_ReadKey_name+".TileEt"     , "Tile Tower Et information in Encoded from cpET"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_TileEtadecorKey    { this, "TileEtadecorKey"    , m_ReadKey_name+".TileEta"    , "Tile Tower Eta information in MeV"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_TilePhidecorKey    { this, "TilePhidecorKey"    , m_ReadKey_name+".TilePhi"    , "Tile Tower Phi information in MeV"};    
        
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_jtowerEtMeVdecorKey{ this, "jtowerEtMeVdecorKey", m_ReadKey_name+".jtowerEtMeV"       , "jFex Tower Et information in MeV"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_SCellEtMeVdecorKey { this, "SCellEtMeVdecorKey" , m_ReadKey_name+".SCellEtMeV"        , "SCell Et sum information in MeV"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_TileEtMeVdecorKey  { this, "TileEtMeVdecorKey"  , m_ReadKey_name+".TileEtMeV"         , "Tile Et information in MeV"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_jTowerEtdecorKey   { this, "jTowerEtdecorKey"   , m_ReadKey_name+".emulated_jtowerEt" , "jFex Tower Et information. ENCODED!"};

        
        //property for Masking
        Gaudi::Property<bool> m_apply_masking {this, "SCellMasking", true , "Applies masking. Only use for data"};
        Gaudi::Property<bool> m_save_extras   {this, "ExtraInfo"   , false, "Saves additional decorated information "};
        
        //property for jFEX mapping
        Gaudi::Property<std::string> m_jFEX2Scellmapping {this, "jFEX2SCmapping"  , PathResolver::find_calib_file("L1CaloFEXByteStream/2022-10-19/jfex_SCID.txt")  , "Text file to convert from simulation ID to SuperCell Identifier"};
        Gaudi::Property<std::string> m_jFEX2Tilemapping  {this, "jFEX2Tilemapping", PathResolver::find_calib_file("L1CaloFEXByteStream/2022-10-19/jfex_TileID.txt"), "Text file to convert from simulation ID to Tile Identifier"};
        
        StatusCode ReadSCfromFile(const std::string& );
        StatusCode ReadTilefromFile(const std::string& );
        bool isBadSCellID(const std::string&) const;
        
        bool m_save_emulated_var = true;
        
        std::unordered_map< uint32_t, std::vector<uint64_t> > m_map_TTower2SCellsEM;
        std::unordered_map< uint32_t, std::vector<uint64_t> > m_map_TTower2SCellsHAD;
        std::unordered_map< uint32_t, std::tuple<uint32_t,float,float> > m_map_TTower2Tile;

};
}
#endif
