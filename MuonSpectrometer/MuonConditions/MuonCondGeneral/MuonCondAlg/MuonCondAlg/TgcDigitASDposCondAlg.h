/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCDIGITASDPOSCONDALG_H
#define TGCDIGITASDPOSCONDALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCondData/TgcDigitASDposData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

class TgcDigitASDposCondAlg : public AthReentrantAlgorithm
{
 public:
  TgcDigitASDposCondAlg (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TgcDigitASDposCondAlg() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
    virtual bool isReEntrant() const override { return false; }
 private:
  SG::ReadCondHandleKey<CondAttrListCollection> m_readKey_ASDpos{this, "ReadKeyAsdPos", "/TGC/DIGIT/ASDPOS", "SG key for TGCDIGITASDPOS"};
  SG::WriteCondHandleKey<TgcDigitASDposData> m_writeKey{this, "WriteKey", "TGCDigitASDposData", "SG Key of TGCDigit AsdPos"};
};

#endif


