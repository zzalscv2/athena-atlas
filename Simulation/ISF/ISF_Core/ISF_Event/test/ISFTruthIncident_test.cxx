/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG

#include "ISF_Event/ISFTruthIncident.h"

// inputs to truthincident
#include "ISF_Event/ISFParticle.h"

// children container typedefs
#include "ISF_Event/ISFParticleContainer.h"

#include "GeoPrimitives/GeoPrimitives.h"

#include "AtlasDetDescr/AtlasRegion.h"
#include "BarcodeEvent/Barcode.h"

#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenEvent.h"

// std::abs
#include <cmath> 

namespace test {

  // global mock data used by several TruthIncident tests 
  // generate ISFParticle parent and children 
  Amg::Vector3D pos(1., 0., 2.);
  Amg::Vector3D mom(5., 4., 3.);
  double mass    = 0.;
  double charge  = 987.;
  int    pdgCode = 675;
  double time    = 923.;
  int    bcid    = 123;
  const ISF::DetRegionSvcIDPair origin( AtlasDetDescr::fAtlasCalo, 2 );
  Barcode::ParticleBarcode partBC = 1;
  ISF::TruthBinding *truth = 0;
  ISF::ISFParticle isp1( pos,
                         mom,
                         mass,
                         charge,
                         pdgCode,
                         time,
                         origin,
                         bcid,
                         partBC,
                         truth );

  Barcode::ParticleBarcode part2BC = 2;
  ISF::ISFParticle isp2( pos,
                         mom,
                         mass,
                         charge,
                         pdgCode,
                         time,
                         isp1, // parent
                         part2BC,
                         truth );
  
  ISF::ISFParticleVector pvec_children {&test::isp2};

  // set this to conversion 
  Barcode::PhysicsProcessCode procBC = 14;

  // generate truth incident
  ISF::ISFTruthIncident truthIncident(test::isp1,
                                      test::pvec_children,
                                      test::procBC,
                                      test::origin.first);
  // rounding 
  double eps = pow(10,-8);

} // end of test namespace



void testConstructors() {
  
  {

    ISF::ISFTruthIncident truthIncident(test::isp1,
					test::pvec_children,
					test::procBC,
					test::origin.first);
    
    assert(test::procBC == truthIncident.physicsProcessCode());
    assert(test::origin.first == truthIncident.geoID());

    assert(test::isp1.momentum().mag2() == truthIncident.parentP2());
    assert(test::isp1.pdgCode() == truthIncident.parentPdgCode());
    assert(test::isp1.barcode() == truthIncident.parentBarcode());

    unsigned int nChildren = truthIncident.numberOfChildren();
    assert(1 == nChildren);
    assert(test::isp2.momentum().mag2() == truthIncident.childP2(0));
    assert(test::isp2.pdgCode() == truthIncident.childPdgCode(0));

  }

}


void testChildP2() {
  assert(test::truthIncident.childP2(0)==test::isp2.momentum().mag2());
}


void testChildPt2() {
  assert(test::truthIncident.childPt2(0) == test::isp2.momentum().perp2());
}


void testChildEkin() {
  assert(test::truthIncident.childEkin(0) == test::isp2.ekin());
}


void testChildPdgCode() {
  assert(test::truthIncident.childPdgCode(0) == test::isp2.pdgCode());
}


void testChildBarcode() {
  assert(test::truthIncident.childBarcode(0) == test::isp2.barcode());
  Barcode::ParticleBarcode undefBC = Barcode::fUndefinedBarcode;
  unsigned int childIndexOutOfRange = 1;
  assert(undefBC == test::truthIncident.childBarcode(childIndexOutOfRange));
}


void testChildParticle() {
 
  // ChildParticle(index, bc):
  // - returns HepMC::GenParticle gP with bc for child with index = index,
  // - assigns the truthincident child barcode bc 

  //--------------------------------------------------------------------
  // prepare test info:
  // used to test gP child: index and bc 
  unsigned int childIndex = 0;
  Barcode::ParticleBarcode childBarcode = 3;

  // used to test TruthIncident: no change to properties, apart from BC
  // snapshot of original child properties
  double originalChildPt2 = test::truthIncident.childPt2(0);
  int originalChildPdgCode = test::truthIncident.childPdgCode(0);

  //--------------------------------------------------------------------
  // get gP:
  std::cout << "LM test::truthIncident.childBarcode(0) pre childParticle  " << test::truthIncident.childBarcode(0) << std::endl;
  std::unique_ptr<HepMC::GenEvent> event(HepMC::newGenEvent(1,1));
  HepMC::GenVertexPtr vertex = HepMC::newGenVertexPtr();
  event->add_vertex(vertex);
  HepMC::GenParticlePtr gPP = test::truthIncident.childParticle(childIndex,childBarcode);
   vertex->add_particle_out(gPP);
#ifdef HEPMC3
  HepMC::suggest_barcode( gPP, childBarcode);
#endif
 std::cout << "LM test::truthIncident.childBarcode(0) post childParticle" << test::truthIncident.childBarcode(0) << std::endl;
  //--------------------------------------------------------------------
  // run tests:
  // do gP properties match original child, apart from barcode?
  assert(test::eps >= std::fabs(gPP->momentum().perp2() - originalChildPt2));  
  assert(gPP->pdg_id() == originalChildPdgCode);
  assert(HepMC::barcode(gPP) == childBarcode);

  // truthIncident: no change to properties, apart from BC?
  assert(test::truthIncident.childPt2(0) == originalChildPt2);
  assert(test::truthIncident.childPdgCode(0) == originalChildPdgCode);
  assert(test::truthIncident.childBarcode(0) == childBarcode);

}


int main() {

  testConstructors();

  // const getters 
  testChildP2();
  testChildPt2();
  testChildEkin();
  testChildPdgCode();
  testChildBarcode();

  // not const getter; returns genParticle, changes TruthIncident child barcode 
  testChildParticle();

  return 0;
  
}

