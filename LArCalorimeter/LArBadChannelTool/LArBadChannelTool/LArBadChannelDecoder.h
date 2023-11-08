/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LArBadChannelDecoder_H
#define LArBadChannelDecoder_H

#include "LArRecConditions/LArBadChannel.h"
#include "LArRecConditions/LArBadFeb.h"
#include "LArBadChannelTool/LArBadChannelState.h"
#include "LArRecConditions/LArBadChanBitPacking.h"
#include "LArRecConditions/LArBadFebBitPacking.h"
#include "Identifier/HWIdentifier.h"

#include <vector>
#include <string>

class LArOnlineID_Base;
class MsgStream;

class LArBadChannelDecoder {
public:

  LArBadChannelDecoder(   const LArOnlineID_Base* onlineID,  bool isSC=false) :
    m_onlineID( onlineID), m_isSC(isSC) {}

  typedef LArBadChannelState                     State;
  typedef LArBadChannelState::BadChanEntry       BadChanEntry;
  typedef std::pair< HWIdentifier, LArBadFeb>    BadFebEntry;

  std::vector<BadChanEntry> readASCII( const std::string& name, 
				       State::CoolChannelEnum coolChan,
                                       MsgStream& log) const; 

  std::vector<BadFebEntry> readFebASCII( const std::string& fname,
                                         MsgStream& log) const;

private:

  enum {barrel_ec, pos_neg, feedthrough, slot, channel}; // for local use only

  const LArOnlineID_Base*   m_onlineID;
  LArBadChanBitPacking m_packing;
  LArBadChanSCBitPacking m_SCpacking;
  LArBadFebBitPacking  m_febPacking;

  bool m_isSC;

  HWIdentifier constructChannelId( const std::vector<int>& intVec, 
				   State::CoolChannelEnum coolChan,
				   MsgStream& log) const;
  std::pair<bool,LArBadChannel> constructStatus( const std::vector<std::string>& vec,
						 MsgStream& log) const;
  std::pair<bool,LArBadFeb> constructFebStatus( const std::vector<std::string>& vec,
						 MsgStream& log) const;
  bool checkId( const HWIdentifier&, int be, int pn, State::CoolChannelEnum) const;

  std::vector<HWIdentifier> constructFebId( const std::vector<int>& intVec, 
					    MsgStream& log) const;

  HWIdentifier constructSingleFebId( const std::vector<int>& v, MsgStream& log) const;

  static MsgStream& insertExpandedID( const std::vector<int>& intVec, MsgStream& log) ;

};

#endif
