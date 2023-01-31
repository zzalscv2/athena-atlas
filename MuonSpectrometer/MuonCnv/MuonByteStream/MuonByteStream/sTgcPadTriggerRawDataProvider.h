/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAM_STGCPADTRIGGERRAWDATAPROVIDER_H
#define MUONBYTESTREAM_STGCPADTRIGGERRAWDATAPROVIDER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"

namespace Muon {
    class sTgcPadTriggerRawDataProvider : public AthReentrantAlgorithm {
    public:
        //! Constructor.
        sTgcPadTriggerRawDataProvider(const std::string &name, ISvcLocator *pSvcLocator);

        //! Initialize
        virtual StatusCode initialize() override;

        //! Execute
        virtual StatusCode execute(const EventContext &ctx) const override;

        //! Destructor
        ~sTgcPadTriggerRawDataProvider() = default;

    private:
        /// Handle for the RawDataProviderTool
        ToolHandle<Muon::IMuonRawDataProviderTool> m_rawDataTool{this, "ProviderTool",
                                                                 "Muon::PadTrig_RawDataProviderToolMT/sTgcPadTriggerRawDataProviderTool"};

    };
}  // namespace Muon

#endif
