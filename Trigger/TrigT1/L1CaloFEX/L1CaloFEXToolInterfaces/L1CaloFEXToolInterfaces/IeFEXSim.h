/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXSim.h  -
//                              -------------------
//     begin                : 23 03 2019
//     email                :  jacob.julian.kempster@cern.ch
//  ***************************************************************************/

#ifndef IeFEXSim_H
#define IeFEXSim_H

#include "GaudiKernel/IAlgTool.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloEvent/CaloCellContainer.h"
#include "L1CaloFEXSim/eFEXOutputCollection.h"
#include "L1CaloFEXSim/eFEXegTOB.h"
#include "L1CaloFEXSim/eFEXtauTOB.h"

namespace LVL1 {
  
/*
Interface definition for eFEXSim
*/

  static const InterfaceID IID_IeFEXSim("LVL1::IeFEXSim", 1, 0);

  class IeFEXSim : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;

    virtual void init(int id) = 0;

    virtual void reset() = 0;

    virtual void execute() = 0;
    virtual int ID() = 0;
    virtual void SetTowersAndCells_SG(int tmp[10][18]) = 0;

    virtual StatusCode NewExecute(int tmp[10][18], eFEXOutputCollection* inputOutputCollection) = 0;
    virtual std::vector<std::unique_ptr<eFEXegTOB>> getEmTOBs() = 0;
    virtual std::vector<std::unique_ptr<eFEXtauTOB>> getTauHeuristicTOBs() = 0;
    virtual std::vector<std::unique_ptr<eFEXtauTOB>> getTauBDTTOBs() = 0;

  private:

  };

  inline const InterfaceID& LVL1::IeFEXSim::interfaceID()
  {
    return IID_IeFEXSim;
  }

} // end of namespace

#endif
