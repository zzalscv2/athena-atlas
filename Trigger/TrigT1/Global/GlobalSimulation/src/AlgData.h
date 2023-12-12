//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBALSIM_ALGDATA_H
#define GLOBALSIM_ALGDATA_H

#include "TrigConfData/L1Connector.h" // TriggerLine

#include <vector>
#include <string>

/*
 * The AlgData struct contains information obtained mainly from the L1Menu, but also
 * graph node information, and is used to instantiated GlobalAlg instances and to build
 * the GlobalAlg call graph.
 */

namespace GlobalSim {
  enum class AlgDataDesc {notSet, decision, sort, count, input, root};
  
  struct AlgData {
    
    std::string m_algName;     // used to retrieve TCS Alg from AlgFactory
    std::string m_className;   // used together with algName to create alg
    std::size_t m_sn;          // Graph node idntifier
    std::string m_category;
    AlgDataDesc m_desc;       
    std::vector<std::string> m_childNames;
    std::vector<std::size_t> m_childSNs;
    std::vector<TrigConf::TriggerLine> m_triggerLines;
    
    std::string toString() const {
      std::stringstream ss;
      ss << "AlgData name: " << m_algName << " sn " << m_sn
	 <<" klass " << m_className
	 <<" category " << m_category
	 << " childNames [" << m_childNames.size() <<"]: ";
      for (const auto& n : m_childNames) {ss << n << " ";}
      ss << " childSNs: "; 
      for (const auto& n : m_childSNs) {ss << n << " ";}

      ss << "description ";
      
	
      if (m_desc == AlgDataDesc::notSet) { ss << " notSet ";
      } else if (m_desc == AlgDataDesc::decision) { ss << " decision ";
      } else if (m_desc == AlgDataDesc::sort) { ss << " sort "; 
      } else if (m_desc == AlgDataDesc::count) { ss << " count "; 
      } else if (m_desc == AlgDataDesc::input) { ss << " input ";
      } else if (m_desc == AlgDataDesc::root) { ss << " root ";

      } else {
	// protection against incomplete future updates of AlgDataDesc
	throw std::runtime_error("AlgData: unknown description");
      }
      return ss.str();
    }
  };
}

#endif
