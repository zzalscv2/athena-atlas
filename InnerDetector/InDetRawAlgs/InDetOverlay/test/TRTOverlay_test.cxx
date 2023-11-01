/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @author John Chapman
 * @brief Tests for TRTOverlay.
 */


// Otherwise we get warnings about mutable members in gmock.
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

// using testing::_ gets maybe-uninitialized warnings from gcc12.
#if __GNUC__ >= 12
# pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

// Tested AthAlgorithm
#include "../InDetOverlay/TRTOverlay.h"

#include <vector>

#include "InDetIdentifier/TRT_ID.h"
#include "IdDictParser/IdDictParser.h"
#include "GaudiKernel/MsgStream.h"

// HepMC includes
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"

// CLHEP includes
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include "GeneratorObjects/McEventCollection.h"
#include "InDetSimData/InDetSimData.h"
#include "InDetSimData/InDetSimDataCollection.h"
#include "TRT_ConditionsData/StrawStatus.h"
#include "TRT_ConditionsData/StrawStatusData.h"

// Framework includes
#include "AthenaBaseComps/AthService.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/PhysicalConstants.h"

//undefine NDEBUG after EDM
#undef NDEBUG
// Google Test
#include "gtest/gtest.h"
// Google Mock
#include "gmock/gmock.h"


namespace OverlayTesting {

  // Athena Tool to emulate TRT_LocalOccupancy Tool
  //
  const std::string mockTRT_LocalOccupancyName = "OverlayTesting::MockTRT_LocalOccupancy/MyTestLocalOccupancy";

  class MockTRT_LocalOccupancy : public extends<AthAlgTool, InDet::ITRT_LocalOccupancy> {
  public:
    /// constructor
    MockTRT_LocalOccupancy(const std::string& type, const std::string& name, const IInterface* parent)
      : base_class(type,name,parent)
    { };

    /// destructor
    virtual ~MockTRT_LocalOccupancy() = default;

    /** Return a map of the occupancy in the barrel (-1,+1) and endcaps (-2,+2) */
    MOCK_METHOD((std::map<int, double>), getDetectorOccupancy, (const EventContext&, const TRT_RDO_Container*), (const));

    // Dummy methods to confirm status
    virtual StatusCode initialize() final {
      ATH_MSG_INFO ("initializing MockTRT_LocalOccupancy: " << name());
      return StatusCode::SUCCESS;
    };

    // dummy methods implementing in pure virtual interface methods (to make class non-abstract)
    /** Return the local occupancy for the sectors crossed by a given track */
    using ITRT_LocalOccupancy::LocalOccupancy;
    virtual float LocalOccupancy(const EventContext&, const Trk::Track& ) const { return 1.0; }; // not used - dummy implementation
    virtual float LocalOccupancy(const EventContext&,const double, const double) const
    { return 1.0; }; // not used - dummy implementation

    /** Return the global occupancy of the event*/
    using ITRT_LocalOccupancy::GlobalOccupancy;
    virtual std::vector<float> GlobalOccupancy(const EventContext&) const
    { std::vector<float> dummyVect{}; return dummyVect; }; // not used - dummy implementation
  };

  DECLARE_COMPONENT( MockTRT_LocalOccupancy )


  // Gaudi Test fixture that provides a clean Gaudi environment for
  // each individual test case
  class GaudiFixture {

  protected:
    GaudiFixture() {
      SetUpGaudi();
    }

    ~GaudiFixture() {
      TearDownGaudi();
    }

    void SetUpGaudi() {
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
      ASSERT_TRUE( m_propMgr->setProperty( "JobOptionsPath", "StoreGateTestCommon.txt" ).isSuccess() );

      m_toolSvc = m_svcLoc->service("ToolSvc");
      ASSERT_TRUE( m_toolSvc.isValid() );

      ASSERT_TRUE( m_appMgr->configure().isSuccess() );
      ASSERT_TRUE( m_appMgr->initialize().isSuccess() );
    }

    void TearDownGaudi() {
      ASSERT_TRUE( m_svcMgr->finalize().isSuccess() );
      ASSERT_TRUE( m_appMgr->finalize().isSuccess() );
      ASSERT_TRUE( m_appMgr->terminate().isSuccess() );
      m_svcLoc->release();
      m_svcMgr->release();
      Gaudi::setInstance( static_cast<IAppMgrUI*>(nullptr) );
    }

    // protected member variables for Core Gaudi components
    IAppMgrUI*               m_appMgr = nullptr;
    SmartIF<ISvcLocator>     m_svcLoc;
    SmartIF<ISvcManager>     m_svcMgr;
    SmartIF<IToolSvc>        m_toolSvc;
    SmartIF<IProperty>       m_propMgr;
  };


  class TRTOverlay_test : public ::testing::Test, public GaudiFixture {

