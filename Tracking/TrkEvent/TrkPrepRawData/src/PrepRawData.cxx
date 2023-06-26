/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PrepRawData.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkPrepRawData/PrepRawData.h"
#include "GaudiKernel/MsgStream.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include <new>

namespace Trk {

PrepRawData::PrepRawData(const Identifier& clusId,
                         const Amg::Vector2D& locpos,
                         const std::vector<Identifier>& rdoList,
                         const Amg::MatrixX& locerr)
  : Trk::ObjectCounter<Trk::PrepRawData>()
  , m_clusId(clusId)
  , m_localPos(locpos)
  , m_rdoList(rdoList)
  , m_localCovariance(locerr)
  , m_indexAndHash()
{
}

PrepRawData::PrepRawData(const Identifier& clusId,
                         const Amg::Vector2D& locpos,
                         std::vector<Identifier>&& rdoList,
                         Amg::MatrixX&& locerr)
  : Trk::ObjectCounter<Trk::PrepRawData>()
  , m_clusId(clusId)
  , m_localPos(locpos)
  , m_rdoList(std::move(rdoList))
  , m_localCovariance(std::move(locerr))
  , m_indexAndHash()
{
}

// Constructor with parameters:
PrepRawData::PrepRawData(const Identifier& clusId,
                         const Amg::Vector2D& locpos,
                         const Amg::MatrixX& locerr)
  : Trk::ObjectCounter<Trk::PrepRawData>()
  , m_clusId(clusId)
  , m_localPos(locpos)
  , m_localCovariance(locerr)
  , m_indexAndHash()
{
  m_rdoList.push_back(clusId);
}

PrepRawData::PrepRawData(const Identifier& clusId,
                         const Amg::Vector2D& locpos,
                         Amg::MatrixX&& locerr)
  : Trk::ObjectCounter<Trk::PrepRawData>()
  , m_clusId(clusId)
  , m_localPos(locpos)
  , m_localCovariance(std::move(locerr))
  , m_indexAndHash()
{
  m_rdoList.push_back(clusId);
}

// Default constructor:
PrepRawData::PrepRawData()
  : Trk::ObjectCounter<Trk::PrepRawData>()
  , m_clusId(0)
  , m_localPos()
  , m_rdoList()
  , m_localCovariance{}
  , m_indexAndHash()
{
}

PrepRawData::~PrepRawData() = default;

MsgStream&
PrepRawData::dump(MsgStream& stream) const
{
  stream << "PrepRawData object" << endmsg;
  stream << "Identifier = (" << this->identify().getString() << "), ";

  stream << "Local Position = (";
  stream << Amg::toString(this->localPosition()) << "), ";

  stream << "Local Covariance = (";
  if (this->m_localCovariance.size() != 0) {
    stream << Amg::toString(this->localCovariance()) << "), ";
  } else {
    stream << "NULL!), ";
  }

  stream << "RDO List = [";
  std::vector<Identifier>::const_iterator rdoIt = this->rdoList().begin();
  std::vector<Identifier>::const_iterator rdoItEnd = this->rdoList().end();
  for (; rdoIt != rdoItEnd; ++rdoIt) {
    stream << rdoIt->getString() << ", ";
  }
  stream << "], ";

  stream << "}" << endmsg;
  return stream;
}

std::ostream&
PrepRawData::dump(std::ostream& stream) const
{
  stream << "PrepRawData object" << std::endl;
  stream << "Identifier " << m_clusId << std::endl;
  stream << "Local Position = (";

  stream << Amg::toString(this->localPosition()) << "), ";
  stream << "Local Covariance = (";
  if (this->m_localCovariance.size() != 0) {
    stream << Amg::toString(this->localCovariance()) << "), ";
  } else {
    stream << "NULL!), ";
  }
  stream << "Collection Hash: " << m_indexAndHash.collHash()
         << "\tIndex in collection: " << m_indexAndHash.objIndex() << std::endl;
  stream << "RDO List = [";
  for (auto it : m_rdoList) {
    stream << it << std::endl;
  }
  stream << "], ";
  return stream;
}

MsgStream&
operator<<(MsgStream& stream, const PrepRawData& prd)
{
  return prd.dump(stream);
}

std::ostream&
operator<<(std::ostream& stream, const PrepRawData& prd)
{
  return prd.dump(stream);
}

} // end of ns

