/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSTGC_CNVTOOLS_NSTTP_ROD_DECODER_H
#define MUONSTGC_CNVTOOLS_NSTTP_ROD_DECODER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonSTGC_CnvTools/INSWTP_ROD_Decoder.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace Muon {
class NSWTP_ROD_Decoder : public INSWTP_ROD_Decoder, public AthAlgTool 
{

 public:
  NSWTP_ROD_Decoder(const std::string& type, const std::string& name, const IInterface* parent);

  // Convert the given ROBFragment to an NSW_NSWTPgerData object, and store it
  // in the RDO container at the appropriate hash ID, if no collection is found
  // at that hash ID.
  StatusCode fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& fragment, xAOD::NSWTPRDOContainer& rdoContainer) const override;
  StatusCode initialize() override;
  

  private:
   ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

};
}  // namespace Muon

#endif  // MUONSTGC_CNVTOOLS_PADTRIG_ROD_DECODER_H