  protected:
    virtual void SetUp() override {
      // DetectorStore and ID Helper
      StoreGateSvc *detStore(nullptr);
      ASSERT_TRUE( m_svcLoc->service("DetectorStore", detStore, true).isSuccess() );
      if (detStore) {
        if (not detStore->contains<TRT_ID>("TRT_ID")) {
          auto trt_id = std::make_unique<TRT_ID>();
          IdDictParser parser;
          parser.register_external_entity ("InnerDetector",
                                           "IdDictInnerDetector.xml");
          IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
          trt_id->initialize_from_dictionary (idd);
          ASSERT_TRUE( detStore->record (std::move (trt_id), "TRT_ID").isSuccess() );
        }
      }
      ASSERT_TRUE( m_svcLoc->service("StoreGateSvc", m_sg, true).isSuccess() );
      ASSERT_TRUE( m_svcLoc->service("ConditionStore", m_cond, true).isSuccess() );

      // the tested Algorithm
      m_alg = new TRTOverlay{"TRTOverlay", m_svcLoc};
      m_alg->addRef();
      // ordering B, A, C, D is on purpose to test for unintended alphabetic ordering
      std::string        inputSigPropertyValue = "'StoreGateSvc+TRT_RDOs_SIG'";
      std::string        inputBkgPropertyValue = "'StoreGateSvc+TRT_RDOs_BKG'";
      std::string          outputPropertyValue = "'StoreGateSvc+TRT_RDOs'";
      std::string     inputSigSDOPropertyValue = "'StoreGateSvc+TRT_SDO_Map_SIG'";
      ASSERT_TRUE( m_alg->setProperty( "SignalInputKey",   inputSigPropertyValue).isSuccess() );
      ASSERT_TRUE( m_alg->setProperty( "BkgInputKey",   inputBkgPropertyValue).isSuccess() );
      ASSERT_TRUE( m_alg->setProperty( "OutputKey", outputPropertyValue).isSuccess() );
      ASSERT_TRUE( m_alg->setProperty( "SignalInputSDOKey",   inputSigSDOPropertyValue).isSuccess() );
      ASSERT_TRUE( m_alg->setProperty( "TRT_LocalOccupancyTool", mockTRT_LocalOccupancyName).isSuccess() );
    }

    virtual void TearDown() override {
      // let the Gaudi ServiceManager finalize all services
      ASSERT_TRUE( m_svcMgr->finalize().isSuccess() );
      ASSERT_TRUE( m_alg->finalize().isSuccess() );
      delete m_alg;
    }

    template<typename T>
    T* retrieveService(const std::string& name) {
      T* service = nullptr;
      SmartIF<IService>& serviceSmartPointer = m_svcLoc->service(name);
      service = dynamic_cast<T*>(serviceSmartPointer.get());
      EXPECT_NE(nullptr, service);
      if(!service) {
        return nullptr;
      }
      EXPECT_TRUE( service->configure().isSuccess() );
      EXPECT_TRUE( m_svcMgr->addService(service).isSuccess() );
      // assert that finalize() gets called once per test case
      EXPECT_CALL( *service, finalize() )
        .Times(1)
        .WillOnce(::testing::Return(StatusCode::SUCCESS));

      return service;
    }

    template<typename T>
    T* retrieveTool(const std::string& name) {
      IAlgTool* toolInterface = nullptr;
      EXPECT_TRUE( m_toolSvc->retrieveTool(name, toolInterface).isSuccess() );
      EXPECT_NE(nullptr, toolInterface);

      T* tool = dynamic_cast<T*>(toolInterface);
      EXPECT_NE(nullptr, tool);
      if(!tool) {
        return nullptr;
      }

      EXPECT_TRUE( tool->configure().isSuccess() );

      // assert that finalize() gets called once per test case
      EXPECT_CALL( *tool, finalize() )
        .Times(1)
        .WillOnce(::testing::Return(StatusCode::SUCCESS));

      return tool;
    }

    static unsigned int encodeDigit(const std::vector<unsigned int>& bits)
    {
      unsigned digit(0);
      const unsigned one(1);
      for (unsigned int bit=0; bit < bits.size(); ++bit) {
        if (bits[bit]==1) {
          digit += one << (31-bit);
        }
      }
      return digit;
    }

    static EventContext initEventContext(StoreGateSvc* sg)
    {
      EventIDBase now(0, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM, 0, 1);
      EventContext ctx(0, 0);
      ctx.setEventID( now );
      ctx.setExtension( Atlas::ExtendedEventContext( sg, 0 ) );
      return ctx;
    }

    static bool initTRTStrawStatusHT()
    {
      constexpr unsigned int max{350848};
      SG::WriteCondHandleKey<TRTCond::StrawStatusData> key{"StrawStatusHTData"};
      if (!key.initialize().isSuccess()) {
        return false;
      }

      SG::WriteCondHandle<TRTCond::StrawStatusData> handle{key};
      auto output = std::make_unique<TRTCond::StrawStatusData>(max);
      for (unsigned int i{}; i < max; i++) {
        IdentifierHash hash{i};
        output->setStatus(hash, TRTCond::StrawStatus::Good);
      }

      EventIDRange range = IOVInfiniteRange::infiniteRunLB();
      return handle.record(range, std::move(output)).isSuccess();
    }

    static bool initMcEventCollection(std::vector<HepMC::GenParticlePtr>& genPartList)
    {
      // create dummy input McEventCollection with a name that
      // HepMcParticleLink knows about
      SG::WriteHandle<McEventCollection> inputTestDataHandle{"TruthEvent"};
      inputTestDataHandle = std::make_unique<McEventCollection>();
      // Add a dummy GenEvent
      const int process_id1(20);
      const int event_number1(17);
      inputTestDataHandle->push_back(HepMC::newGenEvent(process_id1, event_number1));
      HepMC::GenEvent& ge1 = *(inputTestDataHandle->at(0));
      populateGenEvent(ge1,-11,11,genPartList);
      populateGenEvent(ge1,-13,13,genPartList);
      populateGenEvent(ge1,-11,11,genPartList);
      populateGenEvent(ge1,-13,13,genPartList);
      populateGenEvent(ge1,-11,11,genPartList);
      populateGenEvent(ge1,22,22,genPartList);
      return true;
    }

