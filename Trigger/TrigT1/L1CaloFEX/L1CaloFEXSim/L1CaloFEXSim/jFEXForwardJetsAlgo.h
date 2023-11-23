/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//              jFEXForwardJetsAlgo - Algorithm for small R jet Algorithm in jFEX
//                              -------------------
//     begin                : 07 06 2021
//     email                : varsiha.sothilingam@cern.ch
//***************************************************************************

#ifndef jFEXForwardJetsAlgo_H
#define jFEXForwardJetsAlgo_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IjFEXForwardJetsAlgo.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "L1CaloFEXSim/jFEXForwardJetsInfo.h"
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloIdManager.h" 
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "AthenaBaseComps/AthAlgorithm.h" 
#include "StoreGate/StoreGateSvc.h" 
#include "PathResolver/PathResolver.h"


namespace LVL1 {

  class jFEXForwardJetsAlgo : public AthAlgTool, virtual public IjFEXForwardJetsAlgo {

  public:
    /** Constructors */
    jFEXForwardJetsAlgo(const std::string& type, const std::string& name, const IInterface* parent);
   
    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;

    /** Destructor */
    virtual ~jFEXForwardJetsAlgo();

    virtual StatusCode safetyTest() override;
    virtual StatusCode reset() override;

    virtual void setup(int inputTable[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width], int jfex) override;
    virtual std::unordered_map<int, jFEXForwardJetsInfo> FcalJetsTowerIDLists() override;
    virtual std::unordered_map<int, jFEXForwardJetsInfo> calculateJetETs() override;
    virtual void setFPGAEnergy(std::unordered_map<int,std::vector<int> > et_map)  override;

  protected:

  private:
        SG::ReadHandleKey<LVL1::jTowerContainer> m_jFEXForwardJetsAlgo_jTowerContainerKey {this, "MyjTowers", "jTowerContainer", "Input container for jTowers"};
        SG::ReadHandle<jTowerContainer> m_jTowerContainer;
        int m_jFEXalgoTowerID[FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width];
        std::unordered_map<int,std::vector<int> > m_map_Etvalues;
        int m_lowerEM_eta;
        int m_upperEM_eta;
        int m_jfex;
        
        //This flag determines if the TT ID which are in the first and second energy rings are stored
        //in the jFEXForwardJetsInfo class. It is set to false to reduce quantity data stored in class
        bool m_storeEnergyRingTTIDs = true;    
        
        //FWD CVMFS files
        Gaudi::Property<std::string> m_SeedRingStr {this, "SeedRingMap", "Run3L1CaloSimulation/JetMaps/2023_11_21/jFEX_FWD_seed.dat"   , "Contains Trigger tower in seed"};
        Gaudi::Property<std::string> m_1stRingStr  {this, "Energy1stRingMap" , "Run3L1CaloSimulation/JetMaps/2023_11_21/jFEX_FWD_1stRing.dat" , "Contains Trigger tower in 1st Energy ring"};
        Gaudi::Property<std::string> m_2ndRingStr  {this, "Energy2ndRingMap" , "Run3L1CaloSimulation/JetMaps/2023_11_21/jFEX_FWD_2ndRing.dat" , "Contains Trigger tower in 2nd energy ring"};
        Gaudi::Property<std::string> m_CorrStr     {this, "CorrMap"    , "Run3L1CaloSimulation/JetMaps/2023_11_21/jFEX_FWD_corr.dat"    , "Contains Trigger tower to correct displacement (greater than)"};
        Gaudi::Property<std::string> m_Corr2Str    {this, "Corr2Map"   , "Run3L1CaloSimulation/JetMaps/2023_11_21/jFEX_FWD_corr2.dat"   , "Contains Trigger tower to correct displacement (greater or equal than)"};
        Gaudi::Property<std::string> m_SearchGStr  {this, "SearchGMap" , "Run3L1CaloSimulation/JetMaps/2023_11_21/jFEX_FWD_searchG.dat" , "Contains Trigger tower to find local max (greater than)"};
        Gaudi::Property<std::string> m_SearchGeStr {this, "SearchGeMap", "Run3L1CaloSimulation/JetMaps/2023_11_21/jFEX_FWD_searchGe.dat", "Contains Trigger tower to find local max (greater or equal than)"};
        
        std::unordered_map<unsigned int, std::vector<unsigned int> > m_SeedRingMap;
        std::unordered_map<unsigned int, std::vector<unsigned int> > m_1stRingMap;
        std::unordered_map<unsigned int, std::vector<unsigned int> > m_2ndRingMap;
        std::unordered_map<unsigned int, std::vector<unsigned int> > m_CorrMap;
        std::unordered_map<unsigned int, std::vector<unsigned int> > m_Corr2Map;
        std::unordered_map<unsigned int, std::vector<unsigned int> > m_SearchGMap;
        std::unordered_map<unsigned int, std::vector<unsigned int> > m_SearchGeMap;
        
        StatusCode ReadfromFile(const std::string& , std::unordered_map<unsigned int, std::vector<unsigned int> >&);
        
        int SumEtSeed(unsigned int TTID);
        int getEt(unsigned int TTID);
        bool isLM(unsigned int TTID);
        bool isLMabove(unsigned int TTID);
        bool getTTowerSat(unsigned int TTID );
        
        //Conditions for greater
        unsigned int elementsCorr(unsigned int TTID);
        bool condCorr(unsigned int TTID);
        //Conditions for greater or equal
        unsigned int elementsCorr2(unsigned int TTID);
        bool condCorr2(unsigned int TTID);
        
        std::array<float,2> globalEtaPhi(int TTID);
        
  };
}//end of namespace
#endif
