/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <fstream>
#include <stdexcept>
#include <bitset>
#include "TrigT1ZDC.h"


using json = nlohmann::json;

namespace LVL1 {

 //--------------------------------
 // Constructors and destructors
 //--------------------------------

 TrigT1ZDC::TrigT1ZDC (const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm( name, pSvcLocator ) {}

 //---------------------------------
 // initialise()
 //---------------------------------

 StatusCode TrigT1ZDC::initialize()
 {
   ATH_CHECK(m_zdcCTPLocation.initialize());
   ATH_CHECK(m_zdcModuleDataLocation.initialize());
   // Find the full path to filename:
   std::string file = PathResolverFindCalibFile(m_lutFile);
   ATH_MSG_INFO("Reading file " << file);
   std::ifstream fin(file.c_str());
   if(!fin){
      ATH_MSG_ERROR("Can not read file: " << file);
      return StatusCode::FAILURE;
   }
   json data = json::parse(fin);

   // Conversion factor from Run2 Energies (GeV) to dynamic range of LUT (4096 entries)
   std::vector<float> convfact;
   convfact.push_back(m_energyToADCScaleFactor);

   // Will eventually obtain LUTs from COOL, for now obtain them from calibration area
   // A data member to hold the side A LUT values
   std::array<unsigned int, 4096> sideALUT = data["LucrodLowGain"]["LUTs"]["sideA"];
   // A data member to hold the side C LUT values
   std::array<unsigned int, 4096> sideCLUT = data["LucrodLowGain"]["LUTs"]["sideC"];
   // A data member to hold the Combined LUT values
   std::array<unsigned int, 256> combLUT = data["LucrodLowGain"]["LUTs"]["comb"];

   // Construct Simulation Objects
   m_modInputs_p = std::make_shared<ZDCTriggerSim::ModuleAmplInputsFloat>(ZDCTriggerSim::ModuleAmplInputsFloat(convfact));
   m_simTrig = std::make_shared<ZDCTriggerSimModuleAmpls>(ZDCTriggerSimModuleAmpls(sideALUT, sideCLUT, combLUT));

   ATH_MSG_DEBUG("TrigT1ZDC initilized");
   return StatusCode::SUCCESS;
 }

 //----------------------------------------------
 // execute() method called once per event
 //----------------------------------------------

 StatusCode TrigT1ZDC::execute(const EventContext &ctx) const
 {
   // access ZDC modules
   SG::ReadHandle<xAOD::ZdcModuleContainer> zdcModHandle = SG::makeHandle(m_zdcModuleDataLocation, ctx);
   std::vector<float> moduleEnergy = {0., 0., 0., 0., 0., 0., 0., 0.};

   // Read Single Modules
   if (zdcModHandle.isValid())
   {
     const xAOD::ZdcModuleContainer *zdcSingleModules = zdcModHandle.cptr();
     for (const auto zdcSM : *zdcSingleModules)
     {
       if (zdcSM->zdcType() == 0)
       { // type = 0 are big modules, type = 1 the pixels
       ATH_MSG_DEBUG("ZDC Side " << zdcSM->zdcSide() << ", Module: " << zdcSM->zdcModule() << " and Energy: " << zdcSM->auxdataConst<float>("CalibEnergy"));
       // Side A
       if (zdcSM->zdcSide() > 0)
        {
         moduleEnergy.at(zdcSM->zdcModule()) = zdcSM->auxdataConst<float>("CalibEnergy");
        }

       // Side C
       if (zdcSM->zdcSide() < 0)
        {
         moduleEnergy.at(zdcSM->zdcModule() + 4) = zdcSM->auxdataConst<float>("CalibEnergy");
        }
       }
     }
   }

   // Get Output as an integer (0-7)
   m_modInputs_p->setData(moduleEnergy);

   // call ZDCTriggerSim to actually get ZDC Bits
   unsigned int wordOut = m_simTrig->simLevel1Trig(ZDCTriggerSim::SimDataCPtr(m_modInputs_p));

   // convert int to bitset
   std::bitset<3> bin(wordOut);

   // load output into trigger word on correct bits
   unsigned int word0 = 0;
   word0 += (bin[0] << 25);
   word0 += (bin[1] << 26);
   word0 += (bin[2] << 27);

   // form CTP obejct
   SG::WriteHandle<ZdcCTP> zdcCTP = SG::makeHandle(m_zdcCTPLocation, ctx);

   //record CTP object
   ATH_CHECK(zdcCTP.record(std::make_unique<ZdcCTP>(word0)));
   ATH_MSG_DEBUG("Stored ZDC CTP object with words " << std::hex << (zdcCTP->cableWord0()) << " from LUTOutput " << std::dec << wordOut);

   return StatusCode::SUCCESS;
  }
 }
