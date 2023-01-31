/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TriggerDecisionProxyLoader.h, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
// Author: Thomas Gillam (thomas.gillam@cern.ch),
// James Catmore (james.catmore@cern.ch)
// ExpressionParsing library
/////////////////////////////////////////////////////////////////

#ifndef TRIGGERDECISIONPROXYLOADER_H
#define TRIGGERDECISIONPROXYLOADER_H

#include "ExpressionEvaluation/IProxyLoader.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

namespace ExpressionParsing {
  class TriggerDecisionProxyLoader : public IProxyLoader {
    public:

      TriggerDecisionProxyLoader(ToolHandle<Trig::TrigDecisionTool>& trigDecTool) : m_trigDec(trigDecTool) { }
      virtual ~TriggerDecisionProxyLoader();
      virtual void reset();
      virtual IProxyLoader::VariableType variableTypeFromString(const std::string &varname) const;
      virtual int loadIntVariableFromString(const std::string &varname) const;
      virtual double loadDoubleVariableFromString(const std::string &varname) const;
      virtual std::vector<int> loadVecIntVariableFromString(const std::string &varname) const;
      virtual std::vector<double> loadVecDoubleVariableFromString(const std::string &varname) const;

    private:
      ToolHandle<Trig::TrigDecisionTool> m_trigDec;
  };
}

#endif // TRIGGERDECISIONPROXYLOADER_H
