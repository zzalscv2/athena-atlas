/* emacs: this is -*- c++ -*- */
/**
 **
 **   @file         Value.h  
 **
 **                   
 **                   
 **                   
 ** 
 **   @author       M.Sutton  
 **
 **   @date         Wed May  4 11:14:42 BST 2005
 **
 **   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **                   
 **                   
 **
 **/


#ifndef READCARDS_VALUE_H
#define READCARDS_VALUE_H


#include <string>
#include <vector>
#include <iostream>



// using namespace std;

/** tag-value pair class.
 ** contains a string tag and a "value", which can be a 
 ** single value, or a vector
 **
 **/

class Value {
  
public:
  
  Value()                           : m_Tag(""), m_Val(0) { }
  Value(char* s,  const std::vector<std::string>& v) : m_Tag(s),  m_Val(v) { }
  Value(char* s,  const std::string& v)              : m_Tag(s),  m_Val(0) { m_Val.push_back(v); }
  Value(const std::string& s, const std::vector<std::string>& v) : m_Tag(s),  m_Val(v) { }
  Value(const std::string& s, const std::string& v)              : m_Tag(s),  m_Val(0) { m_Val.push_back(v); }
  
  const std::string&              Tag() const {  return m_Tag;  }
  const std::vector<std::string>&  Val() const {  return m_Val;  }
  
private:
  
  std::string              m_Tag;
  std::vector<std::string> m_Val;

};


std::ostream& operator<<(std::ostream& s, const Value& v);


#endif  /* READCARDS_VALUE_H */










