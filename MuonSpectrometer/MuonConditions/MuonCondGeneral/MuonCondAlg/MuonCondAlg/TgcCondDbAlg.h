/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_TGCCONDDBALG_H
#define MUONCONDALG_TGCCONDDBALG_H

// STL includes
#include <zlib.h>

#include <string>
#include <vector>

// Gaudi includes
#include "GaudiKernel/ServiceHandle.h"

// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCondData/TgcCondDbData.h"
#include "MuonCondSvc/MdtStringUtils.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

class TgcCondDbAlg : public AthReentrantAlgorithm {
public:
    TgcCondDbAlg(const std::string& name, ISvcLocator* svc);
    virtual ~TgcCondDbAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual bool isReEntrant() const override { return false; }

private:
    StatusCode loadDetectorStatus(EventIDRange&, std::unique_ptr<TgcCondDbData>&) const;


    Gaudi::Property<bool> m_isOnline{this, "isOnline", false};
    Gaudi::Property<bool> m_isData{this, "isData", false};
    Gaudi::Property<bool> m_isRun1{this, "isRun1", false};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    SG::WriteCondHandleKey<TgcCondDbData> m_writeKey{this, "WriteKey", "TgcCondDbData", "Key of output TGC condition data"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKey_folder_detectorStatus{this, "ReadKey", "/TGC/1/DetectorStatus",
                                                                                  "Key of input TGC condition data"};
};

#endif
