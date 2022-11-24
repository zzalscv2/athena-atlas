/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFEXSysSim.h  -
//                              -------------------
//     begin                : 23 03 2019
//     email                :  jacob.julian.kempster@cern.ch
//  ***************************************************************************/


#ifndef IjFEXSysSim_H
#define IjFEXSysSim_H

#include "GaudiKernel/IAlgTool.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODTrigger/jFexSRJetRoIContainer.h"
#include "xAODTrigger/jFexSRJetRoIAuxContainer.h"
#include "xAODTrigger/jFexLRJetRoIContainer.h"
#include "xAODTrigger/jFexLRJetRoIAuxContainer.h"
#include "xAODTrigger/jFexTauRoIContainer.h"
#include "xAODTrigger/jFexTauRoIAuxContainer.h"
#include "xAODTrigger/jFexFwdElRoIContainer.h"
#include "xAODTrigger/jFexFwdElRoIAuxContainer.h"
#include "xAODTrigger/jFexMETRoIContainer.h"
#include "xAODTrigger/jFexMETRoIAuxContainer.h"
#include "xAODTrigger/jFexSumETRoIContainer.h"
#include "xAODTrigger/jFexSumETRoIAuxContainer.h"
#include "L1CaloFEXSim/jFEXOutputCollection.h"
namespace LVL1 {
  
/*
Interface definition for jFEXSysSim
*/

  static const InterfaceID IID_IjFEXSysSim("LVL1::IjFEXSysSim", 1, 0);

  class IjFEXSysSim : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;
    
    virtual StatusCode execute(jFEXOutputCollection* inputOutputCollection) = 0;

    virtual void init() = 0;

    virtual void cleanup() = 0;

    virtual int calcTowerID(int eta, int phi, int mod) = 0 ;
    
  private:


  };

  inline const InterfaceID& LVL1::IjFEXSysSim::interfaceID()
  {
    return IID_IjFEXSysSim;
  }

} // end of namespace

#endif
