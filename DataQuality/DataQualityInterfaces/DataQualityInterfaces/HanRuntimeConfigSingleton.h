/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef dqiHanRuntimeConfigSingleton_h
#define dqiHanRuntimeConfigSingleton_h

#include <string>

#include "CxxUtils/checker_macros.h"

namespace dqi {
  class HanRuntimeConfigSingleton
  {
  public:
    static HanRuntimeConfigSingleton& getInstance();

    ATLAS_NOT_THREAD_SAFE
    void setPath( const std::string& path ) { m_path = path; parsePathRunNumber(); }
    constexpr const std::string& getPath() const { return m_path; }
    constexpr bool pathIsRunDirectory() const { return m_runNumber > 0; }
    constexpr uint32_t getPathRunNumber() const { return m_runNumber; }

  private:
    HanRuntimeConfigSingleton() = default;
    ~HanRuntimeConfigSingleton() = default;
    HanRuntimeConfigSingleton( const HanRuntimeConfigSingleton& ) = delete;
    HanRuntimeConfigSingleton & operator=( const HanRuntimeConfigSingleton& ) = delete;
    HanRuntimeConfigSingleton( HanRuntimeConfigSingleton&& ) = delete;
    HanRuntimeConfigSingleton & operator=( HanRuntimeConfigSingleton&& ) = delete;

    ATLAS_NOT_THREAD_SAFE
    void parsePathRunNumber();

    std::string m_path;
    uint32_t m_runNumber = 0;
  };
}

#endif //dqiHanRuntimeConfigSingleton_h