/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgTesting/UnitTest.h>

#include <regex>

//
// method implementations
//

namespace asg
{
  namespace detail
  {
    bool matchesRegex (const std::string& regex, const std::string& str)
    {
      return regex_search (str, std::regex (regex));
    }
  }
}
