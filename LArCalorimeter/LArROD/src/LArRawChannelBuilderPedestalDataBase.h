/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/** 
 * @class LArRawChannelBuilderPedestalDataBase
 * @author Rolf Seuster
 * @brief Tool obtaining the Pedestal from the database
 */

#ifndef LARROD_LARRAWCHANNELBUILDERPEDESTALDATABASE_H
#define LARROD_LARRAWCHANNELBUILDERPEDESTALDATABASE_H

#include "LArRawChannelBuilderPedestalToolBase.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "CLHEP/Units/SystemOfUnits.h"

class LArRawChannelBuilderPedestalDataBase
: public LArRawChannelBuilderPedestalToolBase
{
 public:
  
  LArRawChannelBuilderPedestalDataBase(const std::string& type,
				       const std::string& name,
				       const IInterface* parent);
  
  bool pedestal(float& pedestal, MsgStream* pLog);
  
  StatusCode initTool();
  
 private:
  
  SG::ReadCondHandleKey<ILArPedestal> m_larPedestalKey
    { this, "LArPedestalKey", "LArPedestal", "SG key for pedestal object. Needed only if 'minADCforIterInSigma' is set (for the RMS)" };

  LArRawChannelBuilderPedestalDataBase (const LArRawChannelBuilderPedestalDataBase&);
  LArRawChannelBuilderPedestalDataBase& operator= (const LArRawChannelBuilderPedestalDataBase&);
};

#endif
