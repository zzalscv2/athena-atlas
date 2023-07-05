/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ALFA_BeamTransport.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "ALFA_BeamTransport.h"
#include "StoreGate/StoreGateSvc.h"



#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"

// FrameWork includes
#include "GaudiKernel/ITHistSvc.h"
#include "Gaudi/Property.h"


#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/SimpleVector.h"
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/HepMCHelpers.h"

//ROOT headers
#include "TFile.h"
#include "TH1.h"
#include "TString.h"


#include <iostream>
#include <vector>

//================ Constructor =================================================

ALFA_BeamTransport::ALFA_BeamTransport(const std::string& name, ISvcLocator* pSvcLocator)
  :
  AthAlgorithm(name,pSvcLocator)
{
  declareProperty("ConfDir", m_FPConfig.ConfDir="./config");
  declareProperty("UseALFA", m_FPConfig.UseALFA=true);
  declareProperty("Debug", m_WriteDebugOutput=false);
  declareProperty("IP",m_FPConfig.IP = 1);
  declareProperty("UseAper",m_FPConfig.useaper = true);
  declareProperty("apermb",m_FPConfig.apermb = 0.);
  declareProperty("xcol1",m_FPConfig.xcol1 = 999.);//999e-4 but we need open collimators
  declareProperty("xcol2",m_FPConfig.xcol2 = 999.);//999e-4
  declareProperty("BeamEnergy",m_FPConfig.pbeam0 = 7000.);//beam energy
  declareProperty("RPDistance",m_FPConfig.RPDistance = 237.398);
  declareProperty("absZMagMax",m_FPConfig.absZMagMax = 500.);//dont read magnets after this value
  
  //Cuts for production
  declareProperty("EtaCut",m_EtaCut = 8.);
  declareProperty("XiCut",m_XiCut = 0.2); 
  
  declareProperty("FPOutputBeam1", m_FPOutputBeam1="Beam1");
  declareProperty("FPOutputBeam2", m_FPOutputBeam2="Beam2");

}

//================ Destructor =================================================

ALFA_BeamTransport::~ALFA_BeamTransport()
{}


//================ Initialisation =================================================

StatusCode ALFA_BeamTransport::initialize()
{
  ATH_MSG_INFO("Initializing " << name() << "...");
  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_MCKey.initialize());
  // open files for beamtracking output
  // string in char
  char* cBeam1Name = new char[m_FPOutputBeam1.length()];
  strcpy(cBeam1Name, m_FPOutputBeam1.c_str());
  char* cBeam2Name = new char[m_FPOutputBeam2.length()];
  strcpy(cBeam2Name, m_FPOutputBeam2.c_str());

  if (m_WriteDebugOutput) {
    m_FileBeam1.open(cBeam1Name);
    m_FileBeam2.open(cBeam2Name);
  }

  //delete char arrays
  delete[] cBeam1Name;
  delete[] cBeam2Name;  
  
  //Initialisation of FPTracker
  m_BeamTracker.ALFA_BeamTrack::initialize(m_FPConfig);

  //-----------------------------------------------------------------------------------------
  ATH_MSG_INFO( name() << " initialize()"  );
  ATH_MSG_INFO( "initialize() successful in " << name()  );
  return StatusCode::SUCCESS;
}

//================ Finalisation =================================================

StatusCode ALFA_BeamTransport::finalize()
{
  // close outputfiles for FPTracker
  if (m_WriteDebugOutput) {
    m_FileBeam1.close();
    m_FileBeam2.close();
  }
  // Code entered here will be executed once at the end of the program run.
  return StatusCode::SUCCESS;
}

//================ Execution ====================================================

