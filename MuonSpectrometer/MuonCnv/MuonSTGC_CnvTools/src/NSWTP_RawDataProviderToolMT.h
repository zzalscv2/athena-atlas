/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSTGC_CNVTOOLS_NSWTP_RAWDATAPROVIDERTOOLMT_H
#define MUONSTGC_CNVTOOLS_NSWTP_RAWDATAPROVIDERTOOLMT_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "xAODMuonRDO/NSWTPRDOContainer.h"
#include "MuonSTGC_CnvTools/INSWTP_ROD_Decoder.h"

namespace Muon {

class NSWTP_RawDataProviderToolMT : virtual public IMuonRawDataProviderTool, public AthAlgTool 
{
 public:
  using IMuonRawDataProviderTool::convert;
  
  NSWTP_RawDataProviderToolMT(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~NSWTP_RawDataProviderToolMT() = default;

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
  ToolHandle<INSWTP_ROD_Decoder>      m_decoder{this, "Decoder", "Muon::NSWTP_ROD_Decoder/NSWTP_ROD_Decoder"};
  ServiceHandle<IROBDataProviderSvc>    m_robDataProvider;
  SG::WriteHandleKey<xAOD::NSWTPRDOContainer> m_rdoContainerKey{this, "RdoLocation", "", "Name of of the RDO container to write to"};
};

}  // namespace Muon

#endif  // MUONSTGC_CNVTOOLS_NSWTP_RAWDATAPROVIDERTOOLMT_H
