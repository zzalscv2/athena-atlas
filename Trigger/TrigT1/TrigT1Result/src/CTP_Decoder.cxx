/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


// this header file
#include "TrigT1Result/CTP_Decoder.h"

// tdaq-common includes for format definition
#include "CTPfragment/CTPdataformat.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "AthenaKernel/getMessageSvc.h"

CTP_Decoder::CTP_Decoder(){
}

CTP_Decoder::~CTP_Decoder(){
}

MsgStream & getlog(){
  static MsgStream msg(Athena::getMessageSvc( ), "CTP_Decoder");
  return msg;
}

void CTP_Decoder::setRDO(const CTP_RDO* rdo)
{
  m_rdo = rdo;
  unsigned int ctpVersionNumber = m_rdo->getCTPVersionNumber();
  CTPdataformatVersion ctpVersion(ctpVersionNumber);
  
  unsigned int nBunches = rdo->getNumberOfBunches();
  if( getlog().level() <= MSG::VERBOSE )
    getlog() << MSG::VERBOSE << "setRDO> " << nBunches << " bunches" << endreq;
  
  m_BCs.clear();
  m_BCs.resize(nBunches);
  for (std::vector<CTP_BC>::iterator bcIter=m_BCs.begin(); bcIter!=m_BCs.end(); bcIter++) {
    bcIter->setCTPVersion(ctpVersionNumber);
  }
  
  //std::vector<uint32_t> PITs = rdo->getPITWords();
  //std::vector<uint32_t> FPIs = rdo->getFPIWords();
  std::vector<uint32_t> TIPs = rdo->getTIPWords();
  std::vector<uint32_t> TBPs = rdo->getTBPWords();
  std::vector<uint32_t> TAPs = rdo->getTAPWords();
  std::vector<uint32_t> TAVs = rdo->getTAVWords();
  
  for(unsigned int i = 0 ; i < nBunches ; ++i) {
    
    //for(unsigned int pit = 0; pit < ctpVersion.getPITwords(); ++pit) {
//      unsigned int index = i*ctpVersion.getPITwords() + pit;
//      if(index >= PITs.size()) {
//        getlog() << MSG::FATAL <<"Invalid PIT position " << index << endreq;
//        return;
//      }
//      m_BCs[i].setPITWord(PITs[index],pit); //sets m_pit
//    }
//		
//    for(unsigned int fpi = 0; fpi < ctpVersion.getFPIwords(); ++fpi) {
//      unsigned int index = i*ctpVersion.getFPIwords() + fpi;
//      if(index >= FPIs.size()) {
//        getlog() << MSG::FATAL <<"Invalid FPI position " << index << endreq;
//        return;
//      }
//      m_BCs[i].setFPIWord(FPIs[index],fpi); //sets m_fpi
//    }
		
    for(unsigned int tip = 0; tip < ctpVersion.getTIPwords(); ++tip) {
      unsigned int index = i*ctpVersion.getTIPwords() + tip;
      if(index >= TIPs.size()) {
        getlog() << MSG::FATAL <<"Invalid TIP position " << index << endreq;
        return;
      }
      m_BCs[i].setTIPWord(TIPs[index],tip); //sets m_tip
    }
    
    for(unsigned int tbp = 0; tbp < ctpVersion.getTBPwords(); ++tbp) {
      unsigned int index = i * ctpVersion.getTBPwords() + tbp;
      if(index >= TBPs.size()) {
        getlog() << MSG::FATAL <<"Invalid TBP position " << index << endreq;
        return;
      }
      m_BCs[i].setTBPWord(TBPs[index],tbp); //sets m_tbp
    }
    
    for(unsigned int tap = 0; tap < ctpVersion.getTAPwords(); ++tap) {
      unsigned int index = i * ctpVersion.getTAPwords() + tap;
      if(index >= TAPs.size()) {
        getlog() << MSG::FATAL <<"Invalid TAP position " << index << endreq;
        return;
      }
      m_BCs[i].setTAPWord(TAPs[index],tap); //sets m_tap
    }
    
    for(unsigned int tav = 0; tav < ctpVersion.getTAVwords(); ++tav) {
      unsigned int index = i * ctpVersion.getTAVwords() + tav;
      if(index >= TAVs.size()) {
        getlog() << MSG::FATAL <<"Invalid TAV position " << index << endreq;
        return;
      }
      m_BCs[i].setTAVWord(TAVs[index],tav); //set m_tav
    }
  }
}