    static void populateGenEvent(HepMC::GenEvent & ge, int pdgid1, int pdgid2, std::vector<HepMC::GenParticlePtr>& genPartList)
    {
      HepMC::FourVector myPos( 0.0, 0.0, 0.0, 0.0);
      HepMC::GenVertexPtr myVertex = HepMC::newGenVertexPtr( myPos, -1 );
      HepMC::FourVector fourMomentum1( 0.0, 0.0, 1.0, 1.0*CLHEP::TeV);
      HepMC::GenParticlePtr inParticle1 = HepMC::newGenParticlePtr(fourMomentum1, pdgid1, 2);
      myVertex->add_particle_in(inParticle1);
      HepMC::FourVector fourMomentum2( 0.0, 0.0, -1.0, 1.0*CLHEP::TeV);
      HepMC::GenParticlePtr inParticle2 = HepMC::newGenParticlePtr(fourMomentum2, pdgid2, 2);
      myVertex->add_particle_in(inParticle2);
      HepMC::FourVector fourMomentum3( 0.0, 1.0, 0.0, 1.0*CLHEP::TeV);
      HepMC::GenParticlePtr inParticle3 = HepMC::newGenParticlePtr(fourMomentum3, pdgid1, 1);
      myVertex->add_particle_out(inParticle3);
      genPartList.push_back(inParticle3);
      HepMC::FourVector fourMomentum4( 0.0, -1.0, 0.0, 1.0*CLHEP::TeV);
      HepMC::GenParticlePtr inParticle4 = HepMC::newGenParticlePtr(fourMomentum4, pdgid2, 1);
      myVertex->add_particle_out(inParticle4);
      genPartList.push_back(inParticle4);
      ge.add_vertex( myVertex );
      HepMC::set_signal_process_vertex( &ge, myVertex );
      ge.set_beam_particles(inParticle1,inParticle2);
    }

    void setPrivateToolPointers()
    {
      m_mockTRT_LocalOccupancy = dynamic_cast<OverlayTesting::MockTRT_LocalOccupancy*>(&*(m_alg->m_TRT_LocalOccupancyTool));
    }
    // the tested AthAlgorithm
    TRTOverlay* m_alg{};

    StoreGateSvc* m_sg{};
    StoreGateSvc* m_cond{};
    // mocked Athena components
    OverlayTesting::MockTRT_LocalOccupancy* m_mockTRT_LocalOccupancy = nullptr;
    const std::map<int, double> m_empty_occupancy = {{-2,0.0},{-1,0.0},{1,0.0},{2,0.0}};
    const std::map<int, double> m_full_occupancy = {{-2,1.0},{-1,1.0},{1,1.0},{2,1.0}};
  };   // TRTOverlay_test fixture


  // cppcheck-suppress syntaxError
  TEST_F(TRTOverlay_test, missing_inputs_alg_execute) {
    EventContext ctx = initEventContext(m_sg);
    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    ASSERT_TRUE( m_alg->execute(ctx).isFailure() ); //inputs don't exist
  }

