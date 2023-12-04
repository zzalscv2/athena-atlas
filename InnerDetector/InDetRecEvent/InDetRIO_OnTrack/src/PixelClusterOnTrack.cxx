/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PixelClusterOnTrack.cxx, (c) ATLAS Detector Software
///////////////////////////////////////////////////////////////////

#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetPrepRawData/PixelCluster.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "GaudiKernel/MsgStream.h"
#include <ostream>


InDet::PixelClusterOnTrack::PixelClusterOnTrack(
  const InDet::PixelCluster* RIO,
  Trk::LocalParameters&& locpars,
  Amg::MatrixX&& locerr,
  const IdentifierHash& idDE,
  bool,
  bool isbroad)
  : // call base class constructor
  InDet::SiClusterOnTrack(std::move(locpars),
                          std::move(locerr),
                          idDE, RIO->identify(), isbroad)
  , m_hasClusterAmbiguity(RIO->isAmbiguous())
  , m_isFake(RIO->isFake())
  , m_energyLoss(RIO->energyLoss())
  , m_detEl(RIO->detectorElement())
{
  m_rio.setElement(RIO);
  // Set global position
  m_globalPosition = associatedSurface().localToGlobal(localParameters());
}

// Constructor with parameters
InDet::PixelClusterOnTrack::PixelClusterOnTrack(
  const InDet::PixelCluster* RIO,
  Trk::LocalParameters&& locpars,
  Amg::MatrixX&& locerr,
  const IdentifierHash& idDE,
  const Amg::Vector3D& globalPosition,
  bool,
  bool isbroad)
  : // call base class constructor
  InDet::SiClusterOnTrack(std::move(locpars),
                          std::move(locerr),
                          idDE,
                          RIO->identify(),
                          globalPosition,
                          isbroad)
  , m_hasClusterAmbiguity(RIO->isAmbiguous())
  , m_isFake(RIO->isFake())
  , m_energyLoss(RIO->energyLoss())
  , m_detEl(RIO->detectorElement())
{
  m_rio.setElement(RIO);
}



//P->T constructor
InDet::PixelClusterOnTrack::PixelClusterOnTrack
  ( const ElementLinkToIDCPixelClusterContainer& RIO,
    const Trk::LocalParameters& locpars,
    const Amg::MatrixX& locerr,
    const IdentifierHash& idDE,
    const Identifier& id,
    float energyLoss,
    bool isFake,
    bool hasClusterAmbiguity,
    bool isbroad)
 : InDet::SiClusterOnTrack(Trk::LocalParameters(locpars), Amg::MatrixX(locerr), idDE, id, isbroad),
  m_rio (RIO),
  m_hasClusterAmbiguity (hasClusterAmbiguity),
  m_isFake (isFake),
  m_energyLoss (energyLoss),
  m_detEl (nullptr)
{
  // The setting of the global position
  // happens via the setValues method
}



// Default constructor:
InDet::PixelClusterOnTrack::PixelClusterOnTrack()
    :
    InDet::SiClusterOnTrack(),
    m_rio(),
    m_hasClusterAmbiguity(false),
    m_isFake(false),
    m_energyLoss(0.),
    m_detEl(nullptr)
{}




const Trk::Surface& InDet::PixelClusterOnTrack::associatedSurface() const
{ return ( detectorElement()->surface()); }


void InDet::PixelClusterOnTrack::setValues(const Trk::TrkDetElementBase* detEl, const Trk::PrepRawData* /*prd*/)
{
    //set detector element
    m_detEl = dynamic_cast< const InDetDD::SiDetectorElement* >(detEl);
    if(m_detEl){
      //Then set global potition based on it
      m_globalPosition = associatedSurface().localToGlobal(localParameters());
    }
}



MsgStream& InDet::PixelClusterOnTrack::dump( MsgStream& sl ) const
{
    sl<<"PixelClusterOnTrack {"<<endmsg;
    InDet::SiClusterOnTrack::dump(sl); // use common dump(...) from SiClusterOnTrack
    sl<<"Ganged cluster ambiguity: "<<hasClusterAmbiguity()
      <<", fake: " << isFake()
      <<", dedX: " << energyLoss()
      <<endmsg;
    sl<<"}"<<endmsg;
    return sl;
}

std::ostream& InDet::PixelClusterOnTrack::dump( std::ostream& sl ) const
{
    sl<<"PixelClusterOnTrack {"<<std::endl;
    InDet::SiClusterOnTrack::dump(sl);// use common dump(...) from SiClusterOnTrack
    sl<<"Ganged cluster ambiguity: "<<hasClusterAmbiguity()
      <<", fake: " << isFake()
      <<", dedX: " << energyLoss()
      <<std::endl;
    sl<<"}"<<std::endl;
    return sl;
}







