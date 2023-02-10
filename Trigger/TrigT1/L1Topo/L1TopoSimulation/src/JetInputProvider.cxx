/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include <math.h>

#include "JetInputProvider.h"
#include "TrigT1CaloEvent/JetROI_ClassDEF.h"
#include "L1TopoEvent/ClusterTOB.h"
#include "L1TopoEvent/TopoInputEvent.h"

using namespace std;
using namespace LVL1;

JetInputProvider::JetInputProvider(const std::string& type, const std::string& name, 
                                   const IInterface* parent) :
   base_class(type, name, parent),
   m_jetLocation(TrigT1CaloDefs::JetTopoTobLocation)
{
   declareInterface<LVL1::IInputTOBConverter>( this );
   declareProperty( "JetROILocation", m_jetLocation, "Storegate key for the Jet ROIs" );
}

JetInputProvider::~JetInputProvider()
{}

StatusCode
JetInputProvider::initialize() {

   CHECK(m_jetLocation.initialize()); 

   if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());

   return StatusCode::SUCCESS;
}

StatusCode
JetInputProvider::fillTopoInputEvent(TCS::TopoInputEvent& inputEvent) const {
   // https://indico.cern.ch/conferenceDisplay.py?confId=284687
   


   SG::ReadHandle< DataVector<JetCMXTopoData> > jettobdata (m_jetLocation);

   if(!jettobdata.isValid()){
      ATH_MSG_WARNING("No DataVector<JetCMXTopoData> with SG key '" << m_jetLocation.key() << "' found in the event. No JET input for the L1Topo simulation.");
      return StatusCode::RECOVERABLE;
   }


   ATH_MSG_DEBUG("Filling the input event. Number of jet topo data objects: " << jettobdata->size());
   for(const JetCMXTopoData * topoData : * jettobdata) {

      // fill the vector of TOBs
      std::vector< JetTopoTOB > tobs;
      topoData->tobs(tobs);

      ATH_MSG_DEBUG("Jet topo data object has # TOBs: " << tobs.size());
      for(const JetTopoTOB & tob: tobs) {

         ATH_MSG_DEBUG( "JET TOB with : et large = " << setw(4) << tob.etLarge() << ", et small " << tob.etSmall()
                        << ", eta = " << setw(2) << tob.eta() << ", phi = " << tob.phi()
                        << ", ieta = " << setw(2) << tob.ieta() << ", iphi = " << tob.iphi()
                        << ", word = " << hex << tob.roiWord() << dec
                        );

         TCS::JetTOB jet( tob.etLarge(), tob.etSmall(), tob.ieta(), tob.iphi(), tob.roiWord() );
         jet.setEtaDouble( tob.eta() );
         jet.setPhiDouble( tob.phi() );
         inputEvent.addJet( jet );
         auto mon_hPt1 = Monitored::Scalar("TOBPt1", jet.Et1());
         auto mon_hPt2 = Monitored::Scalar("TOBPt2", jet.Et2());
         auto mon_hPhi = Monitored::Scalar("TOBPhi", jet.phi());
         auto mon_hEta = Monitored::Scalar("TOBEta", jet.eta());
         Monitored::Group(m_monTool, mon_hPt1, mon_hPt2, mon_hPhi, mon_hEta);
      }
      if(topoData->overflow()){
          inputEvent.setOverflowFromJetInput(true);
          ATH_MSG_DEBUG("setOverflowFromJetInput : true");
      }
   }
   return StatusCode::SUCCESS;
}
