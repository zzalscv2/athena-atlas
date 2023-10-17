/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include "L1TopoSimulationUtils/Kinematics.h"
#include "L1TopoSimulationUtils/Hyperbolic.h"
#include "L1TopoSimulationUtils/L1TopoDataTypes.h"
#include "L1TopoSimulationUtils/Trigo.h"
#include "L1TopoSimulationUtils/Expo.h"
#include "L1TopoEvent/GenericTOB.h"


unsigned int TSU::Kinematics::calcDeltaPhiBWLegacy(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2){
  int dphiB = std::abs( tob1->phi() - tob2->phi() );
  if(dphiB>64)
    dphiB = dphiB - 64;
  if(dphiB>32)
    dphiB = 64 - dphiB;

  return dphiB ;
}

unsigned int TSU::Kinematics::calcDeltaEtaBWLegacy(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  double deta = std::abs( tob1->eta() - tob2->eta() );
  return deta;
}

unsigned int TSU::Kinematics::calcInvMassBWLegacy(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2){

  auto bit_cosheta = TSU::L1TopoDataTypes<19,7>(TSU::Hyperbolic::Coshleg.at(std::abs(tob1->eta() - tob2->eta())));
  //In case of EM objects / jets / taus the phi angle goes between 0 and 64 while muons are between -32 and 32, applying a shift to keep delta-phi in the allowed range. 
  int phi_tob1 = tob1->phi();
  int phi_tob2 = tob2->phi();
  //those cases should happen only in mixed EM/jets/tau plus mu triggers, if both phi's are in [0,2pi] will not get in
  if ( std::abs(phi_tob1-phi_tob2)>=64 )
    {
      if(phi_tob1 >= 32) phi_tob1 = phi_tob1-64;
      if(phi_tob2 >= 32) phi_tob2 = phi_tob2-64;
    }
  auto bit_cosphi = TSU::L1TopoDataTypes<9,7>(TSU::Trigo::Cosleg.at(std::abs( phi_tob1 - phi_tob2 )));
  TSU::L1TopoDataTypes<11,0> bit_Et1(tob1->Et());
  TSU::L1TopoDataTypes<11,0> bit_Et2(tob2->Et());
  auto bit_invmass2 = bit_Et1*bit_Et2*(bit_cosheta - bit_cosphi)*2;
  return static_cast<unsigned>(bit_invmass2) ;
}

unsigned int TSU::Kinematics::calcTMassBWLegacy(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  auto bit_cosphi = TSU::L1TopoDataTypes<9,7>(TSU::Trigo::Cosleg.at(std::abs(tob1->phi() - tob2->phi())));
  TSU::L1TopoDataTypes<11,0> bit_Et1(tob1->Et());
  TSU::L1TopoDataTypes<11,0> bit_Et2(tob2->Et());
  TSU::L1TopoDataTypes<22,0> bit_tmass2 = 2*bit_Et1*bit_Et2*(1.  - bit_cosphi);
  return static_cast<unsigned>(bit_tmass2) ;
  // end bitwise implementation
}

unsigned int TSU::Kinematics::calcDeltaR2BWLegacy(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  int detaB = std::abs( tob1->eta() - tob2->eta() );
  int dphiB = std::abs( tob1->phi() - tob2->phi() );
  if(dphiB>64) //Probably same error here as in DeltaPhiBW. Check
    dphiB = dphiB - 64;
  if(dphiB>32)
    dphiB = 64 - dphiB;

  unsigned int bit_dr2 = dphiB*dphiB + detaB*detaB;
  return bit_dr2;
}

float TSU::Kinematics::calcCosLegacy(unsigned phi){
  return static_cast<float>(TSU::L1TopoDataTypes<9,7>(TSU::Trigo::Cosleg.at(phi)));
}

float TSU::Kinematics::calcSinLegacy(unsigned phi){
  return static_cast<float>(TSU::L1TopoDataTypes<9,7>(TSU::Trigo::Sinleg.at(phi)));
}

unsigned int TSU::Kinematics::calcDeltaPhiBW(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2){
  int dphiB = std::abs( tob1->phi() - tob2->phi() );
  if(dphiB>128)
    dphiB = dphiB - 128;
  if(dphiB>64)
    dphiB = 128 - dphiB;

  return dphiB ;
}

unsigned int TSU::Kinematics::calcDeltaEtaBW(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  double deta = std::abs( tob1->eta() - tob2->eta() );
  return deta;
}

