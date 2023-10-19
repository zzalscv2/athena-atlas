/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//		jFEXForwardElecAlgo - Algorithm for Forward Electron Algorithm in jFEX
//                              -------------------
//     begin                : 16 11 2021
//     email                : Sergi.Rodriguez@cern.ch
//     email                : sjolin@cern.ch
//***************************************************************************

#ifndef jFEXForwardElecAlgo_H
#define jFEXForwardElecAlgo_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IjFEXForwardElecAlgo.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "L1CaloFEXSim/jFEXForwardElecTOB.h"
#include "L1CaloFEXSim/jFEXForwardElecInfo.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/StoreGateSvc.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"

namespace LVL1 {

  class jFEXForwardElecAlgo : public AthAlgTool, virtual public IjFEXForwardElecAlgo {

  public:
    /** Constructors **/
    jFEXForwardElecAlgo(const std::string& type, const std::string& name, const IInterface* parent);
    
    /** standard Athena-Algorithm method **/
    virtual StatusCode initialize() override;
    
    /** Destructor **/
    virtual ~jFEXForwardElecAlgo();
    
    /** Standard methods **/
    virtual StatusCode safetyTest() override;
    virtual StatusCode reset() override;
   
    virtual void setup(
      int inputTable[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width],
      int jfex, int fpga ) override;
    
    virtual std::unordered_map<uint, jFEXForwardElecInfo> calculateEDM() override;
    virtual void setFPGAEnergy(
      std::unordered_map<int,std::vector<int> > etmapEM,
      std::unordered_map<int,std::vector<int> > etmapHAD) override; 
    
  private:        
    SG::ReadHandleKey<LVL1::jTowerContainer> m_jTowerContainerKey {this, "MyjTowers", "jTowerContainer", "jTower input container"};
    SG::ReadHandle<jTowerContainer> m_jTowerContainer;
    std::unordered_map<int,std::vector<int> > m_map_Etvalues_EM;
    std::unordered_map<int,std::vector<int> > m_map_Etvalues_HAD;
    int m_jFEXalgoTowerID[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width];
    int m_lowerEM_eta;
    int m_upperEM_eta;
    int m_jfex;
    int m_fpga;
    static constexpr float m_2PI = 2*M_PI;
    static constexpr float m_TT_Size_phi = M_PI/32;
    const int m_Edge_dR2 = std::round( (std::pow(2*M_PI/32,2)) * 1e5  );
    const int m_Edge_dR3 = std::round( (std::pow(3*M_PI/32,2)) * 1e5  );
    const int m_Edge_dR4 = std::round( (std::pow(4*M_PI/32,2)) * 1e5  );
        
    Gaudi::Property<std::string> m_SeedRingStr {this, "SeedRingMap", "Run3L1CaloSimulation/JetMaps/2023_02_10/jFEX_FWD_seed.dat"   , "Contains Trigger tower in seed"};
    Gaudi::Property<std::string> m_1stRingStr  {this, "Energy1stRingMap", "Run3L1CaloSimulation/JetMaps/2023_02_10/jFEX_FWD_1stRing.dat" , "Contains Trigger tower in 1st Energy ring"};
    Gaudi::Property<std::string> m_SearchGStr  {this, "SearchGMap", "Run3L1CaloSimulation/JetMaps/2023_02_10/jFEX_FWD_searchG.dat" , "Contains Trigger tower to find local max (greater than)"};
    Gaudi::Property<std::string> m_SearchGeStr {this, "SearchGeMap", "Run3L1CaloSimulation/JetMaps/2023_02_10/jFEX_FWD_searchGe.dat", "Contains Trigger tower to find local max (greater or equal than)"};
    Gaudi::Property<std::string> m_SearchGTauStr  {this, "SearchGTauMap", "Run3L1CaloSimulation/JetMaps/2023_02_10/jFEX_FWD_searchGTau.dat" , "Contains Trigger tower to find local max (greater than)"};
    Gaudi::Property<std::string> m_SearchGeTauStr {this, "SearchGeTauMap", "Run3L1CaloSimulation/JetMaps/2023_02_10/jFEX_FWD_searchGeTau.dat", "Contains Trigger tower to find local max (greater or equal than)"};
       
    std::unordered_map<unsigned int, std::vector<unsigned int> > m_SeedRingMap;
    std::unordered_map<unsigned int, std::vector<unsigned int> > m_1stRingMap;
    std::unordered_map<unsigned int, std::vector<unsigned int> > m_SearchGMap;
    std::unordered_map<unsigned int, std::vector<unsigned int> > m_SearchGeMap;
    std::unordered_map<unsigned int, std::vector<unsigned int> > m_SearchGTauMap;
    std::unordered_map<unsigned int, std::vector<unsigned int> > m_SearchGeTauMap;
    
    std::array<float,2> getEtaPhi(uint);
    std::array<uint,2> getEtEmHad(uint);
    std::unordered_map<uint, jFEXForwardElecInfo> eleClusterList(void);
    StatusCode ReadfromFile(const std::string& , std::unordered_map<unsigned int, std::vector<unsigned int> >&);
  };
  
}//end of namespace

CLASS_DEF( LVL1::jFEXForwardElecAlgo, 71453331, 1 )

#endif