void CTP_Decoder::dumpData() const
{
  dumpData(getlog());
}

void CTP_Decoder::dumpData(MsgStream& msglog) const
{
  unsigned int nBunches = m_rdo->getNumberOfBunches();
  unsigned int vecSize = m_BCs.size();
  
  if(nBunches == 0) {
    if( msglog.level() <= MSG::DEBUG )
      msglog << MSG::DEBUG << "CTP_RDO empty" << endreq;
    return;
  }
  
  if(nBunches != vecSize) {
    msglog << MSG::ERROR << "mismatch: " << nBunches 
    << " bunches, but vector size is " << vecSize << endreq;
  }
  
  if( msglog.level() <= MSG::DEBUG ){
    msglog << MSG::DEBUG << "=================================================" << endreq;
    msglog << MSG::DEBUG << "Event dump" << endreq;
    msglog << MSG::DEBUG << "Time " << m_rdo->getTimeSec() << "s " 
    << std::setw(10) << std::setiosflags(std::ios_base::right) << std::setfill(' ') 
    << m_rdo->getTimeNanoSec() << std::resetiosflags(std::ios_base::right) 
    << "ns" << endreq;
    msglog << MSG::DEBUG << "Number of bunches: " <<  nBunches 
    << " (BC vector size " << vecSize << ")" << endreq;
    msglog << MSG::DEBUG << "L1A position: " << m_rdo->getL1AcceptBunchPosition() << endreq;
  }
  
  for(unsigned int i = 0; i<vecSize; ++i)
    m_BCs[i].dumpData(msglog);
  if( msglog.level() <= MSG::DEBUG )
    msglog << MSG::DEBUG << "=================================================" << endreq;
}

bool CTP_Decoder::checkTrigger(unsigned int itemNo,unsigned int pos)
{
  if(pos >= m_BCs.size()) {
    getlog() << MSG::WARNING << "Trying to access bunch crossing no "
    << pos << ", but in the event are only " << m_BCs.size() << endreq;
  }
  if(itemNo >= getBunchCrossing(pos).getTAV().size()) {
    getlog() << MSG::WARNING << "Checking item no " << itemNo 
    << ", which is more than the maximum : " 
    << getBunchCrossing(pos).getTAV().size() << endreq;
  }
  return getBunchCrossing(pos).getTAV().test(itemNo);
}

bool CTP_Decoder::checkTriggerAfterPrescale(unsigned int itemNo,unsigned int pos)
{
  if(pos >= m_BCs.size()) {
    getlog() << MSG::WARNING << "Trying to access bunch crossing no "
    << pos << ", but in the event are only " << m_BCs.size() << endreq;
  }
  if(itemNo >= getBunchCrossing(pos).getTAP().size()) {
    getlog() << MSG::WARNING << "Checking item no " << itemNo 
    << ", which is more than the maximum : " 
    << getBunchCrossing(pos).getTAP().size() << endreq;
  }
  return getBunchCrossing(pos).getTAP().test(itemNo);
}

std::vector<unsigned int> CTP_Decoder::getAllTriggers(unsigned int pos)
{
  if(pos >= m_BCs.size()) {
    getlog() << MSG::WARNING << "Trying to access bunch crossing no "
    << pos << ", but in the event are only " << m_BCs.size() << endreq;
    return std::vector<unsigned int>();
  }
  std::vector<unsigned int> triggers;
  const CTP_BC& bc = getBunchCrossing(pos);
  if(bc.getTAV().any()) {
    for(unsigned int i = 0; i < bc.getTAV().size() ; ++i) {
      if(bc.getTAV().test(i)) {
        triggers.push_back(i+1);
      }
    }  
  }
  return triggers;
}

CTP_BC::CTP_BC(){}
CTP_BC::~CTP_BC(){
  if (m_ctpVersion) delete m_ctpVersion;
  m_ctpVersion = NULL;

}

void CTP_BC::dumpData() const
{
  dumpData(getlog());
}