unsigned int TSU::Kinematics::calcInvMassBW(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2){

  auto bit_cosheta = TSU::L1TopoDataTypes<25,10>(TSU::Hyperbolic::Cosh.at(std::abs(tob1->eta() - tob2->eta())));
  //In case of EM objects / jets / taus the phi angle goes between 0 and 128 while muons are between -128 and 128, applying a shift to keep delta-phi in the allowed range. 
  //those cases should happen only in mixed EM/jets/tau plus mu triggers, if both phi's are in [0,2pi] will not get in
  int phi_tob1 = tob1->phi();
  int phi_tob2 = tob2->phi();
  if ( std::abs(phi_tob1-phi_tob2)>=128 )
    {
      if(phi_tob1 >= 64) phi_tob1 = phi_tob1-128;
      if(phi_tob2 >= 64) phi_tob2 = phi_tob2-128;
    }
  auto bit_cosphi = TSU::L1TopoDataTypes<12,10>(TSU::Trigo::Cos.at(std::abs( phi_tob1 - phi_tob2 )));
  TSU::L1TopoDataTypes<15,0> bit_Et1(tob1->Et());
  TSU::L1TopoDataTypes<15,0> bit_Et2(tob2->Et());
  auto bit_invmass2 = bit_Et1*bit_Et2*(bit_cosheta - bit_cosphi)*2;

  auto u_invmass2 = static_cast<unsigned long long>(bit_invmass2);

  if (u_invmass2 > std::numeric_limits<int>::max())
    {return std::numeric_limits<int>::max();}
  else
    {return u_invmass2;}
}

unsigned int TSU::Kinematics::calcTMassBW(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  auto bit_cosphi = TSU::L1TopoDataTypes<12,10>(TSU::Trigo::Cos.at(std::abs(tob1->phi() - tob2->phi())));
  TSU::L1TopoDataTypes<11,0> bit_Et1(tob1->Et());
  TSU::L1TopoDataTypes<11,0> bit_Et2(tob2->Et());
  TSU::L1TopoDataTypes<22,0> bit_tmass2 = 2*bit_Et1*bit_Et2*(1.  - bit_cosphi);
  return static_cast<unsigned>(bit_tmass2) ;
  // end bitwise implementation
}

unsigned int TSU::Kinematics::calcDeltaR2BW(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  int detaB = std::abs( tob1->eta() - tob2->eta() );
  int dphiB = std::abs( tob1->phi() - tob2->phi() );
  if(dphiB>128) //Probably same error here as in DeltaPhiBW. Check
    dphiB = dphiB - 128;
  if(dphiB>64)
    dphiB = 128 - dphiB;

  // Use the same granularity for eta and phi (0.025) in dR calculation (need to multiply dphiB*2)
  // Return (40*dR)^2
  dphiB = 2*dphiB;
  unsigned int bit_dr2 = dphiB*dphiB + detaB*detaB;
  return bit_dr2;
}

unsigned long TSU::Kinematics::quadraticSumBW(int i1, int i2){
  unsigned int ui1 = i1, ui2=i2;
  unsigned int a = ui1*ui1 + ui2*ui2; 

  unsigned int result=0;
  int left=0, right=0; 
  unsigned int r=0;
  int sign=0;
  
  //The values for halflength and bitmask enforce the
  //bitwise overflow limit, not the precision of the chosen C++ parameters
  int halflength = 16; //max 16
  unsigned int bitmask = 0b11111111111111111111111111111111; //32 bits
  bitmask >>= (32 - halflength*2); //does nothing unless halflength changes
  
  for(int i = 0; i < halflength; i++){ //16-->4
    right = 1 + (sign<<1) + (result<<2);
    left = (r<<2) + (a >> (2*halflength-2));
    a <<= 2;
    a = a & bitmask;
    r = sign ? (left + right) : (left - right);
    sign = ((r & (1 << (halflength+1) ))  > 0) ? 1 : 0;
    result <<= 1;
    if(sign==0) result += 1;
  }
  return result;
}

unsigned int TSU::Kinematics::calcXi1(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2, unsigned ptShift, unsigned ptScale) {
  //firmware: 19 bits unsigned + 1 sign bit due to L1TopoDataTypes assuming to be signed
  TSU::L1TopoDataTypes<20,0> bit_Et1(static_cast<unsigned>(ptScale*tob1->Et()+ptShift));
  TSU::L1TopoDataTypes<20,0> bit_Et2(static_cast<unsigned>(ptScale*tob2->Et()+ptShift));
  
  //10 bits *unsigned* integer + 10 bits decimals + 1 sign bit since L1TopoDataTypes always assumes to represent signed values!
  auto bit_eeta1 = TSU::L1TopoDataTypes<21,10>(TSU::Expo::E.at(tob1->eta()));
  auto bit_eeta2 = TSU::L1TopoDataTypes<21,10>(TSU::Expo::E.at(tob2->eta()));

  auto xi_bit = bit_Et1*bit_eeta1+bit_Et2*bit_eeta2;
  
  return static_cast<unsigned>(xi_bit);
}