StatusCode ALFA_BeamTransport::execute()
{
       // Code entered here will be executed once per event
     ATH_MSG_DEBUG ("Executing " << name() << "...");
       //-----------------------------------------------------------------------------------------
     int run_number;
     uint64_t evt_number;

     //Set particles counter to zero!
     //counter for particles marked as outgoing in event record
     m_pcount=0;
     //counter for particles marked as incomming in event record
     m_pint=0;

     //Load event info
     SG::ReadHandle<xAOD::EventInfo> eventInfo (m_eventInfoKey,getContext());
     if(!eventInfo.isValid()) {
       ATH_MSG_ERROR("Could not retrieve EventInfo");
       return StatusCode::FAILURE;
     }
     else {
       run_number=eventInfo->runNumber();
       evt_number=eventInfo->eventNumber();
 
       ATH_MSG_DEBUG("run: " << run_number << " event: " << evt_number);
     }

     SG::ReadHandle<McEventCollection> mcColl(m_MCKey, getContext());
     if (!mcColl.isValid()) {
       ATH_MSG_WARNING("Could not retrieve McEventCollection: " << m_MCKey);
       return StatusCode::SUCCESS;
     }
     // Loop over all events in McEventCollectio
     ATH_MSG_INFO("successful load of HepMC info");
     for (const HepMC::GenEvent* itr  : *mcColl){
       HepMC::GenEvent evt = (*itr);
       // convert unit MeV to GeV for energy and momenta
       MeVToGeV(evt);

       HepMC::Print::line(std::cout, evt);

       // Select final state particle from event generator
       // set event status !=1 (final state)
       // fill member variables with particle data
       //	    ALFA_BeamTransport::SelectParticles(evt);

       // There should be a check if the first particle is really the
       // particle for beam 1!!!!

       // run Funktion which does the beamtracking
       //	    ALFA_BeamTransport::DoBeamTracking(evt_number);


       TransportSelectedParticle(evt, evt_number);

       // Print new data collection on screen
       HepMC::Print::line(std::cout, evt);
     }

     ATH_MSG_INFO("after running the process function");

     //-----------------------------------------------------------------------------------------

     return StatusCode::SUCCESS;
}


//convert unit MeV to GeV for energy and momenta
///////////////
void ALFA_BeamTransport::MeVToGeV (HepMC::GenEvent& evt)
{
  for (const HepMC::GenParticlePtr& p:  evt) {
    const HepMC::FourVector fv(p->momentum().px() / 1000.,
                               p->momentum().py() / 1000.,
                               p->momentum().pz() / 1000.,
                               p->momentum().e() / 1000.);

    p->set_momentum(fv);
  }
}

//convert unit MeV to GeV for energy and momenta
///////////////
void ALFA_BeamTransport::GeVToMeV (HepMC::GenEvent& evt)
{
  for (const HepMC::GenParticlePtr& p : evt) {
    const HepMC::FourVector fv(p->momentum().px() * 1000.,
                               p->momentum().py() * 1000.,
                               p->momentum().pz() * 1000.,
                               p->momentum().e() * 1000.);

    p->set_momentum(fv);
  }
}



int ALFA_BeamTransport::DoBeamTracking(int evt_number)
{
     //do particle tracking for beam 1 
     
     //tracking funktion
     m_BeamTracker.ALFA_BeamTrack::CalculatePosRP(m_Particle1);//calculates position and momentum at RP1
     //Position at RP
     m_PosRP1=m_BeamTracker.ALFA_BeamTrack::PosRP();
     //Momentum at RP
     m_MomRP1=m_BeamTracker.ALFA_BeamTrack::MomRP();
     
     //do particle tracking for beam 2 

     //tracking funktion
     m_BeamTracker.ALFA_BeamTrack::CalculatePosRP(m_Particle2);//gives position and momentum at RP3
     //Position at RP
     m_PosRP3=m_BeamTracker.ALFA_BeamTrack::PosRP();
     //Momentum at RP
     m_MomRP3=m_BeamTracker.ALFA_BeamTrack::MomRP();

     //Write Output
     if(m_WriteDebugOutput)
     {
       m_FileBeam1 << evt_number << "\t" << std::setprecision(17)
                   << m_PosRP1.x() << std::setprecision(17) << "\t"
                   << m_PosRP1.y() << "\t" << std::setprecision(17)
                   << m_PosRP1.z() << "\t" << std::setprecision(17)
                   << m_MomRP1.x() << std::setprecision(17) << "\t"
                   << m_MomRP1.y() << "\t" << std::setprecision(17)
                   << m_MomRP1.z() << "\t" << std::setprecision(17)
                   << m_EnergyRP1 << std::endl;
       m_FileBeam2 << evt_number << "\t" << std::setprecision(17)
                   << m_PosRP3.x() << std::setprecision(17) << "\t"
                   << m_PosRP3.y() << "\t" << std::setprecision(17)
                   << m_PosRP3.z() << "\t" << std::setprecision(17)
                   << m_MomRP3.x() << std::setprecision(17) << "\t"
                   << m_MomRP3.y() << "\t" << std::setprecision(17)
                   << m_MomRP3.z() << "\t" << std::setprecision(17)
                   << m_EnergyRP3 << std::endl;
     }

     return true;
}

