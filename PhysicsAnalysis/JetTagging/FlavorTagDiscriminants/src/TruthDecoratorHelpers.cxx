/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "FlavorTagDiscriminants/TruthDecoratorHelpers.h"


namespace FlavorTagDiscriminants {
    namespace TruthDecoratorHelpers {

        bool sort_particles(const xAOD::IParticle* particle_A, 
                            const xAOD::IParticle* particle_B) {
            return particle_A->pt() < particle_B->pt();
        }

        const xAOD::TruthVertex* get_truth_vertex(
            const xAOD::TruthParticle* truth) {
            // no truth
            if ( not truth ) { return nullptr; }

            // no vertex
            const xAOD::TruthVertex* truth_vertex = truth->prodVtx();
            if ( not truth_vertex or truth_vertex->perp() > 440.0 ) { 
                return nullptr;
            }

            return truth_vertex;
        }

        float get_distance(const xAOD::TruthVertex* vertex_A, 
                                                const xAOD::TruthVertex* vertex_B) {
            if ( !vertex_A or !vertex_B ) { return 999.0; }
            return (vertex_A->v4().Vect() - vertex_B->v4().Vect()).Mag();
        }

        const xAOD::TruthParticle* get_parent_hadron(const xAOD::TruthParticle* truth_particle) {
            if ( truth_particle == nullptr ) { return nullptr; }
            if ( truth_particle->isBottomHadron() or truth_particle->isCharmHadron() ) {
                return truth_particle;
            }
            for (unsigned int p = 0; p < truth_particle->nParents(); p++) {
                const xAOD::TruthParticle* parent = truth_particle->parent(p);
                auto parent_hadron = get_parent_hadron(parent);
                if ( parent_hadron != nullptr ) {
                    return parent_hadron;
                }
            }
            return nullptr;
        }

        int get_truth_type(const xAOD::TruthParticle* truth_particle) {
            if (!truth_particle) {
                return ExclusiveType::NoTruth;
            }
            // simple pdgid check for pion/kaon based on 
            // PhysicsAnalysis/MCTruthClassifier/Root/MCTruthClassifierGen.cxx#L1159
            if (std::abs(truth_particle->pdgId()) == 211) {
                return ExclusiveType::Pion * truth_particle->charge();
            }
            if (std::abs(truth_particle->pdgId()) == 321) {
                return ExclusiveType::Kaon * truth_particle->charge();
            }
            if (truth_particle->isElectron()) {
                return ExclusiveType::Electron * truth_particle->charge() * -1;
            }
            if (truth_particle->isMuon()) {
                return ExclusiveType::Muon * truth_particle->charge() * -1;
            }
            if (std::abs(truth_particle->pdgId()) == 22) {
                return ExclusiveType::Photon;
            }
            return ExclusiveType::Other;
        }
        int get_vertex_index(const xAOD::TruthVertex* this_vertex, 
                             const xAOD::TruthVertex* truth_PV, 
                             std::vector<const xAOD::TruthVertex*>& seen_vertices,
                             const float& truthVertexMergeDistance) {
            // no vertex
            if (!this_vertex) {
                return -2;
            }
            // primary vertex
            if (get_distance(this_vertex, truth_PV) < truthVertexMergeDistance) {
                return 0;
            }
            // have we already seen this vertex?
            for ( size_t j = 0; j != seen_vertices.size(); j++) {
                float dr = get_distance(seen_vertices.at(j), this_vertex);
                if ( dr < truthVertexMergeDistance ) {
                    // a vertex is nearby, reuse it
                    return j + 1;
                }
            }
            seen_vertices.push_back(this_vertex);
            return seen_vertices.size();
        }
    }
}