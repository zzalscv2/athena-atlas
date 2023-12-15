#include "MaxCombinationCondition.h"
#include "CombinationsIterator.h"
#include <algorithm>

template<typename T>
MaxCombinationCondition<T>::MaxCombinationCondition(std::size_t k,
						    std::unique_ptr<ICondition> cond):
  m_k{k},m_acceptingCondition{std::move(cond)} {
}

template<typename T>
bool
MaxCombinationCondition<T>::isSatisfied(const HypoJetVector& hjv,
					const std::unique_ptr<ITrigJetHypoInfoCollector>& c) const {
  auto n = hjv.size();
  if (n < m_k) {return false;}
  auto begin = CombinationsIterator(m_k, hjv);
  auto end = begin.endIter();
  
  
  return m_acceptingCondition->isSatisfied(*std::max_element(begin,
							     end,
							     T(hjv)), c);
}


template<typename T>
std::string MaxCombinationCondition<T>::toString() const {
  std::stringstream ss;
  ss <<  "MaxCombinationCondition:"
     << " k " << m_k
     << " capacity " << s_capacity 
     << " accepting condition " << m_acceptingCondition->toString()
     << '\n';
  return ss.str();
  
}
