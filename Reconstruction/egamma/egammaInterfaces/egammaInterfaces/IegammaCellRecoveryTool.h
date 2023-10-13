/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

///////////////////////////////////////////////////////////////////
// IegammaCellRecoveryTool.h, (c) ATLAS Detector software 2023
///////////////////////////////////////////////////////////////////

#ifndef EGAMMAINTERFACES_IEGAMMACELLRECOVERYTOOL_H
#define EGAMMAINTERFACES_IEGAMMACELLRECOVERYTOOL_H

/// @class IegammaCellRecoveryTool
/// @brief Interface for the Reconstruction/egamma/egammaCaloTools/egammaCellRecoveryTool

// Gaudi
#include "GaudiKernel/IAlgTool.h"
// Forward declarations
#include "xAODCaloEvent/CaloClusterFwd.h"

#include <vector>

class CaloCell;

static const InterfaceID IID_IegammaCellRecoveryTool("IegammaCellRecoveryTool", 1, 0);

class IegammaCellRecoveryTool : virtual public IAlgTool {

 public:

  /** @brief Virtual destructor */
  virtual ~IegammaCellRecoveryTool() {};
  
  /** @brief AlgTool interface methods */
  static const InterfaceID& interfaceID();
  
  class Info {
  public:
    double etamax = -999;
    double phimax = -999;
    double eCells[2] = { 0., 0. };
    unsigned short nCells[2] = { 0, 0 };
    std::vector<const CaloCell*> addedCells;
  };
  /** @brief  method: Method to get the info from missing cells*/
  virtual StatusCode execute(const xAOD::CaloCluster& cluster, Info& info) const = 0;
};

inline const InterfaceID& IegammaCellRecoveryTool::interfaceID()
{
  return IID_IegammaCellRecoveryTool;
}

#endif // EGAMMAINTERFACES_IEGAMMACELLRECOVERYTOOL_H
