/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGHLTJETHYPO_DipzMLPLCONDITION_H
#define TRIGHLTJETHYPO_DipzMLPLCONDITION_H

/********************************************************************
 *
 * NAME:     DipzMLPLCondition.h
 * PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
 *
 * AUTHOR:   I. Ochoa
 *********************************************************************/

#include "./ICondition.h"

#include <string>


namespace HypoJet{
  class IJet;
}


class ITrigJetHypoInfoCollector;

class DipzMLPLCondition: public ICondition{
 public:
  DipzMLPLCondition(double wp, 
                unsigned int capacity, // this is the number of jets to be considered in each combination 
                const std::string &decName_z, 
                const std::string &decName_negLogSigma2);

  ~DipzMLPLCondition() override {}

  bool isSatisfied(const HypoJetVector&,
                   const std::unique_ptr<ITrigJetHypoInfoCollector>&) const override;

  std::string toString() const override;
  virtual unsigned int capacity() const override {return m_capacity;}

 private:
  
  double m_workingPoint;
  const unsigned int m_capacity;
  const std::string m_decName_z;
  const std::string m_decName_negLogSigma2;

  float getDipzMLPLDecValue(const pHypoJet &ip,
                         const std::unique_ptr<ITrigJetHypoInfoCollector> &collector,
                         const std::string &decName) const;

  float calcNum(float acmlt, const pHypoJet &ip,
                  const std::unique_ptr<ITrigJetHypoInfoCollector> &collector) const ;     
  float calcDenom(float acmlt, const pHypoJet &ip,
                  const std::unique_ptr<ITrigJetHypoInfoCollector> &collector) const ;                       

  float calcLogTerm(float acmlt, const pHypoJet &ip, float zhat,
                  const std::unique_ptr<ITrigJetHypoInfoCollector> &collector) const ;  

  float safeLogRatio(float num, float denom);                         
  
};

#endif
