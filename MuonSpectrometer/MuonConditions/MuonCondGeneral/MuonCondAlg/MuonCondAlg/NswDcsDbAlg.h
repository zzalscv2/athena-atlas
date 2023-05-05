/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_NSWDCSDBALG_H
#define MUONCONDALG_NSWDCSDBALG_H

// STL includes
#include <string>
#include <vector>

// Gaudi includes
#include "GaudiKernel/ICondSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

// Muon includes
#include "MuonCondData/NswDcsDbData.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"


// Forward declarations
class CondAttrListCollection;


class NswDcsDbAlg: public AthReentrantAlgorithm{

public:

	using AthReentrantAlgorithm::AthReentrantAlgorithm;
	virtual ~NswDcsDbAlg() = default;
	virtual StatusCode initialize() override;
	virtual StatusCode execute (const EventContext&) const override;

 
private:

	using writeKey_t = SG::WriteCondHandleKey<NswDcsDbData>;
	using readKey_t  = SG::ReadCondHandleKey<CondAttrListCollection>;
	
	using writeHandleDcs_t = SG::WriteCondHandle<NswDcsDbData>;

	StatusCode processHvData(const EventContext& ctx) const;

	using DcsTechType = NswDcsDbData::DcsTechType;
	using DcsDataType = NswDcsDbData::DcsDataType;
	StatusCode loadHvData(const EventContext& ctx, const readKey_t& readKey, const DcsTechType tech, 
	                      writeHandleDcs_t& writeHandle, NswDcsDbData* writeCdo) const;

	bool buildChannelId(Identifier& channelId, const DcsTechType tech0, const std::string chanName, bool& isOK) const;

	ServiceHandle<ICondSvc> m_condSvc{this, "CondSvc", "CondSvc"};
	ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
	
	writeKey_t m_writeKey{this, "WriteKey", "NswDcsDbData", "Key of output data object" };
	
	readKey_t m_readKey_mmg_hv {this, "ReadKey_MMG_HV", "/MMG/DCS/TSTHV", "Key of input MMG condition data for HV"};
	readKey_t m_readKey_stg_hv {this, "ReadKey_STG_HV", "/STG/DCS/TSTHV", "Key of input STG condition data for HV"};

	const MuonGM::MuonDetectorManager *m_muDetMgrFromDetStore; 
 
};


#endif
