/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "ParticleJetTools/ParticleJetLabelCommon.h"

#include "TruthUtils/PIDHelpers.h"

// private internal functions
namespace {
  // this returns a charm child if it exists, otherwise returns the
  // particle itself. If the truth record contains broken links return
  // a nullptr.
  const xAOD::TruthParticle* getCharmChild(const xAOD::TruthParticle* p) {
    for (unsigned int n = 0; n < p->nChildren(); n++) {
      const xAOD::TruthParticle* child = p->child(n);
      // nullptr would indicate a broken truth record, but the rest of
      // the code lets this pass silently so we'll do the same here.
      if (!child) return nullptr;
      if (MC::PID::hasCharm(child->pdgId())) {
        return getCharmChild(child);
      }
    }
    return p;
  }
}

namespace ParticleJetTools {

  // the code below is taken from ParticleJetDeltaRLabelTool with
  // minimal modification
  // --------------------------------------------------------------

    // TODO
    // can we do better by only looking at hadrons?
    inline bool isChild
        ( const xAOD::TruthParticle* p
        , const xAOD::TruthParticle* c
        ) {

        if (p->barcode() == c->barcode())
            return false;


        for (size_t iC = 0; iC < p->nChildren(); iC++) {
            const xAOD::TruthParticle* cc = p->child(iC);
	    if(!cc) continue;

            if (cc->barcode() == c->barcode()) {
                return true;
            }

            if (isChild(cc, c)) {
                return true;
            }
        }

        return false;
    }


    void childrenRemoved
        ( const std::vector<const xAOD::TruthParticle*>& parents
        , std::vector<const xAOD::TruthParticle*>& children
        ) {

        // TODO
        // this is probably very inefficient,
        // but it's simple.

        // for instance: if we remove a child from info on one parent,
        // we still loop over the child again for the next parent.

        // also, we're passing around vectors rather than their
        // references.

        // for each of the parents
        for ( size_t ip = 0
            ; ip != parents.size()
            ; ip++ ) {

            const xAOD::TruthParticle* p = parents[ip];
	    if(!p) continue;

            // the current child index
            size_t ic = 0;

            // (x) each of the potential children
            while (ic != children.size()) {

                const xAOD::TruthParticle* c = children[ic];
		if (!c) continue;

                // if citer is (recursively) a child of piter
                // remove it.
                if (isChild(p, c)) {
                    children.erase(children.begin() + ic);
                    // do not increment ic: we just removed a child.
                    continue;

                } else {
                    // increment ic: we did *not* remove a child.
                    ic++;
                    continue;
                }
            }
        }

        return;
    }


  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  // End of code copied from ParticleJetDeltaRLabelTool

  void LabelNames::check() {
    auto chk = [](const std::string& s, const std::string& varname) {
      if (s.empty()) throw std::runtime_error(
        "name for '" + varname + "' is not specified in particle jet tools"
        " configuration");
    };
#define CHECK(var) chk(var, #var)
    CHECK(singleint);
    CHECK(doubleint);
    CHECK(pt);
    CHECK(pt_scaled);
    CHECK(Lxy);
    CHECK(dr);
    CHECK(pdgId);
    CHECK(barcode);
    CHECK(childLxy);
    CHECK(childPt);
    CHECK(childPdgId);
#undef CHECK
  }

  LabelDecorators::LabelDecorators(const LabelNames& n):
    singleint(n.singleint),
    doubleint(n.doubleint),
    pt(n.pt),
    pt_scaled(n.pt_scaled),
    Lxy(n.Lxy),
    dr(n.dr),
    pdgId(n.pdgId),
    barcode(n.barcode),
    childLxy(n.childLxy),
    childPt(n.childPt),
    childPdgId(n.childPdgId)
  {
  }


