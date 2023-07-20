/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/errorcheck.h"
#include "AthLinks/ElementLink.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/PhysicalConstants.h"

#include "GeneratorObjects/McEventCollection.h"
#include "HepMCTruthReader.h"

using std::cout;
using std::endl;

HepMCTruthReader::HepMCTruthReader(const std::string& name, ISvcLocator* svcLoc)
  : AthAlgorithm(name, svcLoc)
{
  /// @todo Provide these names centrally in a Python module and remove these hard-coded versions?
  declareProperty( "HepMCContainerName", m_hepMCContainerName="GEN_EVENT" );
}


StatusCode HepMCTruthReader::initialize() {
  ATH_MSG_INFO("HepMC container name = " << m_hepMCContainerName );
  return StatusCode::SUCCESS;
}


StatusCode HepMCTruthReader::execute() {

  // Retrieve the HepMC truth:
  const McEventCollection* mcColl = nullptr;
  CHECK( evtStore()->retrieve( mcColl, m_hepMCContainerName ) );
  ATH_MSG_INFO("Number of pile-up events in this Athena event: " << mcColl->size()-1);

  // Loop over events
  for (unsigned int cntr = 0; cntr < mcColl->size(); ++cntr) {
    const HepMC::GenEvent* genEvt = (*mcColl)[cntr]; 

    // Print the event particle/vtx contents
    if (cntr==0) ATH_MSG_INFO("Printing signal event...");
    if (cntr>0) ATH_MSG_INFO("Printing pileup events...");  

    if (cntr==0) {
      auto signalProcessVtx = HepMC::signal_process_vertex(genEvt);
      ATH_MSG_INFO("Signal process vertex position: (" << (signalProcessVtx?signalProcessVtx->position().x():0)
		   << ", " << (signalProcessVtx?signalProcessVtx->position().y():0)
           << ", " << (signalProcessVtx?signalProcessVtx->position().z():0)
           << "). Pointer: " << signalProcessVtx);
    }

    printEvent(genEvt);

  }

  return StatusCode::SUCCESS;
}


// Print method for event - mimics the HepMC dump.
// Vertex print method called within here
void HepMCTruthReader::printEvent(const HepMC::GenEvent* event) {
  cout << "--------------------------------------------------------------------------------\n";
  cout << "GenEvent: #" << "NNN" << "\n";
  cout << " Entries this event: " << event->vertices_size() << " vertices, " << event->particles_size() << " particles.\n";
  cout << "                                    GenParticle Legend\n";
  cout << "        Barcode   PDG ID      ( Px,       Py,       Pz,     E ) Stat  DecayVtx\n";
  cout << "--------------------------------------------------------------------------------\n";
#ifdef HEPMC3
  for (const auto& iv: event->vertices()) {  printVertex(iv);  } 
#else
  for (HepMC::GenEvent::vertex_const_iterator iv = event->vertices_begin(); iv != event->vertices_end(); ++iv) {
    printVertex(*iv);
  }
#endif
  cout << "--------------------------------------------------------------------------------\n";
}

