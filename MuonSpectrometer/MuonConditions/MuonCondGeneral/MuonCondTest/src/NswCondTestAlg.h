/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NswCondTestAlg_H
#define NswCondTestAlg_H

// STL
#include <chrono>
#include <string>

// Athena
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonCondData/NswCalibDbThresholdData.h"
#include "MuonCondData/NswCalibDbTimeChargeData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonCondData/Defs.h"
#include "MuonCondData/NswT0Data.h"
// Forward declarations
class ISvcLocator;
class StatusCode;

class NswCondTestAlg : public AthReentrantAlgorithm {
public:
    NswCondTestAlg(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~NswCondTestAlg() override;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &) const override;

private:
    using TimeChargeType = NswCalibDbTimeChargeData::CalibDataType;
    using TimeTech = MuonCond::CalibTechType;

    StatusCode retrieveTdoPdo(const EventContext& ctx, TimeChargeType data, const std::string& tech,
                              const std::string& side, std::chrono::duration<double>& timer) const;
    StatusCode retrieveVmm(const EventContext &, const std::string &, const std::string &, std::chrono::duration<double> &) const;
    StatusCode retrieveT0(const EventContext &, std::chrono::duration<double> &) const;
    std::string timestamp() const;

    SG::ReadCondHandleKey<NswCalibDbTimeChargeData> m_readKey_tdopdo{
        this, "ReadKey_tdopdo", "NswCalibDbTimeChargeData",
        "Key of NswCalibDbTimeChargeData object containing calibration data (TDOs and PDOs)"};
    SG::ReadCondHandleKey<NswCalibDbThresholdData> m_readKey_vmm{
        this, "ReadKey_vmm", "NswCalibDbThresholdData",
        "Key of NswCalibDbThresholdData object containing calibration data (VMM thresholds)"};
    SG::ReadCondHandleKey<NswT0Data> m_readKey_t0{this, "readKey_t0", "NswT0Data", "Key of NswT0Data containing the t0 calibration data"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    
    Gaudi::Property<std::string> m_logName{this,"LogName", "LogFile", "Name of the log file. The file creating the TimeCharge log will be called <LogName>_TDO.txt, the other will be <LogName>_vmm.txt"};
    Gaudi::Property<bool> m_isMC {this, "isMC", false, "flagging the input type"};
    
};  // end of class

#endif
