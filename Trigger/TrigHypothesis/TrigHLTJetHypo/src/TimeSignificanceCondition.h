/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_TIMESIGNIFICANCECONDITION_H
#define TRIGHLTJETHYPO_TIMESIGNIFICANCECONDITION_H

/********************************************************************
 *
 * NAME:     TimeSignificanceCondition.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
 *
 *********************************************************************/

#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/IJet.h"
#include "./ICondition.h"
#include <vector>
#include <string>

class ITrigJetHypoInfoCollector;

class TimeSignificanceCondition: public ICondition {

 public:

  TimeSignificanceCondition(double t_minTimeSignificance, double t_maxTime);
  
  float getTmin(const float pt, const float m_minTimeSignificance) const;

  bool isSatisfied(const HypoJetVector&, const std::unique_ptr<ITrigJetHypoInfoCollector>&) const override;
  
  std::string toString() const override;

  virtual unsigned int capacity() const override { 
    return s_capacity;
  }

 private:

  double m_minTimeSignificance;
  double m_maxTime;
  const static unsigned int s_capacity{1};

};

#endif
