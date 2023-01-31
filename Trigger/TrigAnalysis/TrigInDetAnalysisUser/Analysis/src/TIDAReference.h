/// emacs: this is -*- c++ -*- 
/**
 **   @file         TIDAReference.h  
 **
 **   @author       sutt  
 **   @date         Tue 24 Jan 2023 09:40:04 GMT  
 ** 
 **   Copyright (C) 2023 sutt (sutt@cern.ch) 
 **   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration   
 **
 **/


#ifndef   TIDAREFERENCE_H
#define   TIDAREFERENCE_H

#include "TrigInDetAnalysis/TrackSelector.h"
#include "TrigInDetAnalysis/TrackFilter.h"
#include "TrigInDetAnalysis/TrigObjectMatcher.h"


#include <iostream>
#include <string>
#include <map>

namespace TIDA { 

class Reference {

public:
  
  Reference( const std::string& name, TrackSelector* selector, TrackFilter* filter=0, TrigObjectMatcher* tom=0 ) : 
    m_name(name), m_selector(selector), m_filter(filter), m_tom(tom) {  
  }
  
  ~Reference() {}

  void Clean() { 
    delete m_selector; 
    delete m_tom; 
    m_selector=0; 
    m_tom=0;
  }

  const std::string& name() const { return m_name; }

  TrackSelector*       selector()       { return m_selector; }
  const TrackSelector* selector() const { return m_selector; }
  
  TrackFilter*       filter()       { return m_filter; }
  const TrackFilter* filter() const { return m_filter; }

  TrigObjectMatcher* tom()             { return m_tom; }  
  const TrigObjectMatcher* tom() const { return m_tom; }


protected:

  std::string        m_name;

  TrackSelector*     m_selector;
  TrackFilter*       m_filter;
  TrigObjectMatcher* m_tom;

};


class ReferenceMap : public std::map<std::string,Reference>  { 

public: 

  typedef std::map<std::string,Reference>::iterator        iterator; 
  typedef std::map<std::string,Reference>::const_iterator  const_iterator; 
  typedef std::map<std::string,Reference>::value_type      value_type; 

private:
  
  ReferenceMap() { } 

};


std::ostream& operator<<( std::ostream& s, const Reference& r ) { 
  s << "[ Reference: " << r.name()             << "\n";
  s << "             " << r.selector()->size() << "\n";
  s << "             " << r.tom()->size()      << "\n";
  s << "             " << *r.tom()             << "]";
  return s;
} 


}


#endif  /* TIDAREFERENCE_H */










