/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMM_CNVTOOLS_INSWMMTP_ROD_DECODER_H
#define MUONMM_CNVTOOLS_INSWMMTP_ROD_DECODER_H

#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/IAlgTool.h"

#include "xAODMuonRDO/NSWMMTPRDOContainer.h"

namespace Muon {

  // IAlgTool which facilitates conversion from MM Trigger Processor L1A ROBFragments to RDO
  class INSWMMTP_ROD_Decoder : virtual public IAlgTool {
  public:
    virtual ~INSWMMTP_ROD_Decoder() = default;
    
    DeclareInterfaceID(Muon::INSWMMTP_ROD_Decoder, 1, 0);
    
    // Fill the given MMTP L1A RDO container with data from the MMTP ROB
    // MMTP ROB has 3 L1A elinks (containing hits and segments in a given time window)
    // the 3 elinks are flattened in the collection
    virtual StatusCode fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& fragment, xAOD::NSWMMTPRDOContainer& rdo) const = 0;
  };
}
#endif 
