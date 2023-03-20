/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-
#ifndef LARCOOLCONDITIONS_LAROFCSC_H
#define LARCOOLCONDITIONS_LAROFCSC_H

#include "LArElecCalib/ILArOFC.h" 
#include "LArCOOLConditions/LArCondSuperCellBase.h"
#include "Identifier/IdentifierHash.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArElecCalib/LArCalibErrorCode.h"
#include <vector>

class CondAttrListCollection;

class LArOFCSC: public ILArOFC, public LArCondSuperCellBase {

 private:
  LArOFCSC();

 public:   
  typedef ILArOFC::OFCRef_t OFCRef_t;
  enum {ERRORCODE = LArElecCalib::ERRORCODE};

  LArOFCSC(const CondAttrListCollection* attrList);
  virtual ~LArOFCSC( );

  bool good() const { return m_isInitialized && m_nChannels>0; }

  

  // retrieving coefficients using online ID
  
  virtual OFCRef_t OFC_a(const HWIdentifier&  CellID,
                         int gain,
                         int tbin=0) const ;

  virtual OFCRef_t OFC_b(const HWIdentifier&  CellID,
                         int gain,
                         int tbin=0) const ;
  


  OFCRef_t OFC_a(const IdentifierHash& hs,int gain) const {
    const float* pStart=m_pOFCa[gain]+(hs*m_nSamples);
    if (*pStart==ERRORCODE) 
      return OFCRef_t(NULL,NULL);
    else
      return OFCRef_t(pStart,pStart+m_nSamples);
  }

  OFCRef_t OFC_b(const IdentifierHash& hs,int gain) const {
    const float* pStart=m_pOFCb[gain]+(hs*m_nSamples);
    if (*pStart==ERRORCODE) 
      return OFCRef_t(NULL,NULL);
    else
      return OFCRef_t(pStart,pStart+m_nSamples);
  }


  // retrieving time offset using online/offline ID

  virtual  float timeOffset(const HWIdentifier&  CellID, int gain) const;

  //For the TB / cosmic case: retrieve the number of time-bins (aka "phases")
  virtual unsigned nTimeBins(const HWIdentifier&  CellID, int gain) const;
 
  //For the TB / cosmic case: retrieve the witdth of the time bin (default 24 bins in 25 ns)
  virtual float timeBinWidth(const HWIdentifier&  CellID, int gain) const;


 private: 
  std::vector<const float*> m_pOFCa;
  std::vector<const float*> m_pOFCb;
  std::vector<const float*> m_pTimeOffset;

  unsigned m_nChannels;
  unsigned m_nSamples;
};  
  
#include "AthenaKernel/CondCont.h"
CLASS_DEF( LArOFCSC, 12142771, 1) 
CONDCONT_DEF( LArOFCSC, 41952481, ILArOFC );
 

#endif 
