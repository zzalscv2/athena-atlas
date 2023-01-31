/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXFPGA - Defines FPGA tools
//                              -------------------
//     begin                : 01 04 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#include "L1CaloFEXSim/gFEXFPGA.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ISvcLocator.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"
#include <array>
#include <algorithm>


namespace LVL1 {

   // default constructor for persistency

  gFEXFPGA::gFEXFPGA(const std::string& type,const std::string& name,const IInterface* parent):
     AthAlgTool(type,name,parent)
  {
     declareInterface<IgFEXFPGA>(this);
  }

     /** Destructor */
     gFEXFPGA::~gFEXFPGA()
     {
     }

  //---------------- Initialisation -------------------------------------------------

  StatusCode gFEXFPGA::initialize()
  {

     ATH_CHECK(m_gFEXFPGA_gTowerContainerKey.initialize());

     return StatusCode::SUCCESS;
  }


  StatusCode gFEXFPGA::init(int id)
  {
     m_fpgaId = id;

     return StatusCode::SUCCESS;

  }

  void gFEXFPGA::reset(){

     m_fpgaId = -1;

  }


  void gFEXFPGA::SetTowersAndCells_SG(gTowersCentral tmp_gTowersIDs_subset){

     int rows = tmp_gTowersIDs_subset.size();
     int cols = tmp_gTowersIDs_subset[0].size();

     std::copy(&tmp_gTowersIDs_subset[0][0], &tmp_gTowersIDs_subset[0][0]+(rows*cols),&m_gTowersIDs_central[0][0]);


  }

  void gFEXFPGA::SetTowersAndCells_SG(gTowersForward tmp_gTowersIDs_subset){

     int rows = tmp_gTowersIDs_subset.size();
     int cols = tmp_gTowersIDs_subset[0].size();

     std::copy(&tmp_gTowersIDs_subset[0][0], &tmp_gTowersIDs_subset[0][0]+(rows*cols),&m_gTowersIDs_forward[0][0]);

   }


   void gFEXFPGA::GetEnergyMatrix(gTowersCentral & output_gTower_energies) const{

      int rows = output_gTower_energies.size();
      int cols = output_gTower_energies[0].size();

      SG::ReadHandle<gTowerContainer> gFEXFPGA_gTowerContainer(m_gFEXFPGA_gTowerContainerKey/*,ctx*/);

      for (int myrow = 0; myrow<rows; myrow++){
         for (int mycol = 0; mycol<cols; mycol++){
            output_gTower_energies[myrow][mycol] = 0;
            if (m_gTowersIDs_central[myrow][mycol] == 0) continue;
            const LVL1::gTower * tmpTower = gFEXFPGA_gTowerContainer->findTower(m_gTowersIDs_central[myrow][mycol]);
            if (tmpTower == nullptr) continue;
            output_gTower_energies[myrow][mycol] = tmpTower->getET();
         }
      }
   }

   void gFEXFPGA::GetEnergyMatrix(gTowersForward & output_gTower_energies) const{

      int rows = output_gTower_energies.size();
      int cols = output_gTower_energies[0].size();

      SG::ReadHandle<gTowerContainer> gFEXFPGA_gTowerContainer(m_gFEXFPGA_gTowerContainerKey/*,ctx*/);

      for (int myrow = 0; myrow<rows; myrow++){
         for (int mycol = 0; mycol<cols; mycol++){
            output_gTower_energies[myrow][mycol] = 0;
            if (m_gTowersIDs_forward[myrow][mycol] == 0) continue;
            const LVL1::gTower * tmpTower = gFEXFPGA_gTowerContainer->findTower(m_gTowersIDs_forward[myrow][mycol]);
            if (tmpTower == nullptr) continue;
            output_gTower_energies[myrow][mycol] = tmpTower->getET();

         }
      }
   }

   void gFEXFPGA::FillgTowerEDMCentral(SG::WriteHandle<xAOD::gFexTowerContainer> & gTowersContainer) {

      int IEta = 99;
      int IPhi = 99;
      float Eta = 99;
      float Phi = 99;                                
      int TowerEt = -99;
      int Fpga = m_fpgaId;
      char IsSaturated = 0;
      uint32_t gFEXtowerID = 0;

      // Assign ID based on FPGA (FPGA-A 0->0; FPGA-B 1->10000, FPGA-C 2->20000) and gTower number assigned as per firmware convention
      if (Fpga == 1){
         gFEXtowerID +=10000;
      }

      SG::ReadHandle<gTowerContainer> gFEXFPGA_gTowerContainer(m_gFEXFPGA_gTowerContainerKey/*,ctx*/);

      for(const auto& rowTowerID : m_gTowersIDs_central){
         for(const auto& towerID : rowTowerID){
            if(towerID == 0) continue;
            const LVL1::gTower * tmpTower = gFEXFPGA_gTowerContainer->findTower(towerID);

            if (tmpTower == nullptr) continue;
            IEta = tmpTower->iEta();
            IPhi = tmpTower->iPhi();
            TowerEt = tmpTower->getET();
            getEtaPhi(Eta, Phi, IEta, IPhi);
            std::unique_ptr<xAOD::gFexTower> gTowerEDM (new xAOD::gFexTower());
            gTowersContainer->push_back(std::move(gTowerEDM));
            gTowersContainer->back()->initialize(IEta, IPhi, Eta, Phi, TowerEt, Fpga, IsSaturated, gFEXtowerID);
            gFEXtowerID += 1;
         }
      }
   }

