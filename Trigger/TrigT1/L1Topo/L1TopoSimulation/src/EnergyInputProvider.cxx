/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include <math.h> /* atan2 */

#include "EnergyInputProvider.h"
#include "TrigT1CaloEvent/EnergyRoI_ClassDEF.h"
#include "TrigT1Interfaces/TrigT1CaloDefs.h"

#include "L1TopoEvent/ClusterTOB.h"
#include "L1TopoEvent/TopoInputEvent.h"


using namespace std;
using namespace LVL1;

EnergyInputProvider::EnergyInputProvider(const std::string& type, const std::string& name, 
                                         const IInterface* parent) :
   base_class(type, name, parent),
   m_energyLocation(TrigT1CaloDefs::EnergyTopoDataLocation)
{
   declareInterface<LVL1::IInputTOBConverter>( this );
   declareProperty( "EnergyROILocation", m_energyLocation, "Storegate key for reading the Topo Energy ROI" );

}

EnergyInputProvider::~EnergyInputProvider()
{}

StatusCode
EnergyInputProvider::initialize() {

   CHECK(m_energyLocation.initialize());  

   if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());

   return StatusCode::SUCCESS;
}

StatusCode
EnergyInputProvider::fillTopoInputEvent(TCS::TopoInputEvent& inputEvent) const {

   /** https://indico.cern.ch/conferenceDisplay.py?confId=284687
       Energy ROI:
       | 0 1 0 0 | T | 0  0 0 | 8b Missing ET Sig hits (or 8*0) | XO | 15b Signed Ex       |
       | 0 1 0 1 | T | 0  0 0 | 8b Missing ET hits              | YO | 15b Signed Ey       |
       | 0 1 1 0 | T | 0  0 0 | 8b Sum ET hits                  | TO | 15b Unsigned Sum ET |
       T = sum type: 0-normal, 1-masked
       O = overflow Ex, Ey, Et
    */

//    EnergyRoI * energyROI = 0;
//    if( ! evtStore()->retrieve(energyROI, m_energyLocation).isSuccess()) {
//       ATH_MSG_ERROR("No EnergyROI with key '" << m_energyLocation << "' found in the event. Configuration issue.");
//       return StatusCode::FAILURE;
//    }

//    ATH_MSG_DEBUG("Not filling the input event from EnergyROI. There is not enough information.");
//    ATH_MSG_DEBUG("EnergyROI: word0 = " << hex << energyROI->roiWord0()
//                  << ", word1 = " << hex << energyROI->roiWord1()
//                  << ", word2 = " << hex << energyROI->roiWord2());


   SG::ReadHandle< EnergyTopoData > topoData (m_energyLocation);

   if( !topoData.isValid()){
      ATH_MSG_WARNING("No EnergyTopoData with SG key '" << m_energyLocation.key() << "' found in the event. No MET input for the L1Topo simulation.");
      return StatusCode::RECOVERABLE;
   }

   ATH_MSG_DEBUG( "EnergyTopoData" << dec
                  << ": Ex = " << topoData->Ex()
                  << ", Ey = " << topoData->Ey()
                  << ", Et = " << topoData->Et()
                  << ", ExTC = " << topoData->ExTC()
                  << ", EyTC = " << topoData->EyTC()
                  );

   
   TCS::MetTOB met( -(topoData->Ex()), -(topoData->Ey()), topoData->Et() );
   inputEvent.setMET( met );
   const bool has_overflow = (topoData->ExOverflow() or
                              topoData->EyOverflow() or
                              topoData->EtOverflow());
   if(has_overflow) {
          inputEvent.setOverflowFromEnergyInput(true);
          ATH_MSG_DEBUG("setOverflowFromEnergyInput : true");
   }
   auto mon_hPt = Monitored::Scalar("MET", met.Et());
   auto mon_hPhi = Monitored::Scalar("METPhi", atan2(met.Ey(),met.Ex()));
   Monitored::Group(m_monTool, mon_hPt, mon_hPhi);

   return StatusCode::SUCCESS;
}
