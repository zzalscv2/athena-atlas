/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "ParticleJetTools/ParticleJetLabelCommon.h"

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


  LabelDecorators::LabelDecorators(const LabelNames& n):
    singleint(n.singleint),
    doubleint(n.doubleint),
    pt(n.pt),
    Lxy(n.Lxy)
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
    const std::vector<const xAOD::TruthParticle*>& ipv)
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
    auto partPt = [](const auto& part) -> float {
      if (!part) return NAN;
      return part->pt();
    };
    auto partLxy = [](const auto& part) -> float {
      if (!part) return NAN;
      if ( part->decayVtx() ) { return part->decayVtx()->perp(); }
      else return INFINITY;
    };

    // set truth label for jets above pt threshold
    // hierarchy: b > c > tau > light
    if (particles.b.size()) {
      decs.singleint(jet) = 5;
      const auto maxPtPart = getMaxPtPart(particles.b);
      decs.pt(jet) = partPt(maxPtPart);
      decs.Lxy(jet) = partLxy(maxPtPart);
    } else if (particles.c.size()) {
      decs.singleint(jet) = 4;
      const auto maxPtPart = getMaxPtPart(particles.c);
      decs.pt(jet) = partPt(maxPtPart);
      decs.Lxy(jet) = partLxy(maxPtPart);
    } else if (particles.tau.size()) {
      decs.singleint(jet) = 15;
      const auto maxPtPart = getMaxPtPart(particles.tau);
      decs.pt(jet) = partPt(maxPtPart);
      decs.Lxy(jet) = partLxy(maxPtPart);
    } else {
      decs.singleint(jet) = 0;
      decs.pt(jet) = NAN;
      decs.Lxy(jet) = NAN;
    }

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

}
