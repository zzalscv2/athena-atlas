/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCOOLConditions/LArHVScaleCorrSC.h"

//const float LArHVScaleCorrSC::errorcode=ILArHVScaleCorr::ERRORCODE;

LArHVScaleCorrSC::LArHVScaleCorrSC()
  : LArCondSuperCellBase ("LArHVScaleCorrSC")
{}

LArHVScaleCorrSC::~LArHVScaleCorrSC() {}


LArHVScaleCorrSC::LArHVScaleCorrSC(const CondAttrListCollection* attrList)
  : LArCondSuperCellBase ("LArHVScaleCorrSC")
{
  if (initializeBase().isFailure()) return;
 
  readBlob(attrList,"HVScaleCorr",msg());

  if (m_pValues.size()!=1) {
    ATH_MSG_ERROR( "Found unexpected number of gains (" << m_pValues.size() <<"). Expected exactly one gain." );
  }

  return;
}


const float& LArHVScaleCorrSC::HVScaleCorr(const HWIdentifier& hwid) const {
  const IdentifierHash hash=m_scOnlineID->channel_Hash(hwid);
  return this->getDataByHash(hash, 0);
}

