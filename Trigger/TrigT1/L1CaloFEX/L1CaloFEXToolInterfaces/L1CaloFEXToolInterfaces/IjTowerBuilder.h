/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           IjTowerBuilder.h  -
//                              -------------------
//     begin                : 23 03 2019
//     email                :  jacob.julian.kempster@cern.ch
//  ***************************************************************************/

#ifndef IjTowerBuilder_H
#define IjTowerBuilder_H

#include "GaudiKernel/IAlgTool.h"
#include "CaloEvent/CaloCellContainer.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"

namespace LVL1 {
  
/*
Interface definition for jTowerBuilder
*/

  static const InterfaceID IID_IjTowerBuilder("LVL1::IjTowerBuilder", 1, 0);

  class IjTowerBuilder : virtual public IAlgTool {
  public:
    static const InterfaceID& interfaceID( ) ;
    
    virtual StatusCode initialize() = 0;

    virtual void init(std::unique_ptr<jTowerContainer> & jTowerContainer) = 0;
    virtual void execute(std::unique_ptr<jTowerContainer> & jTowerContainer) = 0;
    virtual void reset() = 0;


  private:

  };

  inline const InterfaceID& LVL1::IjTowerBuilder::interfaceID()
  {
    return IID_IjTowerBuilder;
  }

} // end of namespace

#endif
