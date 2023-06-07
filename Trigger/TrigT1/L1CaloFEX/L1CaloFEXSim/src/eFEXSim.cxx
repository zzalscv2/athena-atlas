/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXSim  -  description
//                              -------------------
//     begin                : 12 07 2019
//     email                : jacob.julian.kempster@cern.ch
//  ***************************************************************************/

#include "L1CaloFEXSim/eFEXSim.h"
#include "L1CaloFEXSim/eTower.h"
#include "L1CaloFEXSim/eFEXFPGA.h"
#include "L1CaloFEXSim/eTowerContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/ServiceHandle.h"

namespace LVL1 {

  eFEXSim::eFEXSim(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent)
  {
    declareInterface<IeFEXSim>(this);
  }


  //---------------- Initialisation -------------------------------------------------

  StatusCode eFEXSim::initialize()
  {
    ATH_CHECK( m_eFEXFPGATool.retrieve() );
    return StatusCode::SUCCESS;
  }

  //---------------- Finalisation -------------------------------------------------

  StatusCode eFEXSim::finalize()
  {
    return StatusCode::SUCCESS;
  }


  void eFEXSim::reset()
  {

    int rows = sizeof m_eTowersIDs / sizeof m_eTowersIDs[0];
    int cols = sizeof m_eTowersIDs[0] / sizeof m_eTowersIDs[0][0];

    m_id = -1;
    m_eFEXFPGACollection.clear();
    for (int i=0; i<rows; i++){
      for (int j=0; j<cols; j++){
	  m_eTowersIDs[i][j] = 0;
	}
    }
    
  }

  void eFEXSim::init(int id)
  {
    m_id = id;
  }

  /** Destructor */
  eFEXSim::~eFEXSim()
  {
  }
  
