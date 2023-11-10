/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <algorithm>

#include "./MaxMultFilter.h"
#include "./HypoJetPreds.h"  // HypoJetPtGreater


MaxMultFilter::MaxMultFilter(std::size_t end):
  m_end(end) {
  m_nToSort = end;
}


HypoJetVector
MaxMultFilter::filter(const HypoJetVector& jv,
		    const std::unique_ptr<ITrigJetHypoInfoCollector>&) const {

  auto nToSort = m_nToSort;
  if (m_nToSort > jv.size()) {
    nToSort = jv.size(); 
  }

  auto filtered = HypoJetVector(jv.cbegin(), jv.cend());

  std::partial_sort(filtered.begin(),
		    filtered.begin() + nToSort,
		    filtered.end(),
		    HypoJetPtGreater());

  return HypoJetVector(filtered.begin(),
		       filtered.begin() + nToSort);
}

std::string MaxMultFilter::toString() const {
  std::stringstream ss;
  const void* address = static_cast<const void*>(this);
  ss << "MaxMultFilter: (" << address << ") "
    << " end " << m_end << '\n';
  return ss.str();
}


std::ostream& operator<<(std::ostream& os, const MaxMultFilter& cf){
  os << cf.toString();
  return os;
}
