/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAM_NSWMMTP_ROD_DECODER_H
#define MUONBYTESTREAM_NSWMMTP_ROD_DECODER_H

#include <string>
#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonMM_CnvTools/INSWMMTP_ROD_Decoder.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace Muon
{
  
  class NSWMMTP_ROD_Decoder : virtual public INSWMMTP_ROD_Decoder, public AthAlgTool 
  {
  public: 
    NSWMMTP_ROD_Decoder(const std::string& type, const std::string& name, const IInterface* parent ) ;

    // Fill the given MMTP L1A RDO container with data from the MMTP ROB
    // MMTP ROB has 3 L1A elinks (containing hits and segments in a given time window)
    // the 3 elinks are flattened in the NSWMMTP RDO
    StatusCode fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& fragment, xAOD::NSWMMTPRDOContainer& rdoContainer) const override;
    StatusCode initialize() override;

  private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

  };
  
}

#endif



