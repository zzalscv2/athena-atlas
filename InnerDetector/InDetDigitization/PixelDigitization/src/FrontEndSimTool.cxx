/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "FrontEndSimTool.h"


FrontEndSimTool::FrontEndSimTool(const std::string& type, const std::string& name, const IInterface* parent) :
  AthAlgTool(type, name, parent) {
  declareInterface<FrontEndSimTool>(this);
}

StatusCode 
FrontEndSimTool::initialize() {
  ATH_CHECK(m_pixelConditionsTool.retrieve());
  ATH_CHECK(m_pixelReadout.retrieve());
  ATH_CHECK(m_chargeDataKey.initialize());
  if (m_cosmics){
    m_numberOfBcid = 8;
    m_timeOffset = 100.;
    m_timeJitter = 25.0;
  }
  
  return StatusCode::SUCCESS;
}

StatusCode 
FrontEndSimTool::finalize() {return StatusCode::FAILURE;}







