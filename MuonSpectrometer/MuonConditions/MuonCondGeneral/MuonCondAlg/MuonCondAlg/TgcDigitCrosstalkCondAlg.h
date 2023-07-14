/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_TGCDIGITCROSSTALKCONDALG_H_
#define MUONCONDALG_TGCDIGITCROSSTALKCONDALG_H_

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCondData/TgcDigitCrosstalkData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

class TgcDigitCrosstalkCondAlg : public AthReentrantAlgorithm
{
 public:
  TgcDigitCrosstalkCondAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TgcDigitCrosstalkCondAlg() = default;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual bool isReEntrant() const override { return false; }

 private:
  SG::ReadCondHandleKey<CondAttrListCollection> m_readKey{this, "ReadKey", "/TGC/DIGIT/XTALK", "SG key for TGCDIGITXTALK"};
  SG::WriteCondHandleKey<TgcDigitCrosstalkData> m_writeKey{this, "WriteKey", "TGCDigitCrosstalkData", "SG Key of TgcDigitCrosstalk"};
};

#endif   // MUONCONDALG_TGCDIGITTIMEOFFSETCONDALG_H_

