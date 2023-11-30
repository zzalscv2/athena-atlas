/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXSysSim - Overall gFEX simulation
//                              -------------------
//     begin                : 01 04 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#include "L1CaloFEXSim/gFEXSysSim.h"
#include "L1CaloFEXSim/gFEXSim.h"
#include "L1CaloFEXSim/gTower.h"
#include "L1CaloFEXSim/gTowerContainer.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"

#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"

namespace LVL1 {

   // default constructor for persistency

   gFEXSysSim::gFEXSysSim(const std::string& type,const std::string& name,const IInterface* parent):
      AthAlgTool(type,name,parent)
   {
      declareInterface<IgFEXSysSim>(this);

   }


   //---------------- Initialisation -------------------------------------------------

   StatusCode gFEXSysSim::initialize()
   {

      ATH_CHECK(m_gTowerContainerSGKey.initialize());

      ATH_CHECK(m_gFEXSimTool.retrieve());

      ATH_CHECK(m_gFexRhoOutKey.initialize()); 

      ATH_CHECK(m_gFexBlockOutKey.initialize());      

      ATH_CHECK(m_gFexJetOutKey.initialize());

      ATH_CHECK(m_gScalarEJwojOutKey.initialize());

      ATH_CHECK(m_gMETComponentsJwojOutKey.initialize());

      ATH_CHECK(m_gMHTComponentsJwojOutKey.initialize());
      
      ATH_CHECK(m_gMSTComponentsJwojOutKey.initialize());

      ATH_CHECK(m_gMETComponentsNoiseCutOutKey.initialize());

      ATH_CHECK(m_gMETComponentsRmsOutKey.initialize());

      ATH_CHECK(m_gScalarENoiseCutOutKey.initialize());

      ATH_CHECK(m_gScalarERmsOutKey.initialize());

      ATH_CHECK(m_l1MenuKey.initialize());

   

      return StatusCode::SUCCESS;
   }



   void gFEXSysSim::cleanup()   {

      m_gFEXCollection.clear();

   }


   int gFEXSysSim::calcTowerID(int eta, int phi, int nphi, int mod) const {

      return ((nphi*eta) + phi + mod);
   }


