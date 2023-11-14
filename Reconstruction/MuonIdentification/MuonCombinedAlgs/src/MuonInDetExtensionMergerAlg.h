/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCOMBINEDALGS_MUONINDETEXTENSIONMERGERALG_H
#define MUONCOMBINEDALGS_MUONINDETEXTENSIONMERGERALG_H



#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonCombinedEvent/InDetCandidateCollection.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"

/// The MuonInDetExtensionMergerAlg merges several InDetCandidate collections and 
/// dumps them into a single container
class MuonInDetExtensionMergerAlg : public AthReentrantAlgorithm {
public:
    MuonInDetExtensionMergerAlg(const std::string& name, ISvcLocator* pSvcLocator);
    ~MuonInDetExtensionMergerAlg() = default;

    StatusCode initialize() override;
    StatusCode execute(const EventContext& ctx) const override;

private:
    SG::ReadHandleKeyArray<InDetCandidateCollection> m_inputCandidates{this, "ToMerge", {"InDetCandidatesStausPromp", "InDetCandidatesStausLRT"}};
    ///Output Key
    SG::WriteHandleKey<InDetCandidateCollection> m_writeKey{this, "ToWrite", "InDetCandidatesStaus"};
   
};

#endif
