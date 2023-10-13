/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCSC_CNVTOOLS_CscRdoToCscPrepDataToolMT_H
#define MUONCSC_CNVTOOLS_CscRdoToCscPrepDataToolMT_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "CSCcabling/CSCcablingSvc.h"
#include "CscCalibTools/ICscCalibTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonCSC_CnvTools/ICSC_RDO_Decoder.h"
#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/CscStripPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonPrepRawData/MuonPrepDataCollection_Cache.h"
#include "StoreGate/ReadCondHandleKey.h"

class CscRawDataContainer;

////////////////////////////////////////////////////////////////////////////////////////
/// algorithm to decode RDO into CscStripPrepData
/// get the RDO container from Storegate
/// loop over the RDO
/// Decode RDO into PrepRawData
/// loop over the PrepRawData and build the PrepRawData container
/// store the PrepRawData container in StoreGate
////////////////////////////////////////////////////////////////////////////////////////

namespace Muon {

    /// This class is only used in a single-thread mode as CscRdoToCscPrepDataToolMT has the
    /// equivalent functions defined for a thread-safe setup
    class CscRdoToCscPrepDataToolMT : public extends<AthAlgTool, IMuonRdoToPrepDataTool> {
    public:
        CscRdoToCscPrepDataToolMT(const std::string& type, const std::string& name, const IInterface* parent);

        virtual ~CscRdoToCscPrepDataToolMT() = default;

        virtual StatusCode initialize() override;
        // debugging
        virtual void printInputRdo(const EventContext& ctx) const override;

        virtual StatusCode decode(const EventContext& ctx, std::vector<IdentifierHash>& givenIdhs, std::vector<IdentifierHash>& decodedIdhs) const override;
        virtual StatusCode decode(const EventContext& ctx, const std::vector<uint32_t>& robIDs) const override;
        virtual StatusCode provideEmptyContainer(const EventContext& ctx) const override;
        
        virtual void printPrepData(const EventContext& ctx) const override;

    protected:
        void printPrepDataImpl(const Muon::CscStripPrepDataContainer* outputCollection) const;

        StatusCode decodeImpl(Muon::CscStripPrepDataContainer* outputCollection, const CscRawDataContainer* rdoContainer,
                              IdentifierHash givenHashId, std::vector<IdentifierHash>& decodedIdhs) const;
        StatusCode decodeImpl(Muon::CscStripPrepDataContainer* outputCollection, const CscRawDataContainer* rdoContainer,
                              std::vector<IdentifierHash>& decodedIdhs) const;


        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muDetMgrKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                         "Key of input MuonDetectorManager condition data"};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        /// CscStripPrepRawData containers
        SG::WriteHandleKey<Muon::CscStripPrepDataContainer> m_outputCollectionKey{this, "OutputCollection", "CSC_Measurements",
                                                                                 "Muon::CscStripPrepDataContainer to record"};

        SG::ReadHandleKey<CscRawDataContainer> m_rdoContainerKey{this, "RDOContainer", "CSCRDO", "CscRawDataContainer to retrieve"};

        /// This is the key for the cache for the CSC PRD containers, can be empty
        SG::UpdateHandleKey<CscStripPrepDataCollection_Cache> m_prdContainerCacheKey{this, "CscStripPrdContainerCacheKey", "" ,
                                                                                     "Optional external cache for the CSC RDO container"};
  

        /// CSC Calibration tools
        ToolHandle<ICscCalibTool> m_cscCalibTool{this, "CscCalibTool", "CscCalibTool/CscCalibTool"};
        ToolHandle<ICSC_RDO_Decoder> m_cscRdoDecoderTool{this, "CscRdoDecoderTool", "Muon::CscRDO_Decoder/CscRDO_Decoder"};

        ServiceHandle<CSCcablingSvc> m_cabling{this, "CablingSvc", "CSCcablingSvc"};
        /// Identifier hash offset
        Gaudi::Property<int> m_cscOffset{this, "CSCHashIdOffset", 22000};

        Gaudi::Property<bool> m_decodeData{this, "DecodeData", true};  //!< toggle on/off the decoding of CSC RDO into CscStripPrepData
    };
}  // namespace Muon
#endif  /// MUONCSC_CNVTOOL_CSCRDOTOCSCPREPDATA_H
