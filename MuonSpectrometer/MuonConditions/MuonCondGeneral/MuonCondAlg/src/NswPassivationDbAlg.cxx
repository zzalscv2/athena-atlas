/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/NswPassivationDbAlg.h"

#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Blob.h"
#include "CoralUtilities/blobaccess.h"
#include "MuonReadoutGeometry/NswPassivationDbData.h"


// Initialize
StatusCode
NswPassivationDbAlg::initialize(){

	// retrievals
	ATH_MSG_DEBUG( "initializing " << name() );				
	ATH_CHECK(m_idHelperSvc.retrieve());

	// read keys
	ATH_CHECK(m_readKey_data_mm.initialize(!m_readKey_data_mm.empty()));

	// write keys	
	ATH_CHECK(m_writeKey.initialize());

	return StatusCode::SUCCESS;
}


// execute
StatusCode 
NswPassivationDbAlg::execute(const EventContext& ctx) const {

	ATH_MSG_DEBUG( "execute " << name() );   
	
	// retrieving data
	ATH_CHECK(loadData(ctx));

	// return	
	return StatusCode::SUCCESS;
}


// loadData
StatusCode
NswPassivationDbAlg::loadData(const EventContext& ctx) const {

	// set up write handle
	SG::WriteCondHandle<NswPassivationDbData> writeHandle{m_writeKey, ctx};
	if (writeHandle.isValid()) {
		ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
			  << " In theory this should not be called, but may happen"
			  << " if multiple concurrent events are being processed out of order.");
		return StatusCode::SUCCESS; 
	}
	std::unique_ptr<NswPassivationDbData> writeCdo{std::make_unique<NswPassivationDbData>(m_idHelperSvc->mmIdHelper())};

	// set up read handle
	SG::ReadCondHandle<CondAttrListCollection> readHandle{m_readKey_data_mm, ctx};
	const CondAttrListCollection* readCdo{*readHandle}; 
	if(readCdo==nullptr){
	  ATH_MSG_ERROR("Null pointer to the read conditions object");
	  return StatusCode::FAILURE; 
	} 
	writeHandle.addDependency(readHandle);
	ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
	ATH_MSG_DEBUG("Range of input is " << readHandle.getRange() << ", range of output is " << writeHandle.getRange());

	// iterate through data
	CondAttrListCollection::const_iterator itr;
	unsigned int nObjs = 0;
	for(itr = readCdo->begin(); itr != readCdo->end(); ++itr) {

		// retrieve data
		const coral::AttributeList& atr = itr->second;
		std::string data = *(static_cast<const std::string *>((atr["data"]).addressOfData()));

		// unwrap the json and process the data
		unsigned int nChns = 0; 
		nlohmann::json yy = nlohmann::json::parse(data);
		for (auto &kt : yy.items()) {
			nlohmann::json jt = kt.value();
			bool isValid=false;
			Identifier channelId = m_idHelperSvc->mmIdHelper().pcbID(int(jt["stationName"]), jt["stationEta"], jt["stationPhi"], jt["multiLayer"], jt["gasGap"], jt["pcbPos"], isValid);
			if(!isValid){
				ATH_MSG_VERBOSE("Cannot find PCB Id!");
				continue;
			}
			writeCdo->setData(channelId, jt["pcbPos"], jt["indiv"], jt["extra"], jt["position"]);
			++nChns;
		}
		ATH_MSG_VERBOSE("Retrieved data for "<<nChns<<" channels.");
		++nObjs;
	}
	ATH_MSG_VERBOSE("Retrieved data for "<<nObjs<<" objects.");

	// insert/write data
	if (writeHandle.record(std::move(writeCdo)).isFailure()) {
		ATH_MSG_FATAL("Could not record NswPassivationDbData " << writeHandle.key() 
			  << " with EventRange " << writeHandle.getRange()
			  << " into Conditions Store");
		return StatusCode::FAILURE;
	}		  
	ATH_MSG_DEBUG("Recorded new " << writeHandle.key() << " with range " << writeHandle.getRange() << " into Conditions Store");

	return StatusCode::SUCCESS;
}


