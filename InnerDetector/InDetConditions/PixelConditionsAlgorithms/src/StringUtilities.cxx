/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "StringUtilities.h"
#include <charconv>  //for std::from_chars
#include <algorithm> //for std::find_if
#include <stdexcept> //for std::runtime_error

/*
  original implementation used two string streams. A more complete std::regex_iterator
  version was tried but results in a x4 increase in parsing time for 164 elements.
  This version is approx ten times faster than the original implementation (local tests)
  and is robust against small variations in the format, e.g. actual delimiters used.
*/

namespace PixelConditionsAlgorithms{
  std::vector<std::pair<int, int>>//module hash, module status
  parseDeadMapString(const std::string & s){
    std::vector<std::pair<int, int>> result;
    //the Trigger_athenaHLT_v1Cosmic CI test returns a pair of empty braces, "{}"; trap this
    //in fact the minimal useful string would be {"d":d}, where d is a digit
    if (s.size()<7) return result;
    //a valid string is json, enclosed in braces.
    const bool is_valid = (s.front() == '{') and (s.back() == '}');
    if (not is_valid) return result;
    auto is_digit=[](const char c)->bool{
      return (c>='0') and (c<='9');
    };
    auto is_quote=[](const char c)->bool{
      return (c=='"');
    };
    const auto *pc=s.data();
    const auto *const pcEnd=pc+s.size();
    int hash{};
    int status{};
    static constexpr  std::errc success{};
    
    //database strings look like : {"12":0,"19":0,"53":0,"64":256}
    for (;pc<pcEnd;++pc){
      //fast-forward to the first quote
      pc=std::find_if(pc,pcEnd,is_quote);
      //the following converts everything up to the first non-digit character 
      //the ptr is pointing to the first non-digit character (which should be a quote)
      const auto &[ptr, errCode] = std::from_chars(++pc, pcEnd, hash);
      if (errCode!= success){
        throw std::runtime_error("Bad hash conversion from database string in PixelConditionsAlgorithms::parseDeadMapString:" +s+".");
      }
      //fast-forward to the next digit
      pc=std::find_if(ptr,pcEnd,is_digit);
      const auto &[ptr2, errCode2] = std::from_chars(pc, pcEnd, status);
      if (errCode2!= success){
        throw std::runtime_error("Bad status conversion from database string in PixelConditionsAlgorithms::parseDeadMapString:"+s+".");
      }
      pc=ptr2;
      result.emplace_back(hash, status);
    }
    
    //
    return result;
  }
}