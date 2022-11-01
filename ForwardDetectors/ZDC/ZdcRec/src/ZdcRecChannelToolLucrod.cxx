/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*
 * ZdcRecChannelToolLucrod.cxx
 *
 *  Created on: July 9, 2022
 *      Author: steinber
 */


#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/MsgStream.h"

#include "ZdcConditions/ZdcLucrodMapRun3.h"
#include "ZdcRec/ZdcRecChannelToolLucrod.h"
#include "ZdcIdentifier/ZdcID.h"

// This will normally be one for HI running, but not for the LHCf run
#define LUCROD_TRIG_ID 0

//==================================================================================================
ZdcRecChannelToolLucrod::ZdcRecChannelToolLucrod(const std::string& name):
  AsgTool(name)
{
	//Declare properties here...

	declareInterface<ZdcRecChannelToolLucrod>(this);
}
//==================================================================================================


void ZdcRecChannelToolLucrod::handle( const Incident& inc )
{
  if ( inc.type() == IncidentType::EndEvent) {

  }
}


//==================================================================================================
StatusCode ZdcRecChannelToolLucrod::initialize()
{
  msg(MSG::INFO) << "Initializing " << name() << endmsg;
  
  const ZdcID* zdcId = nullptr;
  if (detStore()->retrieve( zdcId ).isFailure() ) {
    msg(MSG::ERROR) << "execute: Could not retrieve ZdcID object from the detector store" << endmsg;
    return StatusCode::FAILURE;
  }
  else {
    msg(MSG::DEBUG) << "execute: retrieved ZdcID" << endmsg;
  }
  m_zdcId = zdcId;
  
  ServiceHandle<IIncidentSvc> incidentSvc("IncidentSvc", name());
  CHECK(incidentSvc.retrieve());
  incidentSvc->addListener(this, IncidentType::EndEvent);

  msg(MSG::DEBUG) << "--> ZDC : END OF MODIFICATION 0" << endmsg ;
  return StatusCode::SUCCESS;
  
}

//==================================================================================================

//==================================================================================================
StatusCode ZdcRecChannelToolLucrod::finalize()
{
  msg(MSG::INFO) << "Finalizing " << name() << endmsg;
  return StatusCode::SUCCESS;
}
//==================================================================================================

int ZdcRecChannelToolLucrod::convertLucrod2ZM(const ZdcLucrodDataContainer* lucrodCollection, xAOD::ZdcModuleContainer* zdcModules , xAOD::ZdcModuleContainer* zdcSums) const
{

  ATH_MSG_DEBUG("Trying to convert LUCROD to Modules!");

  SG::AuxElement::Accessor<std::vector<uint16_t>> g0acc("g0data");
  SG::AuxElement::Accessor<std::vector<uint16_t>> g1acc("g1data");

  typedef std::map<uint32_t,xAOD::ZdcModule*> hashmapType;
  hashmapType digits_map;
  Identifier chan_id;

  int Nchan = 0;

  for (const ZdcLucrodData* zld : *lucrodCollection)
    {
      ATH_MSG_DEBUG("Next LUCROD...");
      uint32_t lucrod_id =  zld->GetLucrodID();
      ATH_MSG_DEBUG("Unpacking LUCROD ID " << lucrod_id);

      const std::vector<uint16_t> zlt = zld->GetTrigData();

      for (size_t i=0;i<zld->GetChanDataSize();i++)
	{
	  ATH_MSG_DEBUG("Accessing LUCROD ID " << lucrod_id << " chan data " << i);
	  const ZdcLucrodChannel& zlc = zld->GetChanData(i);
	  uint16_t lucrod_channel = zlc.id;
	  ATH_MSG_DEBUG("lucrod_channel=" << lucrod_channel);
	  ATH_MSG_DEBUG("Accessing ZDC map");
	  int side = ZdcLucrodMapRun3::getInstance()->getLucrod(lucrod_id)["side"][lucrod_channel];
	  int module = ZdcLucrodMapRun3::getInstance()->getLucrod(lucrod_id)["module"][lucrod_channel];
	  int type = ZdcLucrodMapRun3::getInstance()->getLucrod(lucrod_id)["type"][lucrod_channel];
	  int channel = ZdcLucrodMapRun3::getInstance()->getLucrod(lucrod_id)["channel"][lucrod_channel];
	  int gain = ZdcLucrodMapRun3::getInstance()->getLucrod(lucrod_id)["gain"][lucrod_channel];
	  ATH_MSG_DEBUG("Done accessing ZDC map side " << side << " module " << module << " type " << type << " channel " << channel << " gain " << gain);

	  chan_id = m_zdcId->channel_id(side,module,type,channel);
	  const uint32_t chan_hash = chan_id.get_identifier32().get_compact();

	  
	  hashmapType::iterator iter = digits_map.find(chan_hash);
	  if (iter == digits_map.end())
	    {
	      ATH_MSG_DEBUG("new channel for " << chan_id);
	      Nchan++;
	      xAOD::ZdcModule* new_mod = new xAOD::ZdcModule();
	      zdcModules->push_back(xAOD::ZdcModuleContainer::unique_type(new_mod));
	      digits_map.insert(std::pair<uint32_t,xAOD::ZdcModule*>(chan_hash,new_mod));
	      iter = digits_map.find(chan_hash);
	      (*iter).second->setZdcId(chan_hash);
	      (*iter).second->setZdcSide(side);
	      (*iter).second->setZdcModule(module);
	      (*iter).second->setZdcType(type);
	      (*iter).second->setZdcChannel(channel);
	    }
	  
	  if (iter != digits_map.end())
	    {
	      
	      ATH_MSG_DEBUG("adding waveform data for " << chan_id << " gain " << gain);

	      if (gain==0)
		{
		  (*iter).second->setWaveform("g0data",zlc.waveform);
		}
	      if (gain==1)
		{
		  (*iter).second->setWaveform("g1data",zlc.waveform);
		}

	      if (lucrod_id==LUCROD_TRIG_ID) 
		{
		  (*iter).second->auxdata<uint16_t>("LucrodTriggerAmp") = zlt.at(i);
		}
	    }      
	  
	}

      if (lucrod_id==LUCROD_TRIG_ID)
	{
	  for (int iside = 0;iside<2;iside++)
	    {
	      xAOD::ZdcModule* new_sum = new xAOD::ZdcModule();
	      zdcSums->push_back(xAOD::ZdcModuleContainer::unique_type(new_sum));
	      new_sum->setZdcSide(iside);
	      new_sum->auxdata<uint16_t>("LucrodTriggerSideAmp") = (iside==0) ? zld->GetTrigAvgC() : zld->GetTrigAvgA();
	    }
	}
    }


  ATH_MSG_DEBUG("Done trying to convert!!");
  
  return Nchan;

}


