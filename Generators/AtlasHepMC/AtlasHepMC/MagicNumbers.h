/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/* Author: Andrii Verbytskyi andrii.verbytskyi@mpp.mpg.de */

#ifndef ATLASHEPMC_MAGICNUMBERS_H
#define ATLASHEPMC_MAGICNUMBERS_H

#include <limits>
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
namespace HepMC {
/// @brief Constant defining the barcode threshold distinguishing generator record entries from detector sim ones
/// @todo The sim barcodes start at 1M in MC15, so we should update the 200k threshold,
///   but >= 200k is still a valid test for b = 1M so let's keep it this way until MC12 is long-dead.
constexpr int SIM_BARCODE_THRESHOLD = 200000;
constexpr int SIM_REGENERATION_INCREMENT = 1000000;

constexpr int PARTONPDGMAX = 43;
constexpr int NPPDGMIN = 1000000;
constexpr int NPPDGMAX = 8999999;
constexpr int PHOTOSMIN = 10000;
constexpr int SPECIALSTATUS = 10902;

/// @brief This barcode is used by objects matched to particles from pile-up
/// interactions in standard MC Production
constexpr int crazyParticleBarcode(std::numeric_limits<int32_t>::max());

constexpr int INVALID_PARTICLE_BARCODE = -1;

template <class T>  inline bool is_simulation_particle(const T& p){ return (p->barcode()>SIM_BARCODE_THRESHOLD);}
template <>  inline bool is_simulation_particle(const int& b){ return (b>SIM_BARCODE_THRESHOLD);}
#ifdef HEPMC3
template <>  inline bool is_simulation_particle(const ConstGenParticlePtr& p){ return (barcode(p)>SIM_BARCODE_THRESHOLD);}
template <>  inline bool is_simulation_particle(const GenParticlePtr& p){ return (barcode(p)>SIM_BARCODE_THRESHOLD);}
#endif


template <class T>  inline int generations(const T& p){ return (p->barcode()/SIM_REGENERATION_INCREMENT);}
template <>  inline int generations(const int& b){ return (b/SIM_REGENERATION_INCREMENT);}
#ifdef HEPMC3
template <>  inline int generations(const ConstGenParticlePtr& p){ return (barcode(p)/SIM_REGENERATION_INCREMENT);}
template <>  inline int generations(const GenParticlePtr& p){ return (barcode(p)/SIM_REGENERATION_INCREMENT);}
#endif


template <class T>  inline bool is_simulation_vertex(const T& p){ return (p->barcode()<-SIM_BARCODE_THRESHOLD);}
template <>  inline bool is_simulation_vertex(const int& p){ return (p<-SIM_BARCODE_THRESHOLD);}
#ifdef HEPMC3
template <>  inline bool is_simulation_vertex(const ConstGenVertexPtr& p){ return (barcode(p)<-SIM_BARCODE_THRESHOLD);}
template <>  inline bool is_simulation_vertex(const GenVertexPtr& p){ return (barcode(p)<-SIM_BARCODE_THRESHOLD);}
#endif


template <class T>  inline bool is_truthhelper_generator_particle(const T& p) {
    return (!is_simulation_particle(p)) &&
           (p->status() < 200 ||
            p->status() % 1000 == 1 || p->status() % 1000 == 2 ||
            p->status() == 10902);
}



}
#endif
