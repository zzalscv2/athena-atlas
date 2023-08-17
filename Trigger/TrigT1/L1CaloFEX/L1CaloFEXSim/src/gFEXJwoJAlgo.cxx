/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gFEXJwoJAlg - Jets without jets algorithm for gFEX
//                              -------------------
//     begin                : 10 08 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#include <cmath>
#include <vector>

#include "L1CaloFEXSim/gFEXJwoJAlgo.h"
#include "L1CaloFEXSim/gFEXJwoJTOB.h"
#include "L1CaloFEXSim/gTowerContainer.h"
#include "L1CaloFEXSim/gTower.h"

namespace LVL1 {

  // default constructor for persistency
gFEXJwoJAlgo::gFEXJwoJAlgo(const std::string& type, const std::string& name, const IInterface* parent):
    AthAlgTool(type, name, parent)
  {
    declareInterface<IgFEXJwoJAlgo>(this);
  }


StatusCode gFEXJwoJAlgo::initialize(){

  return StatusCode::SUCCESS;

}


void gFEXJwoJAlgo::setAlgoConstant(float aFPGA_A, float bFPGA_A,
                                   float aFPGA_B, float bFPGA_B,
                                   float aFPGA_C, float bFPGA_C,
                                   int gXE_seedThrA, int gXE_seedThrB, int gXE_seedThrC) {
  m_aFPGA_A = aFPGA_A;
  m_bFPGA_A = bFPGA_A;
  m_aFPGA_B = aFPGA_B;
  m_bFPGA_B = bFPGA_B;
  m_aFPGA_C = aFPGA_C;
  m_bFPGA_C = bFPGA_C;
  m_gBlockthresholdA = gXE_seedThrA;
  m_gBlockthresholdB = gXE_seedThrB;
  m_gBlockthresholdC = gXE_seedThrC;

}



std::vector<std::unique_ptr<gFEXJwoJTOB>> gFEXJwoJAlgo::jwojAlgo(const gTowersCentral& Atwr, const gTowersCentral& Btwr,
                                                                 const gTowersForward& CNtwr, const gTowersForward& CPtwr,
                                                                 std::array<uint32_t, 4> & outTOB) const {


  gTowersCentral Ctwr = {{{0}}};
  makeFPGAC(CNtwr, CPtwr, Ctwr);


  // find gBlocks
  gTowersCentral gBLKA;
  gBlockAB(Atwr, gBLKA);


  gTowersCentral gBLKB;
  gBlockAB(Btwr, gBLKB);


  gTowersCentral gBLKC;
  gBlockAB(Ctwr, gBLKC);

  //FPGA A observables
  int A_MHT_x = 0x0;
  int A_MHT_y = 0x0;
  int A_MST_x = 0x0;
  int A_MST_y = 0x0;
  int A_MET_x = 0x0;
  int A_MET_y = 0x0;

  int A_sumEt = 0x0;

  //FPGA B observables
  int B_MHT_x = 0x0;
  int B_MHT_y = 0x0;
  int B_MST_x = 0x0;
  int B_MST_y = 0x0;
  int B_MET_x = 0x0;
  int B_MET_y = 0x0;

  int B_sumEt = 0x0;


  //FPGA C observables
  int C_MHT_x = 0x0;
  int C_MHT_y = 0x0;
  int C_MST_x = 0x0;
  int C_MST_y = 0x0;
  int C_MET_x = 0x0;
  int C_MET_y = 0x0;

  int C_sumEt = 0x0;


  //Global observables
  int MET_x = 0x0;
  int MET_y = 0x0;
  int MET = 0x0;

  int total_sumEt = 0x0;
  int MHT_x = 0x0;
  int MHT_y = 0x0;
  int MST_x = 0x0;
  int MST_y = 0x0;


  metFPGA(Atwr, gBLKA, m_gBlockthresholdA, m_aFPGA_A, m_bFPGA_A, A_MHT_x, A_MHT_y, A_MST_x, A_MST_y, A_MET_x, A_MET_y);
  metFPGA(Btwr, gBLKB, m_gBlockthresholdB, m_aFPGA_B, m_bFPGA_B, B_MHT_x, B_MHT_y, B_MST_x, B_MST_y, B_MET_x, B_MET_y);
  metFPGA(Ctwr, gBLKC, m_gBlockthresholdC, m_aFPGA_C, m_bFPGA_C, C_MHT_x, C_MHT_y, C_MST_x, C_MST_y, C_MET_x, C_MET_y);

  metTotal(A_MET_x, A_MET_y, B_MET_x, B_MET_y, C_MET_x, C_MET_y, MET_x, MET_y, MET);
      
  sumEtFPGA(Atwr, A_sumEt);
  sumEtFPGA(Btwr, B_sumEt);
  sumEtFPGA(Ctwr, C_sumEt);

  sumEt(A_sumEt, B_sumEt, C_sumEt, total_sumEt);
  // Truncate two bits of SumEt to allow for a larger energy range with 800 MeV resolution
  total_sumEt = total_sumEt/4; 
   
  sumEt(A_MHT_x, B_MHT_x, C_MHT_x, MHT_x);
  sumEt(A_MHT_y, B_MHT_y, C_MHT_y, MHT_y);
  sumEt(A_MST_x, B_MST_x, C_MST_x, MST_x);
  sumEt(A_MST_y, B_MST_y, C_MST_y, MST_y);

  //Define a vector to be filled with all the TOBs of one event
  std::vector<std::unique_ptr<gFEXJwoJTOB>> tobs_v;
  tobs_v.resize(4);


  // fill in TOBs
  // The order of the TOBs is given according to the TOB ID (TODO: check how it's done in fw)

  // First TOB is (MET, SumEt)
  outTOB[0] = (total_sumEt &  0x00000FFF) << 0; //set the Quantity2 to the corresponding slot (LSB)
  outTOB[0] = outTOB[0] | (MET  &  0x00000FFF) << 12;//Quantity 1 (in bit number 12)
  if (total_sumEt != 0) outTOB[0] = outTOB[0] | 0x00000001 << 24;//Status bit for Quantity 2 (0 if quantity is null)
  if (MET != 0) outTOB[0] = outTOB[0] | 0x00000001 << 25;//Status bit for Quantity 1 (0 if quantity is null)
  outTOB[0] = outTOB[0] | (1  &  0x0000001F) << 26;//TOB ID is 1 for scalar values (5 bits starting at 26)

// Second TOB is (MET_x, MET_y)
  outTOB[1] = (MET_y &  0x00000FFF) << 0; //set the Quantity2 to the corresponding slot (LSB)
  outTOB[1] = outTOB[1] | (MET_x  &  0x00000FFF) << 12;//Quantity 1 (in bit number 12)
  if (MET_y != 0) outTOB[1] = outTOB[1] | 0x00000001 << 24;//Status bit for Quantity 2 (0 if quantity is null)
  if (MET_x != 0) outTOB[1] = outTOB[1] | 0x00000001 << 25;//Status bit for Quantity 1 (0 if quantity is null)
  outTOB[1] = outTOB[1] | (2  &  0x0000001F) << 26;//TOB ID is 2 for MET_x, MET_y (5 bits starting at 26)

// Third TOB is hard components (MHT_x, MHT_y)
  outTOB[2] = (MHT_y &  0x00000FFF) << 0; //set the Quantity2 to the corresponding slot (LSB)
  outTOB[2] = outTOB[2] | (MHT_x  &  0x00000FFF) << 12;//Quantity 1 (in bit number 12)
  if (MHT_y != 0) outTOB[2] = outTOB[2] | 0x00000001 << 24;//Status bit for Quantity 2 (0 if quantity is null)
  if (MHT_x != 0) outTOB[2] = outTOB[2] | 0x00000001 << 25;//Status bit for Quantity 1 (0 if quantity is null)
  outTOB[2] = outTOB[2] | (3  &  0x0000001F) << 26;//TOB ID is 3 for hard components (5 bits starting at 26)

  // Fourth TOB is hard components (MST_x, MST_y)
  outTOB[3] = (MST_y &  0x00000FFF) << 0; //set the Quantity2 to the corresponding slot (LSB)
  outTOB[3] = outTOB[3] | (MST_x  &  0x00000FFF) << 12;//Quantity 1 (in bit number 12)
  if (MST_y != 0) outTOB[3] = outTOB[3] | 0x00000001 << 24;//Status bit for Quantity 2 (0 if quantity is null)
  if (MST_x != 0) outTOB[3] = outTOB[3] | 0x00000001 << 25;//Status bit for Quantity 1 (0 if quantity is null)
  outTOB[3] = outTOB[3] | (4  &  0x0000001F) << 26;//TOB ID is 4 for soft components (5 bits starting at 26)


  tobs_v[0] = std::make_unique<gFEXJwoJTOB>();
  tobs_v[0]->setWord(outTOB[0]);
  tobs_v[0]->setQuantity1(MET);
  tobs_v[0]->setQuantity2(total_sumEt);
  tobs_v[0]->setSaturation(0); //Always 0 for now, need a threshold?
  tobs_v[0]->setTobID(1);
  if( MET != 0 ) tobs_v[0]->setStatus1(1);
  else tobs_v[0]->setStatus1(0);
  if(total_sumEt!= 0) tobs_v[0]->setStatus2(1);
  else tobs_v[0]->setStatus2(0);

  tobs_v[1] = std::make_unique<gFEXJwoJTOB>();
  tobs_v[1]->setWord(outTOB[1]);
  tobs_v[1]->setQuantity1(MET_x);
  tobs_v[1]->setQuantity2(MET_y);
  tobs_v[1]->setSaturation(0); //Always 0 for now, need a threshold?
  tobs_v[1]->setTobID(2);
  if( MET_x != 0 ) tobs_v[1]->setStatus1(1);
  else tobs_v[1]->setStatus1(0);
  if(MET_y!= 0) tobs_v[1]->setStatus2(1);
  else tobs_v[1]->setStatus2(0);

  tobs_v[2] = std::make_unique<gFEXJwoJTOB>();
  tobs_v[2]->setWord(outTOB[2]);
  tobs_v[2]->setQuantity1(MHT_x);
  tobs_v[2]->setQuantity2(MHT_y);
  tobs_v[2]->setSaturation(0); //Always 0 for now, need a threshold?
  tobs_v[2]->setTobID(3);
  if( MHT_x != 0 ) tobs_v[2]->setStatus1(1);
  else tobs_v[2]->setStatus1(0);
  if(MHT_y!= 0) tobs_v[2]->setStatus2(1);
  else tobs_v[2]->setStatus2(0);

  tobs_v[3] = std::make_unique<gFEXJwoJTOB>();
  tobs_v[3]->setWord(outTOB[3]);
  tobs_v[3]->setQuantity1(MST_x);
  tobs_v[3]->setQuantity2(MST_y);
  tobs_v[3]->setSaturation(0); //Always 0 for now, need a threshold?
  tobs_v[3]->setTobID(4);
  if( MST_x != 0 ) tobs_v[3]->setStatus1(1);
  else tobs_v[2]->setStatus1(0);
  if(MST_y!= 0) tobs_v[3]->setStatus2(1);
  else tobs_v[3]->setStatus2(0);


  return tobs_v;

}

void gFEXJwoJAlgo::makeFPGAC(const gTowersForward& twrsCN, const gTowersForward& twrsCP, gTowersCentral & twrsC) const {

  int rows = twrsC.size();
  int cols = twrsC[0].size();
  for( int irow = 0; irow < rows; irow++ ){ 
    for(int jcolumn = 0; jcolumn<2; jcolumn++){
      if (irow%2 == 0) {
        twrsC[irow][jcolumn] = twrsCN[irow/2][2*jcolumn];
      }
      else {
        twrsC[irow][jcolumn] = twrsCN[irow/2][2*jcolumn+1];
      }
    }
    for(int jcolumn = 2; jcolumn<6; jcolumn++){
      twrsC[irow][jcolumn] = twrsCN[irow][jcolumn+2];

    }
    for(int jcolumn = 6; jcolumn<10; jcolumn++){
      twrsC[irow][jcolumn] = twrsCP[irow][jcolumn-6];

    }
    for(int jcolumn = 10; jcolumn<cols; jcolumn++){
      if(irow%2 == 0) {
        twrsC[irow][jcolumn] = twrsCP[irow/2][2*jcolumn-16];
      }
      else {
        twrsC[irow][jcolumn] = twrsCP[irow/2][(2*jcolumn-15)];
      }
    }
  }
}



void gFEXJwoJAlgo::gBlockAB(const gTowersCentral& twrs, gTowersCentral & gBlkSum) const {

  int rows = twrs.size();
  int cols = twrs[0].size();
  for( int irow = 0; irow < rows; irow++ ){
    for(int jcolumn = 0; jcolumn<cols; jcolumn++){
      // zero jet sum here
      gBlkSum[irow][jcolumn] = 0;
      int krowUp = (irow + 1)%32;
      int krowDn = (irow - 1 +32)%32;
      if( (jcolumn == 0) || (jcolumn == 6) ) {
        //left edge case
        gBlkSum[irow][jcolumn] =
          twrs[irow][jcolumn]   + twrs[krowUp][jcolumn]   + twrs[krowDn][jcolumn]   +
          twrs[irow][jcolumn+1] + twrs[krowUp][jcolumn+1] + twrs[krowDn][jcolumn+1];
            } else if( (jcolumn == 5) || (jcolumn == 11)) {
        //  right edge case
        gBlkSum[irow][jcolumn] =
          twrs[irow][jcolumn]   + twrs[krowUp][jcolumn]   + twrs[krowDn][jcolumn]   +
          twrs[irow][jcolumn-1] + twrs[krowUp][jcolumn-1] + twrs[krowDn][jcolumn-1];
            } else{
        // normal case
        gBlkSum[irow][jcolumn] =
          twrs[irow][jcolumn]   + twrs[krowUp][jcolumn]   + twrs[krowDn][jcolumn]   +
          twrs[irow][jcolumn-1] + twrs[krowUp][jcolumn-1] + twrs[krowDn][jcolumn-1] +
          twrs[irow][jcolumn+1] + twrs[krowUp][jcolumn+1] + twrs[krowDn][jcolumn+1];
        }
        // limit result to an unsigned integer of 12 bits ( 2376 GeV) 
        if ( gBlkSum[irow][jcolumn] < 0 ){
          gBlkSum[irow][jcolumn] = 0;
        }
        if ( gBlkSum[irow][jcolumn] > 4091 ){
          gBlkSum[irow][jcolumn] = 4091;
        }  

    }
  }

}


void gFEXJwoJAlgo::metFPGA(const gTowersCentral& twrs, const gTowersCentral & gBlkSum, int gBlockthreshold,
                           float aFPGA, float bFPGA,
                           int & MHT_x, int & MHT_y,
                           int & MST_x, int & MST_y,
                           int & MET_x, int & MET_y) const {

  int rows = twrs.size();
  int cols = twrs[0].size();
  for( int irow = 0; irow < rows; irow++ ){
    for(int jcolumn = 0; jcolumn<cols; jcolumn++){
      if(gBlkSum[irow][jcolumn] > gBlockthreshold){
        MHT_x += (twrs[irow][jcolumn])*(cosLUT(irow, 5));
        MHT_y += (twrs[irow][jcolumn])*(sinLUT(irow, 5));

      }
      else{
       MST_x += (twrs[irow][jcolumn])*(cosLUT(irow, 5));
       MST_y += (twrs[irow][jcolumn])*(sinLUT(irow, 5));
      }
    }
  }
  MET_x = aFPGA * MHT_x + bFPGA * MST_x;
  MET_y = aFPGA * MHT_y + bFPGA * MST_y;
}

void gFEXJwoJAlgo::metTotal(int A_MET_x, int A_MET_y,
                            int B_MET_x, int B_MET_y,
                            int C_MET_x, int C_MET_y,
                            int & MET_x, int & MET_y, int & MET) const {

  MET_x = A_MET_x + B_MET_x;
  MET_y = A_MET_y + B_MET_y;

  if (FEXAlgoSpaceDefs::ENABLE_JWOJ_C){
    MET_x = MET_x + C_MET_x;
    MET_y = MET_y + C_MET_y;
  }

  MET = std::sqrt((MET_x * MET_x) + (MET_y * MET_y));
}


void gFEXJwoJAlgo::sumEtFPGA(const gTowersCentral& twrs, int & partial_sumEt ) const {

  int rows = twrs.size();
  int cols = twrs[0].size();

  for( int irow = 0; irow < rows; irow++ ){
    for(int jcolumn = 0; jcolumn<cols; jcolumn++){
      partial_sumEt += (twrs[irow][jcolumn] > 0 ? twrs[irow][jcolumn]: 0);
    }
  }
}

void gFEXJwoJAlgo::sumEt(int  A_sumEt, int  B_sumEt, int  C_sumEt, int & total_sumEt ) const {

  total_sumEt = A_sumEt + B_sumEt;
  if (FEXAlgoSpaceDefs::ENABLE_JWOJ_C){
    total_sumEt = total_sumEt + C_sumEt;
  }

}

//----------------------------------------------------------------------------------
// bitwise simulation of sine LUT in firmware
//----------------------------------------------------------------------------------
float gFEXJwoJAlgo::sinLUT(unsigned int phiIDX, unsigned int aw) const
{
  float c = static_cast<float>(phiIDX)/std::pow(2,aw);
  float rad = (2*M_PI) *c;
  float rsin = std::sin(rad);
  return rsin;
}

//----------------------------------------------------------------------------------
// bitwise simulation cosine LUT in firmware
//----------------------------------------------------------------------------------
float gFEXJwoJAlgo::cosLUT(unsigned int phiIDX, unsigned int aw) const
{
  float c = static_cast<float>(phiIDX)/std::pow(2,aw);
  float rad = (2*M_PI) *c;
  float rcos = std::cos(rad);
  return rcos;
}

} // namespace LVL1
