/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "ParticleJetTools/ParticleJetGhostLabelTool.h"
#include "ParticleJetTools/ParticleJetLabelCommon.h"
#include "xAODJet/JetContainer.h"
#include "AsgMessaging/Check.h"

using namespace std;
using namespace xAOD;

ParticleJetGhostLabelTool::ParticleJetGhostLabelTool(const std::string& name)
        : AsgTool(name) {
    declareProperties(*this, &m_labelnames);
    declareProperty("GhostBName", m_ghostbname="GhostBHadronsFinal", "Name of attribute for matched B hadrons.");
    declareProperty("GhostCName", m_ghostcname="GhostCHadronsFinal", "Name of attribute for matched C hadrons.");
    declareProperty("GhostTauName", m_ghosttauname="GhostTausFinal", "Name of attribute for matched Taus.");
    declareProperty("PartPtMin", m_partptmin=5000, "Minimum pT of particles for labeling (MeV)");
}

StatusCode ParticleJetGhostLabelTool::initialize()
{
  m_labelnames.check();
  return StatusCode::SUCCESS;
}

StatusCode ParticleJetGhostLabelTool::decorate(const JetContainer& jets) const
{

  using namespace std;
  using namespace xAOD;

  ATH_MSG_VERBOSE("In " << name() << "::modify()");

  for (const xAOD::Jet* jetptr: jets) {

    const Jet& jet = *jetptr;
    vector<const TruthParticle*> jetlabelpartsb = match(jet, m_ghostbname);
    vector<const TruthParticle*> jetlabelpartsc = match(jet, m_ghostcname);
    vector<const TruthParticle*> jetlabelpartstau = match(jet, m_ghosttauname);

    // remove children whose parent hadrons are also in the jet.
    // don't care about double tau jets
    // so leave them for now.

    using ParticleJetTools::childrenRemoved;
    childrenRemoved(jetlabelpartsb, jetlabelpartsb);
    childrenRemoved(jetlabelpartsb, jetlabelpartsc);
    childrenRemoved(jetlabelpartsc, jetlabelpartsc);

    // set truth label for jets above pt threshold
    // hierarchy: b > c > tau > light
    ParticleJetTools::Particles particles;
    particles.b = jetlabelpartsb;
    particles.c = jetlabelpartsc;
    particles.tau = jetlabelpartstau;
    ParticleJetTools::setJetLabels(jet, particles, m_labelnames);
  }

  return StatusCode::SUCCESS;
}


std::vector<const TruthParticle*>
ParticleJetGhostLabelTool::match(
  const xAOD::Jet& jet, const std::string& ghostname) const {

  ATH_MSG_VERBOSE("In " << name() << "::match()");

  std::vector<const xAOD::TruthParticle*> parton_links
    = jet.getAssociatedObjects<const xAOD::TruthParticle>(ghostname);

  std::vector<const xAOD::TruthParticle*> selected_partons;
  for (const xAOD::TruthParticle* part: parton_links) {
    if (part->pt() > m_partptmin) {
      selected_partons.push_back(part);
    }
  }
  return selected_partons;
}

