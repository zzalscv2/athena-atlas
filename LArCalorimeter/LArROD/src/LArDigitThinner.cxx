/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArDigitThinner.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "Identifier/HWIdentifier.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "LArIdentifier/LArOnlineID.h"
#include "AthContainers/ConstDataVector.h"

#include <bitset>
#include <limits>
#include  <cstdlib>

LArDigitThinner::LArDigitThinner(const std::string& name,
				 ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator)
    , m_onlineID(nullptr)
    , m_caloCellId(nullptr) {
  declareProperty("EnergyCuts_Barrel",  m_energyCuts_barrel);
  declareProperty("EnergyCuts_Endcap",  m_energyCuts_endcap);
  declareProperty("EnergyCut_HEC",  m_energyCut_hec = 5000);
  declareProperty("EnergyCut_FCAL",  m_energyCut_fcal = 20000);
}

LArDigitThinner::~LArDigitThinner() = default;


StatusCode LArDigitThinner::initialize() {

  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_inputKey.initialize());
  ATH_CHECK(m_rawChannelKey.initialize());
  ATH_CHECK(m_outputKey.initialize());
  
  ATH_CHECK(detStore()->retrieve(m_onlineID, "LArOnlineID"));
  ATH_CHECK(detStore()->retrieve(m_caloCellId,"CaloCell_ID"));
  
    
  //Fix up jobOptions if needed:
  int defaultCuts_barrel[] = { 1000, 1000, 1000, 1000 };
  int defaultCuts_endcap[] = { 2000, 2000, 2000, 2000 };

  if (m_energyCuts_barrel.size() != 4) {
    msg(MSG::WARNING) << "Only " << m_energyCuts_barrel.size() 
		      << " energy cut values provided for the endcap : reverting to default" << endmsg;
    for (size_t i=0;i<4;++i) m_energyCuts_barrel.push_back(defaultCuts_barrel[i]);
  }

  if (m_energyCuts_endcap.size() != 4) {
    msg(MSG::WARNING) << "Only " << m_energyCuts_endcap.size() 
		      << " energy cut values provided for the endcap : reverting to default" << endmsg;
    for (size_t i=0;i<4;++i) m_energyCuts_endcap.push_back(defaultCuts_endcap[i]);
  }
  
  msg(MSG::INFO) << "Energy cuts (Barrel) : ";
  for (unsigned int i = 0; i < 4; i++) msg() <<  m_energyCuts_barrel[i] << " ";
  msg(MSG::INFO) << "GeV" << endmsg;
  
  msg() << MSG::INFO << "Energy cuts (Endcap) : ";
  for (unsigned int i = 0; i < 4; i++) msg() <<  m_energyCuts_endcap[i] << " ";
  msg() << MSG::INFO << "GeV" << endmsg;

  return StatusCode::SUCCESS;
}


const std::vector<int>&
LArDigitThinner::initCutValues(const EventContext& ctx) const
{
  if (!m_energyCuts.isValid()) {
    //Note that the cut values per online-ID depends on the LAr-cabling. Even the cabling is 
    //formally a time-dependent condition, it changes very rarely. Therefore putting it
    //into a ConditionsContainer filled by a dedicated conditions algorithm seems an
    //unecessary overkill. 

    std::vector<int> energyCuts;
    energyCuts.assign(m_onlineID->channelHashMax(),std::numeric_limits<int>::max());


    SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl(m_cablingKey,ctx);
    const LArOnOffIdMapping* cabling=*cablingHdl;


    for (const Identifier id : m_caloCellId->cell_range(CaloCell_ID::LAREM)) {
      HWIdentifier chid=cabling->createSignalChannelID(id);
      IdentifierHash onlHash=m_onlineID->channel_Hash(chid);
      int sampling = m_caloCellId->sampling(id);
      if (sampling < 0) continue;
      if (m_caloCellId->is_em_barrel(id)) {
        energyCuts[onlHash]=m_energyCuts_barrel[sampling];
      }
      else { //endcap
        energyCuts[onlHash]=m_energyCuts_endcap[sampling];
        continue;
      }
    }//end loop over EM cells;

    for (const Identifier id : m_caloCellId->cell_range(CaloCell_ID::LARHEC)) {
      HWIdentifier chid=cabling->createSignalChannelID(id);
      IdentifierHash onlHash=m_onlineID->channel_Hash(chid);
      energyCuts[onlHash]= m_energyCut_hec;
    }//End loop over HEC cells
  
    for (const Identifier id : m_caloCellId->cell_range(CaloCell_ID::LARFCAL)) {
      HWIdentifier chid=cabling->createSignalChannelID(id);
      IdentifierHash onlHash=m_onlineID->channel_Hash(chid);
      energyCuts[onlHash]= m_energyCut_fcal;
    }//end loop over FCAL cells

    m_energyCuts.set (std::move (energyCuts));

    ATH_MSG_INFO("Done filling cache of cut values");
  }

  return *m_energyCuts.ptr();
}


StatusCode LArDigitThinner::finalize()
{
  return StatusCode::SUCCESS;
}


StatusCode LArDigitThinner::execute(const EventContext& ctx) const {
  
  const std::vector<int>& energyCuts = initCutValues(ctx); //Should return right away if the cut values are already initialized

  //Get event inputs from read handles:
  SG::ReadHandle<LArDigitContainer> inputContainer(m_inputKey,ctx);
  SG::ReadHandle<LArRawChannelContainer> larRawChannelContainer(m_rawChannelKey,ctx);

  // Create the new digit container
  SG::WriteHandle<ConstDigitCont_t> outputContainer(m_outputKey,ctx);

  ATH_CHECK(outputContainer.record(std::make_unique<ConstDigitCont_t>(SG::VIEW_ELEMENTS)));

  
  std::bitset<200000> keepSet;

  for(const LArRawChannel& chan : *larRawChannelContainer) {
    const IdentifierHash onlHash=m_onlineID->channel_Hash(chan.hardwareID());
    if (abs(chan.energy())>=energyCuts[onlHash]) {
      keepSet.set(onlHash);
    }
  }//end loop over raw channels


  //start loop over digits
  for (const LArDigit* dig : *inputContainer) {
    const IdentifierHash onlHash=m_onlineID->channel_Hash(dig->hardwareID());
    if (keepSet.test(onlHash)) {
      outputContainer->push_back(dig);
    }
  }//end loop over input container

  ATH_MSG_DEBUG("Copied " << outputContainer->size() << " of " 
		<< inputContainer->size() << " digits.");

  return StatusCode::SUCCESS;
}
