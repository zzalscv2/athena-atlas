/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// MultipleProxyLoader.cxx, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
// Author: Thomas Gillam (thomas.gillam@cern.ch)
// ExpressionParsing library
/////////////////////////////////////////////////////////////////


#include "ExpressionEvaluation/MultipleProxyLoader.h"

#include <stdexcept>
#include <iostream>

namespace ExpressionParsing {
  MultipleProxyLoader::MultipleProxyLoader() :
    m_varnameToProxyLoader(proxyCache_t::Updater_t())
  {
  }

  MultipleProxyLoader::~MultipleProxyLoader()
  {
  }


  void MultipleProxyLoader::push_back(IProxyLoader *proxyLoader)
  {
    m_proxyLoaders.push_back(proxyLoader);
  }

  void MultipleProxyLoader::reset()
  {
    for (const auto &proxyLoader : m_proxyLoaders) {
      proxyLoader->reset();
    }
  }

  IProxyLoader::VariableType MultipleProxyLoader::variableTypeFromString(const std::string &varname) const
  {
    auto itr = m_varnameToProxyLoader.find(varname);
    if (itr != m_varnameToProxyLoader.end()) {
      return itr->second->variableTypeFromString(varname);
    }

    IProxyLoader::VariableType result;
    for (const auto &proxyLoader : m_proxyLoaders) {
      try {
        result = proxyLoader->variableTypeFromString(varname);
        if (result == VT_UNK) continue;
      } catch (const std::runtime_error &) {
        continue;
      }
      m_varnameToProxyLoader.emplace(varname, proxyLoader);
      return result;
    }

    throw std::runtime_error("MultipleProxyLoader: unable to find valid proxy loader for "+varname);
  }

  int MultipleProxyLoader::loadIntVariableFromString(const std::string &varname) const
  {
    return m_varnameToProxyLoader.at(varname)->loadIntVariableFromString(varname);
  }

  double MultipleProxyLoader::loadDoubleVariableFromString(const std::string &varname) const
  {
    return m_varnameToProxyLoader.at(varname)->loadDoubleVariableFromString(varname);
  }

  std::vector<int> MultipleProxyLoader::loadVecIntVariableFromString(const std::string &varname) const
  {
    return m_varnameToProxyLoader.at(varname)->loadVecIntVariableFromString(varname);
  }

  std::vector<double> MultipleProxyLoader::loadVecDoubleVariableFromString(const std::string &varname) const
  {
    return m_varnameToProxyLoader.at(varname)->loadVecDoubleVariableFromString(varname);
  }
}
