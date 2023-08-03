/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @author Elmar Ritsch <Elmar.Ritsch@cern.ch>
 * @date June, 2016
 * @brief Tests for ISF::InputConverter.
 */

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#undef NDEBUG

// Framework
#include "GaudiKernel/PhysicalConstants.h"
//#include "GaudiKernel/DeclareFactoryEntries.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/SmartIF.h"
#include "AthenaBaseComps/AthAlgTool.h"

// Framework testing
#include "TestTools/initGaudi.h"

// Google Test
#include "gtest/gtest.h"
// Google Mock
#include "gmock/gmock.h"

// ISF
#include "ISF_Event/ISFParticle.h"
#include "ISF_HepMC_Interfaces/IGenParticleFilter.h"

// Amg
#include "GeoPrimitives/GeoPrimitives.h"

// HepMC
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/Operators.h"
#include "GeneratorObjects/HepMcParticleLink.h"
#include "GeneratorObjects/McEventCollection.h"

// STL includes
#include <cstdlib> // quick_exit

// Tested service
#include "../src/InputConverter.h"

namespace ISFTesting {

// Athena Tool to emulate a GenParticleFilter
class MockFilterTool : public AthAlgTool,
                       public ISF::IGenParticleFilter {

 public:
  MockFilterTool(const std::string& type, const std::string& name, const IInterface* parent)
    : AthAlgTool(type,name,parent)
  { declareInterface<ISF::IGenParticleFilter>(this); };

  virtual ~MockFilterTool() {};

  // mock method which will be called by tested code
#ifdef HEPMC3
  MOCK_CONST_METHOD1(pass, bool(const HepMC::ConstGenParticlePtr&));
#else
  MOCK_CONST_METHOD1(pass, bool(const HepMC::GenParticle&));
#endif
};

DECLARE_COMPONENT( MockFilterTool )


class InputConverter_test: public ::testing::Test {

 protected:
  virtual void SetUp() override {
    m_appMgr = Gaudi::createApplicationMgr();
    ASSERT_TRUE( m_appMgr!=nullptr );

    m_svcLoc = m_appMgr;
    ASSERT_TRUE( m_svcLoc.isValid() );

    m_svcMgr = m_appMgr;
    ASSERT_TRUE( m_svcMgr.isValid() );

    m_propMgr = m_appMgr;
    ASSERT_TRUE( m_propMgr.isValid() );
    ASSERT_TRUE( m_propMgr->setProperty( "EvtSel",         "NONE" ).isSuccess() );
    ASSERT_TRUE( m_propMgr->setProperty( "JobOptionsType", "FILE" ).isSuccess() );
    ASSERT_TRUE( m_propMgr->setProperty( "JobOptionsPath", "InputConverter_test.txt" ).isSuccess() );

    m_toolSvc = m_svcLoc->service("ToolSvc");
    ASSERT_TRUE( m_toolSvc.isValid() );

    ASSERT_TRUE( m_appMgr->configure().isSuccess() );
    ASSERT_TRUE( m_appMgr->initialize().isSuccess() );

    // the tested AthenaService
    SmartIF<IService>& serviceSmartPointer = m_svcLoc->service("ISF::InputConverter/InputConverter");
    m_svc = dynamic_cast<ISF::InputConverter*>(serviceSmartPointer.get());
    EXPECT_NE(nullptr, m_svc);
    ASSERT_TRUE( m_svc->configure().isSuccess() );
  }

  virtual void TearDown() override {
    ASSERT_TRUE( m_svcMgr->removeService(m_svc).isSuccess() );
    ASSERT_TRUE( m_svc->finalize().isSuccess() );
    ASSERT_TRUE( m_svc->terminate().isSuccess() );
    delete m_svc;

    ASSERT_TRUE( m_appMgr->finalize().isSuccess() );
    ASSERT_TRUE( m_appMgr->terminate().isSuccess() );
    Gaudi::setInstance( static_cast<IAppMgrUI*>(nullptr)) ;
  }

