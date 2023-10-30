/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCDIGITENERGYTHRESHOLDALG_H
#define TGCDIGITENERGYTHRESHOLDALG_H


#include <nlohmann/json.hpp>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCondData/TgcDigitThresholdData.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

class TgcDigitEnergyThreshCondAlg : public AthReentrantAlgorithm
{
 public:
    TgcDigitEnergyThreshCondAlg (const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~TgcDigitEnergyThreshCondAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual bool isReEntrant() const override { return false; }
 private:
    /// Load the threshold constants from the JSON blob
    StatusCode parseDataFromJSON(const nlohmann::json& lines,
                                 TgcDigitThresholdData& deadChannels) const;
   
    /// Use an external file to load the Jitter constants from.
    Gaudi::Property<std::string> m_readFromJSON{this, "readFromJSON", "" };
    
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyDb{this, "ReadKey", "", "SG key for Tgc energy thresholds"};
    SG::WriteCondHandleKey<TgcDigitThresholdData> m_writeKey{this, "WriteKey", "TgcEnergyThresholds", "SG Key of TGCDigit AsdPos"};
};

#endif


