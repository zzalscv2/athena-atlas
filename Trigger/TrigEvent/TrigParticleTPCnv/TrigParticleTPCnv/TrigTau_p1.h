/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigParticleTPCnv
 * @Class  : TrigTau_p1
 *
 * @brief persistent partner for TrigTau
 *
 * @author Andrew Hamilton  <Andrew.Hamilton@cern.ch>  - U. Geneva
 * @author Francesca Bucci  <f.bucci@cern.ch>          - U. Geneva
 *
 * File and Version Information:
 * $Id: TrigTau_p1.h,v 1.2 2009-04-01 22:13:31 salvator Exp $
 **********************************************************************************/
#ifndef TRIGPARTICLETPCNV_TRIGTAU_P1_H
#define TRIGPARTICLETPCNV_TRIGTAU_P1_H

#include "AthenaPoolUtilities/TPObjRef.h"

class TrigTau_p1
{
 public:
  
  TrigTau_p1() {}
  friend class TrigTauCnv_p1;
  
 private:

  
  int   m_roiID;
  float m_Zvtx; 
  float m_err_eta;
  float m_err_phi;
  float m_err_Zvtx;
  float m_err_Pt;
  float m_etCalibCluster;
  int   m_nMatchedTracks;

  //the P4PtEtaPhiM base class
  TPObjRef m_p4PtEtaPhiM;

};

#endif
