/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
//
//   @file    RoiUtil.cxx         
//   
//            non-member, non friend RoiDescriptor utility functions
//            to improve encapsulation
//
//   @author M.Sutton
// 
//


#include "IRegionSelector/IRoiDescriptor.h"
#include "IRegionSelector/RoiUtil.h"

#include <cmath>

namespace {
  constexpr double M_2PI = 2*M_PI;
  constexpr float  M_PIF = M_PI;

  constexpr double MAX_R = 1100; // maximum radius of RoI - outer TRT radius ~1070 mm - should be configurable?
}


namespace RoiUtil { 
  
class range_error : public std::exception { 
public:
  range_error( const char* s ) : std::exception(), m_str(s) { } 
  virtual const char* what() const throw() { return m_str.c_str(); }
private:
  std::string m_str;
};
  
}


/// test whether a stub is contained within the roi
bool RoiUtil::contains( const IRoiDescriptor& roi, double z0, double dzdr ) {
  const double zouter = dzdr*MAX_R + z0;

  auto contains_internal = [zouter, z0]( const IRoiDescriptor& rd )  {
    return ( z0<=rd.zedPlus() && z0>=rd.zedMinus() &&
             zouter<=rd.zedOuterPlus() && zouter>=rd.zedOuterMinus() );
  };

  if ( roi.composite() ) {
    for ( const IRoiDescriptor* rd : roi ) {
      if ( contains_internal( *rd ) ) return true;
    }
    return false;
  }
  else return contains_internal( roi );
} 


bool RoiUtil::contains_zrange( const IRoiDescriptor& roi, double z0, double dzdr, double zmin, double zmax ) { 
  const double zouter = dzdr*MAX_R + z0;

  auto contains_internal = [zouter, z0, zmin, zmax]( const IRoiDescriptor& rd )  {
    return ( z0<=zmax && z0>=zmin &&
             zouter<=rd.zedOuterPlus() && zouter>=rd.zedOuterMinus() );
  };

  if ( roi.composite() ) {
    for ( const IRoiDescriptor* rd : roi ) {
      if ( contains_internal( *rd ) ) return true;
    }
    return false;
  }
  else return contains_internal( roi );
}


/// test whether a stub is contained within the roi
bool RoiUtil::containsPhi( const IRoiDescriptor& roi, double phi ) {
  if ( roi.composite() ) {
    for ( const IRoiDescriptor* rd : roi ) {
      if ( RoiUtil::containsPhi( *rd, phi ) ) return true;
    }
    return false;
  }
  else {
    if ( roi.isFullscan() ) return true; 
    if ( roi.phiPlus()>roi.phiMinus() ) return ( phi<roi.phiPlus() && phi>roi.phiMinus() );
    else                                return ( phi<roi.phiPlus() || phi>roi.phiMinus() );
  } 
}


bool RoiUtil::containsZed( const IRoiDescriptor& roi, double z, double r ) {
  if ( roi.composite() ) {
    for ( const IRoiDescriptor* rd : roi ) {
      if ( RoiUtil::containsZed( *rd, z, r ) ) return true;
    }
    return false;
  }
  else { 
    if ( roi.isFullscan() ) return true;
    double zminus = r*roi.dzdrMinus() + roi.zedMinus();
    double zplus  = r*roi.dzdrPlus()  + roi.zedPlus();
    return ( z>=zminus && z<=zplus );
  }
}


bool RoiUtil::contains( const IRoiDescriptor& roi, double z, double r, double phi ) {
  if ( roi.composite() ) {
    for ( const IRoiDescriptor* rd : roi ) {
      if ( RoiUtil::contains( *rd, z, r, phi ) ) return true;
    }
    return false;
  }
  else { 
    return ( RoiUtil::containsZed( roi, z, r ) && RoiUtil::containsPhi( roi, phi ) );
  }
}
  

double RoiUtil::phicheck(double phi) {
  while ( phi> M_PIF ) phi-=M_2PI;
  while ( phi<-M_PIF ) phi+=M_2PI;
  if ( !(phi >= -M_PIF && phi <= M_PIF) ) { // use ! of range rather than range to also catch nan etc
    throw range_error( (std::string("phi out of range: ")+std::to_string(phi)).c_str() ); 
  } 
  return phi;
}


double RoiUtil::etacheck(double eta) {
  if ( !(eta>-100  && eta<100) ) { // check also for nan
    throw range_error( (std::string("eta out of range: ")+std::to_string(eta)).c_str() ); 
  } 
  return eta;
}


double RoiUtil::zedcheck(double zed ) {
  if ( !(zed>-100000  && zed<100000 ) ) { // check also for nan
    throw range_error( (std::string("zed out of range: ")+std::to_string(zed)).c_str() ); 
  } 
  return zed;
}


bool operator==( const IRoiDescriptor& roi0,  const IRoiDescriptor& roi1 ) { 
  
  /// trivial self comparison
  if ( &roi0 == &roi1 ) return true;

  /// same compositness ?
  if ( roi0.composite() != roi1.composite() ) return false;

  if ( !roi0.composite() ) { 
    /// not composite

    /// check full scan - all non-composite full scan rois are equivalent
    if ( roi0.isFullscan() != roi1.isFullscan() ) return false;
    if ( roi0.isFullscan() ) return true;

    /// check geometry 
    if ( std::fabs(roi0.zed()     -roi1.zed()     )>1e-7 ) return false;
    if ( std::fabs(roi0.zedPlus() -roi1.zedPlus() )>1e-7 ) return false;
    if ( std::fabs(roi0.zedMinus()-roi1.zedMinus())>1e-7 ) return false;
   
    if ( std::fabs(roi0.eta()     -roi1.eta()     )>1e-7 ) return false;
    if ( std::fabs(roi0.etaPlus() -roi1.etaPlus() )>1e-7 ) return false;
    if ( std::fabs(roi0.etaMinus()-roi1.etaMinus())>1e-7 ) return false;

    /// Fixme: naive phi differwnce - should test for the phi=pi boundary 
    ///        for the case of very close angles but wrapped differently
    if ( std::fabs(roi0.phi()     -roi1.phi()    ) >1e-7 ) return false;
    if ( std::fabs(roi0.phiPlus() -roi1.phiPlus()) >1e-7 ) return false;
    if ( std::fabs(roi0.phiMinus()-roi1.phiMinus())>1e-7 ) return false;
  } 
  else { 
    /// check constituents
    if ( roi0.size() != roi1.size() ) return false;
    for ( unsigned i=roi0.size() ; i-- ; ) if ( !( *roi0.at(i) == *roi1.at(i) ) ) return false;  
  }
  
  return true;
}



bool operator!=( const IRoiDescriptor& roi0,  const IRoiDescriptor& roi1 ) { return !(roi0==roi1); } 
