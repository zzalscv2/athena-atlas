/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
namespace HepMC {
/// @brief Constant defining the barcode threshold for simulated particles, eg. can be used to separate generator event record entries from simulated particles
constexpr int SIM_BARCODE_THRESHOLD = 200000;

/// @brief Constant defining the barcode threshold for regenerated particles, i.e. particles surviving an interaction
constexpr int SIM_REGENERATION_INCREMENT = 1000000;

/// @brief Constant defining the barcode threshold for regenerated particles, i.e. particles surviving an interaction
constexpr int SIM_STATUS_INCREMENT = 100000;

/// @brief Constant definiting the status threshold for simulated particles, eg. can be used to separate generator event record entries from simulated particles
constexpr int SIM_STATUS_THRESHOLD = 20000;

namespace BarcodeBased {
inline int generations(const int& b){ return (b/SIM_REGENERATION_INCREMENT);}
inline bool  is_sim_secondary(const int& b){ return (b%SIM_REGENERATION_INCREMENT > SIM_BARCODE_THRESHOLD); }
inline bool is_simulation_vertex(const int& b){ return (b<-SIM_BARCODE_THRESHOLD);}
}

namespace StatusBased {
inline bool is_simulation_vertex(const int& s){ return (s>SIM_STATUS_THRESHOLD);}
}

/// @brief Functions for converting between the old and new barcode/status schemes
inline int new_particle_status_from_old(int oldStatus, int barcode) { return oldStatus + SIM_STATUS_INCREMENT*BarcodeBased::generations(barcode) + (BarcodeBased::is_sim_secondary(barcode)? SIM_STATUS_THRESHOLD : 0); }
inline int old_particle_status_from_new(int newStatus) { return newStatus%SIM_STATUS_THRESHOLD; }

inline int new_vertex_status_from_old(int oldStatus, int barcode) { return (BarcodeBased::is_simulation_vertex(barcode)? SIM_STATUS_THRESHOLD : 0) + oldStatus; }
inline int old_vertex_status_from_new(int newStatus) { return (StatusBased::is_simulation_vertex(newStatus) ? -SIM_STATUS_THRESHOLD : 0) + newStatus; }

}
