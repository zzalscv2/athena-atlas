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

}



void testConstructors() {
  
  {
    ISF::ISFTruthIncident truthIncident = test::truthIncident;

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


void testChildParticle() {
 
  unsigned int childIndex = 0;
  Barcode::ParticleBarcode childBarcode = 3;
  Barcode::ParticleBarcode originalChildBarcode = test::truthIncident.childBarcode(0);
  HepMC::GenParticlePtr gPP = test::truthIncident.childParticle(childIndex,childBarcode);
  // truthIncident should remain unmodified, GenParticle gets the assigned barcode
  assert(test::truthIncident.childBarcode(0)==originalChildBarcode);
  assert(gPP->barcode()==childBarcode);
  // genparticle pdgid
  assert(gPP->pdg_id()==test::truthIncident.childPdgCode(0));
  
  return;
}



int main() {

  testConstructors();
  testChildParticle();

  return 0;
  
}

