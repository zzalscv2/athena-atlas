/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************

NAME:     CaloCellWeightCorrection.h
PACKAGE:  offline/Calorimeter/CaloUtils

AUTHORS:  Kyle Cranmer <cranmer@cern.ch>
CREATED:  February, 2005

PURPOSE: Loops over list of ICellWeightTools and applies weight to cell
       
********************************************************************/

// Calo Header files:

#include "CaloCellWeightCorrection.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloCell.h"

// For Gaudi
#include "GaudiKernel/ListItem.h"
#include "GaudiKernel/IService.h"
#include "GaudiKernel/IToolSvc.h"

// CONSTRUCTOR:

CaloCellWeightCorrection::CaloCellWeightCorrection(const std::string& type, 
				     const std::string& name, 
				     const IInterface* parent) 
  : CaloCellCorrection(type, name, parent)
{ }

// DESTRUCTOR:

CaloCellWeightCorrection::~CaloCellWeightCorrection()
= default;

//////////////////////////////////////////////////////////////
// Gaudi INITIALIZE method
//////////////////////////////////////////////////////////////

StatusCode CaloCellWeightCorrection::initialize() {

  ATH_CHECK(m_cellWeightToolNames.retrieve());
  
  // Return status code.
  return StatusCode::SUCCESS;
}


//////////////////////////////////////////////////////////////
// EXECUTE method: Correct cells in input cell container
//////////////////////////////////////////////////////////////

StatusCode
CaloCellWeightCorrection::execute (CaloCellContainer* cellCollection,
                                   const EventContext& ctx) const
{
  ATH_MSG_DEBUG( "Executing CaloCellWeightCorrection"  );

  if (!cellCollection) 
  {
    ATH_MSG_DEBUG( " Cell Correction tool receives invalid cell Collection " );
    return StatusCode::FAILURE;
  }

  // Loop over all the CaloCell Objects and call Make Correction.
  // Note that this is the base class of all the concrete correction
  // classes which implement a Make Correction method.

  for (CaloCell* cell : *cellCollection) {
    MakeCorrection ( cell, ctx );
  }

  // Done, Return success

  return StatusCode::SUCCESS;

}

void
CaloCellWeightCorrection::MakeCorrection ( CaloCell* theCell,
                                           const EventContext& /*ctx*/) const
{
  double weight = 1.0;
  for (const auto& tool : m_cellWeightToolNames) {
    // need to be able to initialize tool (i.e. set container)    
    weight *= tool->wtCell( theCell );    
  }

  theCell->scaleEnergy( weight );
}
