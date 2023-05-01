

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_STRINGUTILS_H
#define MUONREADOUTGEOMETRYR4_STRINGUTILS_H
#include <string>
#include <string_view>
#include <vector>

namespace MuonGMR4 {

std::vector<std::string> tokenize(const std::string& the_str,
                                  const std::string& delimiters);

int atoi(std::string_view str);

double atof(std::string_view str);

}  // namespace MuonGMR4
#endif
