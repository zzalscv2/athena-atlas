/* emacs: this is -*- c++ -*- */
/**
 **     @file    grl_run.h
 **
 **     @basic   basic run class for the good runs list 
 **
 **              contsins the run number and a vector of 
 **              ( first, last ) pairs of valid lumi block 
 **              ranges                   
 **
 **     @author  mark sutton
 **     @date    Mon 24 Oct 2016 17:12:10 CEST 
 **
 **     Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef  GRLRUN_H
#define  GRLRUN_H

#include <iostream>
#include <vector>
#include <map>


class grl_run : public std::vector<std::pair<int,int> >   {

public:

  grl_run( int run ) : m_run(run), m_size(0), m_first(0), m_last(0) { }

  grl_run( int run, int first, int last ) : m_run(run), m_size(0), m_first(0), m_last(0) {
    this->push_back( std::pair<int,int>( first, last ) );
    m_size += last-first+1;
    m_first = first;
    m_last  = last;
  } 
  
  virtual ~grl_run() { } 

  void add_range( int first, int last ) { 
    this->push_back( std::pair<int,int>( first, last ) );
    m_size += last-first+1;
    if ( last>m_last ) m_last = last;
    if ( m_first==0 || first<m_first )   m_first = first;
 
  }

  int run()   const { return m_run; }
  int first() const { return m_first; }
  int last()  const { return m_last; }

  int lbsize() const { return m_size; }

  static  bool comparison( const grl_run* r1, const grl_run* r2 ) { 
    return r1->lbsize()>r2->lbsize(); 
  }


private:
  
  int m_run;
  
  int m_size;
  
  int m_first;
  int m_last;

};



inline std::ostream& operator<<( std::ostream& s, const grl_run& g ) {
  return s << "[run: " << g.run() << "\t: " << g.first() << "\t - " << g.last() << "\t: size " << g.lbsize() << " ]";
}


#endif  // GRLRUN_H 










