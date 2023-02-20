/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// RpcPrepData.cxx
//   Implementation file for class RpcPrepData
///////////////////////////////////////////////////////////////////


#include <new>
#include <sstream>
#include "MuonPrepRawData/RpcPrepData.h"
#include "GaudiKernel/MsgStream.h"

namespace Muon
{



RpcPrepData::RpcPrepData( const Identifier& RDOId,
                          const IdentifierHash &idDE,
                          const Amg::Vector2D& locpos,
                          const std::vector<Identifier>& rdoList,
                          const Amg::MatrixX& locErrMat,
                          const MuonGM::RpcReadoutElement* detEl,
                          const float time,
                          const int triggerInfo,
                          const int ambiguityFlag) :
    MuonCluster(RDOId, idDE, locpos, rdoList, locErrMat), //call base class constructor
    m_detEl(detEl),
    m_time(time),
    m_triggerInfo(triggerInfo),
    m_ambiguityFlag(ambiguityFlag)
{ }

RpcPrepData::RpcPrepData( const Identifier& RDOId,
                          const IdentifierHash &idDE,
                          const Amg::Vector2D& locpos,
                          const std::vector<Identifier>& rdoList,
                          const Amg::MatrixX& locErrMat,
                          const MuonGM::RpcReadoutElement* detEl,
                          const float time,
                          const float timeOverThresh,
                          const int triggerInfo,
                          const int ambiguityFlag) :
    MuonCluster(RDOId, idDE, locpos, rdoList, locErrMat), //call base class constructor
    m_detEl(detEl),
    m_time(time),
    m_timeOverThreshold(timeOverThresh),
    m_triggerInfo(triggerInfo),
    m_ambiguityFlag(ambiguityFlag)
{ }

RpcPrepData::RpcPrepData( const Identifier& RDOId,
                          const IdentifierHash &idDE,
                          const Amg::Vector2D& locpos,
                          const std::vector<Identifier>& rdoList,
                          const Amg::MatrixX& locErrMat,
                          const MuonGM::RpcReadoutElement* detEl,
                          const float time,
                          const int ambiguityFlag) :
    MuonCluster(RDOId, idDE, locpos, rdoList, locErrMat), //call base class constructor
    m_detEl(detEl),
    m_time(time),
    m_ambiguityFlag(ambiguityFlag){}

RpcPrepData::RpcPrepData( const Identifier& RDOId,
                          const IdentifierHash &idDE,
                          const Amg::Vector2D& locpos,
                          std::vector<Identifier>&& rdoList,
                          Amg::MatrixX&& locErrMat,
                          const MuonGM::RpcReadoutElement* detEl,
                          const float time,
                          const int triggerInfo,
                          const int ambiguityFlag) :
    MuonCluster(RDOId, idDE, locpos, std::move(rdoList), std::move(locErrMat)), //call base class constructor
    m_detEl(detEl),
    m_time(time),
    m_triggerInfo(triggerInfo),
    m_ambiguityFlag(ambiguityFlag)
{ }

RpcPrepData::RpcPrepData( const Identifier& RDOId,
                          const IdentifierHash &idDE,
                          const Amg::Vector2D& locpos,
                          std::vector<Identifier>&& rdoList,
                          Amg::MatrixX&& locErrMat,
                          const MuonGM::RpcReadoutElement* detEl,
                          const float time,
                          const float timeOverThresh,
                          const int triggerInfo,
                          const int ambiguityFlag) :
    MuonCluster(RDOId, idDE, locpos, std::move(rdoList), std::move(locErrMat)), //call base class constructor
    m_detEl(detEl),
    m_time(time),
    m_timeOverThreshold(timeOverThresh),
    m_triggerInfo(triggerInfo),
    m_ambiguityFlag(ambiguityFlag)
{ }

RpcPrepData::RpcPrepData( const Identifier& RDOId,
                          const IdentifierHash &idDE,
                          const Amg::Vector2D& locpos,
                          std::vector<Identifier>&& rdoList,
                          Amg::MatrixX&& locErrMat,
                          const MuonGM::RpcReadoutElement* detEl,
                          const float time,
                          const int ambiguityFlag) :
    MuonCluster(RDOId, idDE, locpos, std::move(rdoList), std::move(locErrMat)), //call base class constructor
    m_detEl(detEl),
    m_time(time),
    m_ambiguityFlag(ambiguityFlag)
{
  
}

MsgStream&
RpcPrepData::dump( MsgStream&    stream) const
  {
    stream << MSG::INFO<<"RpcPrepData {"<<std::endl;
    
    MuonCluster::dump(stream);
 
    stream <<"time = "<<this->time()<<", ";
    stream<<"triggerInfo = "<<this->triggerInfo()<<", ";
    stream<<"ambiguityFlag = "<<this->ambiguityFlag()<<", ";
    stream<<"}"<<endmsg;

    return stream;
  }

std::ostream&
RpcPrepData::dump( std::ostream&    stream) const
  {
    stream << "RpcPrepData {"<<std::endl;
    
    MuonCluster::dump(stream);
    std::stringstream s;
    s << std::setiosflags(std::ios::fixed);
    s << std::setprecision(4);
    s <<"time = "<<this->time()<<", ";
    s <<"triggerInfo = "<<this->triggerInfo()<<", ";
    s <<"ambiguityFlag = "<<this->ambiguityFlag()<<", ";
    s <<"}"<<std::endl;
    stream << s.str();

    return stream;
  }
//end of classdef
}//end of ns