void CTP_BC::dumpData(MsgStream& msglog) const
{
  if( msglog.level() <= MSG::DEBUG ){
    msglog << MSG::DEBUG << "--------------------------------------------------" << endreq;
    msglog << MSG::DEBUG << "BCID        : " << getBCID() << endreq;
    msglog << MSG::DEBUG << "Random trig     : " << getRandomTrig()
    << " (binary: " << printRandomTrig() << ")" << endreq;
    msglog << MSG::DEBUG << "Prescaled clock : " << getPrescaledClock()
    << " (binary: " << printPrescaledClock() << ")" << endreq;
  }
  
  if(m_tip.any()) {
    std::ostringstream outstream;
    outstream << "PIT with input (max 160): ";
    int count = 0;
    for(unsigned int i = 0; i<m_tip.size() ; ++i) {
      if(m_tip.test(i)) {
        if(count%20==0) outstream << "\n\t\t\t  ";
        outstream << std::setw(3) << std::setfill('0') << (i+1) << " ";
        ++count;
      }
    }
    if( msglog.level() <= MSG::DEBUG )
      msglog << MSG::DEBUG << outstream.str() << endreq;
  } 
  else {
    if( msglog.level() <= MSG::DEBUG )
      msglog << MSG::DEBUG << "No TIP!" << endreq;
  }
  
  if(m_tbp.any()) {
    std::ostringstream outstream;
    outstream << "Fired TBP item number (max 256): ";
    for(unsigned int i = 0; i<m_tbp.size() ; ++i) {
      if(m_tbp.test(i)) outstream << (i+1) << " ";
    }
    if( msglog.level() <= MSG::DEBUG )
      msglog << MSG::DEBUG << outstream.str() << endreq;
  } 
  else {
    if( msglog.level() <= MSG::DEBUG )
      msglog << MSG::DEBUG << "No TBP fired!" << endreq;
  }
  
  if(m_tap.any()) {
    std::ostringstream outstream;
    outstream << "Fired TAP item number (max 256): ";
    for(unsigned int i = 0; i<m_tap.size() ; ++i) {
      if(m_tap.test(i)) outstream << (i+1) << " ";
    }
    if( msglog.level() <= MSG::DEBUG )
      msglog << MSG::DEBUG << outstream.str() << endreq;
  } 
  else {
    if( msglog.level() <= MSG::DEBUG )
      msglog << MSG::DEBUG << "No TAP fired!" << endreq;
  }
  
  if(m_tav.any()) {
    std::ostringstream outstream;
    outstream << "Fired TAV item number (max 256): ";
    for(unsigned int i = 0; i<m_tav.size() ; ++i) {
      if(m_tav.test(i)) outstream << (i+1) << " ";
    }
    if( msglog.level() <= MSG::DEBUG )
      msglog << MSG::DEBUG << outstream.str() << endreq;
  } 
  else {
    if( msglog.level() <= MSG::DEBUG )
      msglog << MSG::DEBUG << "No TAV fired!" << endreq;
  }
  
  if( msglog.level() <= MSG::VERBOSE ){
    msglog << MSG::VERBOSE << "TIP - total size: " << m_tip.size() << ", with input: " 
    << m_tip.count() << ", pattern:" << std::endl << printTIP() << endreq;
    msglog << MSG::VERBOSE << "TBP " << std::endl << printTBP() << endreq;
    msglog << MSG::VERBOSE << "TAP " << std::endl << printTAP() << endreq;
    msglog << MSG::VERBOSE << "TAV " << std::endl << printTAV() << endreq;
  }
}

std::bitset<32> CTP_BC::getBCIDBitSet() const
{
  std::bitset<32> bcid = (m_pitAux >> m_ctpVersion->getBcidShift());
  bcid &= m_ctpVersion->getBcidMask();
  return bcid;
}

uint32_t CTP_BC::getBCID() const
{
  return getBCIDBitSet().to_ulong();
}

std::string CTP_BC::printBCID() const
{
  std::bitset<32> bcid = getBCIDBitSet();
  return bcid.to_string<char, 
  std::char_traits<char>, std::allocator<char> >();
}

std::bitset<32> CTP_BC::getRandomTrigBitSet() const
{
  std::bitset<32> rnd = (m_pitAux >> m_ctpVersion->getRandomTrigShift());
  rnd &= m_ctpVersion->getRandomTrigMask();
  return rnd;
}

uint32_t CTP_BC::getRandomTrig() const
{
  return getRandomTrigBitSet().to_ulong();
}

