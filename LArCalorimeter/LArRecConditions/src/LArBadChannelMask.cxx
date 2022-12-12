/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LArRecConditions/LArBadChannelMask.h"

const LArBadChanBitPacking LArBadChannelMask::s_bitPacking;  //init static member
const LArBadChanSCBitPacking LArBadChannelMask::s_bitSCPacking;  //init static member

StatusCode LArBadChannelMask::buildBitMask(const std::vector<std::string>& problemsToMask, MsgStream& msg) {
  m_bitMask = 0x0;
  for (const std::string& prblm : problemsToMask) {
    if(m_isSC) {
       std::pair<bool, LArBadChannel::LArBadChannelSCEnum::ProblemType> result
                   = s_bitSCPacking.enumName(prblm); 
       if(result.first)
         s_bitSCPacking.setBit(result.second, m_bitMask); 
       else {
         msg << MSG::ERROR << "The problem flag '" << prblm << "' was not recognized." << endmsg;
         return StatusCode::FAILURE;
       }
    } else {
       std::pair<bool, LArBadChannel::LArBadChannelEnum::ProblemType> result
                   = s_bitPacking.enumName(prblm); 
       if(result.first)
         s_bitPacking.setBit(result.second, m_bitMask); 
       else {
         msg << MSG::ERROR << "The problem flag '" << prblm << "' was not recognized." << endmsg;
         return StatusCode::FAILURE;
       }
    }
  }//end loop over vector<string>
  return StatusCode::SUCCESS;
}

