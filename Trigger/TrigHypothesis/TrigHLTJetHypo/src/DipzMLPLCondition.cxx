/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "./DipzMLPLCondition.h"
#include "./ITrigJetHypoInfoCollector.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/HypoJetDefs.h"

#include <sstream>
#include <cmath>
#include <algorithm>
#include <numeric>

DipzMLPLCondition::DipzMLPLCondition(double wp,
                            unsigned int capacity,
                            const std::string &decName_z,
                            const std::string &decName_negLogSigma2) :
                            //const std::string &decName_isValid) :
  m_workingPoint(wp),
  m_capacity(capacity),
  m_decName_z(decName_z),
  m_decName_negLogSigma2(decName_negLogSigma2)
{

}

float safeRatio(float num, float denom) {
  float ratio = (denom == 0 ? INFINITY : num / denom);
  return ratio;
}

float DipzMLPLCondition::getDipzMLPLDecValue(const pHypoJet &ip,
                                     const std::unique_ptr<ITrigJetHypoInfoCollector> &collector,
                                     const std::string &decName) const
{

  float momentValue = -1;
  if (!(ip->getAttribute(decName, momentValue)))
  {
    if (collector)
    {
      auto j_addr = static_cast<const void *>(ip.get());

      std::stringstream ss0;
      ss0 << "DipzMLPLCondition: "
          << " unable to retrieve " << decName << '\n';
      std::stringstream ss1;
      ss1 << "     jet : (" << j_addr << ")";
      collector->collect(ss0.str(), ss1.str());
    }

    throw std::runtime_error("Impossible to retrieve decorator \'" + decName + "\' for jet hypo");
  }

  return momentValue;
}

float DipzMLPLCondition::calcNum(float acml, const pHypoJet &ip, 
                      const std::unique_ptr<ITrigJetHypoInfoCollector> &collector) const {
    float zoversigma = safeRatio(getDipzMLPLDecValue(ip, collector, m_decName_z), std::pow(getDipzMLPLDecValue(ip, collector, m_decName_negLogSigma2),2));
    return acml + zoversigma;
}


float DipzMLPLCondition::calcDenom(float acml, const pHypoJet &ip, 
                      const std::unique_ptr<ITrigJetHypoInfoCollector> &collector) const {
    float oneoversigma = safeRatio(1, std::pow(getDipzMLPLDecValue(ip, collector, m_decName_negLogSigma2),2));
    return acml + oneoversigma;
}

float DipzMLPLCondition::calcLogTerm(float acml, const pHypoJet &ip,
                      const float zhat, const std::unique_ptr<ITrigJetHypoInfoCollector> &collector) const {

  float dipz_z = getDipzMLPLDecValue(ip, collector, m_decName_z);
  float dipz_sigma = getDipzMLPLDecValue(ip, collector, m_decName_negLogSigma2);
  float log_sigma = (dipz_sigma <= 0 ? -INFINITY : std::log( dipz_sigma ));

  float logterm = -0.5 * std::log(2.0 * M_PI) - log_sigma - safeRatio(std::pow(zhat - dipz_z, 2), (2.0 * std::pow(dipz_sigma, 2)));

  return acml + logterm;

}


bool DipzMLPLCondition::isSatisfied(const HypoJetVector& ips,
				      const std::unique_ptr<ITrigJetHypoInfoCollector>& collector) const {
  
  auto zhat = std::accumulate(ips.begin(),
        ips.end(),
        0.0,
        [&collector,this](float sum, const pHypoJet& jp) {
          return this->calcNum(sum, jp, collector);});
        
  zhat = safeRatio(zhat, std::accumulate(ips.begin(),
                          ips.end(),
                          0.0,
                          [&collector,this](float sum, const pHypoJet& jp) {
                          return this->calcDenom(sum, jp, collector);})
          );

  auto logproduct = std::accumulate(ips.begin(), 
        ips.end(), 
        0.0, 
        [&collector,&zhat,this](float sum, const pHypoJet& jp) {
          return this->calcLogTerm(sum, jp, zhat, collector);});

  bool pass = logproduct >= m_workingPoint;
  
  if(collector){
    std::stringstream ss0;
    const void* address = static_cast<const void*>(this);
    ss0 << "DipzMLPLCondition: (" << address << ") logproduct term "
	<< logproduct << " >= "
  << m_workingPoint << ": "
	<< std::boolalpha << pass <<  " jet group: \n";
    
    std::stringstream ss1;
    
    for(const auto& ip : ips){
      address = static_cast<const void*>(ip.get());
      ss1 << "    "  << address << " " << ip->eta() << " e " << ip->e() << '\n';
    }
    ss1 << '\n';
    collector -> collect(ss0.str(), ss1.str());
  }
  
  return pass;
  
}

std::string DipzMLPLCondition::toString() const {
  std::stringstream ss;
  const void* address = static_cast<const void*>(this);

  ss << "DipzMLPLCondition: (" << address << ") Capacity: " << m_capacity
    << " working point: " << m_workingPoint;
  ss <<'\n';

  return ss.str();
}