/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASTOOLS_FASTSIMULATIONBASE_H
#define G4ATLASTOOLS_FASTSIMULATIONBASE_H

// Base classes
#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IFastSimulation.h"

// Members
#include "G4Types.hh"
#include "G4VFastSimulationModel.hh"
#ifdef G4MULTITHREADED
#  include "tbb/concurrent_unordered_map.h"
#endif

// STL library
#include <string>
#include <vector>
#include <thread>

/// @class FastSimulationBase
/// @todo TODO needs class documentation
class FastSimulationBase : public extends<AthAlgTool, IFastSimulation> {
 public:
  FastSimulationBase(const std::string& type, const std::string& name,
                     const IInterface *parent);
  virtual ~FastSimulationBase();

  /// @brief Construct and setup the fast simulation model.
  ///
  /// This method invokes the makeFastSimModel of the derived concrete tool type
  /// and assigns the configured regions. Errors are reported if regions are
  /// missing. In multi-threading jobs, this method is called once per worker
  /// thread.
  StatusCode initializeFastSim() override;

  /** Begin of an athena event - do anything that needs to be done at the beginning of each *athena* event. */
  virtual StatusCode BeginOfAthenaEvent() override { return StatusCode::SUCCESS; }

  /** End of an athena event - do any tidying up required at the end of each *athena* event. */
  virtual StatusCode EndOfAthenaEvent() override { return StatusCode::SUCCESS; }

 protected:
  /// Retrieve the current Fast Simulation Model. In MT, this means the
  /// thread-local Fast Simulation Model. Otherwise, it is simply the single
  /// Fast Simulation Model.
  G4VFastSimulationModel* getFastSimModel();

  /// All the regions to which this fast sim is assigned
  Gaudi::Property<std::vector<std::string> > m_regionNames{this, "RegionNames", {}};
  /// This Fast Simulation has no regions associated with it.
  Gaudi::Property<bool> m_noRegions{this, "NoRegions", false};

 private:

  /// Set the current model. In hive, this gets assigned as the thread-local model
  void setFastSimModel(G4VFastSimulationModel*);
  
  /// Delete the current model.
  void deleteFastSimModel();

#ifdef G4MULTITHREADED
  /// Thread-to-FastSimModel concurrent map type
  typedef tbb::concurrent_unordered_map < std::thread::id,
                                          G4VFastSimulationModel*,
                                          std::hash<std::thread::id> > FastSimModelThreadMap_t;
  /// Concurrent map of Fast Sim Models, one for each thread
  FastSimModelThreadMap_t m_fastsimmodelThreadMap;
#else
  /// The Fast Simulation Model to which this thing corresponds
  G4VFastSimulationModel* m_FastSimModel{};
#endif
};

#endif
