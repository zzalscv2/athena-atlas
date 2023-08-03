/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONMM_CNVTOOLS_NSWMMTP_RAWDATAPROVIDERTOOLMT_H
#define MUONMM_CNVTOOLS_NSWMMTP_RAWDATAPROVIDERTOOLMT_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "xAODMuonRDO/NSWMMTPRDOContainer.h"
#include "MuonMM_CnvTools/INSWMMTP_ROD_Decoder.h"

namespace Muon {

  class NSWMMTP_RawDataProviderToolMT : virtual public IMuonRawDataProviderTool, public AthAlgTool 
  {
  public:
    using IMuonRawDataProviderTool::convert;
  
    NSWMMTP_RawDataProviderToolMT(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~NSWMMTP_RawDataProviderToolMT() = default;

    StatusCode initialize() override;

    // unimplemented
    StatusCode convert() const override;
    StatusCode convert(const ROBFragmentList&) const override;
    StatusCode convert(const std::vector<IdentifierHash>&) const override;
    StatusCode convert(const ROBFragmentList&, const std::vector<IdentifierHash>&) const override;

    // implemented
    StatusCode convert(const ROBFragmentList& fragments, const EventContext& ctx) const override;
    StatusCode convert(const EventContext& ctx) const override;

  private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    ToolHandle<INSWMMTP_ROD_Decoder>      m_decoder     {this, "Decoder", "Muon::NSWMMTP_ROD_Decoder/NSWMMTP_ROD_Decoder"};
    ServiceHandle<IROBDataProviderSvc>    m_robDataProvider;
    SG::WriteHandleKey<xAOD::NSWMMTPRDOContainer> m_rdoContainerKey{this, "RdoLocation", "", "NSWMMTPRDOContainer"};
  };

}  // namespace Muon

#endif