int ALFA_BeamTransport::TransportSelectedParticle(HepMC::GenEvent& evt, int evt_number){
     HepMC::GenParticlePtr p1{nullptr};
     HepMC::GenParticlePtr p2{nullptr};
     
	
     std::vector<FPTracker::Point> PosAtRP1;
     std::vector<FPTracker::Point> PosAtRP3;
     std::vector<FPTracker::Point> MomAtPR1;
     std::vector<FPTracker::Point> MomAtPR3;
     std::vector<double> EnergyRP1;
     std::vector<double> EnergyRP3;
     
     
     double mom=0.;
     double eta=0.;
     double theta=0.;

     // First we have to select the final state particles from the MC
     for (const HepMC::GenParticlePtr& p : evt) {

       // Simple Eta Pt cut to remove particles from BeamTransportation which
       // have no chance to reach RP plane
       mom = std::sqrt(std::pow(p->momentum().px(), 2) +
                       std::pow(p->momentum().py(), 2) +
                       std::pow(p->momentum().pz(), 2));
       theta = std::acos(std::abs(p->momentum().pz()) / mom);
       eta = -std::log(std::tan(theta / 2));

       if (MC::isStable(p) &&
           (!p->end_vertex())) { // TODO What is end_vertex()???
         // Change the status code from Pythia (1) to 201 //added 120124
         p->set_status(HepMC::PYTHIA8NOENDVERTEXSTATUS);

         int pid = p->pdg_id();
         if (eta > m_EtaCut &&
             1 - std::abs(mom / m_FPConfig.pbeam0) < m_XiCut) {

           // save a copy of the particles which passed the cut
           HepMC::FourVector Position = p->production_vertex()->position();

           HepMC::FourVector Momentum = p->momentum();

           HepMC::GenVertexPtr Vertex =
             HepMC::newGenVertexPtr(Position); // copy of the vertex
           HepMC::GenParticlePtr Particle =
             HepMC::newGenParticlePtr(Momentum, pid, 202);

           Vertex->add_particle_out(Particle);
           evt.add_vertex(Vertex);

           // select direction of particle
           if (p->momentum().pz() > 0. &&
               pid == 2212) { // Beam1 TODO Tracking only works for protons!!!!
             p1 = p;
             // now we want to track the final particle if it's a protons
             // Positions are given in mm FPTracker needs them in meter
             m_Particle1 = FPTracker::Particle(
               p1->production_vertex()->position().x() / 1000.,
               p1->production_vertex()->position().y() / 1000.,
               p1->production_vertex()->position().z() / 1000.,
               p1->momentum().px(),
               p1->momentum().py(),
               p1->momentum().pz());

             // do particle tracking for beam 1

             // tracking funktion
             m_BeamTracker.ALFA_BeamTrack::CalculatePosRP(
               m_Particle1); // calculates position and momentum at RP1
             // Position at RP
             m_PosRP1 = m_BeamTracker.ALFA_BeamTrack::PosRP();
             PosAtRP1.push_back(m_PosRP1);
             // Momentum at RP
             m_MomRP1 = m_BeamTracker.ALFA_BeamTrack::MomRP();
             MomAtPR1.push_back(m_MomRP1);
             // no Energy change between IP and RP
             m_EnergyRP1 = p1->momentum().e();
             EnergyRP1.push_back(m_EnergyRP1);
             if (m_WriteDebugOutput) {
               m_FileBeam1 << evt_number << "\t" << std::setprecision(17)
                           << m_PosRP1.x() << std::setprecision(17) << "\t"
                           << m_PosRP1.y() << "\t" << std::setprecision(17)
                           << m_PosRP1.z() << "\t" << std::setprecision(17)
                           << m_MomRP1.x() << std::setprecision(17) << "\t"
                           << m_MomRP1.y() << "\t" << std::setprecision(17)
                           << m_MomRP1.z() << "\t" << std::setprecision(17)
                           << m_EnergyRP1 << std::endl;
             }

           } else if (p->momentum().pz() < 0. && pid == 2212) { // beam 2
             p2 = p;

             m_Particle2 = FPTracker::Particle(
               p2->production_vertex()->position().x() / 1000.,
               p2->production_vertex()->position().y() / 1000.,
               p2->production_vertex()->position().z() / 1000.,
               p2->momentum().px(),
               p2->momentum().py(),
               p2->momentum().pz());
             // tracking funktion
             m_BeamTracker.ALFA_BeamTrack::CalculatePosRP(
               m_Particle2); // gives position and momentum at RP3
             // Position at RP
             m_PosRP3 = m_BeamTracker.ALFA_BeamTrack::PosRP();
             PosAtRP3.push_back(m_PosRP3);
             // Momentum at RP
             m_MomRP3 = m_BeamTracker.ALFA_BeamTrack::MomRP();
             MomAtPR3.push_back(m_MomRP3);

             m_EnergyRP3 = p2->momentum().e();
             EnergyRP3.push_back(m_EnergyRP3);
             // Write Output
             if (m_WriteDebugOutput) {
               m_FileBeam2 << evt_number << "\t" << std::setprecision(17)
                           << m_PosRP3.x() << std::setprecision(17) << "\t"
                           << m_PosRP3.y() << "\t" << std::setprecision(17)
                           << m_PosRP3.z() << "\t" << std::setprecision(17)
                           << m_MomRP3.x() << std::setprecision(17) << "\t"
                           << m_MomRP3.y() << "\t" << std::setprecision(17)
                           << m_MomRP3.z() << "\t" << std::setprecision(17)
                           << m_EnergyRP3 << std::endl;
             }

           } else {
             ATH_MSG_ERROR("Strange: Particle rests at IP");
           }

           if (pid == 2212) { // Find the protons
             // change status code
             //	(*p)->set_status(201); //Todo  here we have to find a convetion
             m_pcount++;
             if (m_pcount > 2) {
               ATH_MSG_ERROR("Strange: More than two protons in this event!");
             }
           }
         }
       }
     }

     //from here the transported particles are added to HepMC
     
     ATH_MSG_INFO("Add transproted particle into HepMC event record Beam 1");
     //Add Data for HepMC Collection
     
     for(int i=0;i<(int)PosAtRP1.size();i++){//Beam1
	  
     
     	//The factor 1000 comes from the fact that HepMC saves length in mm
     
     HepMC::FourVector PositionVectorRP1 = HepMC::FourVector(PosAtRP1.at(i).x()*1000.,PosAtRP1.at(i).y()*1000.,PosAtRP1.at(i).z()*1000.,0.*1000.);
     
     HepMC::FourVector MomentumVectorRP1 = HepMC::FourVector(MomAtPR1.at(i).x(),MomAtPR1.at(i).y(),MomAtPR1.at(i).z(),EnergyRP1.at(i));
     
     	HepMC::GenVertexPtr VertexRP1 = HepMC::newGenVertexPtr(PositionVectorRP1);
     	HepMC::GenParticlePtr ParticleRP1 = HepMC::newGenParticlePtr(MomentumVectorRP1,2212,1); //save the transported particle with status code 1 (added 120124) preview was 201
     	//Add particle to vertex
     	VertexRP1->add_particle_out(ParticleRP1);
     	//add new vertex to HepMC event record
     	if(m_PosRP1.x()!=-99){ // add vertex to event record if the particle in the first beam was not lost
		  evt.add_vertex(VertexRP1);
     	}
     }
     ATH_MSG_INFO ("Add transproted particle into HepMC event record Beam 2" );
     for(int i=0;i<(int)PosAtRP3.size();i++){
	  
	  //Add Data for HepMC Collection			
	  
	  //RP3		
	  HepMC::FourVector PositionVectorRP3 = HepMC::FourVector(PosAtRP3.at(i).x()*1000.,PosAtRP3.at(i).y()*1000.,PosAtRP3.at(i).z()*1000.,0.*1000.);
	  
	  HepMC::FourVector MomentumVectorRP3 = HepMC::FourVector(MomAtPR3.at(i).x(),MomAtPR3.at(i).y(),MomAtPR3.at(i).z(),EnergyRP3.at(i));
	  
	  HepMC::GenVertexPtr VertexRP3 = HepMC::newGenVertexPtr(PositionVectorRP3);
	  HepMC::GenParticlePtr ParticleRP3 = HepMC::newGenParticlePtr(MomentumVectorRP3,2212,1);//save the transported particle with status code 1 (added 120124) preview was 201 
	  
	  VertexRP3->add_particle_out(ParticleRP3);

          if (m_PosRP3.x() != -99) { // add vertex to event record if the
                                     // particle in the second beam was not lost
            evt.add_vertex(VertexRP3);
          }
	  
     }
    
    //convert HepMC data back to HepMC standart ( momentum and energy in MeV)
    ALFA_BeamTransport::GeVToMeV(evt);

    return true;
}
