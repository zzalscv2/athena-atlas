/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1TGC/TGCRPhiCoincidenceMatrix.h"

#include <iostream>
#include <cstdlib>

#include "TrigT1TGC/TGCRPhiCoincidenceOut.h"
#include "TrigT1TGC/BigWheelCoincidenceLUT.h"
#include "TrigT1TGC/TGCSectorLogic.h"

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IMessageSvc.h"

namespace LVL1TGCTrigger {

void TGCRPhiCoincidenceMatrix::inputR(int rIn, int dRIn, int ptRIn)
{
  m_r=rIn;
  m_dR=dRIn;
  m_ptR=ptRIn;
  
#ifdef TGCDEBUG
  std::cout <<"LVL1TGCTrigger::TGCRPhiCoincidenceMatrix  "
    	    <<"inputR r=" <<m_r <<" dR=" <<m_dR <<" H/L=" <<m_ptR <<std::endl;
#endif
}

void TGCRPhiCoincidenceMatrix::inputPhi(int phiIn, int dPhiIn, int ptPhiIn)
{
  if(m_nPhiHit<MaxNPhiHit){
    m_phi[m_nPhiHit]=phiIn;
    m_dPhi[m_nPhiHit]=dPhiIn;
    m_ptPhi[m_nPhiHit]=ptPhiIn;

#ifdef TGCDEBUG
  std::cout <<"LVL1TGCTrigger::TGCRPhiCoincidenceMatrix  "
            << "inputPhi phi" << m_nPhiHit << "="<< m_phi[m_nPhiHit] 
	    << " dPhi=" << m_dPhi[m_nPhiHit] << " H/L=" << m_ptPhi[m_nPhiHit] 
	    << std::endl;
#endif
    
    m_nPhiHit++;
  }
}

void TGCRPhiCoincidenceMatrix::clear()
{
  m_SSCId=0;
  m_r=m_dR=m_ptR=0;

  m_nPhiHit=0;
  int i;
  for( i=0; i<MaxNPhiHit; i+=1)
    m_phi[i]=m_dPhi[i]=m_ptPhi[i]=0;
}

TGCRPhiCoincidenceOut* TGCRPhiCoincidenceMatrix::doCoincidence()
{
  TGCRPhiCoincidenceOut* out = new TGCRPhiCoincidenceOut;
  out->clear();

  if(m_nPhiHit ==0)  return out;

  out->setIdSSC(m_SSCId);

  int j0 = -1;
  int ptMax = 1;
  for( int j=m_nPhiHit-1; j>=0; j-=1){     // left half-SSC has priority when both output same pT
    int subsector;
    int chargeOut = 2;
    int CoincidenceTypeOut=-1;

    if(m_sectorLogic->getRegion()==Endcap){
      subsector = 4*(2*m_SSCId+m_r-1)+m_phi[j];
    } else {
      subsector = 4*(2*m_SSCId+m_r)+m_phi[j];
    }

    // calculate pT of muon candidate
    int type = m_lut->getMapType(m_ptR, m_ptPhi[j]);
    int pt = m_lut->test(m_sideId, m_sectorLogic->getOctantID(), m_sectorLogic->getModuleID(),
                         subsector,type,m_dR,m_dPhi[j]);
    uint8_t ptOut = std::abs(pt);   // 0 is no candidate.
    chargeOut = pt<0 ? 0:1;
    // the charge is inverted on the C-side.
    chargeOut = m_sideId == 0 ? chargeOut : !chargeOut; 

    CoincidenceTypeOut=(type==0);

    // Trigger Out (only pT>0 candidate)
    if(ptOut >= ptMax) {
      ptMax = ptOut;
      out->clear();    
      out->setIdSSC(m_SSCId);
      out->setpT(ptMax);
      out->setR(m_r);
      out->setPhi(m_phi[j]);
      out->setDR(m_dR);
      out->setDPhi(m_dPhi[j]);
      out->setRoI(subsector);
      out->setCharge(chargeOut);
      out->setCoincidenceType(CoincidenceTypeOut);
      j0 = j;
    }
  }

  if (tgcArgs()->MSGLEVEL() <= MSG::DEBUG){
    IMessageSvc* msgSvc = 0;
    ISvcLocator* svcLocator = Gaudi::svcLocator();
    if (svcLocator->service("MessageSvc", msgSvc) != StatusCode::FAILURE) {
      MsgStream log(msgSvc, "LVL1TGCTrigger::TGCRPhiCoincidenceMatrix");
      if (j0>0) {
	log << MSG::DEBUG << " Trigger Out : "
	      << " pt =" << ptMax+1 << " R=" << m_r << " Phi=" << m_phi[j0]
	      << " ptR=" << m_ptR << " dR=" << m_dR 
	      << " ptPhi=" << m_ptPhi[j0] << " dPhi=" << m_dPhi[j0] 
	      << endmsg;
      } else {
	log << MSG::DEBUG << "NO Trigger Out : " << endmsg;
      }
    }
  }
  
  //m_matrixOut = out;  
  return out;
}

void TGCRPhiCoincidenceMatrix::setCoincidenceLUT(std::shared_ptr<const LVL1TGC::BigWheelCoincidenceLUT> lut) {
  this->m_lut = lut;
}

TGCRPhiCoincidenceMatrix::TGCRPhiCoincidenceMatrix(const TGCArguments* tgcargs,const TGCSectorLogic* sL)
  : m_sectorLogic(sL),
    m_matrixOut(0), m_lut(0),
    m_nPhiHit(0), m_SSCId(0), m_r(0), m_dR(0), m_ptR(0), m_sideId(0), m_tgcArgs(tgcargs)
{
  for (int i=0; i<MaxNPhiHit; i++) {
    m_phi[i]=0;
    m_dPhi[i]=0;
    m_ptPhi[i]=0;
  }
}

TGCRPhiCoincidenceMatrix::~TGCRPhiCoincidenceMatrix()
{
  m_matrixOut=0;
}

TGCRPhiCoincidenceMatrix& TGCRPhiCoincidenceMatrix::operator=(const TGCRPhiCoincidenceMatrix& right)
{
  if (this != &right){
    m_sideId = right.m_sideId;
    m_tgcArgs = right.m_tgcArgs;
    m_sectorLogic = right.m_sectorLogic;
    delete m_matrixOut;
    m_matrixOut =0;
    m_nPhiHit = 0;
    m_SSCId   = 0;
    m_r       = 0;
    m_dR      = 0;
    m_ptR     = 0;
    for (int i=0; i<MaxNPhiHit; i++) {
      m_phi[i]=0;
      m_dPhi[i]=0;
      m_ptPhi[i]=0;
    }
  }
  return *this;
}

} //end of namespace bracket
