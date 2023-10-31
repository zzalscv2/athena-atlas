/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#undef NDEBUG


#include "CxxUtils/StringUtils.h"
#include <string>
#include <limits>
#include <iostream>

int main() {
    /// Test conversion of simple integers
    std::string intStr{"42"};
    if (CxxUtils::atoi(intStr) != 42) {
        std::cerr<<"String conversion of '"<<intStr<<"' failed. Expected 42 but got "<<CxxUtils::atoi(intStr)<<std::endl;
        return 1;
    }
    std::string negIntStr{"-42"};
    if (CxxUtils::atoi(negIntStr) != -42) {
        std::cerr<<"String conversion of '"<<negIntStr<<"' failed. Expected -42 but got "<<CxxUtils::atoi(negIntStr)<<std::endl;
        return 1;
    }
    std::string strWspace{"42 "};
    if (CxxUtils::atoi(strWspace) != 42) {
        std::cerr<<"String conversion of '"<<strWspace<<"' failed. Expected 42 but got "<<CxxUtils::atoi(strWspace)<<std::endl;
        return 1;
    }
    std::string strWspace1{" 42"};
    if (CxxUtils::atoi(strWspace1) != 42) {
        std::cerr<<"String conversion of '"<<strWspace1<<"' failed. Expected 42 but got "<<CxxUtils::atoi(strWspace1)<<std::endl;
        return 1;
    }

    /// check the conversion of floating point numbers
    std::string floatStr{"42.66"};
    if (CxxUtils::atoi(floatStr) != 42) {
        std::cerr<<"String conversion of '"<<floatStr<<"' failed. Expected 42 but got "<<CxxUtils::atoi(floatStr)<<std::endl;
        return 1;
    }
    if (std::abs(CxxUtils::atof(floatStr) - 42.66) > std::numeric_limits<float>::epsilon()) {
        std::cerr<<"String conversion of "<<floatStr<<" failed. Expected 42.66 but got "<<CxxUtils::atoi(floatStr)<<std::endl;
        return 1;        
    }
    /// check an empty string
    std::string emptyStr{};
    if (CxxUtils::atoi(emptyStr) || CxxUtils::atof(emptyStr)){
        std::cerr<<"Empty strings should be converted to zero. Instead function returned "
                 <<CxxUtils::atoi(emptyStr)<<" & "<<CxxUtils::atof(emptyStr)<<std::endl;
        return 1;
    }
    /// Let's check the string list
    std::vector<std::string> items{"Forklift","RitterKokusnuss", "Train","Cake", "Tortoise", "PolarBear"};
    std::string itemStr{};
    for (const std::string& it : items) itemStr+=it+";";
    std::vector<std::string> splitList = CxxUtils::tokenize(itemStr, ";");
    if (splitList.size() != items.size()) {
        std::cerr<<"The list "<<itemStr<<" is expected to be split into 6 tokens. But got "<<splitList.size()<<" elements."<<std::endl;
        for (const std::string& el : splitList) {
            std::cerr<<" *** "<<el<<std::endl;
        }
        return 1;
    }
    for (size_t i = 0 ; i < items.size(); ++i){
        if (items[i] != splitList[i]) {
            std::cerr<<"The "<<i<<"-th element is converted wrongly. Expected '"<<items[i]<<"' got '"<<splitList[i]<<std::endl;
            return 1;
        }
    }
    return 0;
}