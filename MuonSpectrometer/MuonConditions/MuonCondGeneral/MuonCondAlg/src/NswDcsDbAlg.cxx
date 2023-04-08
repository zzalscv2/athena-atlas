/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/NswDcsDbAlg.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include <string>
#include <regex>

// Initialize
StatusCode
NswDcsDbAlg::initialize(){

	// retrievals
	ATH_MSG_DEBUG( "initializing " << name() );                
	ATH_CHECK(m_condSvc     .retrieve());
	ATH_CHECK(m_idHelperSvc.retrieve());
	
	// initialize read keys
	ATH_CHECK(m_readKey_mmg_hv.initialize(!m_readKey_mmg_hv.empty()));
	ATH_CHECK(m_readKey_stg_hv.initialize(!m_readKey_stg_hv.empty()));
	
	// write key for time/charge data
	ATH_CHECK(m_writeKey.initialize());
	
	return StatusCode::SUCCESS;
}

// execute
StatusCode 
NswDcsDbAlg::execute(const EventContext& ctx) const {

	ATH_MSG_DEBUG( "execute " << name() );   

	// set up write handles
	SG::WriteCondHandle<NswDcsDbData> wrHdl{m_writeKey, ctx};
	if (wrHdl.isValid()) {
		ATH_MSG_DEBUG("CondHandle " << wrHdl.fullKey() << " is already valid."
		    << " In theory this should not be called, but may happen"
		    << " if multiple concurrent events are being processed out of order.");
		return StatusCode::SUCCESS;
	}
	ATH_MSG_DEBUG("Range of time/charge output is " << wrHdl.getRange());
	std::unique_ptr<NswDcsDbData> wrCdo{std::make_unique<NswDcsDbData>(m_idHelperSvc->mmIdHelper(), m_idHelperSvc->stgcIdHelper())};

	// load data
	ATH_CHECK(loadHvData(ctx, m_readKey_mmg_hv, DcsTechType::MMG, wrHdl, wrCdo.get()));
	ATH_CHECK(loadHvData(ctx, m_readKey_mmg_hv, DcsTechType::MMD, wrHdl, wrCdo.get()));
	ATH_CHECK(loadHvData(ctx, m_readKey_stg_hv, DcsTechType::STG, wrHdl, wrCdo.get()));
  
	// insert/write data
	if (wrHdl.record(std::move(wrCdo)).isFailure()) {
		ATH_MSG_FATAL("Could not record " << wrHdl.key() 
		       << " with EventRange " << wrHdl.getRange()
		       << " into Conditions Store");
		return StatusCode::FAILURE;
	}      
	ATH_MSG_DEBUG("Recorded new " << wrHdl.key() << " with range " << wrHdl.getRange() << " into Conditions Store");
	
	return StatusCode::SUCCESS;

}

// loadHvData
StatusCode 
NswDcsDbAlg::loadHvData(const EventContext& ctx, const readKey_t& readKey, const DcsTechType tech, writeHandleDcs_t& writeHandle, NswDcsDbData* writeCdo) const {

	// set up read handle
	SG::ReadCondHandle<CondAttrListCollection> readHandle{readKey, ctx};
	const CondAttrListCollection* readCdo{*readHandle}; 
	if(!readCdo){
		ATH_MSG_ERROR("Null pointer to the read conditions object");
		return StatusCode::FAILURE; 
	} 
	writeHandle.addDependency(readHandle);
	ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
	ATH_MSG_DEBUG("Range of input is " << readHandle.getRange() << ", range of output is " << writeHandle.getRange());

	// iterate through data
	CondAttrListCollection::const_iterator itr;
	unsigned int nChns = 0;
	for(itr = readCdo->begin(); itr != readCdo->end(); ++itr) {

		// channel ID and name
		const unsigned int chanNum  = itr->first;
		const std::string& chanName = readCdo->chanName(chanNum);
		Identifier channelId{0};
		bool isOK = false;
		bool found = buildChannelId(channelId, tech, chanName, isOK);
		if(!found){
			if(!isOK)
				ATH_MSG_DEBUG("Could not identify valid channelId for channel "<<chanNum<<" with name "<< chanName<<"! Skipping...");
			continue;
		}

		// payload
		const coral::AttributeList& atr = itr->second;

		NswDcsDbData::DcsConstants dcs_data{};
		dcs_data.v0set     = *(static_cast<const float*>((atr["v0Set"]).addressOfData()));
		dcs_data.v1set     = *(static_cast<const float*>((atr["v1Set"]).addressOfData()));
		dcs_data.fsmState  = NswDcsDbData::getFsmStateEnum(*(static_cast<const std::string*>((atr["fsmCurrentState"]).addressOfData())));
		
		writeCdo->setData(tech, channelId, dcs_data);
		++nChns;
	}
	ATH_MSG_VERBOSE("Retrieved data for "<<nChns<<" channels.");

	return StatusCode::SUCCESS;
}