   StatusCode gFEXSysSim::execute(gFEXOutputCollection* gFEXOutputs)   {

      const EventContext& ctx = Gaudi::Hive::currentContext();
    
      SG::ReadHandle<LVL1::gTowerContainer> this_gTowerContainer(m_gTowerContainerSGKey,ctx);
      if(!this_gTowerContainer.isValid()){
         ATH_MSG_FATAL("Could not retrieve gTowerContainer " << m_gTowerContainerSGKey.key());
         return StatusCode::FAILURE;
      }

      // remove TOBs of the previous events from the array
      m_allgRhoTobs.clear();
      m_allgBlockTobs.clear();
      m_allgJetTobs.clear();
      m_allgScalarEJwojTobs.clear();
      m_allgMETComponentsJwojTobs.clear();
      m_allgMHTComponentsJwojTobs.clear();
      m_allgMSTComponentsJwojTobs.clear();


      // int centralNphi = 32;
      // int forwardNphi = 16;

      int fcalEta = 19; int fcalPhi = 0; int fcalMod = 900000;
      int initialFCAL = calcTowerID(fcalEta,fcalPhi,FEXAlgoSpaceDefs::forwardNphi,fcalMod);//900304
      int transfcalEta = 15; int transfcalPhi = 0; int transfcalMod = 700000;
      int initialTRANSFCAL = calcTowerID(transfcalEta,transfcalPhi,FEXAlgoSpaceDefs::centralNphi,transfcalMod);//700480
      int emecEta = 11; int emecPhi = 0; int emecMod = 500000;
      int initialEMEC = calcTowerID(emecEta,emecPhi,FEXAlgoSpaceDefs::centralNphi,emecMod);//500384
      int transembEta = 7; int transembPhi = 0; int transembMod = 300000;
      int initialTRANSEMB = calcTowerID(transembEta,transembPhi,FEXAlgoSpaceDefs::centralNphi,transembMod);///300224
      int embEta = 6; int embPhi = 0; int embMod = 100000;
      int initialEMB = calcTowerID(embEta,embPhi,FEXAlgoSpaceDefs::centralNphi,embMod);//100192


      int embposEta = 0; int embposPhi = 0; int embposMod = 200000;
      int initialposEMB = calcTowerID(embposEta,embposPhi,FEXAlgoSpaceDefs::centralNphi,embposMod);//200000
      int transembposEta = 7; int transembposPhi = 0; int transembposMod = 400000;
      int initialposTRANSEMB = calcTowerID(transembposEta,transembposPhi,FEXAlgoSpaceDefs::centralNphi,transembposMod);//400224
      int emecposEta = 8; int emecposPhi = 0; int emecposMod = 600000;
      int initialposEMEC = calcTowerID(emecposEta,emecposPhi,FEXAlgoSpaceDefs::centralNphi,emecposMod);//600256
      int transfcalposEta = 12; int transfcalposPhi = 0; int transfcalposMod = 800000;
      int initialposTRANSFCAL = calcTowerID(transfcalposEta,transfcalposPhi,FEXAlgoSpaceDefs::centralNphi,transfcalposMod);//800416
      int fcalposEta = 16; int fcalposPhi = 0; int fcalposMod = 1000000;
      int initialposFCAL = calcTowerID(fcalposEta,fcalposPhi,FEXAlgoSpaceDefs::forwardNphi,fcalposMod);//1000240


      // Since gFEX consists of a single module, here we are just (re)assigning the gTowerID

      // Defining a matrix 32x40 corresponding to the gFEX structure (32 phi x 40 eta in the most general case - forward region has 16 phi bins)
      typedef  std::array<std::array<int, FEXAlgoSpaceDefs::totalNeta>, FEXAlgoSpaceDefs::centralNphi> gTowersIDs;
      gTowersIDs tmp_gTowersIDs_subset;

      int rows = tmp_gTowersIDs_subset.size();
      int cols = tmp_gTowersIDs_subset[0].size();

      // set the FCAL negative part
      for(int thisCol=0; thisCol<4; thisCol++){
         for(int thisRow=0; thisRow<rows/2; thisRow++){
            int towerid = initialFCAL - ((thisCol) * (FEXAlgoSpaceDefs::forwardNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      // set the TRANSFCAL negative part (FCAL-EMEC overlap)
      for(int thisCol=4; thisCol<8; thisCol++){
         for(int thisRow=0; thisRow<rows; thisRow++){
            int towerid = initialTRANSFCAL - ((thisCol-4) * (FEXAlgoSpaceDefs::centralNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      // set the EMEC negative part
      for(int thisCol=8; thisCol<12; thisCol++){
         for(int thisRow=0; thisRow<rows; thisRow++){
            int towerid = initialEMEC - ((thisCol-8) * (FEXAlgoSpaceDefs::centralNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      // set the TRANSEMB (EMB-EMEC overlap) negative part
      for(int thisRow = 0; thisRow < rows; thisRow++){
         int thisCol = 12;
         int towerid = initialTRANSEMB + thisRow;
         tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
      }

      // set the EMB negative part
      for(int thisCol = 13; thisCol < 20; thisCol++){
         for(int thisRow=0; thisRow<rows; thisRow++){
           int towerid = initialEMB - ( (thisCol-13) * (FEXAlgoSpaceDefs::centralNphi)) + thisRow;
           tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

         // set the EMB positive part
      for(int thisCol = 20; thisCol < 27; thisCol++){
         for(int thisRow=0; thisRow<rows; thisRow++){
            int towerid = initialposEMB + ( (thisCol-20) * (FEXAlgoSpaceDefs::centralNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      // set the TRANSEMB (EMB-EMEC overlap) positive part
      for(int thisRow = 0; thisRow < rows; thisRow++){
         int thisCol = 27;
         int towerid = initialposTRANSEMB + thisRow;
         tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
      }
      // set the EMEC positive part
      for(int thisCol=28; thisCol<32; thisCol++){
         for(int thisRow=0; thisRow<rows; thisRow++){
            int towerid = initialposEMEC + ((thisCol-28) * (FEXAlgoSpaceDefs::centralNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      // set the TRANSFCAL positive part (EMEC-FCAL overlap)
      for(int thisCol=32; thisCol<36; thisCol++){
         for(int thisRow=0; thisRow<rows; thisRow++){
            int towerid = initialposTRANSFCAL + ((thisCol-32) * (FEXAlgoSpaceDefs::centralNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      // set the FCAL positive part
      for(int thisCol=36; thisCol<cols; thisCol++){
         for(int thisRow=0; thisRow<rows/2; thisRow++){
            int towerid = initialposFCAL + ((thisCol-36) * (FEXAlgoSpaceDefs::forwardNphi)) + thisRow;
            tmp_gTowersIDs_subset[thisRow][thisCol] = towerid;
         }
      }

      if(false){
         ATH_MSG_DEBUG("CONTENTS OF gFEX : ");
         for (int thisRow=rows-1; thisRow>=0; thisRow--){
            for (int thisCol=0; thisCol<cols; thisCol++){
               int tmptowerid = tmp_gTowersIDs_subset[thisRow][thisCol];
               const float tmptowereta = this_gTowerContainer->findTower(tmptowerid)->eta();
               const float tmptowerphi = this_gTowerContainer->findTower(tmptowerid)->phi();
               if(thisCol != cols-1){ ATH_MSG_DEBUG("|   " << tmptowerid << "([" << tmptowerphi << "][" << tmptowereta << "])   "); }
               else { ATH_MSG_DEBUG("|   " << tmptowerid << "([" << tmptowereta << "][" << tmptowerphi << "])   |"); }
            }
         }
      }

      ATH_CHECK(m_gFEXSimTool->executegFEXSim(tmp_gTowersIDs_subset, gFEXOutputs));
      
      m_allgRhoTobs = m_gFEXSimTool->getgRhoTOBs();
      m_allgBlockTobs = m_gFEXSimTool->getgBlockTOBs();
      m_allgJetTobs = m_gFEXSimTool->getgJetTOBs();
       
      m_allgScalarEJwojTobs = m_gFEXSimTool->getgScalarEJwojTOBs();
      m_allgMETComponentsJwojTobs = m_gFEXSimTool->getgMETComponentsJwojTOBs();
      m_allgMHTComponentsJwojTobs = m_gFEXSimTool->getgMHTComponentsJwojTOBs();
      m_allgMSTComponentsJwojTobs = m_gFEXSimTool->getgMSTComponentsJwojTOBs();

      m_allgMETComponentsNoiseCutTobs = m_gFEXSimTool->getgMETComponentsNoiseCutTOBs();
      m_allgMETComponentsRmsTobs = m_gFEXSimTool->getgMETComponentsRmsTOBs();
      m_allgScalarENoiseCutTobs = m_gFEXSimTool->getgScalarENoiseCutTOBs();
      m_allgScalarERmsTobs = m_gFEXSimTool->getgScalarERmsTOBs();

      m_gFEXSimTool->reset();

      //Makes containers for different gFEX Jet objects
      m_gRhoContainer = std::make_unique<xAOD::gFexJetRoIContainer> ();
      m_gRhoAuxContainer = std::make_unique<xAOD::gFexJetRoIAuxContainer> ();
      m_gRhoContainer->setStore(m_gRhoAuxContainer.get());

      m_gBlockContainer = std::make_unique<xAOD::gFexJetRoIContainer> ();
      m_gBlockAuxContainer = std::make_unique<xAOD::gFexJetRoIAuxContainer> ();
      m_gBlockContainer->setStore(m_gBlockAuxContainer.get());

      m_gJetContainer = std::make_unique<xAOD::gFexJetRoIContainer> ();
      m_gJetAuxContainer = std::make_unique<xAOD::gFexJetRoIAuxContainer> ();
      m_gJetContainer->setStore(m_gJetAuxContainer.get());

      //Makes containers for different gFEX Global objects (for JwoJ algorithm quantities)
      m_gScalarEJwojContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      m_gScalarEJwojAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      m_gScalarEJwojContainer->setStore(m_gScalarEJwojAuxContainer.get());

      m_gMETComponentsJwojContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      m_gMETComponentsJwojAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      m_gMETComponentsJwojContainer->setStore(m_gMETComponentsJwojAuxContainer.get());

      m_gMHTComponentsJwojContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      m_gMHTComponentsJwojAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      m_gMHTComponentsJwojContainer->setStore(m_gMHTComponentsJwojAuxContainer.get());

      m_gMSTComponentsJwojContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      m_gMSTComponentsJwojAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      m_gMSTComponentsJwojContainer->setStore(m_gMSTComponentsJwojAuxContainer.get());

      //Makes containers for different gFEX Global objects (for Noise Cut and RMS algorithms quantities)
      m_gMETComponentsNoiseCutContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      m_gMETComponentsNoiseCutAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      m_gMETComponentsNoiseCutContainer->setStore(m_gMETComponentsNoiseCutAuxContainer.get());
      
      m_gMETComponentsRmsContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      m_gMETComponentsRmsAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      m_gMETComponentsRmsContainer->setStore(m_gMETComponentsRmsAuxContainer.get());

      m_gScalarENoiseCutContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      m_gScalarENoiseCutAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      m_gScalarENoiseCutContainer->setStore(m_gScalarENoiseCutAuxContainer.get());

      m_gScalarERmsContainer = std::make_unique<xAOD::gFexGlobalRoIContainer> ();
      m_gScalarERmsAuxContainer = std::make_unique<xAOD::gFexGlobalRoIAuxContainer> ();
      m_gScalarERmsContainer->setStore(m_gScalarERmsAuxContainer.get());


      // Retrieve the L1 menu configuration
      SG::ReadHandle<TrigConf::L1Menu> l1Menu (m_l1MenuKey,ctx);
      ATH_CHECK(l1Menu.isValid());

      auto & thr_gJ = l1Menu->thrExtraInfo().gJ();
      auto & thr_gLJ = l1Menu->thrExtraInfo().gLJ();
      auto & thr_gXE = l1Menu->thrExtraInfo().gXE();
      auto & thr_gTE = l1Menu->thrExtraInfo().gTE();

      int gJ_scale = thr_gJ.resolutionMeV();
      int gLJ_scale = thr_gLJ.resolutionMeV();
      int gXE_scale = thr_gXE.resolutionMeV();
      int gTE_scale = thr_gTE.resolutionMeV();


      //iterate over all gRho Tobs and fill EDM with them
      for(auto &tob : m_allgRhoTobs){
         ATH_CHECK(fillgRhoEDM(tob, gJ_scale));
      }
      //iterate over all gBlock Tobs and fill EDM with them
      for(auto &tob : m_allgBlockTobs){
         ATH_CHECK(fillgBlockEDM(tob, gJ_scale));
      }

      //iterate over all gJet Tobs and fill EDM with them
      for(auto &tob : m_allgJetTobs){
         ATH_CHECK(fillgJetEDM(tob, gLJ_scale));   
      }

      //iterate over all JwoJ scalar energy Tobs and fill EDM with them (should be only one)
      for(auto &tob : m_allgScalarEJwojTobs){
         ATH_CHECK(fillgScalarEJwojEDM(tob, gXE_scale, gTE_scale));
      }
      //iterate over all JwoJ METcomponents Tobs and fill EDM with them (should be only one)
      for(auto &tob : m_allgMETComponentsJwojTobs){
         ATH_CHECK(fillgMETComponentsJwojEDM(tob, gXE_scale, gXE_scale));
      }
      //iterate over all JwoJ MHTcomponents Tobs and fill EDM with them (should be only one)
      for(auto &tob : m_allgMHTComponentsJwojTobs){
         ATH_CHECK(fillgMHTComponentsJwojEDM(tob, gXE_scale, gXE_scale));
      }
      //iterate over all JwoJ MSTcomponents Tobs and fill EDM with them (should be only one)
      for(auto &tob : m_allgMSTComponentsJwojTobs){
         ATH_CHECK(fillgMSTComponentsJwojEDM(tob, gXE_scale, gXE_scale));
      }


      //iterate over all NoiseCut METcomponents Tobs and fill EDM with them (should be only one)
      for(auto &tob : m_allgMETComponentsNoiseCutTobs){
         ATH_CHECK(fillgMETComponentsNoiseCutEDM(tob, gXE_scale, gXE_scale));
      }
      //iterate over all RMS METcomponents Tobs and fill EDM with them (should be only one)
      for(auto &tob : m_allgMETComponentsRmsTobs){
         ATH_CHECK(fillgMETComponentsRmsEDM(tob, gXE_scale, gXE_scale));
      }
      //iterate over all NoiseCut scalar energy Tobs and fill EDM with them (should be only one)
      for(auto &tob : m_allgScalarENoiseCutTobs){
         ATH_CHECK(fillgScalarENoiseCutEDM(tob, gXE_scale, gTE_scale));
      }
      //iterate over all RMS scalar energy Tobs and fill EDM with them (should be only one)
      for(auto &tob : m_allgScalarERmsTobs){
         ATH_CHECK(fillgScalarERmsEDM(tob, gXE_scale, gTE_scale));
      }

      
      SG::WriteHandle<xAOD::gFexJetRoIContainer> outputgFexRhoHandle(m_gFexRhoOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgFexRhoHandle.key() << " = " << "..." );
      ATH_CHECK(outputgFexRhoHandle.record(std::move(m_gRhoContainer),std::move(m_gRhoAuxContainer)));

      SG::WriteHandle<xAOD::gFexJetRoIContainer> outputgFexBlockHandle(m_gFexBlockOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgFexBlockHandle.key() << " = " << "..." );
      ATH_CHECK(outputgFexBlockHandle.record(std::move(m_gBlockContainer),std::move(m_gBlockAuxContainer)));

      SG::WriteHandle<xAOD::gFexJetRoIContainer> outputgFexJetHandle(m_gFexJetOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgFexJetHandle.key() << " = " << "..." );
      ATH_CHECK(outputgFexJetHandle.record(std::move(m_gJetContainer),std::move(m_gJetAuxContainer)));


      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgScalarEJwojHandle(m_gScalarEJwojOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgScalarEJwojHandle.key() << " = " << "..." );
      ATH_CHECK(outputgScalarEJwojHandle.record(std::move(m_gScalarEJwojContainer),std::move(m_gScalarEJwojAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgMETComponentsJwojHandle(m_gMETComponentsJwojOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgMETComponentsJwojHandle.key() << " = " << "..." );
      ATH_CHECK(outputgMETComponentsJwojHandle.record(std::move(m_gMETComponentsJwojContainer),std::move(m_gMETComponentsJwojAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgMHTComponentsJwojHandle(m_gMHTComponentsJwojOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgMHTComponentsJwojHandle.key() << " = " << "..." );
      ATH_CHECK(outputgMHTComponentsJwojHandle.record(std::move(m_gMHTComponentsJwojContainer),std::move(m_gMHTComponentsJwojAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgMSTComponentsJwojHandle(m_gMSTComponentsJwojOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgMSTComponentsJwojHandle.key() << " = " << "..." );
      ATH_CHECK(outputgMSTComponentsJwojHandle.record(std::move(m_gMSTComponentsJwojContainer),std::move(m_gMSTComponentsJwojAuxContainer)));


      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgMETComponentsNoiseCutHandle(m_gMETComponentsNoiseCutOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgMETComponentsNoiseCutHandle.key() << " = " << "..." );
      ATH_CHECK(outputgMETComponentsNoiseCutHandle.record(std::move(m_gMETComponentsNoiseCutContainer),std::move(m_gMETComponentsNoiseCutAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgMETComponentsRmsHandle(m_gMETComponentsRmsOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgMETComponentsRmsHandle.key() << " = " << "..." );
      ATH_CHECK(outputgMETComponentsRmsHandle.record(std::move(m_gMETComponentsRmsContainer),std::move(m_gMETComponentsRmsAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgScalarENoiseCutHandle(m_gScalarENoiseCutOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgScalarENoiseCutHandle.key() << " = " << "..." );
      ATH_CHECK(outputgScalarENoiseCutHandle.record(std::move(m_gScalarENoiseCutContainer),std::move(m_gScalarENoiseCutAuxContainer)));

      SG::WriteHandle<xAOD::gFexGlobalRoIContainer> outputgScalarERmsHandle(m_gScalarERmsOutKey,ctx);
      ATH_MSG_DEBUG("   write: " << outputgScalarERmsHandle.key() << " = " << "..." );
      ATH_CHECK(outputgScalarERmsHandle.record(std::move(m_gScalarERmsContainer),std::move(m_gScalarERmsAuxContainer)));


      return StatusCode::SUCCESS;
   }

   StatusCode gFEXSysSim::fillgRhoEDM(uint32_t tobWord, int gJ_scale){

      std::unique_ptr<xAOD::gFexJetRoI> myEDM (new xAOD::gFexJetRoI());
      m_gRhoContainer->push_back(std::move(myEDM));
      m_gRhoContainer->back()->initialize(tobWord, gJ_scale);

      return StatusCode::SUCCESS;
   }

   StatusCode gFEXSysSim::fillgBlockEDM(uint32_t tobWord, int gJ_scale){

      std::unique_ptr<xAOD::gFexJetRoI> myEDM (new xAOD::gFexJetRoI());
      m_gBlockContainer->push_back(std::move(myEDM));
      m_gBlockContainer->back()->initialize(tobWord, gJ_scale);

      return StatusCode::SUCCESS;
   }

   StatusCode gFEXSysSim::fillgJetEDM(uint32_t tobWord, int gLJ_scale){

      std::unique_ptr<xAOD::gFexJetRoI> myEDM (new xAOD::gFexJetRoI());
      m_gJetContainer->push_back(std::move(myEDM));
      m_gJetContainer->back()->initialize(tobWord, gLJ_scale);

      return StatusCode::SUCCESS;
   }

   StatusCode gFEXSysSim::fillgMETComponentsJwojEDM(uint32_t tobWord, int scale1, int scale2){

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      m_gMETComponentsJwojContainer->push_back(std::move(myEDM));
      m_gMETComponentsJwojContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

   StatusCode gFEXSysSim::fillgMHTComponentsJwojEDM(uint32_t tobWord, int scale1, int scale2){

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      m_gMHTComponentsJwojContainer->push_back(std::move(myEDM));
      m_gMHTComponentsJwojContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

   StatusCode gFEXSysSim::fillgMSTComponentsJwojEDM(uint32_t tobWord, int scale1, int scale2){

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      m_gMSTComponentsJwojContainer->push_back(std::move(myEDM));
      m_gMSTComponentsJwojContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

   StatusCode gFEXSysSim::fillgScalarEJwojEDM(uint32_t tobWord, int scale1, int scale2){

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      m_gScalarEJwojContainer->push_back(std::move(myEDM));
      m_gScalarEJwojContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

   StatusCode gFEXSysSim::fillgMETComponentsNoiseCutEDM(uint32_t tobWord, int scale1, int scale2){

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      m_gMETComponentsNoiseCutContainer->push_back(std::move(myEDM));
      m_gMETComponentsNoiseCutContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

   StatusCode gFEXSysSim::fillgMETComponentsRmsEDM(uint32_t tobWord, int scale1, int scale2){

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      m_gMETComponentsRmsContainer->push_back(std::move(myEDM));
      m_gMETComponentsRmsContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

   StatusCode gFEXSysSim::fillgScalarENoiseCutEDM(uint32_t tobWord, int scale1, int scale2){

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      m_gScalarENoiseCutContainer->push_back(std::move(myEDM));
      m_gScalarENoiseCutContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }

   StatusCode gFEXSysSim::fillgScalarERmsEDM(uint32_t tobWord, int scale1, int scale2){

      std::unique_ptr<xAOD::gFexGlobalRoI> myEDM (new xAOD::gFexGlobalRoI());
      m_gScalarERmsContainer->push_back(std::move(myEDM));
      m_gScalarERmsContainer->back()->initialize(tobWord, scale1, scale2);

      return StatusCode::SUCCESS;
   }


} // end of namespace bracket