// Print method for vertex - mimics the HepMC dump.
// Particle print method called within here
void HepMCTruthReader::printVertex(const HepMC::ConstGenVertexPtr& vertex) {
  std::ios::fmtflags f( cout.flags() ); 
  cout << "GenVertex (" << vertex << "):";
  if (HepMC::barcode(vertex) != 0) {
    if (vertex->position().x() != 0.0 && vertex->position().y() != 0.0 && vertex->position().z() != 0.0) {
      cout.width(9);
      cout <<HepMC::barcode(vertex);
      cout << " ID:";
      cout.width(5);
      cout << vertex->id();
      cout << " (X,cT)=";
      cout.width(9);
      cout.precision(2);
      cout.setf(std::ios::scientific, std::ios::floatfield);
      cout.setf(std::ios_base::showpos);
      cout << vertex->position().x() << ",";
      cout.width(9);
      cout.precision(2);
      cout << vertex->position().y() << ",";
      cout.width(9);
      cout.precision(2);
      cout << vertex->position().z() << ",";
      cout.width(9);
      cout.precision(2);
      cout << vertex->position().t();
      cout.setf(std::ios::fmtflags(0), std::ios::floatfield);
      cout.unsetf(std::ios_base::showpos);
      cout << endl;
    } else {
      cout.width(9);
      cout << HepMC::barcode(vertex);
      cout << " ID:";
      cout.width(5);
      cout << vertex->id();
      cout << " (X,cT): 0";
      cout << endl;
    }
  } else {
    // If the vertex doesn't have a unique barcode assigned, then
    //  we print its memory address instead... so that the
    //  print out gives us a unique tag for the particle.
    if (vertex->position().x() != 0.0 && vertex->position().y() != 0.0 && vertex->position().z() != 0.0) {
      cout.width(9);
      cout << HepMC::raw_pointer(vertex);
      cout << " ID:";
      cout.width(5);
      cout << vertex->id();
      cout << " (X,cT)=";
      cout.width(9);
      cout.precision(2);
      cout.setf(std::ios::scientific, std::ios::floatfield);
      cout.setf(std::ios_base::showpos);
      cout << vertex->position().x();
      cout.width(9);
      cout.precision(2);
      cout << vertex->position().y();
      cout.width(9);
      cout.precision(2);
      cout << vertex->position().z();
      cout.width(9);
      cout.precision(2);
      cout << vertex->position().t();
      cout.setf(std::ios::fmtflags(0), std::ios::floatfield);
      cout.unsetf(std::ios_base::showpos);
      cout << endl;
    } else {
      cout.width(9);
      cout << HepMC::raw_pointer(vertex);
      cout << " ID:";
      cout.width(5);
      cout << vertex->id();
      cout << " (X,cT):0";
      cout << endl;
    }
  }

  // Print out all the incoming, then outgoing particles
#ifdef HEPMC3
  for (const auto&  iPIn: vertex->particles_in()) {       
    if ( iPIn == vertex->particles_in().front() ) {
      cout << " I: ";
      cout.width(2);
      cout << vertex->particles_in().size();
    } else cout << "      ";
    printParticle(iPIn);
  }
  for (const auto& iPOut: vertex->particles_out()) {
    if ( iPOut == vertex->particles_out().front()) {
      cout << " O: ";
      cout.width(2);
      cout << vertex->particles_out().size();
    } else cout << "      ";
    printParticle(iPOut);
  }  
#else  
  for (HepMC::GenVertex::particles_in_const_iterator iPIn = vertex->particles_in_const_begin();
       iPIn != vertex->particles_in_const_end(); ++iPIn) {       
    if ( iPIn == vertex->particles_in_const_begin() ) {
      cout << " I: ";
      cout.width(2);
      cout << vertex->particles_in_size();
    } else cout << "      ";
    printParticle(*iPIn);
  }
  for (HepMC::GenVertex::particles_out_const_iterator iPOut = vertex->particles_out_const_begin();
       iPOut != vertex->particles_out_const_end(); ++iPOut) {
    if ( iPOut == vertex->particles_out_const_begin() ) {
      cout << " O: ";
      cout.width(2);
      cout << vertex->particles_out_size();
    } else cout << "      ";
    printParticle(*iPOut);
  }

#endif
  cout.flags(f); 
}


// Print method for particle - mimics the HepMC dump.
void HepMCTruthReader::printParticle(const HepMC::ConstGenParticlePtr& particle) {
  std::ios::fmtflags f( cout.flags() ); 
  cout << " ";
  cout.width(9);
  cout << HepMC::barcode(particle);
  cout.width(9);
  cout << particle->pdg_id() << " ";
  cout.width(9);
  cout.precision(2);
  cout.setf(std::ios::scientific, std::ios::floatfield);
  cout.setf(std::ios_base::showpos);
  cout << particle->momentum().px() << ",";
  cout.width(9);
  cout.precision(2);
  cout << particle->momentum().py() << ",";
  cout.width(9);
  cout.precision(2);
  cout << particle->momentum().pz() << ",";
  cout.width(9);
  cout.precision(2);
  cout << particle->momentum().e() << " ";
  cout.setf(std::ios::fmtflags(0), std::ios::floatfield);
  cout.unsetf(std::ios_base::showpos);
  if ( particle->status()==2 ) {
    if ( HepMC::barcode(particle->end_vertex())!=0 ) {
      cout.width(3);
      cout << particle->status() << " ";
      cout.width(9);
      cout << HepMC::barcode(particle->end_vertex());
    }
  } else {
    cout.width(3);
    cout << particle->status();
  }
  cout << endl;
  cout.flags(f); 
}


