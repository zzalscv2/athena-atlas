/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_MAXMULTFILTER_H
#define TRIGHLTJETHYPO_MAXMULTFILTER_H

#include "./IHypoJetVectorFilter.h"
#include <sstream>
#include <ostream>

class MaxMultFilter: public IHypoJetVectorFilter  {
 public:

  MaxMultFilter(){};
  MaxMultFilter(std::size_t end);

  // find the subset of jets which satisfy a sequence of ranges
  virtual HypoJetVector
  filter (const HypoJetVector& jv,
	  const std::unique_ptr<ITrigJetHypoInfoCollector>&
	  ) const override;
  
  virtual std::string toString() const override;  
private:
  std::size_t m_end{0};
  long unsigned int m_nToSort{0u};
};

std::ostream& operator<<(std::ostream&, const MaxMultFilter&);


#endif
