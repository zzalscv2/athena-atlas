/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTruthBase_TruthStrategyManager_H
#define MCTruthBase_TruthStrategyManager_H

// Framework include
#include "CxxUtils/checker_macros.h"

// ISF include
#include "ISF_Interfaces/ITruthSvc.h"
#include "ISF_Interfaces/IGeoIDSvc.h"

/// Forward declarations
class G4Step;


/// @brief Singleton class for creating truth incidents.
/// This class is gradually being refactored out of existence.

class TruthStrategyManager
{

public:

  /// Retrieve the (const) singleton instance
  static const TruthStrategyManager& GetStrategyManager();

  /// Retrieve the (non-const) singleton instance
  static TruthStrategyManager& GetStrategyManager_nc ATLAS_NOT_THREAD_SAFE ();

  /// Returns true if any of the truth strategies return true
  bool CreateTruthIncident(const G4Step*, int subDetVolLevel) const;

  /// Define which ISF TruthService to use
  void SetISFTruthSvc(ISF::ITruthSvc *truthSvc);

  /// Define which ISF GeoIDSvc to use
  void SetISFGeoIDSvc(ISF::IGeoIDSvc *geoIDSvc);

private:
  TruthStrategyManager();
  TruthStrategyManager(const TruthStrategyManager&) = delete;
  TruthStrategyManager& operator=(const TruthStrategyManager&) = delete;

  /// ISF Services the TruthStrategyManager talks to
  ISF::ITruthSvc* m_truthSvc;
  ISF::IGeoIDSvc* m_geoIDSvc;
};

#endif