// buildChannelId
bool
NswDcsDbAlg::buildChannelId(Identifier& channelId, const DcsTechType tech0, const std::string chanName, bool& isOK) const {

	// prepare regex
	std::regex reMMG("^([A-Za-z]{1})([0-9]{2})_ML([0-9])P([0-9])_(IP|HO)R([0-9])__HV");
	std::regex reMMD("^([A-Za-z]{1})([0-9]{2})_DRIFT_(IP|HO)R([0-9])__HV");
	std::regex reSTG("^([A-Za-z]{1})([0-9]{2})_ML([0-9])_(IP|HO)R([0-9])__HV");
	std::regex re;
	DcsTechType tech = DcsTechType::MMG; 

	// match regex
	if(std::regex_match(chanName, reMMG)) {
		re   = reMMG;
		tech = DcsTechType::MMG;
	}
	else if(std::regex_match(chanName, reMMD)) {
		re   = reMMD;
		tech = DcsTechType::MMD;
	}
	else if(std::regex_match(chanName, reSTG)) {
		re   = reSTG;
		tech = DcsTechType::STG;
	}
	else {
		ATH_MSG_DEBUG("Could not identify channel with name "<<chanName);
		isOK = false;
		return false;
	}

	// sanity check
	if(tech0!=tech) {
		isOK = true; // need to distinguish error in parsing and simple difference between required technologies
		return false;
	}

	// build channel Id
	bool isValid{false};
	std::smatch m;
	std::regex_match(chanName, m, re);

	// copy to vec of strings
	std::vector<std::string> res;
	for(unsigned int i=0; i<m.size(); ++i) res.push_back(m[i].str());

	// MMG Channel
	if(tech==DcsTechType::MMG){
		int wheel         = res[1]=="A"? 1 : -1;
		int sector        = std::stoi(res[2]);
		int stationName   = sector%2==0 ? 55 : 56;
		int stationEta    = wheel*std::stoi(res[6]);
		int stationPhi    = (sector-1)/2+1;
		int multiLayer    = res[5]=="IP" ? 1 : 2;
		int gasGap        = std::stoi(res[3]);
		int pcb           = std::stoi(res[4]);
		Identifier chnlId = m_idHelperSvc->mmIdHelper().pcbID(stationName, stationEta, stationPhi, multiLayer, gasGap, pcb, isValid);
		if(!isValid){
			ATH_MSG_DEBUG("Could not extract valid channelId for MMG channel "<<chanName);
			isOK = false;
			return false;
		}
		channelId = chnlId;
	}

	// MMG Drift Channel
	else if(tech==DcsTechType::MMD){
		int wheel         = res[1]=="A"? 1 : -1;
		int sector        = std::stoi(res[2]);
		int stationName   = sector%2==0 ? 55 : 56;
		int stationEta    = wheel*std::stoi(res[4]);
		int stationPhi    = (sector-1)/2+1;
		int multiLayer    = res[3]=="IP" ? 1 : 2;
		Identifier modId  = m_idHelperSvc->mmIdHelper().elementID(stationName, stationEta, stationPhi, isValid);
		if(!isValid){
			ATH_MSG_DEBUG("Could not extract valid elementId for MMG channel "<<chanName);
			isOK = false;
			return false;
		}
		Identifier chnlId = m_idHelperSvc->mmIdHelper().multilayerID(modId, multiLayer, isValid);
		if(!isValid){
			ATH_MSG_DEBUG("Could not extract valid multilayerId for MMG channel "<<chanName);
			isOK = false;
			//return false;
		}
		channelId = chnlId;
	}

	// STG
	else if(tech==DcsTechType::STG){
		int wheel         = res[1]=="A"? 1 : -1;
		int sector        = std::stoi(res[2]);
		int stationName   = sector%2==0 ? 55 : 56;
		int radius        = std::stoi(res[5]);
		int stationEta    = wheel*(radius<=2 ? 1 : radius-1);
		int channel       = radius==2 ? 100 : 1; // DCS has two HV channels for first board; store this info in channel number
		int stationPhi    = (sector-1)/2+1;
		int multiLayer    = res[4]=="IP" ? 1 : 2;
		int gasGap        = std::stoi(res[3]);
		Identifier chnlId = m_idHelperSvc->stgcIdHelper().channelID(stationName, stationEta, stationPhi, multiLayer, gasGap, 1, channel, isValid);
		if(!isValid){
			ATH_MSG_DEBUG("Could not extract valid channelId for STG channel "<<chanName);
			isOK = false;
			return false;
		}
		channelId = chnlId;
	}

	return true;
}