unsigned int TSU::Kinematics::calcXi2(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2, unsigned ptShift, unsigned ptScale) {
  TSU::L1TopoDataTypes<20,0> bit_Et1(static_cast<unsigned>(ptScale*tob1->Et()+ptShift));
  TSU::L1TopoDataTypes<20,0> bit_Et2(static_cast<unsigned>(ptScale*tob2->Et()+ptShift));
  auto bit_eeta1 = TSU::L1TopoDataTypes<21,10>(TSU::Expo::E.at(-tob1->eta()));
  auto bit_eeta2 = TSU::L1TopoDataTypes<21,10>(TSU::Expo::E.at(-tob2->eta()));

  auto xi_bit = bit_Et1*bit_eeta1+bit_Et2*bit_eeta2;

  return static_cast<unsigned>(xi_bit);
}

float TSU::Kinematics::calcCos(unsigned phi){
  return static_cast<float>(TSU::L1TopoDataTypes<12,10>(TSU::Trigo::Cos.at(phi)));
}

float TSU::Kinematics::calcSin(unsigned phi){
  return static_cast<float>(TSU::L1TopoDataTypes<12,10>(TSU::Trigo::Sin.at(phi)));
}

/*------------------------------------------ NON-BITWISE --------------------------------------------------*/

unsigned int TSU::Kinematics::calcDeltaPhiLegacy(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  double dphi = std::fabs( tob1->phiDouble() - tob2->phiDouble() );
  if(dphi>M_PI)
    dphi = 2*M_PI - dphi;

  return round( 10 * dphi );
}

unsigned int TSU::Kinematics::calcDeltaEtaLegacy(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  double deta = std::fabs( tob1->etaDouble() - tob2->etaDouble() );
  return round( 10 * deta );
}

unsigned int TSU::Kinematics::calcDeltaR2Legacy(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  double deta = ( tob1->etaDouble() - tob2->etaDouble() );
  double dphi = std::fabs( tob1->phiDouble() - tob2->phiDouble() );
  if(dphi>M_PI)
    dphi = 2*M_PI - dphi;

  return round ( 100 * ((dphi)*(dphi) + (deta)*(deta) )) ;
}

unsigned int TSU::Kinematics::calcDeltaPhi(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  double dphi = std::fabs( tob1->phiDouble() - tob2->phiDouble() );
  if(dphi>M_PI)
    dphi = 2*M_PI - dphi;

  return round( 20 * dphi );
}

unsigned int TSU::Kinematics::calcDeltaEta(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  double deta = std::fabs( tob1->etaDouble() - tob2->etaDouble() );
  return round( 40 * deta );
}

unsigned int TSU::Kinematics::calcInvMass(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  double deta = std::fabs( tob1->etaDouble() - tob2->etaDouble() );
  double dphi = std::fabs( tob1->phiDouble() - tob2->phiDouble() );
  if(dphi>M_PI)
    dphi = 2*M_PI - dphi;

  double cosheta = cosh ( deta);
  double cosphi = cos ( dphi);
  double invmass2 = 2*tob1->Et()*tob2->Et()*(cosheta - cosphi);
  return round( invmass2 );
}

unsigned int TSU::Kinematics::calcTMass(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  double dphi = std::fabs( tob1->phiDouble() - tob2->phiDouble() );
  if(dphi>M_PI)
    dphi = 2*M_PI - dphi;
      
  double cosphi = cos ( dphi);
  double tmass2 = 2*tob1->Et()*tob2->Et()*(1 - cosphi);
  return round( tmass2 );
}

unsigned int TSU::Kinematics::calcDeltaR2(const TCS::GenericTOB* tob1, const TCS::GenericTOB* tob2) {
  double deta = ( tob1->etaDouble() - tob2->etaDouble() );
  double dphi = std::fabs( tob1->phiDouble() - tob2->phiDouble() );
  if(dphi>M_PI)
    dphi = 2*M_PI - dphi;

  // Return (40*dR)^2 consistent with BW calculation
  return round ( 40*40 * ((dphi)*(dphi) + (deta)*(deta) )) ;
}
