/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

#ifndef LARCOOLCONDITIONS_LARAUTOCORRSUPERCELL_H
#define LARCOOLCONDITIONS_LARAUTOCORRSUPERCELL_H

#include "LArElecCalib/ILArAutoCorr.h" 
#include "LArCOOLConditions/LArCondSuperCellBase.h"
#include "LArCOOLConditions/LArAutoCorrBlob.h"

class CondAttrListCollection;

class LArAutoCorrSC: 
  public ILArAutoCorr, public LArCondSuperCellBase, public LArAutoCorrBlob
{  
 private:
  LArAutoCorrSC();

 public:  
  LArAutoCorrSC(const CondAttrListCollection* attrList);
  virtual ~LArAutoCorrSC( );

  bool good() const { return m_isInitialized && m_nChannels>0; }

  virtual  AutoCorrRef_t autoCorr(const HWIdentifier&  CellID, int gain) const ;  
  virtual  AutoCorrRef_t autoCorr(const Identifier&  CellID, int gain) const;
  
};

CLASS_DEF( LArAutoCorrSC , 219258685, 1 )
//ConditionsContainer clid for athenaMT
#include "AthenaKernel/CondCont.h"
CLASS_DEF( CondCont<LArAutoCorrSC> , 169266589 , 1 )
#include "SGTools/BaseInfo.h"
SG_BASE( CondCont<LArAutoCorrSC>, CondContBase );


#endif 
