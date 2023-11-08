/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <CxxUtils/StringUtils.h>

#include <limits>
#include <array>
#include <functional>
#include <iostream>
#include <sstream>
#include <charconv>

namespace CxxUtils {
    std::vector<std::string> tokenize(const std::string& str,
                                      std::string_view delimiters) {
                                    
        std::vector<std::string> tokens{};
        // Skip delimiters at beginning.
        std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
        // Find first "non-delimiter".
        std::string::size_type pos = str.find_first_of(delimiters, lastPos);
    
        while (std::string::npos != pos || std::string::npos != lastPos) {
            // Found a token, add it to the vector.
            tokens.push_back(str.substr(lastPos, pos - lastPos));
            // Skip delimiters.  Note the "not_of"
            lastPos = str.find_first_not_of(delimiters, pos);
            // Find next "non-delimiter"
            pos = str.find_first_of(delimiters, lastPos);
        }
        return tokens;
    }
    std::vector<double> tokenizeDouble(const std::string& the_str,
                                       std::string_view delimiter){
        const std::vector<std::string> strTokens = tokenize(the_str, delimiter);
        std::vector<double> toReturn{};
        std::transform(strTokens.begin(), strTokens.end(), std::back_inserter(toReturn), 
                    [](const std::string& token){
                        return atof(token);
                    });
        return toReturn;
    }
    std::string_view eraseWhiteSpaces(std::string_view str) {
        if (str.empty()) return str;
        size_t begin{0}, end{str.size() -1};
        while (std::isspace(str[begin])){
            ++begin;
        }
        while (end > 0 && std::isspace(str[end])){
            --end;
        }
        return str.substr(begin, end + 1);        
    }
    std::vector<int> tokenizeInt(const std::string& the_str,
                                 std::string_view delimiter) {
        const std::vector<std::string> strTokens = tokenize(the_str, delimiter);
        std::vector<int> toReturn{};
        std::transform(strTokens.begin(), strTokens.end(), std::back_inserter(toReturn), 
                        [](const std::string& token){
                            return atoi(token);
                        });
        return toReturn;
    }
    template <class dType> void convertToNumber(std::string_view str, dType& number) {
        /// Allow for trailing & leading white spaces
        if (str.empty()) {
            number = 0;
            return;   
        }
        if (std::find_if(str.begin(), str.end(), [](const char c){ return std::isspace(c);}) != str.end()) {
            std::string_view spaceFreeStr = eraseWhiteSpaces(str);
            /// To avoid infinite recursion because of string like '42 24' check that white spaces have been indeed removed
            if (spaceFreeStr.size() != str.size()) {
                convertToNumber(spaceFreeStr, number);
                return;
            }
        }
        if (std::from_chars(str.data(), str.data() + str.size(), number).ec !=  std::errc{}) {
            std::stringstream err_except{};
            err_except<<"CxxUtils::convertToNumber() - The string '"<<str<<"'. Contains unallowed chars";
            throw std::runtime_error(err_except.str());
        }
    }
    int atoi(std::string_view str) { 
        int result{std::numeric_limits<int>::max()};
       convertToNumber(str, result);       
        return result;
    }

    double atof(std::string_view str) {       
        double result{std::numeric_limits<double>::max()};
        convertToNumber(str, result);
        return result;
    }
}