std::string CTP_BC::printRandomTrig() const
{
  std::bitset<32> rnd = getRandomTrigBitSet();
  return rnd.to_string<char, 
  std::char_traits<char>, std::allocator<char> >();
}

std::bitset<32> CTP_BC::getPrescaledClockBitSet() const
{
	std::bitset<32> prcl;
	
	if (!m_ctpVersion->getNumPrescaledClocks()) {
		getlog() << MSG::WARNING << "Trying to get prescaled clocks but there are none in this data format version. 0's will be returned." << endreq;
		return prcl;
	}
	
  prcl = (m_pitAux >> m_ctpVersion->getPrescaledClockShift());
  prcl &= m_ctpVersion->getPrescaledClockMask();
  return prcl;
}

uint32_t CTP_BC::getPrescaledClock() const
{
  return getPrescaledClockBitSet().to_ulong();
}

std::string CTP_BC::printPrescaledClock() const
{
  std::bitset<32> prcl = getPrescaledClockBitSet();
  return prcl.to_string<char, 
  std::char_traits<char>, std::allocator<char> >();
}



//void CTP_BC::setPITWord( uint32_t word, uint32_t pos) 
//{
//  if(pos >= m_ctpVersion->getPITwords()) {
//    getlog() << MSG::ERROR <<"Invalid PIT position " << pos <<endreq;
//    return;
//  }
//  
//  if( getlog().level() <= MSG::VERBOSE )
//    getlog() << MSG::VERBOSE <<"PIT pos " << pos << " word 0x" << std::hex << word 
//    << std::dec << " (" << word << ")" <<endreq;
//  
//  std::bitset<320> bs = word;
//  
//  if( getlog().level() <= MSG::VERBOSE )
//    getlog() << MSG::VERBOSE <<"bitset " 
//    << bs.to_string<char, std::char_traits<char>, std::allocator<char> >()
//    << std::endl << " (" << bs.to_ulong() << ")" <<endreq;
//  
//  bs <<= (pos * CTP_RDO::SIZEOF_WORDS);
//  
//  if( getlog().level() <= MSG::VERBOSE ){
//    getlog() << MSG::VERBOSE <<"bitset shifted by " << (pos * CTP_RDO::SIZEOF_WORDS) << ": " << std::endl 
//    << bs.to_string<char, std::char_traits<char>, std::allocator<char> >()
//    <<endreq;
//    
//    getlog() << MSG::VERBOSE <<"PIT before " << std::endl 
//    << printPIT()
//    <<endreq;
//  }
//  
//  //if( pos < (m_ctpVersion->getAuxTIPwordPos()-1) ) m_pit |= bs;
//  if( pos < (m_ctpVersion->getPITwords()-1) ) m_pit |= bs;
//  else setPITWordAux(word);
//  
//  if( getlog().level() <= MSG::VERBOSE )
//    getlog() << MSG::VERBOSE <<"PIT after  " << std::endl 
//    << printPIT()
//    <<endreq;
//  
//}
//
//void CTP_BC::setPIT(std::vector<uint32_t> words)
//{
//  for(uint32_t i = 0; i<words.size();++i) setPITWord(words[i],i);
//}
//
//std::string CTP_BC::printPIT() const
//{
//  return m_pit.to_string<char, 
//  std::char_traits<char>, std::allocator<char> >();
//}

std::string CTP_BC::printPITWordAux() const
{
  return m_pitAux.to_string<char, 
  std::char_traits<char>, std::allocator<char> >();
}



//void CTP_BC::setFPIWord( uint32_t word, uint32_t pos) 
//{
//	if (!(m_ctpVersion->getFPIwords())) {
//		getlog() << MSG::VERBOSE <<"No front panel inputs in this CTP version." <<endreq;
//		return;
//	}else if(pos >= m_ctpVersion->getFPIwords()) {
//    getlog() << MSG::ERROR <<"Invalid FPI position " << pos <<endreq;
//    return;
//  }
//  std::bitset<192> bs = word;
//  bs <<= (pos * CTP_RDO::SIZEOF_WORDS);
//  m_fpi |= bs;
//}
//
//void CTP_BC::setFPI(std::vector<uint32_t> words)
//{
//  for(uint32_t i = 0; i<words.size();++i) setFPIWord(words[i],i);
//}
//
//std::string CTP_BC::printFPI() const
//{
//  return m_fpi.to_string<char, 
//  std::char_traits<char>, std::allocator<char> >();
//}