  // key might be added back if we figure out how to get the store
  // gate key from a read handle in analysis base
  IParticleLinker::IParticleLinker(
    const SG::ReadHandleKey<xAOD::TruthParticleContainer>& /* key */,
    const std::string& linkname):
    m_dec(linkname)
  {
  }
  void IParticleLinker::decorate(
    const xAOD::Jet& jet,
    const std::vector<const xAOD::TruthParticle*>& ipv) const
  {
    IPLV links;
    for (const xAOD::TruthParticle* ip: ipv) {
      // I copied this whole song and dance from setAssociatedObjects
      // in the jet edm. It would be much easier if we could store the
      // container hash in this object and use ElementLink(sgkey,
      // index) but that seems to break in AnalysisBase
      IPLV::value_type link;
      const auto* ipc = dynamic_cast<const xAOD::IParticleContainer*>(
        ip->container());
      link.toIndexedElement(*ipc, ip->index());
      links.push_back(link);
    }
    m_dec(jet) = links;
  }

  void setJetLabels(const xAOD::Jet& jet,
                    const Particles& particles,
                    const LabelDecorators& decs) {

    // we also want to save information about the maximum pt particle of the labeling partons
    auto getMaxPtPart = [](const auto& container) -> const xAOD::TruthParticle* {
      if (container.size() == 0) return nullptr;
      auto itr = std::max_element(container.begin(), container.end(),
                                  [](auto* p1, auto* p2) {
                                    return p1->pt() < p2->pt();
                                  });
      return *itr;
    };

    // set truth label for jets above pt threshold
    // hierarchy: b > c > tau > light
    int label = 0; // default: light
    const xAOD::TruthParticle* labelling_particle = nullptr;
    const xAOD::TruthParticle* child_particle = nullptr;
    if (particles.b.size()) {
      label = 5;
      labelling_particle = getMaxPtPart(particles.b);
      child_particle = getCharmChild(labelling_particle);
    } else if (particles.c.size()) {
      label = 4;
      labelling_particle = getMaxPtPart(particles.c);
    } else if (particles.tau.size()) {
      label = 15;
      labelling_particle = getMaxPtPart(particles.tau);
    }

    // decorate info about the labelling particle
    decs.singleint(jet) = label;
    if (label == 0) {
      decs.pt(jet) = NAN;
      decs.pt_scaled(jet) = NAN;
      decs.Lxy(jet) = NAN;
      decs.dr(jet) = NAN;
      decs.pdgId(jet) = 0;
      decs.barcode(jet) = -1;
      decs.childLxy(jet) = NAN;
      decs.childPt(jet) = NAN;
      decs.childPdgId(jet) = 0;
    } else {
      decs.pt(jet) = partPt(labelling_particle);
      decs.pt_scaled(jet) = partPt(labelling_particle) / jet.pt();
      decs.Lxy(jet) = partLxy(labelling_particle);
      decs.dr(jet) = partDR(labelling_particle, jet);
      decs.pdgId(jet) = partPdgId(labelling_particle);
      decs.barcode(jet) = labelling_particle ?
        labelling_particle->barcode() : -1;
      decs.childLxy(jet) = partLxy(child_particle);
      decs.childPt(jet) = partPt(child_particle);
      decs.childPdgId(jet) = partPdgId(child_particle);
    }

    // extended flavour label
    if (particles.b.size()) {
      if (particles.b.size() >= 2)
        decs.doubleint(jet) = 55;

      else if (particles.c.size())
        decs.doubleint(jet) = 54;

      else
        decs.doubleint(jet) = 5;

    } else if (particles.c.size()) {
      if (particles.c.size() >= 2)
        decs.doubleint(jet) = 44;

      else
        decs.doubleint(jet) = 4;

    } else if (particles.tau.size())
      decs.doubleint(jet) = 15;

    else
      decs.doubleint(jet) = 0;

  }

  void setJetLabels(const xAOD::Jet& jet,
                    const Particles& particles,
                    const LabelNames& names) {
    setJetLabels(jet, particles, LabelDecorators(names));
  }

  float partPt(const xAOD::TruthParticle* part) {
    if (!part) return NAN;
    return part->pt();
  }
  float partLxy(const xAOD::TruthParticle* part) {
    if (!part) return NAN;
    if ( part->decayVtx() ) { return part->decayVtx()->perp(); }
    else return INFINITY;
  }
  float partDR(const xAOD::TruthParticle* part, const xAOD::Jet& jet) {
    if (!part) return NAN;
    return part->p4().DeltaR(jet.p4());
  }
  int partPdgId(const xAOD::TruthParticle* part) {
    if (!part) return 0;
    return part->pdgId();
  }

}
