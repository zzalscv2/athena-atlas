/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TestProxyLoader.h, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
// Author: Thomas Gillam (thomas.gillam@cern.ch)
// ExpressionParsing library
/////////////////////////////////////////////////////////////////

#ifndef TEST_PROXY_LOADER_H
#define TEST_PROXY_LOADER_H

#include "ExpressionEvaluation/IProxyLoader.h"

#include <atomic>

namespace ExpressionParsing {
  class TestProxyLoader : public IProxyLoader {
    public:
      TestProxyLoader() : m_intAccessCount(0) { }
      virtual ~TestProxyLoader();

      virtual void reset();

      virtual IProxyLoader::VariableType variableTypeFromString(const std::string &varname) const;

      virtual int loadIntVariableFromString(const std::string &varname) const;
      virtual double loadDoubleVariableFromString(const std::string &varname) const;
      virtual std::vector<int> loadVecIntVariableFromString(const std::string &varname) const;
      virtual std::vector<double> loadVecDoubleVariableFromString(const std::string &varname) const;

    private:
      mutable std::atomic<unsigned int> m_intAccessCount;
  };
}

#endif // TEST_PROXY_LOADER_H
