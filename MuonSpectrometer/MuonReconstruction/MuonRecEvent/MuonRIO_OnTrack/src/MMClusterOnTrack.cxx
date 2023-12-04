/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonRIO_OnTrack/MMClusterOnTrack.h"
#include "TrkEventPrimitives/LocalParameters.h"

namespace Muon
{


  // Constructor with parameters
  MMClusterOnTrack::MMClusterOnTrack(const MMPrepData* RIO,
                                     Trk::LocalParameters&& locpos,
                                     Amg::MatrixX&& locerr,
                                     double positionAlongStrip,
                                     std::vector<float>&& stripDriftDists,
                                     std::vector<Amg::MatrixX>&& stripDriftDistErrors) :
    MuonClusterOnTrack(std::move(locpos), std::move(locerr), RIO->identify(), positionAlongStrip),
    m_detEl( RIO->detectorElement() ),
    m_stripDriftDists(std::move(stripDriftDists)),
    m_stripDriftDistErrors(std::move(stripDriftDistErrors)) {
    //Set EL
    m_rio.setElement(RIO);
  }

  MMClusterOnTrack::MMClusterOnTrack(const ElementLinkToIDC_MM_Container& RIO,
                                     Trk::LocalParameters&& locpos,
                                     Amg::MatrixX&& locerr,
                                     const Identifier& id,
                                     const MuonGM::MMReadoutElement* detEl,
                                     double positionAlongStrip,
                                     std::vector<float>&& stripDriftDists,
                                     std::vector<Amg::MatrixX>&& stripDriftDistErrors) :
    MuonClusterOnTrack(std::move(locpos), std::move(locerr), id, positionAlongStrip),  // call base class constructor
    m_rio( RIO ),
    m_detEl( detEl ),
    m_stripDriftDists(stripDriftDists),
    m_stripDriftDistErrors(stripDriftDistErrors)
  {
  }



  MsgStream& MMClusterOnTrack::dump( MsgStream&    stream) const
  {
    stream << MSG::INFO<<"MMClusterOnTrack {"<<std::endl;

    MuonClusterOnTrack::dump(stream);

    stream<<"}"<<endmsg;
    return stream;
  }

  std::ostream& MMClusterOnTrack::dump( std::ostream&    stream) const
  {
    stream << "MMClusterOnTrack {"<<std::endl;

    MuonClusterOnTrack::dump(stream);

    stream<<"}"<<std::endl;
    return stream;
  }


}


