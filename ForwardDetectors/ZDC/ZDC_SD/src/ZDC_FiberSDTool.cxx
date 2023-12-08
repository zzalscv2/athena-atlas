/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


// Class header
#include "ZDC_FiberSDTool.h"

// For the SD itself
#include "ZDC_FiberSD.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ZDC_FiberSDTool::ZDC_FiberSDTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase(type,name,parent)
{
  declareProperty("readoutPos",m_readoutPos = 511.8);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode ZDC_FiberSDTool::Gather()
{
  ATH_MSG_VERBOSE( "ZDC_FiberSDTool::Gather()" );
  if(!getSD())
    {
      ATH_MSG_ERROR ("Gather: ZDC_FiberSD never created!");
      return StatusCode::FAILURE;
    }
  else
    {
      ZDC_FiberSD *localSD = dynamic_cast<ZDC_FiberSD*>(getSD());
      if(!localSD)
        {
          ATH_MSG_ERROR ("Gather: Failed to cast m_SD into ZDC_FiberSD.");
          return StatusCode::FAILURE;
        }
      localSD->EndOfAthenaEvent();
    }
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* ZDC_FiberSDTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing ZDC_FiberSD" );

  return new ZDC_FiberSD(name(), m_outputCollectionNames[0], m_readoutPos);
}
