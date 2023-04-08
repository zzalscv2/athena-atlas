/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NswDcsTestAlg_H
#define NswDcsTestAlg_H

// STL
#include <chrono>
#include <string>

// Athena
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonCondData/NswDcsDbData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

// Forward declarations
class ISvcLocator;
class StatusCode;

class NswDcsTestAlg : public AthReentrantAlgorithm {
public:
    NswDcsTestAlg(const std::string &name, ISvcLocator *pSvcLocator);
    virtual ~NswDcsTestAlg() override;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &) const override;

private:
	using DcsTechType = NswDcsDbData::DcsTechType;
	using DcsDataType = NswDcsDbData::DcsDataType;
	StatusCode retrieveData(const EventContext& ctx, const DcsDataType data, const DcsTechType tech, 
	                        const std::string& side, std::chrono::duration<double>& timer) const;
    std::string timestamp() const;

    SG::ReadCondHandleKey<NswDcsDbData> m_readKey{
        this, "ReadKey", "NswDcsDbData",
        "Key of NswDcsDbData object containing DCS conditions data"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    
    Gaudi::Property<std::string> m_logName{this,"LogName", "LogFile", "Name of the log file. The file creating the TimeCharge log will be called <LogName>_TDO.txt, the other will be <LogName>_vmm.txt"};
    
};  // end of class

#endif
