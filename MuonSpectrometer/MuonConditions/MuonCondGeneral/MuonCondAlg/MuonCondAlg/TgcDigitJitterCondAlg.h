/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_TGCDIGITJITTERCONDALG_H
#define MUONCONDALG_TGCDIGITJITTERCONDALG_H

// Gaudi includes
#include <nlohmann/json.hpp>

// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCondData/TgcDigitJitterData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"



class TgcDigitJitterCondAlg : public AthReentrantAlgorithm {
public:
    TgcDigitJitterCondAlg(const std::string& name, ISvcLocator* svc);
    virtual ~TgcDigitJitterCondAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual bool isReEntrant() const override { return false; }

private:
    /// Load the Jitter constants from the JSON format
    StatusCode parseDataFromJSON(const nlohmann::json& lines,
                                 TgcDigitJitterData& jitterChannels) const;
   
    /// Use an external JSON file to load the Jitter constants from
    Gaudi::Property<std::string> m_readFromJSON{this, "readFromJSON", "" };

    SG::WriteCondHandleKey<TgcDigitJitterData> m_writeKey{this, "WriteKey", "TgcJitterData", "Key of output TGC condition data"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyDb{this, "ReadKey", "", "Key of input TGC condition data"};
};

#endif
