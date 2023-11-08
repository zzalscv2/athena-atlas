

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef CXXUTILS_STRINGUTILS_H
#define CXXUTILS_STRINGUTILS_H

#include <string>
#include <string_view>
#include <vector>
#include <iostream>


namespace CxxUtils {
    /// Helper functions to unpack numbers decoded in string into integers and doubles
    ///  The strings are required to contain only digits, a floating point dot, the signs +- or a whitespace
    ///  Exceptions are thrown otherwise

    /// Converts a string into an integer  
    int atoi(std::string_view str);
    /// Converts a string into a double / float
    double atof(std::string_view str);
    /// Removes all trailing and starting whitespaces from a string
    std::string_view eraseWhiteSpaces(std::string_view str);

    /// Splits the string into smaller substrings
    std::vector<std::string> tokenize(const std::string& the_str,
                                      std::string_view delimiters);
    
    std::vector<double> tokenizeDouble(const std::string& the_str,
                                       std::string_view delimiter);
                                   
    std::vector<int> tokenizeInt(const std::string& the_str,
                                 std::string_view delimiter);
    

}
#endif