   void gFEXFPGA::FillgTowerEDMForward(SG::WriteHandle<xAOD::gFexTowerContainer> & gTowersContainer) {

      int IEta = 99;
      int IPhi = 99;
      float Eta = 99;
      float Phi = 99;                                
      int TowerEt = -99;
      int Fpga = 99;
      char IsSaturated = 0;

      SG::ReadHandle<gTowerContainer> gFEXFPGA_gTowerContainer(m_gFEXFPGA_gTowerContainerKey/*,ctx*/);

      for(const auto& rowTowerID : m_gTowersIDs_forward){
         for(const auto& towerID : rowTowerID){
            if(towerID == 0) continue;
            const LVL1::gTower * tmpTower = gFEXFPGA_gTowerContainer->findTower(towerID);

            if (tmpTower == nullptr) continue;
            IEta = tmpTower->iEta();
            IPhi = tmpTower->iPhi();
            TowerEt = tmpTower->getET();
            getEtaPhi(Eta, Phi, IEta, IPhi);
            Fpga = m_fpgaId;
            uint32_t gFEXtowerID = getTowerIDForward(IEta, IPhi, Eta);
            std::unique_ptr<xAOD::gFexTower> gTowerEDM (new xAOD::gFexTower());
            gTowersContainer->push_back(std::move(gTowerEDM));
            gTowersContainer->back()->initialize(IEta, IPhi, Eta, Phi, TowerEt, Fpga, IsSaturated, gFEXtowerID);
         }
      }
      
   }

   // This is needed to rearrange the most forward towers such that the forward matrix will be 12(ieta)x32(iphi)
   // i.e. the FPGA-C will have the same format (12*32) as FPGA-A and FPGA-B. This is what happens in firmware.
   // Note that for the most forward towers, the Phi index and Eta index are modified accordingly, 
   // but the float values of Phi and Eta remain unchanged. 
   uint32_t gFEXFPGA::getTowerIDForward ( int &iEta, int &iPhi, float Eta) const{

      uint32_t gFEXtowerID = 20000;

      if ( Eta < 0 ){

         if ( iEta == 0 ){
            gFEXtowerID = gFEXtowerID + (iPhi*24);
            iPhi = iPhi*2;
            iEta = 2;
         }
         else if ( iEta == 1 ){
            gFEXtowerID = gFEXtowerID + ((iPhi*24) + 12);
            iPhi = (iPhi*2)+1;
            iEta = 2;
         }   
         else if ( iEta == 2 ){
            gFEXtowerID = gFEXtowerID + ((iPhi*24) + 1);
            iPhi = iPhi*2;
            iEta = 3;
         } 
         else if ( iEta == 3 ){
            gFEXtowerID = gFEXtowerID + ((iPhi*24) + 13);
            iPhi = (iPhi*2)+1;
            iEta = 3;
         }
         else if ( iEta >= 4 and iEta <= 7 ){
            gFEXtowerID = gFEXtowerID + ((iPhi*12) + (iEta -2));
         } 

      } 

      else if ( Eta > 0 ){

         if ( iEta >= 32 and iEta <= 35){
            gFEXtowerID = gFEXtowerID + (iPhi*12) + (iEta -32 +6);
         }
         else if ( iEta == 36 ){
            gFEXtowerID = gFEXtowerID + ((iPhi*24) + 22);
            iPhi = (iPhi*2)+1;
            iEta = 36;
         }   
         else if ( iEta == 37 ){
            gFEXtowerID = gFEXtowerID + ((iPhi*24) + 10);
            iPhi = iPhi*2;
            iEta = 36;
         } 
         else if ( iEta == 38 ){
            gFEXtowerID = gFEXtowerID + ((iPhi*24) + 23);
            iPhi = (iPhi*2)+1;
            iEta = 37;
         }
         else if ( iEta == 39 ){
            gFEXtowerID = gFEXtowerID + ((iPhi*24) + 11);
            iPhi = iPhi*2;
            iEta = 37;
         }  
      } 

      return gFEXtowerID;
    
   }

void gFEXFPGA::getEtaPhi ( float &Eta, float &Phi, int iEta, int iPhi) const{
    
   float s_centralPhiWidth = (2*M_PI)/32; //In central region, gFex has 32 bins in phi
   float s_forwardPhiWidth = (2*M_PI)/16; //In forward region, gFex has 16 bins in phi (before rearranging bins)

   constexpr std::array<float, 40> s_EtaCenter = { -4.7, -4.2, -3.7, -3.4, -3.2, -3, 
                                            -2.8, -2.6, -2.35, -2.1, -1.9, -1.7, -1.5, -1.3, -1.1, -0.9,  
                                            -0.7, -0.5, -0.3, -0.1, 0.1, 0.3, 0.5, 0.7, 0.9, 1.1,                                                 
                                            1.3, 1.5, 1.7, 1.9, 2.1, 2.35, 2.6, 2.8, 3.0,
                                            3.2, 3.4, 3.7, 4.2, 4.7};

   Eta = s_EtaCenter.at(iEta); 

   float Phi_gFex = -99;
   if (( iEta <= 3 ) || ( (iEta >= 36) )){
      Phi_gFex = ( (iPhi * s_forwardPhiWidth) + s_forwardPhiWidth/2);
   }  
   else {
      Phi_gFex = ( (iPhi * s_centralPhiWidth) + s_centralPhiWidth/2);
   }
   
   if (Phi_gFex < M_PI) {
      Phi = Phi_gFex;
   }
   else {
      Phi = (Phi_gFex - 2*M_PI);
   }
   
}











} // end of namespace bracket