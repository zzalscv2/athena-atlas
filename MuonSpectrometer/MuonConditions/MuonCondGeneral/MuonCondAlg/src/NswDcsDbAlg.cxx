/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/NswDcsDbAlg.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include <string>
#include <regex>
#include "nlohmann/json.hpp"

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
	ATH_CHECK(m_readKey_mmg_tdaq.initialize(!m_readKey_mmg_tdaq.empty()));
	ATH_CHECK(m_readKey_stg_tdaq.initialize(!m_readKey_stg_tdaq.empty()));
	ATH_CHECK(m_readKey_mmg_eltx.initialize(!m_readKey_mmg_eltx.empty()));
	
	// write key for time/charge data
	ATH_CHECK(m_writeKey.initialize());

	ATH_CHECK(detStore()->retrieve(m_muDetMgrFromDetStore));
	
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
	std::unique_ptr<NswDcsDbData> wrCdo{std::make_unique<NswDcsDbData>(m_idHelperSvc->mmIdHelper(), m_idHelperSvc->stgcIdHelper(), m_muDetMgrFromDetStore)};

	// load HV data
	ATH_CHECK(loadHvData(ctx, m_readKey_mmg_hv, DcsTechType::MMG, wrHdl, wrCdo.get()));
	ATH_CHECK(loadHvData(ctx, m_readKey_mmg_hv, DcsTechType::MMD, wrHdl, wrCdo.get()));
	ATH_CHECK(loadHvData(ctx, m_readKey_stg_hv, DcsTechType::STG, wrHdl, wrCdo.get()));

	// load TDAQ data
	ATH_CHECK(loadTDaqData(ctx, m_readKey_mmg_tdaq, DcsTechType::MMG, wrHdl, wrCdo.get()));
	ATH_CHECK(loadTDaqData(ctx, m_readKey_stg_tdaq, DcsTechType::STG, wrHdl, wrCdo.get()));
	if(m_loadScas){
		ATH_CHECK(loadELTXData(ctx, m_readKey_mmg_eltx, DcsTechType::MMG, wrHdl, wrCdo.get()));
	}
  
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
		if(chanName.empty()){
			ATH_MSG_DEBUG("Channel number "<< chanNum <<"has empty name");
			continue;
		}
		Identifier channelId{0};
		bool isOK = false;
		bool found = buildChannelIdForHv(channelId, tech, chanName, isOK);
		if(!found){
			if(!isOK){
				ATH_MSG_ERROR("Could not identify valid channelId for channel "<<chanNum<<" with name "<< chanName<<"!");
				throw std::runtime_error("NswDcsDbAlg: Could not identify valid channelId for HV channel");
			}
		continue;
		}

		// payload
		const coral::AttributeList& atr = itr->second;

		NswDcsDbData::DcsConstants dcs_data{};
		dcs_data.v0set     = *(static_cast<const float*>((atr["v0Set"]).addressOfData()));
		dcs_data.v1set     = *(static_cast<const float*>((atr["v1Set"]).addressOfData()));
		dcs_data.fsmState  = NswDcsDbData::getFsmStateEnum(*(static_cast<const std::string*>((atr["fsmCurrentState"]).addressOfData())));
		ATH_MSG_DEBUG("channel " << chanName << " has fsm state " << *(static_cast<const std::string*>((atr["fsmCurrentState"]).addressOfData()))<< " has v0 state " << *(static_cast<const float*>( (atr["v0Set"]).addressOfData()))<< " has v1 " << *(static_cast<const float*>((atr["v1Set"]).addressOfData())));
		
		writeCdo->setDataHv(tech, channelId, dcs_data);
		++nChns;
	}
	ATH_MSG_VERBOSE("Retrieved data for "<<nChns<<" channels.");

	return StatusCode::SUCCESS;
}


// loadTDaqData
StatusCode 
NswDcsDbAlg::loadTDaqData(const EventContext& ctx, const readKey_t& readKey, const DcsTechType tech, writeHandleDcs_t& writeHandle, NswDcsDbData* writeCdo) const {
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
	for(itr = readCdo->begin(); itr != readCdo->end(); ++itr) {

		// retrieve data
		const coral::AttributeList& atr = itr->second;
		std::string data = *(static_cast<const std::string *>((atr["data_array"]).addressOfData()));

		// unwrap the json and process the data
		nlohmann::json jx = nlohmann::json::parse(data);
		unsigned int nLB = 0; 

		// loop over lumi blocks and channels
		for (auto &yy : jx.items()) {
			nlohmann::json jt = yy.value();

			// channel ID and name
			Identifier channelId{0};
			bool isOK = false;
			uint elink{0};
			bool found = buildChannelIdForTDaq(channelId, elink, tech, jt["channelName"], isOK);
			if(!found){
				if(!isOK)
					ATH_MSG_DEBUG("Could not identify valid channelId for channel "<<jt["channelId"]<<" with name "<< jt["channelName"]<<"! Skipping...");
				continue;
			}
			// write data
			writeCdo->setDataTDaq(tech, channelId, jt["hole_lbSince"], jt["hole_lbUntil"], elink);
			++nLB;
		}
		ATH_MSG_VERBOSE("Retrieved data for "<<nLB<<" entries (combinations of lumi block and channel).");
	}

	return StatusCode::SUCCESS;
}

