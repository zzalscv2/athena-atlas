/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/


///////////////////////////////////////////////////////////////////
// RIO_OnTrack.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

//Trk
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkEventPrimitives/LocalParameters.h"
#include "TrkPrepRawData/PrepRawData.h"
// Gaudi & AthenaCommon
#include "Identifier/IdentifierHash.h"
#include "GaudiKernel/MsgStream.h"
#include <string>
#include <typeinfo>


// Constructor with parameters:
Trk::RIO_OnTrack::RIO_OnTrack(const Trk::LocalParameters& locpars,
                              const Amg::MatrixX& loccov,
                              const Identifier& id)
  : MeasurementBase(locpars, loccov)
  , Trk::ObjectCounter<Trk::RIO_OnTrack>()
  , m_identifier(id)
{
}

MsgStream& Trk::RIO_OnTrack::dump( MsgStream& sl ) const
{
    sl << "Trk::RIO_OnTrack { "<< endmsg;
    sl << "\t  identifier = "<< identify() << endmsg;
    sl << "\t  position = (" 
       << localParameters() 
       << endmsg;
    sl << "\t  has Error Matrix: "<< endmsg;
    sl<< localCovariance() <<"}"<< endmsg; 

    if (prepRawData()!=nullptr) {
        sl<<"PrepRawData: "<< (*prepRawData()) << endmsg;
    }else{
        sl<<"PrepRawData: NULL"<<endmsg;
    }
    return sl;
}

std::ostream& Trk::RIO_OnTrack::dump( std::ostream& sl ) const
{
    sl << "Trk::RIO_OnTrack { "<<std::endl;
    sl << "\t  identifier = "<< identify() << std::endl;
    sl << "\t  position = (" 
       << localParameters()
       << std::endl;
    sl << "\t  has Error Matrix: " << std::endl;
    sl << localCovariance() <<" }" << std::endl; 
    
    if (prepRawData()!=nullptr) {
        sl <<"PrepRawData: "<< (*prepRawData()) << std::endl;
    }else{
        sl<<"PrepRawData: NULL" << std::endl;
    }    
    return sl;
}

