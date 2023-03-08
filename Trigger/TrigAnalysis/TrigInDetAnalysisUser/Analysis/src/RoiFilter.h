/* emacs: this is -*- c++ -*- */
/**
 **     @file    RoiFilter.h
 **
 **     @author  mark sutton
 **     @date    Sun 31 Jul 2016 05:22:41 CEST 
 **
 **     Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef  ROIFILTER_H
#define  ROIFILTER_H

#include <iostream>
#include <cmath>

#include "TrigInDetAnalysis/TIDARoiDescriptor.h"


class RoiFilter {

public:

  RoiFilter( double eta=0, bool comp=false, size_t m=1 ) : m_eta(eta), m_comp(comp), m_mult(m) { }
  RoiFilter(TIDARoiDescriptor): m_eta(0.0),m_comp(false),m_mult(0) { }

  double       eta() const { return m_eta; }
  bool   composite() const { return m_comp; }
  size_t       size() const { return m_mult; }

  bool filter( const TIDARoiDescriptor* roi ) { 

    if ( !composite() ) { 
      if ( !roi->composite() && std::fabs(roi->eta())<=eta() ) return true;
      else                                                      return false;
    }

    if ( roi->size()<size() ) return false;
    
    size_t mult = 0;
    for ( size_t i=0 ; i<roi->size() ; i++ ) {
      if ( std::fabs(roi->at(i)->eta())<=eta() ) mult++;
    }
    
    if ( mult<size() ) return false;

    return true;
  } 

  bool filter( const TIDARoiDescriptor& roi ) { return filter( &roi ); }

private:

  double  m_eta;
  bool    m_comp;
  size_t  m_mult;


};

inline std::ostream& operator<<( std::ostream& s, const RoiFilter& r ) { 
  s << "[ RoiFilter: eta: " << r.eta();
  if ( r.composite() ) s << "\tcomposite multiplicity: " << r.size();
  return s << " ]"; 
}


#endif  // ROIFILTER_H 










