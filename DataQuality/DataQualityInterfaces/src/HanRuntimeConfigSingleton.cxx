/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "DataQualityInterfaces/HanRuntimeConfigSingleton.h"

#include <charconv>
#include <string_view>
#include <system_error>

namespace dqi {

    HanRuntimeConfigSingleton&
    HanRuntimeConfigSingleton::
    getInstance()
    {
         //Thread safe according to:
         // - paragraph 6.7.4 of C++11 and C++14 standards
         // - paragraph 9.7.4 of C++17 standard
         // - paragraph 8.8.4 of C++20 standard
        static HanRuntimeConfigSingleton instance ATLAS_THREAD_SAFE; 
        return instance;
    }

    void
    HanRuntimeConfigSingleton::
    parsePathRunNumber()
    {
        std::string_view path = m_path;
        while ( !path.empty() && path.front() == '/' ) path.remove_prefix( 1 );
        while ( !path.empty() && path.back() == '/' ) path.remove_suffix( 1 );
        if ( path.empty() ) return;

        using std::operator""sv;
        // Replace by starts_with in C++20
        if ( path.substr( 0, 4 ) != "run_"sv ) return;
        path.remove_prefix( 4 );
        auto begin = path.data();
        auto end = begin + path.size();
        auto [ptr, err] = std::from_chars( begin, end, m_runNumber );
        if ( err != std::errc() && ptr != end ) m_runNumber = 0;
    }
}