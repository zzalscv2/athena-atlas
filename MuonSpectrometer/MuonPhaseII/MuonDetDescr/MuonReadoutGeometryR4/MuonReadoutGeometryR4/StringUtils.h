

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_STRINGUTILS_H
#define MUONREADOUTGEOMETRYR4_STRINGUTILS_H

#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>

#include <string>
#include <string_view>
#include <vector>
#include <iostream>
namespace MuonGMR4 {

/// Searches the string for the given delimiter and constructs a 
/// vector of tokens
std::vector<std::string> tokenize(const std::string& the_str,
                                  const std::string& delimiters);

std::vector<double> tokenizeDouble(const std::string& the_str,
                                   const std::string& delimiter);
int atoi(std::string_view str);

double atof(std::string_view str);

/// Dumps the transformation to string
std::string to_string(const Amg::Transform3D& trans) ; 

}  // namespace MuonGMR4
#endif