 void eFEXSim::execute(){

 }

StatusCode eFEXSim::NewExecute(int tmp_eTowersIDs_subset[10][18], eFEXOutputCollection* inputOutputCollection){
  m_emTobObjects.clear();
  m_tauHeuristicTobObjects.clear();
  m_tauBDTTobObjects.clear();

  std::copy(&tmp_eTowersIDs_subset[0][0], &tmp_eTowersIDs_subset[0][0]+(10*18),&m_eTowersIDs[0][0]);

  int tmp_eTowersIDs_subset_FPGA[10][6];

  
  //FPGA 0----------------------------------------------------------------------------------------------------------------------------------------------
  memset(tmp_eTowersIDs_subset_FPGA, 0, sizeof tmp_eTowersIDs_subset_FPGA);
  for (int myrow = 0; myrow<10; myrow++){
    for (int mycol = 0; mycol<6; mycol++){
      tmp_eTowersIDs_subset_FPGA[myrow][mycol] = tmp_eTowersIDs_subset[myrow][mycol];
    }
  }
  ATH_CHECK(m_eFEXFPGATool->init(0, m_id));
  m_eFEXFPGATool->SetTowersAndCells_SG(tmp_eTowersIDs_subset_FPGA);
  ATH_CHECK(m_eFEXFPGATool->execute(inputOutputCollection));
  m_emTobObjects.push_back(m_eFEXFPGATool->getEmTOBs());
  m_tauHeuristicTobObjects.push_back(m_eFEXFPGATool->getTauHeuristicTOBs());
  m_tauBDTTobObjects.push_back(m_eFEXFPGATool->getTauBDTTOBs());
  m_eFEXFPGATool->reset();
  //FPGA 0----------------------------------------------------------------------------------------------------------------------------------------------
  
  //FPGA 1----------------------------------------------------------------------------------------------------------------------------------------------
  memset(tmp_eTowersIDs_subset_FPGA, 0, sizeof tmp_eTowersIDs_subset_FPGA);
  for (int myrow = 0; myrow<10; myrow++){
    for (int mycol = 4; mycol<10; mycol++){
      tmp_eTowersIDs_subset_FPGA[myrow][mycol-4] = tmp_eTowersIDs_subset[myrow][mycol];
    }
  }
  ATH_CHECK(m_eFEXFPGATool->init(1, m_id));
  m_eFEXFPGATool->SetTowersAndCells_SG(tmp_eTowersIDs_subset_FPGA);
  ATH_CHECK(m_eFEXFPGATool->execute(inputOutputCollection));
  m_emTobObjects.push_back(m_eFEXFPGATool->getEmTOBs());
  m_tauHeuristicTobObjects.push_back(m_eFEXFPGATool->getTauHeuristicTOBs());
  m_tauBDTTobObjects.push_back(m_eFEXFPGATool->getTauBDTTOBs());
  m_eFEXFPGATool->reset();
  //FPGA 1----------------------------------------------------------------------------------------------------------------------------------------------


  //FPGA 2----------------------------------------------------------------------------------------------------------------------------------------------
  memset(tmp_eTowersIDs_subset_FPGA, 0, sizeof tmp_eTowersIDs_subset_FPGA);
  for (int myrow = 0; myrow<10; myrow++){
    for (int mycol = 8; mycol<14; mycol++){
      tmp_eTowersIDs_subset_FPGA[myrow][mycol-8] = tmp_eTowersIDs_subset[myrow][mycol];
    }
  }
  ATH_CHECK(m_eFEXFPGATool->init(2, m_id));
  m_eFEXFPGATool->SetTowersAndCells_SG(tmp_eTowersIDs_subset_FPGA);
  ATH_CHECK(m_eFEXFPGATool->execute(inputOutputCollection));
  m_emTobObjects.push_back(m_eFEXFPGATool->getEmTOBs());
  m_tauHeuristicTobObjects.push_back(m_eFEXFPGATool->getTauHeuristicTOBs());
  m_tauBDTTobObjects.push_back(m_eFEXFPGATool->getTauBDTTOBs());
  m_eFEXFPGATool->reset();
  //FPGA 2----------------------------------------------------------------------------------------------------------------------------------------------

  //FPGA 3----------------------------------------------------------------------------------------------------------------------------------------------
  memset(tmp_eTowersIDs_subset_FPGA, 0, sizeof tmp_eTowersIDs_subset_FPGA);
  for (int myrow = 0; myrow<10; myrow++){
    for (int mycol = 12; mycol<18; mycol++){
      tmp_eTowersIDs_subset_FPGA[myrow][mycol-12] = tmp_eTowersIDs_subset[myrow][mycol];
    }
  }
  ATH_CHECK(m_eFEXFPGATool->init(3, m_id));
  m_eFEXFPGATool->SetTowersAndCells_SG(tmp_eTowersIDs_subset_FPGA);
  ATH_CHECK(m_eFEXFPGATool->execute(inputOutputCollection));
  m_emTobObjects.push_back(m_eFEXFPGATool->getEmTOBs());
  m_tauHeuristicTobObjects.push_back(m_eFEXFPGATool->getTauHeuristicTOBs());
  m_tauBDTTobObjects.push_back(m_eFEXFPGATool->getTauBDTTOBs());
  m_eFEXFPGATool->reset();
  //FPGA 3----------------------------------------------------------------------------------------------------------------------------------------------

  return StatusCode::SUCCESS;

}


std::vector<std::unique_ptr<eFEXegTOB>> eFEXSim::getEmTOBs()
{

  std::vector<std::unique_ptr<eFEXegTOB>> tobsSort;
  tobsSort.clear();
  //bool first = true;

  // concatonate tobs from the fpgas
  // As we're using unique_ptrs here we have to move rather than copy
  for(auto &j : m_emTobObjects){
    for (auto &k : j) {
       tobsSort.push_back(std::move(k));
    }
  }

  ATH_MSG_DEBUG("number of tobs: " <<tobsSort.size() << " in eFEX: " << m_id);

  // Moving all TOB sorting to eFEXSysSim to allow xTOB generation
  // Keep this just in case a more subtle need is discovered
  /*
  // sort the tobs from the fpgas by their et (last 12 bits of 32 bit word)
  std::sort (tobsSort.begin(), tobsSort.end(), TOBetSort<eFEXegTOB>);

  // return the 6 highest ET TOBs from the efex
  if (tobsSort.size() > 6) tobsSort.resize(6);
  */
  return tobsSort;
}


std::vector<std::unique_ptr<eFEXtauTOB>> eFEXSim::getTauTOBs(std::vector<std::vector<std::unique_ptr<eFEXtauTOB>> >& tauTobObjects)
{

  std::vector<std::unique_ptr<eFEXtauTOB>> tobsSort;
  tobsSort.clear();

  // concatenate tobs from the fpgas
  // As we're using unique_ptrs here we have to move rather than copy
  for( auto &j : tauTobObjects ){
    for (auto &k : j) {
       tobsSort.push_back(std::move(k));
    }
  }

  ATH_MSG_DEBUG("number of tau tobs: " << tobsSort.size() << " in eFEX: " << m_id);

  // Moving all TOB sorting to eFEXSysSim to allow xTOB generation
  // Keep this just in case a more subtle need is discovered
  /*
  // sort the tobs from the fpgas by their et (last 12 bits of 32 bit word)
  std::sort( tobsSort.begin(), tobsSort.end(), TOBetSort<eFEXtauTOB>);

  // return the tob 6 highest ET TOBs from the efex
  if (tobsSort.size() > 6) tobsSort.resize(6);
  */
  return tobsSort;
}

std::vector<std::unique_ptr<eFEXtauTOB>> eFEXSim::getTauHeuristicTOBs()
{
  return getTauTOBs(m_tauHeuristicTobObjects);
}

std::vector<std::unique_ptr<eFEXtauTOB>> eFEXSim::getTauBDTTOBs()
{
  return getTauTOBs(m_tauBDTTobObjects);
}

void eFEXSim::SetTowersAndCells_SG(int tmp_eTowersIDs_subset[10][18]){ // METHOD USING ONLY IDS

  std::copy(&tmp_eTowersIDs_subset[0][0], &tmp_eTowersIDs_subset[0][0]+(10*18),&m_eTowersIDs[0][0]);
  
  int tmp_eTowersIDs_subset_FPGA[10][6];
  
  //FPGA 0----------------------------------------------------------------------------------------------------------------------------------------------
  memset(tmp_eTowersIDs_subset_FPGA, 0, sizeof tmp_eTowersIDs_subset_FPGA);
  for (int myrow = 0; myrow<10; myrow++){
    for (int mycol = 0; mycol<6; mycol++){
      tmp_eTowersIDs_subset_FPGA[myrow][mycol] = tmp_eTowersIDs_subset[myrow][mycol];
    }
  }  
  m_eFEXFPGACollection.at(0)->SetTowersAndCells_SG(tmp_eTowersIDs_subset_FPGA);
  //FPGA 0----------------------------------------------------------------------------------------------------------------------------------------------

  //FPGA 1----------------------------------------------------------------------------------------------------------------------------------------------
  memset(tmp_eTowersIDs_subset_FPGA, 0, sizeof tmp_eTowersIDs_subset_FPGA);
  for (int myrow = 0; myrow<10; myrow++){
    for (int mycol = 4; mycol<10; mycol++){
      tmp_eTowersIDs_subset_FPGA[myrow][mycol-4] = tmp_eTowersIDs_subset[myrow][mycol];
    }
  }
  m_eFEXFPGACollection.at(1)->SetTowersAndCells_SG(tmp_eTowersIDs_subset_FPGA);
  //FPGA 1----------------------------------------------------------------------------------------------------------------------------------------------
  

  //FPGA 2----------------------------------------------------------------------------------------------------------------------------------------------
  memset(tmp_eTowersIDs_subset_FPGA, 0, sizeof tmp_eTowersIDs_subset_FPGA);
  for (int myrow = 0; myrow<10; myrow++){
    for (int mycol = 8; mycol<14; mycol++){
      tmp_eTowersIDs_subset_FPGA[myrow][mycol-8] = tmp_eTowersIDs_subset[myrow][mycol];
    }
  }
  m_eFEXFPGACollection.at(2)->SetTowersAndCells_SG(tmp_eTowersIDs_subset_FPGA);
  //FPGA 2----------------------------------------------------------------------------------------------------------------------------------------------
  
  //FPGA 3----------------------------------------------------------------------------------------------------------------------------------------------
  memset(tmp_eTowersIDs_subset_FPGA, 0, sizeof tmp_eTowersIDs_subset_FPGA);
  for (int myrow = 0; myrow<10; myrow++){
    for (int mycol = 12; mycol<18; mycol++){
      tmp_eTowersIDs_subset_FPGA[myrow][mycol-12] = tmp_eTowersIDs_subset[myrow][mycol];
    }
  }
  m_eFEXFPGACollection.at(3)->SetTowersAndCells_SG(tmp_eTowersIDs_subset_FPGA);
  //FPGA 3----------------------------------------------------------------------------------------------------------------------------------------------
  
}

} // end of namespace bracket

