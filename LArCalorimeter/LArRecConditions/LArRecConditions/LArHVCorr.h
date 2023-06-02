/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARHVCORR_H
#define LARHVCORR_H

#include "Identifier/IdentifierHash.h"
#include "LArElecCalib/ILArHVScaleCorr.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloIdentifier/CaloCell_Base_ID.h"

#include <vector>

class LArHVCorr : public ILArHVScaleCorr {
 
  public:

   LArHVCorr()=delete;
 
   LArHVCorr(std::vector<float>&& vVec, const LArOnOffIdMapping* cabling, const CaloCell_Base_ID* caloidhelper);
   ~LArHVCorr () {};


   // retrieving HVScaleCorr using online ID  
   virtual const float& HVScaleCorr(const HWIdentifier& chid) const override final;

   // retrieving HVScaleCorr using offline ID  
   virtual const float& HVScaleCorr(const Identifier& chid) const;

   const float& HVScaleCorr_oflHash(const IdentifierHash& h) const {
     if (h<m_hvCorr.size()) //Catches also Tile Ids 
       return m_hvCorr[h]; 
     else 
       return m_noCorr;
   }


 private:
   const LArOnOffIdMapping* m_larCablingSvc;
   const CaloCell_Base_ID*  m_calo_id;

   std::vector<float>       m_hvCorr;
   const float              m_noCorr;
};

#include "AthenaKernel/CondCont.h"
CLASS_DEF( LArHVCorr, 52206080, 1)
CONDCONT_MIXED_DEF( LArHVCorr, 24667986, ILArHVScaleCorr);

#endif
