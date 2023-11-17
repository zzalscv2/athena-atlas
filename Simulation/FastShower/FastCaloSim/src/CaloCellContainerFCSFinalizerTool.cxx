/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
// Used in ATLFAST3

/********************************************************************

NAME:     CaloCellContainerFCSFinalizerTool
PACKAGE:  athena/Simulation/FastShower/FastCaloSim

AUTHORS:  David Rousseau (modified by Xiaozhong Huang)
CREATED:  Jan 25,2019

PURPOSE:  Apply necessary finalising operation to CaloCellContainer
          Remove any checks since the CaloCellContainer is complete
          and ordered from the beginning
********************************************************************/

#include "CaloCellContainerFCSFinalizerTool.h"

#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloConstCellContainer.h"

/////////////////////////////////////////////////////////////////////
// CONSTRUCTOR:
/////////////////////////////////////////////////////////////////////

CaloCellContainerFCSFinalizerTool::CaloCellContainerFCSFinalizerTool(
                                                                     const std::string& type,
                                                                     const std::string& name,
                                                                     const IInterface* parent)
  :base_class(type, name, parent)
{
}


template <class CONTAINER>
StatusCode CaloCellContainerFCSFinalizerTool::doProcess(CONTAINER* theCont ) const
{

  theCont->updateCaloIterators();

  return StatusCode::SUCCESS;
}


StatusCode
CaloCellContainerFCSFinalizerTool::process (CaloCellContainer * theCont,
                                            const EventContext& /*ctx*/) const
{
  CHECK( doProcess (theCont) );
  return StatusCode::SUCCESS;
}


StatusCode
CaloCellContainerFCSFinalizerTool::process (CaloConstCellContainer * theCont,
                                            const EventContext& /*ctx*/) const
{
  // Container will automatically be locked when recorded.
  return doProcess (theCont);
}
