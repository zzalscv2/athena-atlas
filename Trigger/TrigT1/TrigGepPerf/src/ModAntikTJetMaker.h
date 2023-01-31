/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MODANTIKTJETMAKER_H
#define MODANTIKTJETMAKER_H

#include "./IJetMaker.h"

#include "./Jet.h"
#include "./Cluster.h"

namespace Gep{
  class ModAntikTJetMaker: public IJetMaker
  {

  public:

    ModAntikTJetMaker(const std::string& alg = "ModAntikT", int nIter = 10000, float jetR = 0.4) :
      m_jetAlg{alg},
      m_nIter{nIter},
      m_jetR{jetR} {}

    virtual std::vector<Gep::Jet> makeJets( const std::vector<Gep::Cluster> &clusters) const override;
    virtual std::string toString() const override;

    void setName(const std::string& jetAlg) { m_jetAlg = jetAlg;}
    //For running modified algorithm
    void setNIter(int nIter) { m_nIter = nIter;}
    void setJetR(float jetR) { m_jetR = jetR;}


  private:

    std::string m_jetAlg;

    // For modified anti-kT
    int m_nIter;
    float m_jetR;

  };
}

#endif

      