  //
  // wrappers for private methods
  // NB: This works because InputConverter_test is a friend
  //     of the tested InputConverter service
  //
  template<typename... Args>
  ISF::ISFParticle* convertParticle(Args&&... args) const {
    return m_svc->convertParticle(std::forward<Args>(args)...);
  }

  template<typename... Args>
  bool passesFilters(Args&&... args) const {
    return m_svc->passesFilters(std::forward<Args>(args)...);
  }

  ToolHandleArray<ISF::IGenParticleFilter>& getGenParticleFilters() const {
    return m_svc->m_genParticleFilters;
  }
  //
  // protected member variables
  //

  // Core Gaudi components
  IAppMgrUI*             m_appMgr = nullptr;
  SmartIF<ISvcLocator>   m_svcLoc;
  SmartIF<ISvcManager>   m_svcMgr;
  SmartIF<IToolSvc>      m_toolSvc;
  SmartIF<IProperty>     m_propMgr;

  ISF::InputConverter*   m_svc; // the tested AthenaService

};  // InputConverter_test fixture


// cppcheck-suppress syntaxError
TEST_F(InputConverter_test, initialize_empty) {
  ASSERT_TRUE( m_svc->initialize().isSuccess() );
}


TEST_F(InputConverter_test, convertParticle_nullptr) {
  ISF::ISFParticle* expected = nullptr;
  ASSERT_EQ( expected, convertParticle(nullptr, EBC_MAINEVCOLL) );
}


TEST_F(InputConverter_test, convertParticle_without_production_vertex) {
  HepMC::FourVector mom(12.3, 45.6, 78.9, 0.12);
  HepMC::GenParticlePtr  genPart = HepMC::newGenParticlePtr(mom,
                                                       123, // pdg
                                                       1 // status
                                                      );
  ISF::ISFParticle* expected = nullptr;
  ASSERT_EQ( expected, convertParticle(genPart, EBC_FIRSTPUEVCOLL) );
#ifdef HEPMC3
  //When compiled with HepMC3, genPart is smart pointer
#else
  delete genPart;
#endif
}


TEST_F(InputConverter_test, convertParticle_using_generated_mass) {
  ASSERT_TRUE( m_svc->setProperty("UseGeneratedParticleMass", "True").isSuccess() );
  ASSERT_TRUE( m_svc->initialize().isSuccess() );

  const int particleBarcode(546);
  HepMC::FourVector mom(12.3, 45.6, 78.9, 0.12);
  // dynamic allocation necessary as particle ownership is
  // handed over to a HepMC::GenVertex later
  HepMC::GenParticlePtr  genPart = HepMC::newGenParticlePtr(mom,
                                                       11, // pdg id (e-)
                                                       1 // status
                                                      );
  genPart->set_generated_mass(1234.56);

  HepMC::FourVector pos(9.8, 7.65, 4.3, 0.321); // NB: 4th component is time*c
  int vtx_id = -123;
  HepMC::GenVertexPtr  prodVtx = HepMC::newGenVertexPtr(pos, vtx_id);
  prodVtx->add_particle_out(genPart);

  // create dummy input McEventCollection containing a dummy GenEvent
  SG::WriteHandle<McEventCollection> inputTestDataHandle{"GEN_EVENT_HighPtPU"};
  inputTestDataHandle = std::make_unique<McEventCollection>();
  inputTestDataHandle->push_back(new HepMC::GenEvent());
  HepMC::GenEvent& ge = *(inputTestDataHandle->at(0));
  ge.add_vertex( prodVtx );
  //AV: we set barcode here because only here the particle in HepMC3 enters event and can have a meaningful barcode.
  HepMC::suggest_barcode(genPart,particleBarcode);
  HepMC::fillBarcodesAttribute(&ge);
  HepMcParticleLink* trackLink = new HepMcParticleLink(particleBarcode, 0, EBC_SECONDPUEVCOLL);

  Amg::Vector3D expectedPos(9.8, 7.65, 4.3);
  Amg::Vector3D expectedMom(12.3, 45.6, 78.9);
  ISF::DetRegionSvcIDPair expectedHistory(AtlasDetDescr::fUndefinedAtlasRegion, ISF::fEventGeneratorSimID);
  auto expectedTruthBinding = new ISF::TruthBinding(genPart);
  const int expectedBCID(1); // FIXME for now convertParticle forces
                             // the bcid for pile-up
                             // McEventCollections to be 1.
  ISF::ISFParticle expected(expectedPos,
                            expectedMom,
                            1234.56,
                            -1., // charge
                            11, // pdg id
                            1, ///status                            
                            0.321/Gaudi::Units::c_light, // time
                            expectedHistory,
                            expectedBCID, // bcid
                            particleBarcode, // barcode
                            expectedTruthBinding,
                            trackLink);

  // call the InputConverter's private method
  ISF::ISFParticle* returned = convertParticle(genPart, EBC_SECONDPUEVCOLL);
  ASSERT_TRUE( returned );

  ASSERT_EQ( expected, *returned );
}


TEST_F(InputConverter_test, convertParticle_using_particleDataTable_photon) {
  ASSERT_TRUE( m_svc->setProperty("UseGeneratedParticleMass", "False").isSuccess() );
  ASSERT_TRUE( m_svc->initialize().isSuccess() );

  const int particleBarcode(546);
  HepMC::FourVector mom(12.3, 45.6, 78.9, 0.12);
  // dynamic allocation necessary as particle ownership is
  // handed over to a HepMC::GenVertex later
  HepMC::GenParticlePtr  genPart = HepMC::newGenParticlePtr(mom,
                                                       22, // pdg id (gamma)
                                                       1 // status
                                                      );
  genPart->set_generated_mass(1234.56); // should be ignored later on

  HepMC::FourVector pos(9.8, 7.65, 4.3, 0.321); // NB: 4th component is time*c
  int vtx_id = -123;
  HepMC::GenVertexPtr  prodVtx = HepMC::newGenVertexPtr(pos, vtx_id);
  prodVtx->add_particle_out(genPart);

  // create dummy input McEventCollection containing a dummy GenEvent
  SG::WriteHandle<McEventCollection> inputTestDataHandle{"GEN_EVENT"};
  inputTestDataHandle = std::make_unique<McEventCollection>();
  inputTestDataHandle->push_back(new HepMC::GenEvent());
  HepMC::GenEvent& ge = *(inputTestDataHandle->at(0));
  ge.add_vertex( prodVtx );
  //AV: we set barcode here because only here the particle in HepMC3 enters event and can have a meaningful barcode.
  HepMC::suggest_barcode(genPart,particleBarcode);
  HepMC::fillBarcodesAttribute(&ge);
  HepMcParticleLink* trackLink = new HepMcParticleLink(particleBarcode);

  Amg::Vector3D expectedPos(9.8, 7.65, 4.3);
  Amg::Vector3D expectedMom(12.3, 45.6, 78.9);
  ISF::DetRegionSvcIDPair expectedHistory(AtlasDetDescr::fUndefinedAtlasRegion, ISF::fEventGeneratorSimID);
  auto expectedTruthBinding = new ISF::TruthBinding(genPart);
  const int expectedBCID(0);
  ISF::ISFParticle expected(expectedPos,
                            expectedMom,
                            0., // mass from ParticleDataTable
                            0., // charge
                            22, // pdg id
                            1,  ///status
                            0.321/Gaudi::Units::c_light, // time
                            expectedHistory,
                            expectedBCID, // bcid
                            particleBarcode, // barcode
                            expectedTruthBinding,
                            trackLink
                            );

  // call the InputConverter's private method
  ISF::ISFParticle* returned = convertParticle(genPart, EBC_MAINEVCOLL);
  ASSERT_TRUE( returned );

  ASSERT_EQ( expected, *returned );
}


TEST_F(InputConverter_test, convertParticle_using_particleDataTable_electron) {
  ASSERT_TRUE( m_svc->setProperty("UseGeneratedParticleMass", "False").isSuccess() );
  ASSERT_TRUE( m_svc->initialize().isSuccess() );

  const int particleBarcode(546);
  HepMC::FourVector mom(12.3, 45.6, 78.9, 0.12);
  // dynamic allocation necessary as particle ownership is
  // handed over to a HepMC::GenVertex later
  HepMC::GenParticlePtr  genPart = HepMC::newGenParticlePtr(mom,
                                                       11, // pdg id (e-)
                                                       1 // status
                                                      );
  genPart->set_generated_mass(1234.56); // should be ignored later on

  HepMC::FourVector pos(9.8, 7.65, 4.3, 0.321); // NB: 4th component is time*c
  int vtx_id = -123;
  HepMC::GenVertexPtr  prodVtx = HepMC::newGenVertexPtr(pos, vtx_id);
  prodVtx->add_particle_out(genPart);

  // create dummy input McEventCollection containing a dummy GenEvent
  SG::WriteHandle<McEventCollection> inputTestDataHandle{"GEN_EVENT_PU"};
  inputTestDataHandle = std::make_unique<McEventCollection>();
  inputTestDataHandle->push_back(new HepMC::GenEvent());
  HepMC::GenEvent& ge = *(inputTestDataHandle->at(0));
  ge.add_vertex( prodVtx );
  HepMC::suggest_barcode(genPart,particleBarcode);
  HepMcParticleLink* trackLink = new HepMcParticleLink(particleBarcode, 0, EBC_FIRSTPUEVCOLL);

  Amg::Vector3D expectedPos(9.8, 7.65, 4.3);
  Amg::Vector3D expectedMom(12.3, 45.6, 78.9);
  ISF::DetRegionSvcIDPair expectedHistory(AtlasDetDescr::fUndefinedAtlasRegion, ISF::fEventGeneratorSimID);
  auto expectedTruthBinding = new ISF::TruthBinding(genPart);
  const int expectedBCID(1); // FIXME for now convertParticle forces
                             // the bcid for pile-up
                             // McEventCollections to be 1.
  ISF::ISFParticle expected(expectedPos,
                            expectedMom,
                            0.51099891/Gaudi::Units::MeV, // from particle
                            -1., // charge
                            11, // pdg id
                            1, ///status
                            0.321/Gaudi::Units::c_light, // time
                            expectedHistory,
                            expectedBCID, // bcid
                            particleBarcode, // barcode
                            expectedTruthBinding,
                            trackLink
                            );

  // call the InputConverter's private method
  ISF::ISFParticle* returned = convertParticle(genPart, EBC_FIRSTPUEVCOLL);
  ASSERT_TRUE( returned );

  ASSERT_EQ( expected, *returned );
}


TEST_F(InputConverter_test, passesFilters_empty_filters_defaultconstructed_genpart) {
  ASSERT_TRUE( m_svc->initialize().isSuccess() );

#ifdef HEPMC3
  auto genPart=HepMC::newGenParticlePtr();
#else
  const HepMC::GenParticle genPart{};
#endif
  ASSERT_TRUE( passesFilters(genPart) );
}


TEST_F(InputConverter_test, passesFilters_empty_filters) {
  ASSERT_TRUE( m_svc->initialize().isSuccess() );

  HepMC::FourVector mom(12.3, 45.6, 78.9, 0.12);
#ifdef HEPMC3
  //It seems this test makes no sense for HepMC3
  HepMC::GenParticlePtr genPart=HepMC::newGenParticlePtr(mom,
                              11, // pdg id (e-)
                              1 // status
                             );
  ASSERT_TRUE( true );
#else
  const int particleBarcode(546);
  HepMC::GenParticle genPart(mom,
                              11, // pdg id (e-)
                              1 // status
                             );
  genPart.set_generated_mass(1234.56);
  HepMC::suggest_barcode( genPart,particleBarcode);
  const HepMC::GenParticle constGenPart(std::move(genPart));

  ASSERT_TRUE( passesFilters(constGenPart) );
#endif
}


TEST_F(InputConverter_test, passesFilters_one_pass_filter) {
  // retrieve mockable GenParticleFilter tool and point InputConverter to the same instance
  ASSERT_TRUE( m_svc->setProperty("GenParticleFilters", "['ISFTesting::MockFilterTool/DummyFilter']").isSuccess() );
  ASSERT_TRUE( m_svc->initialize().isSuccess() );
  ToolHandleArray<ISF::IGenParticleFilter>& genParticleFilters = getGenParticleFilters();
  const unsigned int expectedSize(1);
  ASSERT_EQ (genParticleFilters.size(), expectedSize);
  MockFilterTool* filterTool = dynamic_cast<MockFilterTool*>(&*(genParticleFilters[0]));
  ASSERT_TRUE( filterTool );
#ifdef  HEPMC3
  HepMC::ConstGenParticlePtr genPart{};
#else
  const HepMC::GenParticle genPart{};
#endif
  HepMC::FourVector mom(12.3, 45.6, 78.9, 0.12);
  HepMC::GenParticle genPart2(mom,
                              11, // pdg id (e-)
                              1 // status
                              );

  EXPECT_CALL(*filterTool, pass(genPart))
             .Times(1)
             .WillOnce(::testing::Return(true));

  ASSERT_TRUE( passesFilters(genPart) );
}


TEST_F(InputConverter_test, passesFilters_one_nonpass_filter) {
  // retrieve mockable GenParticleFilter tool and point InputConverter to the same instance
  ASSERT_TRUE( m_svc->setProperty("GenParticleFilters", "['ISFTesting::MockFilterTool/DummyFilter']").isSuccess() );
  ASSERT_TRUE( m_svc->initialize().isSuccess() );
  ToolHandleArray<ISF::IGenParticleFilter>& genParticleFilters = getGenParticleFilters();
  ASSERT_EQ (genParticleFilters.size(), 1U);
  MockFilterTool* filterTool = dynamic_cast<MockFilterTool*>(&*(genParticleFilters[0]));
  ASSERT_TRUE( filterTool );

#ifdef  HEPMC3
  HepMC::ConstGenParticlePtr genPart{};
#else
  const HepMC::GenParticle genPart{};
#endif
  HepMC::FourVector mom(12.3, 45.6, 78.9, 0.12);
  HepMC::GenParticle genPart2(mom,
                              11, // pdg id (e-)
                              1 // status
                              );

  EXPECT_CALL(*filterTool, pass(genPart))
      .Times(1)
      .WillOnce(::testing::Return(false));

  ASSERT_FALSE( passesFilters(genPart) );
}


TEST_F(InputConverter_test, passesFilters_two_filters) {
  // retrieve mockable GenParticleFilter tool and point InputConverter to the same instance
  ASSERT_TRUE( m_svc->setProperty("GenParticleFilters", "['ISFTesting::MockFilterTool/DummyFilterZ', 'ISFTesting::MockFilterTool/DummyFilterY']").isSuccess() );
  ASSERT_TRUE( m_svc->initialize().isSuccess() );
  ToolHandleArray<ISF::IGenParticleFilter>& genParticleFilters = getGenParticleFilters();
  ASSERT_EQ (genParticleFilters.size(), 2U);
  MockFilterTool* filterTool1 = dynamic_cast<MockFilterTool*>(&*(genParticleFilters[0]));
  ASSERT_TRUE( filterTool1 );
  MockFilterTool* filterTool2 = dynamic_cast<MockFilterTool*>(&*(genParticleFilters[1]));
  ASSERT_TRUE( filterTool2 );

#ifdef  HEPMC3
  HepMC::ConstGenParticlePtr genPart{};
#else
  const HepMC::GenParticle genPart{};
#endif
  HepMC::FourVector mom(12.3, 45.6, 78.9, 0.12);
  HepMC::GenParticle genPart2(mom,
                              11, // pdg id (e-)
                              1 // status
                              );

  EXPECT_CALL(*filterTool1, pass(genPart))
      .Times(1)
      .WillOnce(::testing::Return(true));

  EXPECT_CALL(*filterTool2, pass(genPart))
      .Times(1)
      .WillOnce(::testing::Return(true));

  ASSERT_TRUE( passesFilters(genPart) );

}


} // <-- namespace ISFTesting

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest( &argc, argv );

  // gets stuck forever while trying to finalize boost inside SGTools:
  //return RUN_ALL_TESTS();
  // skips proper finalization:
  std::quick_exit( RUN_ALL_TESTS() );
}