  TEST_F(TRTOverlay_test, empty_containers_alg_execute) {
    EventContext ctx = initEventContext(m_sg);
    SG::WriteHandle<TRT_RDO_Container> inputSigDataHandle{"StoreGateSvc+TRT_RDOs_SIG"};
    const unsigned int containerSize(19008);
    inputSigDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    SG::WriteHandle<TRT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+TRT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    SG::WriteHandle<InDetSimDataCollection> inputSigSDODataHandle{"StoreGateSvc+TRT_SDO_Map_SIG"};
    inputSigSDODataHandle = std::make_unique<InDetSimDataCollection>();
    initTRTStrawStatusHT();

    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    setPrivateToolPointers();
    EXPECT_CALL( *m_mockTRT_LocalOccupancy, getDetectorOccupancy(::testing::_, ::testing::_) )
      .Times(1)
      .WillOnce(::testing::Return(m_full_occupancy));
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<TRT_RDO_Container> outputDataHandle{"StoreGateSvc+TRT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    ASSERT_EQ( outputDataHandle->numberOfCollections(), 0u );
  }

  TEST_F(TRTOverlay_test, containers_with_matching_empty_collections) {
    EventContext ctx = initEventContext(m_sg);
    SG::WriteHandle<TRT_RDO_Container> inputSigDataHandle{"StoreGateSvc+TRT_RDOs_SIG"};
    const unsigned int containerSize(19008);
    IdentifierHash sigElementHash(10026);
    IdentifierHash bkgElementHash(10026);
    inputSigDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> sigCollection = std::make_unique<TRT_RDO_Collection>(sigElementHash);
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<TRT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+TRT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> bkgCollection = std::make_unique<TRT_RDO_Collection>(bkgElementHash);
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),bkgElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle
    SG::WriteHandle<InDetSimDataCollection> inputSigSDODataHandle{"StoreGateSvc+TRT_SDO_Map_SIG"};
    inputSigSDODataHandle = std::make_unique<InDetSimDataCollection>();
    initTRTStrawStatusHT();

    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    setPrivateToolPointers();
    EXPECT_CALL( *m_mockTRT_LocalOccupancy, getDetectorOccupancy(::testing::_, ::testing::_) )
      .Times(1)
      .WillOnce(::testing::Return(m_full_occupancy));
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<TRT_RDO_Container> outputDataHandle{"StoreGateSvc+TRT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    ASSERT_EQ( outputDataHandle->numberOfCollections(),1u );
    const TRT_RDO_Collection *outputCollection = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_NE( outputCollection, nullptr );
    ASSERT_TRUE( outputCollection->empty() );
  }

  TEST_F(TRTOverlay_test, containers_with_different_empty_collections) {
    EventContext ctx = initEventContext(m_sg);
    SG::WriteHandle<TRT_RDO_Container> inputSigDataHandle{"StoreGateSvc+TRT_RDOs_SIG"};
    const unsigned int containerSize(19008);
    IdentifierHash sigElementHash(10026);
    IdentifierHash bkgElementHash(10025);
    inputSigDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> sigCollection = std::make_unique<TRT_RDO_Collection>(sigElementHash);
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<TRT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+TRT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> bkgCollection = std::make_unique<TRT_RDO_Collection>(bkgElementHash);
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),bkgElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle
    SG::WriteHandle<InDetSimDataCollection> inputSigSDODataHandle{"StoreGateSvc+TRT_SDO_Map_SIG"};
    inputSigSDODataHandle = std::make_unique<InDetSimDataCollection>();
    initTRTStrawStatusHT();

    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    setPrivateToolPointers();
    EXPECT_CALL( *m_mockTRT_LocalOccupancy, getDetectorOccupancy(::testing::_, ::testing::_) )
      .Times(1)
      .WillOnce(::testing::Return(m_full_occupancy));
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<TRT_RDO_Container> outputDataHandle{"StoreGateSvc+TRT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    ASSERT_EQ( outputDataHandle->numberOfCollections(), 2u );
    const TRT_RDO_Collection *outputCollection1 = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_NE( outputCollection1, nullptr );
    ASSERT_TRUE( outputCollection1->empty() );
    const TRT_RDO_Collection *outputCollection2 = outputDataHandle->indexFindPtr(bkgElementHash);
    ASSERT_NE( outputCollection2, nullptr );
    ASSERT_TRUE( outputCollection2->empty() );
  }

  TEST_F(TRTOverlay_test, containers_with_matching_collections_one_with_an_RDO) {
    EventContext ctx = initEventContext(m_sg);
    SG::WriteHandle<TRT_RDO_Container> inputSigDataHandle{"StoreGateSvc+TRT_RDOs_SIG"};
    const unsigned int containerSize(19008);
    const IdentifierHash sigElementHash(10026);
    const IdentifierHash bkgElementHash(10026);
    Identifier::value_type value = 0x1612282000000000;
    Identifier sigStrawID = Identifier(value); //Digit ID 2229569 Digit 2147483696
    const std::vector<unsigned int> sigBits = {
      1, // overlay flag
      0,0,0,0, // msb and unused bits - always zero
      0, // leading HT bit - always zero
      0,0,0,0,0,0,0,0, // LT bits for leading BC
      0, // In-time HT bit
      0,0,0,0,0,0,0,0, // LT bits for in-time BC
      0, // trailing HT bit - always zero
      0,0,1,1,0,0,0,0  // LT bits for trailing BC (last 4 LT bits always zero)
    };
    const unsigned int sigWord = encodeDigit(sigBits);
    const unsigned int checkWord(2147483696); //10000000000000000000000000110000
    ASSERT_EQ( sigWord, checkWord ); // Cross-check of encodeDigit function
    inputSigDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> sigCollection = std::make_unique<TRT_RDO_Collection>(sigElementHash);
    //Add a TRT_LoLumRawData object
    std::unique_ptr<TRT_LoLumRawData> sigDigit = std::make_unique<TRT_LoLumRawData>(sigStrawID,sigWord);
    const auto sigHT=sigDigit->highLevel();
    const auto sigTOT=sigDigit->timeOverThreshold();
    const auto sigDriftTimeBin=sigDigit->driftTimeBin();
    sigCollection->push_back(sigDigit.release());
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<TRT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+TRT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> bkgCollection = std::make_unique<TRT_RDO_Collection>(bkgElementHash);
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),bkgElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle
    SG::WriteHandle<InDetSimDataCollection> inputSigSDODataHandle{"StoreGateSvc+TRT_SDO_Map_SIG"};
    inputSigSDODataHandle = std::make_unique<InDetSimDataCollection>();
    initTRTStrawStatusHT();

    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    setPrivateToolPointers();
    EXPECT_CALL( *m_mockTRT_LocalOccupancy, getDetectorOccupancy(::testing::_, ::testing::_) )
      .Times(1)
      .WillOnce(::testing::Return(m_full_occupancy));
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<TRT_RDO_Container> outputDataHandle{"StoreGateSvc+TRT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    ASSERT_EQ( outputDataHandle->numberOfCollections(), 1u );
    const TRT_RDO_Collection *outputCollection1 = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_NE( outputCollection1, nullptr );
    ASSERT_EQ( outputCollection1->size(), 1u );
    const TRT_LoLumRawData* outputDigit1 = dynamic_cast<const TRT_LoLumRawData*>(outputCollection1->at(0));
    ASSERT_NE( outputDigit1, nullptr );
    ASSERT_EQ( outputDigit1->highLevel(), sigHT );
    ASSERT_EQ( outputDigit1->timeOverThreshold(), sigTOT );
    ASSERT_EQ( outputDigit1->driftTimeBin(), sigDriftTimeBin );
  }

  TEST_F(TRTOverlay_test, containers_with_different_collections_one_RDO_each) {
    EventContext ctx = initEventContext(m_sg);
    SG::WriteHandle<TRT_RDO_Container> inputSigDataHandle{"StoreGateSvc+TRT_RDOs_SIG"};
    const unsigned int containerSize(19008);
    const IdentifierHash sigElementHash(10026);
    const Identifier::value_type sigValue = 0x1612282000000000;
    const Identifier sigStrawID = Identifier(sigValue);
    const std::vector<unsigned int> sigBits = {
      1, // overlay flag
      0,0,0,0, // msb and unused bits - always zero
      0, // leading HT bit - always zero
      0,0,0,0,0,0,0,0, // LT bits for leading BC
      0, // In-time HT bit
      0,0,0,0,0,0,0,0, // LT bits for in-time BC
      0, // trailing HT bit - always zero
      0,0,1,1,0,0,0,0  // LT bits for trailing BC (last 4 LT bits always zero)
    };
    const unsigned int sigWord = encodeDigit(sigBits);
    inputSigDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> sigCollection = std::make_unique<TRT_RDO_Collection>(sigElementHash);
    //Add a TRT_LoLumRawData object
    std::unique_ptr<TRT_LoLumRawData> sigDigit = std::make_unique<TRT_LoLumRawData>(sigStrawID,sigWord);
    const auto sigHT=sigDigit->highLevel();
    const auto sigTOT=sigDigit->timeOverThreshold();
    const auto sigDriftTimeBin=sigDigit->driftTimeBin();
    sigCollection->push_back(sigDigit.release());
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<TRT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+TRT_RDOs_BKG"};
    const IdentifierHash bkgElementHash(10027);
    const Identifier::value_type bkgValue = 0x16122ce000000000;
    const Identifier bkgStrawID = Identifier(bkgValue);
    const std::vector<unsigned int> bkgBits = {
      1, // overlay flag
      0,0,0,0, // msb and unused bits - always zero
      0, // leading HT bit - always zero
      0,0,0,0,0,0,0,0, // LT bits for leading BC
      0, // In-time HT bit
      0,0,0,1,1,1,1,1, // LT bits for in-time BC
      0, // trailing HT bit - always zero
      1,1,0,0,0,0,0,0  // LT bits for trailing BC (last 4 LT bits always zero)
    };
    const unsigned int bkgWord = encodeDigit(bkgBits);
    inputBkgDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> bkgCollection = std::make_unique<TRT_RDO_Collection>(bkgElementHash);
    //Add a TRT_LoLumRawData object
    std::unique_ptr<TRT_LoLumRawData> bkgDigit = std::make_unique<TRT_LoLumRawData>(bkgStrawID,bkgWord);
    const auto bkgHT=bkgDigit->highLevel();
    const auto bkgTOT=bkgDigit->timeOverThreshold();
    const auto bkgDriftTimeBin=bkgDigit->driftTimeBin();
    bkgCollection->push_back(bkgDigit.release());
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),bkgElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle
    SG::WriteHandle<InDetSimDataCollection> inputSigSDODataHandle{"StoreGateSvc+TRT_SDO_Map_SIG"};
    inputSigSDODataHandle = std::make_unique<InDetSimDataCollection>();
    initTRTStrawStatusHT();

    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    setPrivateToolPointers();
    EXPECT_CALL( *m_mockTRT_LocalOccupancy, getDetectorOccupancy(::testing::_, ::testing::_) )
      .Times(1)
      .WillOnce(::testing::Return(m_full_occupancy));
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<TRT_RDO_Container> outputDataHandle{"StoreGateSvc+TRT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    const TRT_RDO_Collection *outputCollection1 = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_NE( outputCollection1, nullptr );
    ASSERT_EQ( outputCollection1->size(), 1u );
    const TRT_LoLumRawData* outputDigit1 = dynamic_cast<const TRT_LoLumRawData*>(outputCollection1->at(0));
    ASSERT_NE( outputDigit1, nullptr );
    ASSERT_EQ( outputDigit1->highLevel(), sigHT );
    ASSERT_EQ( outputDigit1->timeOverThreshold(), sigTOT );
    ASSERT_EQ( outputDigit1->driftTimeBin(), sigDriftTimeBin );
    const TRT_RDO_Collection *outputCollection2 = outputDataHandle->indexFindPtr(bkgElementHash);
    ASSERT_NE( outputCollection2, nullptr );
    ASSERT_EQ( outputCollection2->size(), 1u );
    const TRT_LoLumRawData* outputDigit2 = dynamic_cast<const TRT_LoLumRawData*>(outputCollection2->at(0));
    ASSERT_NE( outputDigit2, nullptr );
    ASSERT_EQ( outputDigit2->highLevel(), bkgHT );
    ASSERT_EQ( outputDigit2->timeOverThreshold(), bkgTOT );
    ASSERT_EQ( outputDigit2->driftTimeBin(), bkgDriftTimeBin );
  }

  TEST_F(TRTOverlay_test, containers_with_matching_collections_with_matching_RDOs) {
    EventContext ctx = initEventContext(m_sg);
    SG::WriteHandle<TRT_RDO_Container> inputSigDataHandle{"StoreGateSvc+TRT_RDOs_SIG"};
    const unsigned int containerSize(19008);
    const IdentifierHash sigElementHash(10027);
    const Identifier::value_type sigValue = 0x16122ce000000000;
    const Identifier sigStrawID = Identifier(sigValue);
    const std::vector<unsigned int> sigBits = {
      1, // overlay flag
      0,0,0,0, // msb and unused bits - always zero
      0, // leading HT bit - always zero
      0,0,0,0,0,0,0,0, // LT bits for leading BC
      0, // In-time HT bit
      0,0,0,1,1,1,1,1, // LT bits for in-time BC
      0, // trailing HT bit - always zero
      1,1,0,0,0,0,0,0  // LT bits for trailing BC (last 4 LT bits always zero)
    };
    const unsigned int sigWord = encodeDigit(sigBits);
    inputSigDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> sigCollection = std::make_unique<TRT_RDO_Collection>(sigElementHash);
    //Add a TRT_LoLumRawData object
    std::unique_ptr<TRT_LoLumRawData> sigDigit = std::make_unique<TRT_LoLumRawData>(sigStrawID,sigWord);
    const auto sigHT=sigDigit->highLevel();
    const auto sigTOT=sigDigit->timeOverThreshold();
    const auto sigDriftTimeBin=sigDigit->driftTimeBin();
    sigCollection->push_back(sigDigit.release());
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<TRT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+TRT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> bkgCollection = std::make_unique<TRT_RDO_Collection>(sigElementHash);
    //Add a TRT_LoLumRawData object
    std::unique_ptr<TRT_LoLumRawData> bkgDigit = std::make_unique<TRT_LoLumRawData>(sigStrawID,sigWord);
    bkgCollection->push_back(bkgDigit.release());
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),sigElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle

    std::vector<HepMC::GenParticlePtr> genPartList;
    initMcEventCollection(genPartList);
    SG::WriteHandle<InDetSimDataCollection> inputSigSDODataHandle{"StoreGateSvc+TRT_SDO_Map_SIG"};
    inputSigSDODataHandle = std::make_unique<InDetSimDataCollection>();
    auto pGenParticle = genPartList.at(0);
    HepMcParticleLink trkLink(HepMC::barcode(pGenParticle),pGenParticle->parent_event()->event_number());
    InDetSimData::Deposit deposit( trkLink, 0.0 );
    std::vector<InDetSimData::Deposit> depositVector(1);
    depositVector.push_back(deposit);
    inputSigSDODataHandle->insert(std::make_pair(sigStrawID, InDetSimData(depositVector)));

    initTRTStrawStatusHT();

    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    setPrivateToolPointers();
    EXPECT_CALL( *m_mockTRT_LocalOccupancy, getDetectorOccupancy(::testing::_, ::testing::_) )
      .Times(1)
      .WillOnce(::testing::Return(m_full_occupancy));
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<TRT_RDO_Container> outputDataHandle{"StoreGateSvc+TRT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    const TRT_RDO_Collection *outputCollection1 = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_NE( outputCollection1, nullptr );
    ASSERT_EQ( outputCollection1->size(), 1u );
    const TRT_LoLumRawData* outputDigit1 = dynamic_cast<const TRT_LoLumRawData*>(outputCollection1->at(0));
    ASSERT_NE( outputDigit1, nullptr );
    ASSERT_EQ( outputDigit1->highLevel(), sigHT );
    ASSERT_EQ( outputDigit1->timeOverThreshold(), sigTOT );
    ASSERT_EQ( outputDigit1->driftTimeBin(), sigDriftTimeBin );
  }

  TEST_F(TRTOverlay_test, containers_with_matching_collections_with_differing_RDOs_same_strawID) {
    EventContext ctx = initEventContext(m_sg);
    SG::WriteHandle<TRT_RDO_Container> inputSigDataHandle{"StoreGateSvc+TRT_RDOs_SIG"};
    const unsigned int containerSize(19008);
    const IdentifierHash sigElementHash(10027);
    const Identifier::value_type sigValue = 0x16122ce000000000;
    const Identifier sigStrawID = Identifier(sigValue);
    const std::vector<unsigned int> sigBits = {
      1, // overlay flag
      0,0,0,0, // msb and unused bits - always zero
      0, // leading HT bit - always zero
      0,0,0,0,0,0,1,1, // LT bits for leading BC
      1, // In-time HT bit
      1,1,1,1,1,1,1,1, // LT bits for in-time BC
      0, // trailing HT bit - always zero
      1,1,1,1,0,0,0,0  // LT bits for trailing BC (last 4 LT bits always zero)
    };

    const unsigned int sigWord = encodeDigit(sigBits);
    const std::vector<unsigned int> bkgBits = {
      0, // overlay flag
      0,0,0,0, // msb and unused bits - always zero
      0, // leading HT bit - always zero
      0,0,0,0,0,1,1,1, // LT bits for leading BC
      1, // In-time HT bit
      1,1,1,1,0,0,0,0, // LT bits for in-time BC
      0, // trailing HT bit - always zero
      0,0,0,0,0,0,0,0  // LT bits for trailing BC (last 4 LT bits always zero)
    };
    const unsigned int bkgWord = encodeDigit(bkgBits);
    inputSigDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> sigCollection = std::make_unique<TRT_RDO_Collection>(sigElementHash);
    //Add a TRT_LoLumRawData object
    std::unique_ptr<TRT_LoLumRawData> sigDigit = std::make_unique<TRT_LoLumRawData>(sigStrawID,sigWord);
    const auto sigHT=sigDigit->highLevel();
    sigCollection->push_back(sigDigit.release());
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<TRT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+TRT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> bkgCollection = std::make_unique<TRT_RDO_Collection>(sigElementHash);
    //Add a TRT_LoLumRawData object
    std::unique_ptr<TRT_LoLumRawData> bkgDigit = std::make_unique<TRT_LoLumRawData>(sigStrawID,bkgWord);
    bkgCollection->push_back(bkgDigit.release());
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),sigElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle

    std::vector<unsigned int> outBits(32,0);
    for(unsigned int i=0; i<32; ++i) {
      outBits[i]=std::max(sigBits[i], bkgBits[i]);
    }

    std::vector<HepMC::GenParticlePtr> genPartList;
    initMcEventCollection(genPartList);
    SG::WriteHandle<InDetSimDataCollection> inputSigSDODataHandle{"StoreGateSvc+TRT_SDO_Map_SIG"};
    inputSigSDODataHandle = std::make_unique<InDetSimDataCollection>();
    auto pGenParticle = genPartList.at(0);
    HepMcParticleLink trkLink(HepMC::barcode(pGenParticle),pGenParticle->parent_event()->event_number());
    InDetSimData::Deposit deposit( trkLink, 0.0 );
    std::vector<InDetSimData::Deposit> depositVector(1);
    depositVector.push_back(deposit);
    inputSigSDODataHandle->insert(std::make_pair(sigStrawID, InDetSimData(depositVector)));

    initTRTStrawStatusHT();

    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    setPrivateToolPointers();
    EXPECT_CALL( *m_mockTRT_LocalOccupancy, getDetectorOccupancy(::testing::_, ::testing::_) )
      .Times(1)
      .WillOnce(::testing::Return(m_full_occupancy));
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<TRT_RDO_Container> outputDataHandle{"StoreGateSvc+TRT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    const TRT_RDO_Collection *outputCollection1 = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_NE( outputCollection1, nullptr );
    ASSERT_EQ( outputCollection1->size(), 1u );
    const TRT_LoLumRawData* outputDigit1 = dynamic_cast<const TRT_LoLumRawData*>(outputCollection1->at(0));
    ASSERT_NE( outputDigit1, nullptr );
    ASSERT_EQ( outputDigit1->highLevel(), sigHT );
    ASSERT_EQ( outputDigit1->timeOverThreshold(), 46.875 );
    ASSERT_EQ( outputDigit1->driftTimeBin(), 5 );
  }

  TEST_F(TRTOverlay_test, containers_with_matching_collections_with_differing_LT_RDOs_same_strawID) {
    EventContext ctx = initEventContext(m_sg);
    SG::WriteHandle<TRT_RDO_Container> inputSigDataHandle{"StoreGateSvc+TRT_RDOs_SIG"};
    const unsigned int containerSize(19008);
    const IdentifierHash sigElementHash(10027);
    const Identifier::value_type sigValue = 0x16122ce000000000;
    const Identifier sigStrawID = Identifier(sigValue);
    const std::vector<unsigned int> sigBits = {
      0, // overlay flag
      0,0,0,0, // msb and unused bits - always zero
      0, // leading HT bit - always zero
      0,0,0,0,0,0,1,1, // LT bits for leading BC
      0, // In-time HT bit
      1,1,1,1,1,1,1,1, // LT bits for in-time BC
      0, // trailing HT bit - always zero
      1,1,1,1,0,0,0,0  // LT bits for trailing BC (last 4 LT bits always zero)
    };
    const unsigned int sigWord = encodeDigit(sigBits);
    // const std::vector<unsigned int> bkgBits = {
    //   0,0,0,0,0, // msb and unused bits - always zero
    //   0, // leading HT bit - always zero
    //   1,1,1,1,1,1,1,1, // LT bits for leading BC
    //   1, // In-time HT bit
    //   1,1,1,1,0,0,0,0, // LT bits for in-time BC
    //   0, // trailing HT bit - always zero
    //   0,0,0,0,0,0,0,0  // LT bits for trailing BC (last 4 LT bits always zero)
    // };
    const std::vector<unsigned int> bkgBits = {
      0, // overlay flag
      0,0,0,0, // msb and unused bits - always zero
      0, // leading HT bit - always zero
      0,0,0,0,0,1,1,1, // LT bits for leading BC
      0, // In-time HT bit
      1,1,1,1,0,0,0,0, // LT bits for in-time BC
      0, // trailing HT bit - always zero
      0,0,0,0,0,0,0,0  // LT bits for trailing BC (last 4 LT bits always zero)
    };
    const unsigned int bkgWord = encodeDigit(bkgBits);
    inputSigDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> sigCollection = std::make_unique<TRT_RDO_Collection>(sigElementHash);
    //Add a TRT_LoLumRawData object
    std::unique_ptr<TRT_LoLumRawData> sigDigit = std::make_unique<TRT_LoLumRawData>(sigStrawID,sigWord);
    const auto sigHT=sigDigit->highLevel();
    sigCollection->push_back(sigDigit.release());
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<TRT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+TRT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> bkgCollection = std::make_unique<TRT_RDO_Collection>(sigElementHash);
    //Add a TRT_LoLumRawData object
    std::unique_ptr<TRT_LoLumRawData> bkgDigit = std::make_unique<TRT_LoLumRawData>(sigStrawID,bkgWord);
    bkgCollection->push_back(bkgDigit.release());
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),sigElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle
    initTRTStrawStatusHT();

    std::vector<unsigned int> outBits(32,0);
    for(unsigned int i=0; i<32; ++i) {
      outBits[i]=std::max(sigBits[i], bkgBits[i]);
    }
    std::vector<HepMC::GenParticlePtr> genPartList;
    initMcEventCollection(genPartList);
    SG::WriteHandle<InDetSimDataCollection> inputSigSDODataHandle{"StoreGateSvc+TRT_SDO_Map_SIG"};
    inputSigSDODataHandle = std::make_unique<InDetSimDataCollection>();
    auto pGenParticle = genPartList.at(0);
    HepMcParticleLink trkLink(HepMC::barcode(pGenParticle),pGenParticle->parent_event()->event_number());
    ASSERT_EQ(trkLink.cptr()->pdg_id(), -11); // Sanity check to confirm that we are linking to a positron as expected
    InDetSimData::Deposit deposit( trkLink, 0.0 );
    std::vector<InDetSimData::Deposit> depositVector(1);
    depositVector.push_back(deposit);
    inputSigSDODataHandle->insert(std::make_pair(sigStrawID, InDetSimData(depositVector)));
    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    setPrivateToolPointers();
    EXPECT_CALL( *m_mockTRT_LocalOccupancy, getDetectorOccupancy(::testing::_, ::testing::_) )
      .Times(1)
      .WillOnce(::testing::Return(m_full_occupancy));
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<TRT_RDO_Container> outputDataHandle{"StoreGateSvc+TRT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    const TRT_RDO_Collection *outputCollection1 = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_NE( outputCollection1, nullptr );
    ASSERT_EQ( outputCollection1->size(), 1u );
    const TRT_LoLumRawData* outputDigit1 = dynamic_cast<const TRT_LoLumRawData*>(outputCollection1->at(0));
    ASSERT_NE( outputDigit1, nullptr );
    ASSERT_EQ( outputDigit1->highLevel(), sigHT );
    ASSERT_EQ( outputDigit1->timeOverThreshold(), 46.875 );
    ASSERT_EQ( outputDigit1->driftTimeBin(), 5 );
  }

  TEST_F(TRTOverlay_test, containers_with_matching_collections_with_differing_LT_RDOs_same_strawID_ForceHTbit) {
    EventContext ctx = initEventContext(m_sg);
    SG::WriteHandle<TRT_RDO_Container> inputSigDataHandle{"StoreGateSvc+TRT_RDOs_SIG"};
    const unsigned int containerSize(19008);
    const IdentifierHash sigElementHash(10027);
    const Identifier::value_type sigValue = 0x16122ce000000000;
    const Identifier sigStrawID = Identifier(sigValue);
    const std::vector<unsigned int> sigBits = {
      0, // overlay flag
      0,0,0,0, // msb and unused bits - always zero
      0, // leading HT bit - always zero
      0,0,0,0,0,0,1,1, // LT bits for leading BC
      0, // In-time HT bit
      1,1,1,1,1,1,1,1, // LT bits for in-time BC
      0, // trailing HT bit - always zero
      1,1,1,1,0,0,0,0  // LT bits for trailing BC (last 4 LT bits always zero)
    };
    const unsigned int sigWord = encodeDigit(sigBits);

    const std::vector<unsigned int> bkgBits = {
      0, // overlay flag
      0,0,0,0, // msb and unused bits - always zero
      0, // leading HT bit - always zero
      0,0,0,0,0,1,1,1, // LT bits for leading BC
      0, // In-time HT bit
      1,1,1,1,0,0,0,0, // LT bits for in-time BC
      0, // trailing HT bit - always zero
      0,0,0,0,0,0,0,0  // LT bits for trailing BC (last 4 LT bits always zero)
    };
    const unsigned int bkgWord = encodeDigit(bkgBits);

    std::vector<unsigned int> outBits(32,0);
    for(unsigned int i=0; i<32; ++i) {
      outBits[i]=std::max(sigBits[i], bkgBits[i]);
    }
    outBits[17]=1;    // force in-time HT bit to be true

    inputSigDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> sigCollection = std::make_unique<TRT_RDO_Collection>(sigElementHash);
    //Add a TRT_LoLumRawData object
    std::unique_ptr<TRT_LoLumRawData> sigDigit = std::make_unique<TRT_LoLumRawData>(sigStrawID,sigWord);
    sigCollection->push_back(sigDigit.release());
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<TRT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+TRT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<TRT_RDO_Container>(containerSize);
    std::unique_ptr<TRT_RDO_Collection> bkgCollection = std::make_unique<TRT_RDO_Collection>(sigElementHash);
    //Add a TRT_LoLumRawData object
    std::unique_ptr<TRT_LoLumRawData> bkgDigit = std::make_unique<TRT_LoLumRawData>(sigStrawID,bkgWord);
    bkgCollection->push_back(bkgDigit.release());
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),sigElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle
    initTRTStrawStatusHT();

    std::vector<HepMC::GenParticlePtr> genPartList;
    initMcEventCollection(genPartList);
    SG::WriteHandle<InDetSimDataCollection> inputSigSDODataHandle{"StoreGateSvc+TRT_SDO_Map_SIG"};
    inputSigSDODataHandle = std::make_unique<InDetSimDataCollection>();
    auto pGenParticle = genPartList.at(0);
    HepMcParticleLink trkLink(HepMC::barcode(pGenParticle),pGenParticle->parent_event()->event_number());
    ASSERT_EQ(trkLink.cptr()->pdg_id(), -11); // Sanity check to confirm that we are linking to a positron as expected
    InDetSimData::Deposit deposit( trkLink, 0.0 );
    std::vector<InDetSimData::Deposit> depositVector(1);
    depositVector.push_back(deposit);
    inputSigSDODataHandle->insert(std::make_pair(sigStrawID, InDetSimData(depositVector)));

    // Override Occupancy correction, so that HT bit will always be set
    ASSERT_TRUE( m_alg->setProperty( "TRT_HT_OccupancyCorrectionBarrel", 1.0).isSuccess() );
    ASSERT_TRUE( m_alg->setProperty( "TRT_HT_OccupancyCorrectionEndcap", 1.0).isSuccess() );
    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    setPrivateToolPointers();
    EXPECT_CALL( *m_mockTRT_LocalOccupancy, getDetectorOccupancy(::testing::_, ::testing::_) )
      .Times(1)
      .WillOnce(::testing::Return(m_full_occupancy));
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );

    // check output makes sense
    SG::ReadHandle<TRT_RDO_Container> outputDataHandle{"StoreGateSvc+TRT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    const TRT_RDO_Collection *outputCollection1 = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_NE( outputCollection1, nullptr );
    ASSERT_EQ( outputCollection1->size(), 1u );
    const TRT_LoLumRawData* outputDigit1 = dynamic_cast<const TRT_LoLumRawData*>(outputCollection1->at(0));
    ASSERT_NE( outputDigit1, nullptr );
    ASSERT_EQ( outputDigit1->highLevel(), true );
    ASSERT_EQ( outputDigit1->timeOverThreshold(), 46.875 );
    ASSERT_EQ( outputDigit1->driftTimeBin(), 5 );
  }

} // <-- namespace OverlayTesting


int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest( &argc, argv );

  //return RUN_ALL_TESTS();
  // if the above gets stuck forever while trying to finalize Boost stuff
  // inside SGTools, try to use the following:
  // skips proper finalization:
  std::quick_exit( RUN_ALL_TESTS() );
}
