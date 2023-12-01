/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_EMFCONDITION_H
#define TRIGHLTJETHYPO_EMFCONDITION_H

/********************************************************************
 *
 * NAME:     PtCondition.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
 *
 * AUTHOR:   P. Sherwood
 *********************************************************************/

#include <string>
#include "./ICondition.h"

namespace HypoJet{
  class IJet;
}

class ITrigJetHypoInfoCollector;

class EMFCondition: public ICondition{
 public:
  EMFCondition(double threshold);
  
  bool isSatisfied(const HypoJetVector&,
                   const std::unique_ptr<ITrigJetHypoInfoCollector>&) const override;

  virtual unsigned int capacity() const override{return s_capacity;}


  std::string toString() const override;
 private:
  
  double m_min;
  bool isSatisfied(const pHypoJet&,
                   const std::unique_ptr<ITrigJetHypoInfoCollector>&) const;
  
  const static  unsigned int s_capacity{1};
  
};

#endif
