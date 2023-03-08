/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAM_MMTPL1A_ROD_DECODER_H
#define MUONBYTESTREAM_MMTPL1A_ROD_DECODER_H

#include <string>
#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonMM_CnvTools/IMMTP_L1A_ROD_Decoder.h"

namespace Muon
{
  
  class MMTP_L1A_ROD_Decoder : virtual public IMMTP_L1A_ROD_Decoder, public AthAlgTool 
  {
  public: 
    MMTP_L1A_ROD_Decoder(const std::string& type, const std::string& name, const IInterface* parent ) ;

    // Fill the given MMTP L1A RDO container with data from the MMTP ROB
    // MMTP ROB has 3 L1A elinks (containing hits and segments in a given time window)
    // the 3 elinks are flattened in the NSW_MMTP_RawDataCollection, which has a commint header and 2 DataVectors for hits and segments
    StatusCode fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& fragment, NSW_MMTP_RawDataContainer& rdo) const override;

  };
  
}

#endif



