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
        
        //Readhandle for Scell container
        SG::ReadHandleKey < xAOD::jFexTowerContainer > m_jTowersReadKey    {this,"jTowersReadKey"   ,"L1_jFexDataTowers", "Read jFexEDM Trigger Tower container"};
        
        //WriteDecorHandle
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_SCellEtdecorKey    { this, "SCellEtdecorKey"    , "L1_jTowers.SCellEt"    , "SCell Et information of the jTower"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_SCellEtadecorKey   { this, "SCellEtadecorKey"   , "L1_jTowers.SCellEta"   , "SCell Eta information of the jTower"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_SCellPhidecorKey   { this, "SCellPhidecorKey"   , "L1_jTowers.SCellPhi"   , "SCell Phi information of the jTower"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_SCellIDdecorKey    { this, "SCellIDdecorKey"    , "L1_jTowers.SCellID"    , "SCell IDs information of the jTower"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_jtowerEtMeVdecorKey{ this, "jtowerEtMeVdecorKey", "L1_jTowers.jtowerEtMeV", "jFex Tower Et information in MeV"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_TileEtMeVdecorKey  { this, "TileEtMeVdecorKey"  , "L1_jTowers.TileEtMeV"  , "Tile Tower Et information in MeV"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_TileEtadecorKey    { this, "TileEtadecorKey"    , "L1_jTowers.TileEta"    , "Tile Tower Eta information in MeV"};
        SG::WriteDecorHandleKey<xAOD::jFexTowerContainer> m_TilePhidecorKey    { this, "TilePhidecorKey"    , "L1_jTowers.TilePhi"    , "Tile Tower Phi information in MeV"};

        
        //property for jFEX mapping
        Gaudi::Property<std::string> m_jFEX2Scellmapping {this, "jFEX2SCmapping", "/afs/cern.ch/user/s/serodrig/public/L1CaloScellMapping/jfex_SCID.txt", "Text file to convert from simulation ID to SuperCell Identifier"};
        Gaudi::Property<std::string> m_jFEX2Tilemapping {this, "jFEX2Tilemapping", "/afs/cern.ch/user/s/serodrig/public/L1CaloScellMapping/jfex_TileID.txt", "Text file to convert from simulation ID to Tile Identifier"};
        
        StatusCode ReadSCfromFile(const std::string& );
        StatusCode ReadTilefromFile(const std::string& );
        bool isBadSCellID(const std::string&) const;
        
        std::unordered_map< uint32_t, std::vector<uint64_t> > m_map_TTower2SCellsEM;
        std::unordered_map< uint32_t, std::vector<uint64_t> > m_map_TTower2SCellsHAD;
        std::unordered_map< uint32_t, std::tuple<uint32_t,float,float> > m_map_TTower2Tile;

};
}
#endif
