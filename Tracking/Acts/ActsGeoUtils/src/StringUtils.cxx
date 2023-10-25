/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <ActsGeoUtils/StringUtils.h>
#include <ActsGeoUtils/AlgebraHelpers.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>

#include <charconv>
#include <limits>
#include <functional>
#include <iostream>

namespace ActsTrk {
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
                        return stof(token);
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
        int result{std::numeric_limits<int>::max()};
        std::from_chars(str.data(), str.data() + str.size(), result);
        return result;
    }

    double stof(std::string_view str) {
        double result{std::numeric_limits<double>::max()};
        std::from_chars(str.data(), str.data() + str.size(), result);
        return result;
    }
    std::string to_string(const Amg::Transform3D& trans, unsigned int prec) {
        std::stringstream sstr{};
        bool printed{false};
        if (trans.translation().mag() > std::numeric_limits<float>::epsilon()) {
            sstr<<"translation: "<<Amg::toString(trans.translation(),prec);
            printed = true;
        }
        if (!doesNotDeform(trans)) {
            if (printed) sstr<<", ";
            sstr<<"rotation: {"<<Amg::toString(trans.linear()*Amg::Vector3D::UnitX(),prec)<<",";
            sstr<<Amg::toString(trans.linear()*Amg::Vector3D::UnitY(),prec)<<",";
            sstr<<Amg::toString(trans.linear()*Amg::Vector3D::UnitZ(),prec)<<"}";
            printed = true;
        }
        if (!printed) sstr<<"Identity matrix ";
        return sstr.str();
    }
}