StatusCode NswDcsDbAlg::loadELTXData(const EventContext& ctx, const readKey_t& readKey, const DcsTechType tech, writeHandleDcs_t& writeHandle, NswDcsDbData* writeCdo) const {

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

	// This code is under development so lets mute unused variable warnings for now
	(void) writeCdo;
	(void) tech;

	// iterate through data
	CondAttrListCollection::const_iterator itr;
	unsigned int nChns = 0;
	for(itr = readCdo->begin(); itr != readCdo->end(); ++itr) {

		// channel ID and name
		const unsigned int chanNum  = itr->first;
		const std::string& chanName = readCdo->chanName(chanNum);
		if(chanName.empty()){
			ATH_MSG_DEBUG("Channel number "<< chanNum <<"has empty name");
			continue;
		}
		const coral::AttributeList& atr = itr->second;
		ATH_MSG_DEBUG("found SCA " << chanName << " with status " << atr["online"]);
		++nChns;
	}
	return StatusCode::SUCCESS;

}


// buildChannelIdForHv
bool
NswDcsDbAlg::buildChannelIdForHv(Identifier& channelId, const DcsTechType tech0, const std::string chanName, bool& isOK) const {

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
		ATH_MSG_ERROR("Could not identify channel with name "<<chanName);
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
		const std::string stationName   = sector%2==0 ? "MMS" : "MML";
		int stationEta    = wheel*std::stoi(res[6]);
		int stationPhi    = (sector-1)/2+1;
		int multiLayer    = res[5]=="IP" ? 1 : 2;
		int gasGap        = std::stoi(res[3]);
		int pcb           = std::stoi(res[4]);
		Identifier chnlId = m_idHelperSvc->mmIdHelper().pcbID(stationName, stationEta, stationPhi, multiLayer, gasGap, pcb, isValid);
		if(!isValid){
			ATH_MSG_DEBUG("Could not extract valid channelId for MMG channel "<<chanName);
			ATH_MSG_DEBUG("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer);
			isOK = false;
			return false;
		}
		channelId = chnlId;
	}

	// MMG Drift Channel
	else if(tech==DcsTechType::MMD){
		int wheel         = res[1]=="A"? 1 : -1;
		int sector        = std::stoi(res[2]);
		const std::string stationName   = sector%2==0 ? "MMS" : "MML";
		int stationEta    = wheel*std::stoi(res[4]);
		int stationPhi    = (sector-1)/2+1;
		int multiLayer    = res[3]=="IP" ? 1 : 2;
		Identifier modId  = m_idHelperSvc->mmIdHelper().elementID(stationName, stationEta, stationPhi, isValid);
		if(!isValid){
			ATH_MSG_DEBUG("Could not extract valid elementId for MMGD channel "<<chanName);
			ATH_MSG_DEBUG("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer);
			isOK = false;
			return false;
		}
		Identifier chnlId = m_idHelperSvc->mmIdHelper().multilayerID(modId, multiLayer, isValid);
		if(!isValid){
			ATH_MSG_DEBUG("Could not extract valid multilayerId for MMG channel "<<chanName);
			ATH_MSG_DEBUG("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer);
			isOK = false;
			//return false;
		}
		channelId = chnlId;
	}

	// STG
	else if(tech==DcsTechType::STG){
		int wheel         = res[1]=="A"? 1 : -1;
		int sector        = std::stoi(res[2]);
		const std::string stationName   = sector%2==0 ? "STS" : "STL";
		int radius        = std::stoi(res[5]);
		int stationEta    = wheel*(radius<=2 ? 1 : radius-1);
		int channel       = radius==2 ? 100 : 1; // DCS has two HV channels for first board; store this info in channel number
		int stationPhi    = (sector-1)/2+1;
		int multiLayer    = res[4]=="IP" ? 1 : 2;
		int gasGap        = std::stoi(res[3]);
		Identifier chnlId = m_idHelperSvc->stgcIdHelper().channelID(stationName, stationEta, stationPhi, multiLayer, gasGap, 1, channel, isValid);
		if(!isValid){
			ATH_MSG_DEBUG("Could not extract valid channelId for STG channel "<<chanName);
			ATH_MSG_DEBUG("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer);
			isOK = false;
			return false;
		}
		channelId = chnlId;
	}

	return true;
}


// buildChannelIdForTDaq
bool
NswDcsDbAlg::buildChannelIdForTDaq(Identifier& channelId, uint& elink ,const DcsTechType tech0, const std::string chanName, bool& isOK) const {

	// prepare regex
	std::regex reMMG("^ELink-MM-(A|C)/V([0-9]{1})/L1A/Strip/S([0-9]{1,2})/L([0-9]{1})/R([0-9]{1,2})/E([0-9]{1})");
	std::regex reSTG("^ELink-sTGC-(A|C)/V([0-9]{1})/L1A/(Strip|Pad)/S([0-9]{1,2})/L([0-9]{1})/R([0-9]{1})/E([0-9]{1})");
	std::regex reSTGTrigProc("^ELink-sTGC-A/V0/L1A/TrigProc/");
	std::regex reSTGPadTrig("^ELink-sTGC-A/V0/L1A/PadTrig/");
	
	std::regex re;
	DcsTechType tech = DcsTechType::MMG; 

	// match regex
	if(std::regex_match(chanName, reMMG)) {
		re   = reMMG;
		tech = DcsTechType::MMG;
	}
	else if(std::regex_match(chanName, reSTG)) {
		re   = reSTG;
		tech = DcsTechType::STG;
	} else if(std::regex_match(chanName, reSTGPadTrig) || std::regex_match(chanName, reSTGTrigProc)){ // those are trigger elinks that are not needed in athena
		isOK = true;
		return false;
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
		int sector        = std::stoi(res[3])+1; // elx counts from 0 athena from 1 -->need a +1
		int stationName   = sector%2==0 ? 55 : 56;
		/*
		res[4] -> L
		res[5] -> R
		res[6] -> E
		*/
		int stationEta    = wheel*(std::stoi(res[5])<10? 1 : 2); // boards 0-9 are und the first quad, boards 10-15 und the second one
		int stationPhi    = (sector-1)/2+1;
		int multiLayer    = std::stoi(res[4])< 4 ? 1 : 2; // layers 0-3 are on multilayer 1 and layers 4-7 are on multilayer 2
		int gasGap        = (std::stoi(res[4])%4) + 1; // identifies layer within multilayer --> counts from  0-3; +1 because athena counts from 1-4 
		int radius = std::stoi(res[5]);
		Identifier chnlId = m_idHelperSvc->mmIdHelper().febID(stationName, stationEta, stationPhi, multiLayer, gasGap, radius, isValid);
		elink = std::stoi(res[6]);
		if(!isValid){
			ATH_MSG_DEBUG("Could not extract valid channelId for MMG channel "<<chanName);
			ATH_MSG_DEBUG("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer<<" " << gasGap<<" " << radius<<" " << elink);
			isOK = false;
			return false;
		}
		channelId = chnlId;
	}

	// STG
	else if(tech==DcsTechType::STG){
		int wheel         = res[1]=="A"? 1 : -1;
		int sector        = std::stoi(res[4]) + 1;
		std::string stationName   = sector%2==0 ? "STS" : "STL";
		
		/*
		res[3] -> Strip/Pad
		res[5] -> L
		res[6] -> R
		res[7] -> E
		*/
		
		int	radius        = std::stoi(res[6]);
		int stationEta    = wheel*(radius+1);
		int stationPhi    = (sector-1)/2+1;
		int multiLayer    = (std::stoi(res[5]) < 4 ?  1 : 2);
		int gasGap        = ((std::stoi(res[5]))%4)+1;
		uint channelType  = (res[3] == "Pad" ? sTgcIdHelper::sTgcChannelTypes::Pad : sTgcIdHelper::sTgcChannelTypes::Strip);
		Identifier chnlId = m_idHelperSvc->stgcIdHelper().febID(stationName, stationEta, stationPhi, multiLayer, gasGap, channelType, isValid);
		if(!isValid){
			ATH_MSG_DEBUG("Could not extract valid channelId for STG channel "<<chanName);
			ATH_MSG_DEBUG("Fields: "<< wheel << " "<<sector<<" " << stationName<< " " << stationEta<<" "<<stationPhi<<" "<<multiLayer<<" " << gasGap<<" " << radius<<" " << elink);
			isOK = false;
			return false;
		}
		channelId = chnlId;
		elink = std::stoi(res[7]);	
	}

	return true;
}
