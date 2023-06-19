/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometryR4/StringUtils.h>

#include <charconv>
#include <limits>
#include <functional>

namespace MuonGMR4 {
/// We should remove the MdtStringUtils class at some point
/// https://gitlab.cern.ch/atlas/athena/-/blob/master/MuonSpectrometer/MuonConditions/MuonCondGeneral/MuonCondSvc/src/MdtStringUtils.cxx
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
        //	std::cout << "Added token : " << tokens[tokens.size()-1] << "!"
        //<< std::endl;
    }
    return tokens;
}
std::vector<double> tokenizeDouble(const std::string& the_str,
                                   const std::string& delimiter){
    const std::vector<std::string> strTokens = tokenize(the_str, delimiter);
    std::vector<double> toReturn{};
    std::transform(strTokens.begin(), strTokens.end(), std::back_inserter(toReturn), [](const std::string& token){
        return stof(token);
    });
    return toReturn;
}
int atoi(std::string_view str) {
    int result{std::numeric_limits<int>::max()};
    std::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}

double stof(std::string_view str) {
    double result{std::numeric_limits<double>::max()};
    std::from_chars(str.data(), str.data() + str.size(), result);
    return result;
}
}  // namespace MuonGMR4
