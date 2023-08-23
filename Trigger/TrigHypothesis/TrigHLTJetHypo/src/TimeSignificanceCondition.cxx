/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "./TimeSignificanceCondition.h"
#include "./ITrigJetHypoInfoCollector.h"
#include <sstream>
#include <stdexcept>
#include <limits>
#include <memory>

TimeSignificanceCondition::TimeSignificanceCondition(double t_minTimeSignificance, double t_maxTime):
  m_minTimeSignificance(t_minTimeSignificance), m_maxTime(t_maxTime) {
}
  

float TimeSignificanceCondition::getTmin(const float pt, const float minTimeSignificance) const {

  // for now these are hardcoded, but they should eventually be taken from the conditions db so they can be updated more easily
  float a {-0.812};
  float b {-0.014};
  float c {0.682};

  float minTime = minTimeSignificance*(std::exp(a+b*(pt/1000)) + c);
  return minTime;
}

bool TimeSignificanceCondition::isSatisfied(const HypoJetVector& ips, const std::unique_ptr<ITrigJetHypoInfoCollector>& collector) const{

  if (ips.size() != 1) {
    std::stringstream ss;
    ss << "TimeSignificanceCondition::isSatisfied must see exactly 1 particle, but received " << ips.size() << '\n';
    throw std::runtime_error(ss.str());
  }

  auto jet = ips[0];

  float timing {0.};
  if (!(jet->getAttribute("Timing", timing))) {
    throw std::runtime_error("ERROR: TimeSignificanceCondition cannot retrieve jet moment 'Timing'");
  }

  float pt {0.};
  if (!(pt = jet->pt())) {
    throw std::runtime_error("ERROR: TimeSignificanceCondition cannot retrieve jet pt");
  }
  
  float t_minTime = getTmin(pt, m_minTimeSignificance);  

  bool pass = (timing >= t_minTime) and (timing < m_maxTime);

  if (collector) {
    std::stringstream ss0;
    const void* address = static_cast<const void*>(this);
    ss0 << "TimeSignificanceCondition: (" << address << ") timing[" << t_minTime << ", " << m_maxTime << "]" 
        << " pass: " << std::boolalpha << pass <<  '\n';

    auto j_addr = static_cast<const void*>(jet.get());
    std::stringstream ss1;

    ss1 << "  jet : "  << j_addr << ")  timing "  << timing << '\n';
    collector->collect(ss0.str(), ss1.str());
  }
  return pass;

}

std::string TimeSignificanceCondition::toString() const {

  std::stringstream ss;

  const void* address = static_cast<const void*>(this);
  ss << "TimeSignificanceCondition: (" << address << ") Capacity: " << s_capacity
     << " timeSignificanceMin "<<  m_minTimeSignificance 
     << " timeMax " << m_maxTime 
     <<'\n';
  
  return ss.str();
}
