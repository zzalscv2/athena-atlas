/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORUTILS_STRINGPARSE_H
#define GENERATORUTILS_STRINGPARSE_H
#include <string>
#include <vector>
#include <sstream>

/// @brief Utility object for parsing a string into tokens and returning them as a variety of types
///
/// This code is used to parse a string, tokenising space-separated parts into component 'pieces'.
///
/// Methods can then be called to
///   1. Return the nth component as a string (piece method)
///   2. Return the nth component as an integer (intpiece and longpiece methods) if conversion is possible
///      (returns -1 if it is off the end and 0 if it cannot be converted)
///   3. Return the nth component as a double (numpiece method) if conversion is possible
///      (returns -1.1 if it is off the end and 0 if it cannot be converted)
///
/// Note that the 'num' index used to retrieve the pieces starts at 1, not the C-conventional 0.
///
/// @author Ian Hinchliffe, April 2000
/// @author Yun-Ha.Shin, June 2006
/// @author Andy Buckley, September 2012
/// @author Andrii Verbytskyi, Jul 2023
///
class StringParse {
public:

  /// Constructor, taking a string of whitespace-separated tokens
  StringParse(const std::string& input)
    : m_lstring (input)
  {
    std::istringstream instring(input);
    std::string token;
    while (instring >> token) {
      m_lsubstring.push_back(token);
    }
  }

  /// Templated function to get the num'th token as any type (via stringstream)
  template <typename T> T piece(size_t num) const; 

  /// Function to get the num'th token as an int
  int intpiece(size_t num) const;

  /// Function to get the num'th token as a long int
  long longpiece(size_t num) const;

  /// Function to get the num'th token as a double
  double numpiece(size_t num) const;
  
  std::string piece(size_t num) const;

  /// Number of tokens in the input string
  size_t num_pieces() const { return m_lsubstring.size(); }

private:

  std::string m_lstring;
  std::vector<std::string> m_lsubstring;

};

  template <> double StringParse::piece(size_t num) const {
    try {
      std::string token = (num > num_pieces()) ? "?!?" : m_lsubstring[num-1];
      size_t idx = 0;
      auto ret = std::stod(token,&idx);
      return (idx == token.size()) ? ret : std::stod("-1");
    } catch (const std::invalid_argument& ex) {
      return std::stod("-1");
    }
  }


  template <> int StringParse::piece(size_t num) const {
    try {
      std::string token = (num > num_pieces()) ? "?!?" : m_lsubstring[num-1];
      size_t idx = 0;
      auto ret = std::stoi(token,&idx);
      return (idx == token.size()) ? ret : std::stoi("-1");
    } catch (const std::invalid_argument& ex) {
      return std::stoi("-1");
    }
  }

  template <> long StringParse::piece(size_t num) const {
    try {
      std::string token = (num > num_pieces()) ? "?!?" : m_lsubstring[num-1];
      size_t idx = 0;
      auto ret = std::stol(token,&idx);
      return (idx == token.size()) ? ret : std::stol("-1");
    } catch (const std::invalid_argument& ex) {
      return std::stol("-1");
    }
  }

  /// Function to get the num'th token as a string
  template <> std::string StringParse::piece(size_t num)  const{
    return m_lsubstring.at(num-1);
  }
  
  
  /// Function to get the num'th token as an int
  int StringParse::intpiece(size_t num) const {
    if (num > num_pieces()) return -1;
    return piece<int>(num);
  }

  /// Function to get the num'th token as a long int
  long StringParse::longpiece(size_t num) const {
    if (num > num_pieces()) return -1;
    return piece<long>(num);
  }

  /// Function to get the num'th token as a double
  double StringParse::numpiece(size_t num) const {
    if (num > num_pieces()) return -1;
    return piece<double>(num);
  }

  std::string StringParse::piece(size_t num) const {
    return piece<std::string>(num);
  }
#endif
