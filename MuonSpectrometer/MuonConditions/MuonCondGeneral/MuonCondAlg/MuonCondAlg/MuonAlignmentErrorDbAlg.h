/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
   MuonAlignmentErrorDbAlg reads raw condition data and writes derived condition data (MuonAlignmentErrorData) to the condition store
*/

#ifndef MUONCONDSVC_MUONALIGNMENTERRORDBALG_H
#define MUONCONDSVC_MUONALIGNMENTERRORDBALG_H

#include <GaudiKernel/EventIDRange.h>
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonAlignmentData/MuonAlignmentErrorData.h"

class MuonAlignmentErrorDbAlg : public AthReentrantAlgorithm {
public:

    MuonAlignmentErrorDbAlg(const std::string& name, ISvcLocator* pSvcLocator);

    ~MuonAlignmentErrorDbAlg() override = default;

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;
    bool isReEntrant() const override { return false; }

private:
    std::tuple<std::string, EventIDRange> getDbClobContent(const EventContext& ctx) const;
    std::tuple<std::string, EventIDRange> getFileClobContent() const;

    SG::ReadCondHandleKey<CondAttrListCollection> m_readKey{this, "ReadKey", "/MUONALIGN/ERRS",
                                                            "Key of input muon alignment error condition data"};
    SG::WriteCondHandleKey<MuonAlignmentErrorData> m_writeKey{this, "WriteKey", "MuonAlignmentErrorData",
                                                              "Key of output muon alignment error condition data"};
    Gaudi::Property<std::string> m_clobFileOverride{this, "clobFileOverride", "",
        "Set this to the location of a CLOB file to override the DB setting"};
};

#endif
