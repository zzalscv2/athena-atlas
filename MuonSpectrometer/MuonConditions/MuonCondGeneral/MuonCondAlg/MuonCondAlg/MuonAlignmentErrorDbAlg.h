/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
   MuonAlignmentErrorDbAlg reads raw condition data and writes derived condition data (MuonAlignmentErrorData) to the condition store
*/

#ifndef MUONCONDSVC_MUONALIGNMENTERRORDBALG_H
#define MUONCONDSVC_MUONALIGNMENTERRORDBALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonAlignmentData/MuonAlignmentErrorData.h"

class MuonAlignmentErrorDbAlg : public AthReentrantAlgorithm {
public:
    MuonAlignmentErrorDbAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MuonAlignmentErrorDbAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    virtual bool isReEntrant() const override { return false; }

private:
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKey{this, "ReadKey", "/MUONALIGN/ERRS",
                                                            "Key of input muon alignment error condition data"};
    SG::WriteCondHandleKey<MuonAlignmentErrorData> m_writeKey{this, "WriteKey", "MuonAlignmentErrorData",
                                                              "Key of output muon alignment error condition data"};
};

#endif
