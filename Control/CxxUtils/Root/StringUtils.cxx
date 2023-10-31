/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <CxxUtils/StringUtils.h>

#include <limits>
#include <array>
#include <functional>
#include <iostream>
#include <sstream>

namespace {
    static constexpr std::array<char, 3> allowedChars{'.', '+', '-'};
}

#define TEST_STRING(THE_STR)                                                                             \
    for (const char ch : THE_STR){                                                                       \
        if (std::isdigit(ch)|| std::isspace(ch)  ||                                                      \
            std::find(allowedChars.begin(), allowedChars.end(), ch)!= allowedChars.end()) continue;      \
        std::stringstream except_str{};                                                                  \
        except_str<<"CxxUtils::"<<__func__<<"()  "<<__LINE__<<": ";                                      \
        except_str<<"Invalid character detected in "<<THE_STR;                                           \
        throw std::runtime_error(except_str.str());                                                      \
    }                                                                                                    \

namespace CxxUtils {
    std::vector<std::string> tokenize(const std::string& str,
                                      const std::string& delimiters) {
                                    
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
                                       const std::string& delimiter){
        const std::vector<std::string> strTokens = tokenize(the_str, delimiter);
        std::vector<double> toReturn{};
        std::transform(strTokens.begin(), strTokens.end(), std::back_inserter(toReturn), 
                    [](const std::string& token){
                        return atof(token);
                    });
        return toReturn;
    }

    std::vector<int> tokenizeInt(const std::string& the_str,
                                 const std::string& delimiter) {
        const std::vector<std::string> strTokens = tokenize(the_str, delimiter);
        std::vector<int> toReturn{};
        std::transform(strTokens.begin(), strTokens.end(), std::back_inserter(toReturn), 
                        [](const std::string& token){
                            return atoi(token);
                        });
        return toReturn;
    }

    int atoi(std::string_view str) {
        TEST_STRING(str);
        if (str.empty()) return 0;
        int result{std::numeric_limits<int>::max()};
        std::stringstream sstr{};
        sstr<<str;
        sstr>>result;
        return result;
    }

    double atof(std::string_view str) {
        TEST_STRING(str);
        if (str.empty()) return 0.;
        double result{std::numeric_limits<double>::max()};
        std::stringstream sstr{};
        sstr<<str;
        sstr>>result;       
        return result;
    }
}
#undef TEST_STRING