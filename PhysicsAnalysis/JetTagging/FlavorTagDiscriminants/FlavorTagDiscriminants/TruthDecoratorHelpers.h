/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTH_DECORATOR_HELPERS_HH
#define TRUTH_DECORATOR_HELPERS_HH

#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/TruthParticle.h"


namespace FlavorTagDiscriminants {

    namespace TruthDecoratorHelpers {
        enum ExclusiveType {
            NoTruth  = 0,
            Other    = 1,
            Pion     = 2,
            Kaon     = 3,
            Electron = 4,
            Muon     = 5,
            Photon   = 6
        };
        bool sort_particles(const xAOD::IParticle* particle_A, const xAOD::IParticle* particle_B);
        const xAOD::TruthVertex* get_truth_vertex(const xAOD::TruthParticle* truth );
        float get_distance(const xAOD::TruthVertex* vertex_A, const xAOD::TruthVertex* vertex_B);
        int get_truth_type(const xAOD::TruthParticle* truth_particle);
        const xAOD::TruthParticle* get_parent_hadron(const xAOD::TruthParticle* truth_particle);

        int get_vertex_index(const xAOD::TruthVertex* vertex, 
                                    const xAOD::TruthVertex* truth_PV, 
                                    std::vector<const xAOD::TruthVertex*>& seen_vertices,
                                    const float& truthVertexMergeDistance);
    }
}

#endif