void CTP_BC::setTIPWord( uint32_t word, uint32_t pos) 
{
  if(pos >= m_ctpVersion->getTIPwords()) {
    getlog() << MSG::ERROR <<"Invalid TIP position " << pos <<endreq;
    return;
  }
  
  if( getlog().level() <= MSG::VERBOSE )
    getlog() << MSG::VERBOSE <<"TIP pos " << pos << " word 0x" << std::hex << word 
    << std::dec << " (" << word << ")" <<endreq;
  
  std::bitset<512> bs = word;
  
  if( getlog().level() <= MSG::VERBOSE )
    getlog() << MSG::VERBOSE <<"bitset " 
    << bs.to_string<char, std::char_traits<char>, std::allocator<char> >()
    << std::endl << " (" << bs.to_ulong() << ")" <<endreq;
  
  bs <<= (pos * CTP_RDO::SIZEOF_WORDS);
  
  if( getlog().level() <= MSG::VERBOSE ){
    getlog() << MSG::VERBOSE <<"bitset shifted by " << (pos * CTP_RDO::SIZEOF_WORDS) << ": " << std::endl 
    << bs.to_string<char, std::char_traits<char>, std::allocator<char> >()
    <<endreq;
    
    getlog() << MSG::VERBOSE <<"TIP before " << std::endl 
    << printTIP()
    <<endreq;
  }
  
  if( pos < (m_ctpVersion->getTIPwords()-1) ) m_tip |= bs;
  else setPITWordAux(word);
  
  if( getlog().level() <= MSG::VERBOSE )
    getlog() << MSG::VERBOSE <<"TIP after  " << std::endl 
    << printTIP()
    <<endreq;
  
}

void CTP_BC::setTIP(std::vector<uint32_t> words)
{
  for(uint32_t i = 0; i<words.size();++i) setTIPWord(words[i],i);
}

std::string CTP_BC::printTIP() const
{
  return m_tip.to_string<char, 
  std::char_traits<char>, std::allocator<char> >();
}



void CTP_BC::setTBPWord( uint32_t word, uint32_t pos) 
{
  if(pos >= m_ctpVersion->getTBPwords()) {
    getlog() << MSG::ERROR <<"Invalid TBP position " << pos <<endreq;
    return;
  }
  std::bitset<512> bs = word;
  bs <<= (pos * CTP_RDO::SIZEOF_WORDS);
  m_tbp |= bs;
}

void CTP_BC::setTBP(std::vector<uint32_t> words)
{
  for(uint32_t i = 0; i<words.size();++i) setTBPWord(words[i],i);
}

std::string CTP_BC::printTBP() const
{
  return m_tbp.to_string<char, 
  std::char_traits<char>, std::allocator<char> >();
}

void CTP_BC::setTAPWord( uint32_t word, uint32_t pos) 
{
  if(pos >= m_ctpVersion->getTAPwords()) {
    getlog() << MSG::ERROR <<"Invalid TAP position " << pos <<endreq;
    return;
  }
  std::bitset<512> bs = word;
  bs <<= (pos * CTP_RDO::SIZEOF_WORDS);
  m_tap |= bs;
}

void CTP_BC::setTAP(std::vector<uint32_t> words)
{
  for(uint32_t i = 0; i<words.size();++i)
    setTAPWord(words[i],i);
}

std::string CTP_BC::printTAP() const
{
  return m_tap.to_string<char, 
  std::char_traits<char>, std::allocator<char> >();
}

void CTP_BC::setTAVWord( uint32_t word, uint32_t pos) 
{
  if(pos >= m_ctpVersion->getTAVwords()) {
    getlog() << MSG::ERROR <<"Invalid TAV position " << pos <<endreq;
    return;
  }
  std::bitset<512> bs = word;
  bs <<= (pos * CTP_RDO::SIZEOF_WORDS);
  m_tav |= bs;
}

void CTP_BC::setTAV(std::vector<uint32_t> words)
{
  for(uint32_t i = 0; i<words.size();++i)
    setTAVWord(words[i],i);
}

std::string CTP_BC::printTAV() const
{
  return m_tav.to_string<char, 
  std::char_traits<char>, std::allocator<char> >();
}
