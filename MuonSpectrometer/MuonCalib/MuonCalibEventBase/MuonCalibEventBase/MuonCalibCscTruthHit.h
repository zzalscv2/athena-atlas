/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//*********************************************************//
// Class to store truth information in                     //
// Author: Zdenko van Kesteren                             //
// Date  : 11 October 2006                                 //
//*********************************************************//

#ifndef MuonCalibCscTruthHit_h
#define MuonCalibCscTruthHit_h

#include <iostream>

#include "MuonCalibIdentifier/MuonFixedId.h"

namespace MuonCalib {
  /**
     @class MuonCalibCscTruthHit
     Class with truth information of a RPC hit, containing:
     - an identifier, stored in the MuonFixedId format (not an Identifier)
     - barcode
     - time
  */
  class MuonCalibCscTruthHit{
    public:
    MuonCalibCscTruthHit();  //!< default constructor
    MuonCalibCscTruthHit( MuonFixedId id, int barCode, double time ); //!< constructor initializing all attributes of the truth hit
    MuonCalibCscTruthHit( const MuonCalibCscTruthHit &truth );        //!< copy constructor
    ~MuonCalibCscTruthHit() {}; //!< destructor

    MuonCalibCscTruthHit& operator=( const MuonCalibCscTruthHit &rhs ); //!< assignment operator
    
    std::ostream& dump( std::ostream &stream ) const;        //!< dump to be used for operator<<() to dump the MuonCalibCscTruthHit

    MuonFixedId identify() const { return m_ID; };           //!< retrieve the MuonFixedId
    int barCode() const { return m_barCode; };               //!< retrieve the barcode
    double time() const { return m_time; };                  //!< retrieve the time
    
    void setIdentifier( MuonFixedId id ) { m_ID = id; };     //!< sets the MuonFixedId
    void setBarCode( int barCode ) { m_barCode = barCode; }; //!< sets the barcode
    void setTime ( double time ) { m_time = time; };         //!< sets the time

    private:
    MuonFixedId m_ID;  //!< identifier of the truth hit, (a MuonFixedId, not an Identifier) 
    int     m_barCode; //!< barcode
    double  m_time;    //!< time
  };

}// namespace MuonCalib

std::ostream& operator << ( std::ostream &stream, const MuonCalib::MuonCalibCscTruthHit &truth );

#endif //MuonCalibCscTruth_h
