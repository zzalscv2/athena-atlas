/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/


#ifndef PARTICLE_JET_LABEL_COMMON_H
#define PARTICLE_JET_LABEL_COMMON_H

#include "xAODTruth/TruthParticle.h"
#include "xAODJet/Jet.h"
#include "xAODTruth/TruthVertex.h"
#include "AsgDataHandles/ReadHandleKey.h"

#include <vector>

namespace ParticleJetTools {

  struct LabelNames {
    std::string singleint;
    std::string doubleint;
    std::string pt;
    std::string pt_scaled;
    std::string Lxy;
    std::string dr;
    std::string pdgId;
  };

  struct LabelDecorators {
    LabelDecorators(const LabelNames&);
    SG::AuxElement::Decorator<int> singleint;
    SG::AuxElement::Decorator<int> doubleint;
    SG::AuxElement::Decorator<float> pt;
    SG::AuxElement::Decorator<float> pt_scaled;
    SG::AuxElement::Decorator<float> Lxy;
    SG::AuxElement::Decorator<float> dr;
    SG::AuxElement::Decorator<int> pdgId;
  };

  class IParticleLinker {
  public:
    IParticleLinker(const SG::ReadHandleKey<xAOD::TruthParticleContainer>&,
                    const std::string& linkName);
    void decorate(const xAOD::Jet&,
                  const std::vector<const xAOD::TruthParticle*>&) const;
  private:
    using IPLV = std::vector<ElementLink<xAOD::IParticleContainer>>;
    SG::AuxElement::Decorator<IPLV> m_dec;
  };

  struct Particles {
    std::vector<const xAOD::TruthParticle*> b;
    std::vector<const xAOD::TruthParticle*> c;
    std::vector<const xAOD::TruthParticle*> tau;
  };

  void setJetLabels(const xAOD::Jet& jet,
                    const Particles& particles,
                    const LabelNames& names);
  void setJetLabels(const xAOD::Jet& jet,
                    const Particles& particles,
                    const LabelDecorators& decs);

  float partPt(const xAOD::TruthParticle* part);
  float partLxy(const xAOD::TruthParticle* part);
  float partDR(const xAOD::TruthParticle* part, const xAOD::Jet& jet);
  int partPdgId(const xAOD::TruthParticle* part);
      
  void childrenRemoved
  ( const std::vector<const xAOD::TruthParticle*>& parents
    , std::vector<const xAOD::TruthParticle*>& children
    );
}

#endif
