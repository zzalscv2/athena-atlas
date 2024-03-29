/* emacs: this is -*- c++ -*- */
/**
 **
 **   @file         ReadCards.h  
 **
 **                   
 **                   
 **                   
 ** 
 **   @author       M.Sutton  
 **
 **   @date         Tue Apr 26 13:44:30 CEST 2005
 **
 **   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **                   
 **                   
 **
 **/


#ifndef READCARDS_READCARDS_H
#define READCARDS_READCARDS_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <string>

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::endl;
using std::ifstream;
using std::ostringstream;
using std::exit;


#include "Value.h"

/** Get tag-value pairs from a file.
 ** Reads in all the tag-value pairs from a file 
 ** (values can actually be a vector of values) and  
 ** makes them available by tagname.
 **
 **/

class ReadCards {

public:

  ReadCards()  {  }
  ReadCards(const std::string& filename) { Construct(filename); }

  ~ReadCards() { } 

  int GetNValues() const { return m_Values.size(); }

  bool isTagDefined(const string& tag) const { 
    for ( int i=0 ;  i<GetNValues() ; i++ ) { 
      if ( tag==m_Values[i].Tag() ) return true;
    }
    return false;
  }

  const std::vector<double>  GetVector(unsigned i)  const  { 
    std::vector<double> dValues;
    if ( i<m_Values.size() ) {
      for ( unsigned j=0 ;  j<m_Values[i].Val().size() ; j++ ) {
	dValues.push_back(handle(m_Values[i].Val()[j]));
      }
    }
    return dValues;
  }

  string GetValueString(unsigned i) const  { 
    if ( i<m_Values.size() ) return m_Values[i].Tag();
    else return string("");
  }


  double GetValue(const string& tag) const {
    return handle(GetString(tag));
  }

  double GetValue(int i) const { 
    return (handle(GetString(i)));
  }

  
  std::string GetString(const std::string& tag) const {
    return GetString(GetIndex(tag));
  }

  std::string GetString(const unsigned i) const {
    if ( i<m_Values.size() ) return m_Values[i].Val()[0];
    return "";
  }
  
  const std::vector<double>  GetVector(const std::string& tag) const {
    return GetVector(GetIndex(tag));
  }


  const std::vector<int>  GetIntVector(const std::string& tag) const {
    std::vector<double> s = GetVector(GetIndex(tag));
    std::vector<int> a; a.reserve(s.size());
    for ( size_t i=0 ; i<s.size() ; i++ ) a.push_back( int(s[i]) );
    return a;
  }


  std::vector<std::string>  GetStringVector(const std::string& tag) const {
    return m_Values[GetIndex(tag)].Val();
  }




  void Set(const string& tag, double& value ) const {
    if ( isTagDefined(tag) ) value = handle(GetString(tag));
  }

  void Set(const string& tag, int& value ) const {
    if ( isTagDefined(tag) ) value = int(handle(GetString(tag)));
  }
  
  void Set(const std::string& tag, std::string& value) const {
    if ( isTagDefined(tag) ) value = GetString(GetIndex(tag));
  }
  
  void Set(const string& tag, std::vector<double>& value ) const {
    if ( isTagDefined(tag) ) value = GetVector(GetIndex(tag));
  }
  
  void Set(const string& tag, std::vector<int>& value ) const {
    if ( isTagDefined(tag) ) { 
      std::vector<double> v = GetVector(GetIndex(tag));
      value.clear();
      value.resize(v.size());
      for ( unsigned i=v.size() ; i-- ; ) value[i] = v[i];
    }
  }    

  void Set(const std::string& tag, std::vector<std::string>& value) const {
    if ( isTagDefined(tag) ) value = m_Values[GetIndex(tag)].Val();
  }

  void print();


  std::vector<string> Tags(const std::string& pattern="") const { 
    std::vector<string> tags;
    if ( pattern=="" ) { 
      for ( unsigned i=0 ; i<m_Values.size() ; i++ ) tags.push_back(m_Values[i].Tag());
    }
    else { 
      for ( unsigned i=0 ; i<m_Values.size() ; i++ ) {
	if ( m_Values[i].Tag().find(pattern)!=std::string::npos ) tags.push_back(m_Values[i].Tag());
      }
    }
    return tags;
  }

  //  void AddToPath(const std::string& dir) { mPath.push_back(dir); } 

  //  const std::vector<std::string>& GetPath() const { return mPath; } 
  template<class T>
  void declareProperty( const std::string& key, T& t ) { 
    if ( isTagDefined(key) ) t = GetValue(key);
  }
  
  template<class T>
  void declareProperty( const std::string& key, std::vector<T>& t ) { 
    if ( isTagDefined(key) ) t = GetVector(key);
  }
  


  void declareProperty( const std::string& key, std::string& t ) { 
    if ( isTagDefined(key) ) t = GetString(key);
  }

  void declareProperty( const std::string& key, std::vector<std::string>& t ) { 
    if ( isTagDefined(key) ) t = GetStringVector(key);
  }
  

private:

  void ReadParam();
  void clean();

  int GetIndex(const std::string& tag) const { 
    for ( int i=GetNValues() ; i-- ; )  if ( tag == m_Values[i].Tag() ) return i;
    cerr << "ReadCards::GetValue() no tag: " << tag << " in file: " << m_FileName << endl;
    exit(-1);
    return 0;
  }

  bool AddTag(const string& tag, const vector<string>& values) { 
    for ( unsigned i=0 ; i<m_Values.size() ; i++ ) {
      if ( tag == m_Values[i].Tag() ) {
	cerr << "ReadCards::addTag() tag " << tag << " already defined in file " << m_FileName << endl;
	exit(-1);
      }
    }
    m_Values.push_back(Value(tag,values));
    return true;
  }


private:

  std::string m_FileName;

  ifstream m_File;

  std::string    m_String;

  std::vector<Value>   m_Values;
  std::vector<int>     m_ValuesFlag;

  static std::vector<std::string> m_Path;

private:

  void Construct(const std::string& filename);

  void CreatePath();

  void           parse();      // parse a line
  string         parseleft(string& s);  // parse the lvalue
  vector<string> parseright(string& s); // parse the rvalue


  // error reporting routine
  void error(const std::string& s) {
    cerr << "ReadCards() syntax error in file " << m_FileName << " : " << s << endl;
    exit(0);
  }


  double handle(const std::string& s, const std::string& ="") const {
    double d;
    char   temps[1024] = ""; 
    int    r=0;
    if ( (r = std::sscanf(s.c_str(), "%lf%1023s", &d, temps))==0 || r!=1 || string(temps)!="" ) {
      cerr << "ReadCards() error converting string to number : " << s << endl;
      exit(0);
    }
    //    cout << "r=" << r << "   d=" << d <<"  temps=" << temps << endl;
    return d;
  }

};

#endif  /* READCARDS_READCARDS_H */










