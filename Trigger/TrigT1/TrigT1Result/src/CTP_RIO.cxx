/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "TrigT1Result/CTP_RIO.h"

CTP_RIO::CTP_RIO() : 
  m_sourceId(0),m_runNumber(0),m_lvl1Id(0),m_bcid(0),m_lvl1TriggerType(0),
  m_detEventType(0),m_nDataWords(0),m_nStatusWords(0) {
}

CTP_RIO::~CTP_RIO() {
}

void CTP_RIO::dumpData() const
{
  IMessageSvc*  msgSvc;
  ISvcLocator* svcLoc = Gaudi::svcLocator( );
  StatusCode sc = svcLoc->service( "MessageSvc", msgSvc );
  if ( sc.isFailure() ) {
    return;
  }
  MsgStream log(msgSvc, "CTP_RIO");
  dumpData(log);
}

void CTP_RIO::dumpData(MsgStream& log) const
{
  log << MSG::DEBUG << "=================================================" << endmsg;
  log << MSG::DEBUG << "CTP ROD Header / Trailer data" << endmsg;
  log << MSG::DEBUG << "Source ID               :  0x" << MSG::hex << getSourceId() << MSG::dec << endmsg;
  log << MSG::DEBUG << "Run number              :  " << getRunNumber() << endmsg;
  log << MSG::DEBUG << "Ext. LVL1 ID            :  " << getLvl1Id() << endmsg;
  log << MSG::DEBUG << "BCID                    :  " << getBCID() << endmsg;
  log << MSG::DEBUG << "Trigger type            :  " << getLvl1TriggerType() << endmsg;
  log << MSG::DEBUG << "Det. event type         :  " << getDetectorEventType() << endmsg;
  log << MSG::DEBUG << "No data words           :  " << getNumberDataWords() << endmsg;
  log << MSG::DEBUG << "No status words         :  " << getNumberStatusWords() << endmsg;
  for(uint32_t i = 0 ; i<getNumberStatusWords(); ++i)    
    log << MSG::DEBUG << "Status word " << i << "           :  0x" << MSG::hex 
	<< getStatusWords()[i] << MSG::dec << endmsg;
  log << MSG::DEBUG << "=================================================" << endmsg;
}
