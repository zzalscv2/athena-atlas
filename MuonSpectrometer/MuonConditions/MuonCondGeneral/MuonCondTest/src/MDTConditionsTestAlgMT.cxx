/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MDTConditionsTestAlgMT.h"

// Constructor
MDTConditionsTestAlgMT::MDTConditionsTestAlgMT(const std::string& name, ISvcLocator* pSvcLocator) : AthAlgorithm(name, pSvcLocator) {}

// Destructor
MDTConditionsTestAlgMT::~MDTConditionsTestAlgMT() = default;

// Initialize
StatusCode MDTConditionsTestAlgMT::initialize() {
    ATH_MSG_INFO("Calling initialize");
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}

// Execute
StatusCode MDTConditionsTestAlgMT::execute() {   

    ATH_MSG_INFO("Calling execute");
   
    SG::ReadCondHandle<MdtCondDbData> readHandle{m_readKey};
    const MdtCondDbData* readCdo{*readHandle};
    if (!readCdo) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    } else {
       ATH_MSG_INFO("Loaded successfully the dead channel data");
    }

    {
        std::stringstream sstr{};
        for (const Identifier& dead_tube : readCdo->getDeadTubesId()) {
            sstr<<"   **** "<<m_idHelperSvc->toString(dead_tube)<<std::endl;
        }
        ATH_MSG_INFO("Found "<<readCdo->getDeadTubes().size()<<" dead tubes"<<std::endl<<sstr.str());
    }
    
    {
        std::stringstream sstr{};
        for (const Identifier& dead_lay : readCdo->getDeadLayersId()) {
            sstr<<"   **** "<<m_idHelperSvc->toString(dead_lay)<<std::endl;
        }
        ATH_MSG_INFO("Found "<<readCdo->getDeadLayersId().size()<<" dead layers"<<std::endl<<sstr.str());
    }
    {
        std::stringstream sstr{};
        for (const Identifier& dead_ml : readCdo->getDeadMultilayersId()) {
            sstr<<"   **** "<<m_idHelperSvc->toString(dead_ml)<<std::endl;
        }
        ATH_MSG_INFO("Found "<<readCdo->getDeadMultilayersId().size()<<" dead multi layers"<<std::endl<<sstr.str());
    }

    {
        std::stringstream sstr{};
        for (const Identifier& dead_cham : readCdo->getDeadStationsId()) {
            sstr<<"   **** "<<m_idHelperSvc->toString(dead_cham)<<std::endl;
        }
        ATH_MSG_INFO("Found "<<readCdo->getDeadStationsId().size()<<" dead stations"<<std::endl<<sstr.str());
    }
    {
        std::stringstream sstr{};
        for (const Identifier& dead_cham : readCdo->getDeadChambersId()) {
            sstr<<"   **** "<<m_idHelperSvc->toString(dead_cham)<<std::endl;
        }
        ATH_MSG_INFO("Found "<<readCdo->getDeadChambersId().size()<<" dead stations"<<std::endl<<sstr.str());

    }

    ATH_MSG_INFO("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$");

    ATH_MSG_INFO("Now going to probe some Ids");
    ATH_MSG_INFO("ID=1699348480; isGood? " << readCdo->isGood(Identifier(1699348480)));

    ATH_MSG_INFO("MADE IT TO THE END!!");
    return StatusCode::SUCCESS;
}
