/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "ZDC_G4CalibSDTool.h"
#include "ZDC_G4CalibSD.h"
#include "CaloG4Sim/EscapedEnergyRegistry.h"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ZDC_G4CalibSDTool::ZDC_G4CalibSDTool(const std::string &type, const std::string &name, const IInterface *parent)
    : SensitiveDetectorBase(type, name, parent)
{
  declareProperty("deadSD",m_deadSD = false);
  declareProperty("doPid",m_doPid = false);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

StatusCode ZDC_G4CalibSDTool::Gather()
{
  ATH_MSG_VERBOSE( "ZDC_G4CalibSDTool::Gather()" );
  if(!getSD())
    {
      ATH_MSG_ERROR ("Gather: ZDC_G4CalibSD never created!");
      return StatusCode::FAILURE;
    }
  else
    {
      ZDC_G4CalibSD *localSD = dynamic_cast<ZDC_G4CalibSD*>(getSD());
      if(!localSD)
        {
          ATH_MSG_ERROR ("Gather: Failed to cast m_SD into ZDC_G4CalibSD.");
          return StatusCode::FAILURE;
        }
      localSD->EndOfAthenaEvent();
    }
  return StatusCode::SUCCESS;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VSensitiveDetector* ZDC_G4CalibSDTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing ZDC_G4CalibSD" );

  ZDC_G4CalibSD *theCalibSD = new ZDC_G4CalibSD(name(), m_outputCollectionNames[0], m_doPid);

  //Create ZDC_EscapedEnergyProcessing if desired
  if (m_deadSD){
    ATH_MSG_DEBUG("Creating EscapedEnergyProcessing and adding to registry");
    // Initialize the escaped energy processing for ZDC volumes.
    std::unique_ptr<CaloG4::VEscapedEnergyProcessing> eep(new ZDC_EscapedEnergyProcessing(theCalibSD));

    CaloG4::EscapedEnergyRegistry* registry = CaloG4::EscapedEnergyRegistry::GetInstance();
    registry->AddAndAdoptProcessing( "ZDC::", std::move(eep) );
  }

  return theCalibSD;
}
