/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGJETHYPOTOOLCONFIG_MAXMULTFILTER_H
#define TRIGJETHYPOTOOLCONFIG_MAXMULTFILTER_H

#include "ITrigHypoJetVectorFilterConfig.h"
#include "AthenaBaseComps/AthAlgTool.h"

/*
 * maxmult filter - orders jets (currently by pt), and returns
 * iterators to the 0 to end / N positions of the ordered container, if N (number of jets) < max mult requested.
 */

class TrigJetHypoToolConfig_maxmultfilter:
  public extends<AthAlgTool, ITrigHypoJetVectorFilterConfig> {
  
public:
  
  TrigJetHypoToolConfig_maxmultfilter(const std::string& type,
				    const std::string& name,
				    const IInterface* parent);
  
  virtual StatusCode initialize() override;
  virtual FilterPtr getHypoJetVectorFilter() const override;
  
 private:

  //Gaudi::Property<std::size_t>
  //m_begin{this, "begin", {0u}, "first position in range"};

  Gaudi::Property<std::size_t>
  m_end{this, "end", {0u}, "end (last + 1)  position in range"};
  
  StatusCode checkVals()  const;
 
};
#endif
