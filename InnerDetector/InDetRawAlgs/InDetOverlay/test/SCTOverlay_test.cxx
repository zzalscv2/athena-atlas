/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @author John Chapman
 * @brief Tests for SCTOverlay.
 */

// Need to include this before the #undef NDEBUG to avoid odr violations.
#include "TrkEventPrimitives/TrkObjectCounter.h"
#undef NDEBUG

// Framework
#include "TestTools/initGaudi.h"

// Google Test
#include "gtest/gtest.h"

#include <vector>

// Tested AthAlgorithm
#include "../InDetOverlay/SCTOverlay.h"
#include "InDetIdentifier/SCT_ID.h"
#include "IdDictParser/IdDictParser.h"
#include "GaudiKernel/MsgStream.h"

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY; // Use of global g_svcLoc is not thread safe.

namespace OverlayTesting {

  // needed every time an AthAlgorithm, AthAlgTool or AthService is instantiated
  ISvcLocator* g_svcLoc = nullptr;

  // global test environment takes care of setting up Gaudi
  class GaudiEnvironment : public ::testing::Environment {
  protected:
    virtual void SetUp() override {
      Athena_test::initGaudi("StoreGateTestCommon.txt", OverlayTesting::g_svcLoc);
    }
  };

  class SCTOverlay_test : public ::testing::Test {

  protected:
    virtual void SetUp() override {
      m_alg = new SCTOverlay{"SCTOverlay", g_svcLoc};
      ASSERT_TRUE( g_svcLoc->service("StoreGateSvc", m_sg) );

      StoreGateSvc *detStore(nullptr);
      ASSERT_TRUE( OverlayTesting::g_svcLoc->service("DetectorStore", detStore, true).isSuccess() );
      if (detStore) {
        if (not detStore->contains<SCT_ID>("SCT_ID")) {
          auto sct_id = std::make_unique<SCT_ID>();
          IdDictParser parser;
          parser.register_external_entity ("InnerDetector",
                                           "IdDictInnerDetector.xml");
          IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
          sct_id->initialize_from_dictionary (idd);
          ASSERT_TRUE( detStore->record (std::move (sct_id), "SCT_ID").isSuccess() );
        }
      }
    }

    virtual void TearDown() override {
      ASSERT_TRUE( m_sg->clearStore(true).isSuccess() );
      ASSERT_TRUE( m_alg->finalize().isSuccess() );
      delete m_alg;
    }

    void configureSCTOverlay() {
      // ordering C, A, B is on purpose to test for unintended alphabetic ordering
      std::string  inputSigPropertyValue = "'StoreGateSvc+SCT_RDOs_SIG'";
      std::string  inputBkgPropertyValue = "'StoreGateSvc+SCT_RDOs_BKG'";
      std::string    outputPropertyValue = "'StoreGateSvc+SCT_RDOs'";
      ASSERT_TRUE( m_alg->setProperty( "SignalInputKey",   inputSigPropertyValue).isSuccess() );
      ASSERT_TRUE( m_alg->setProperty( "BkgInputKey",   inputBkgPropertyValue).isSuccess() );
      ASSERT_TRUE( m_alg->setProperty( "OutputKey", outputPropertyValue).isSuccess() );
    }
    SCTOverlay* m_alg{};
    StoreGateSvc* m_sg{};
  };   // SCTOverlay_test fixture


  // cppcheck-suppress syntaxError
  TEST_F(SCTOverlay_test, set_properties) {
    EventContext ctx(0,0);
    ctx.setExtension( Atlas::ExtendedEventContext( m_sg, 0 ) );
    configureSCTOverlay();
    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    ASSERT_TRUE( m_alg->execute(ctx).isFailure() ); //inputs don't exist
  }

  TEST_F(SCTOverlay_test, empty_containers_alg_execute) {
    EventContext ctx(0,0);
    ctx.setExtension( Atlas::ExtendedEventContext( m_sg, 0 ) );
    SG::WriteHandle<SCT_RDO_Container> inputSigDataHandle{"StoreGateSvc+SCT_RDOs_SIG"};
    const unsigned int containerSize(1188);
    inputSigDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    SG::WriteHandle<SCT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+SCT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);

    configureSCTOverlay();
    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
  }

  TEST_F(SCTOverlay_test, containers_with_matching_empty_collections) {
    EventContext ctx(0,0);
    ctx.setExtension( Atlas::ExtendedEventContext( m_sg, 0 ) );
    SG::WriteHandle<SCT_RDO_Container> inputSigDataHandle{"StoreGateSvc+SCT_RDOs_SIG"};
    const unsigned int containerSize(1188);
    IdentifierHash sigElementHash(1);
    IdentifierHash bkgElementHash(1);
    inputSigDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> sigCollection = std::make_unique<SCT_RDO_Collection>(sigElementHash);
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<SCT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+SCT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> bkgCollection = std::make_unique<SCT_RDO_Collection>(bkgElementHash);
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),bkgElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle

    configureSCTOverlay();
    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<SCT_RDO_Container> outputDataHandle{"StoreGateSvc+SCT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    const SCT_RDO_Collection *outputCollection = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_TRUE( outputCollection!=nullptr );
    ASSERT_TRUE( outputCollection->empty() );
  }

  TEST_F(SCTOverlay_test, containers_with_different_empty_collections) {
    EventContext ctx(0,0);
    ctx.setExtension( Atlas::ExtendedEventContext( m_sg, 0 ) );
    SG::WriteHandle<SCT_RDO_Container> inputSigDataHandle{"StoreGateSvc+SCT_RDOs_SIG"};
    const unsigned int containerSize(1188);
    IdentifierHash sigElementHash(1);
    IdentifierHash bkgElementHash(2);
    inputSigDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> sigCollection = std::make_unique<SCT_RDO_Collection>(sigElementHash);
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<SCT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+SCT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> bkgCollection = std::make_unique<SCT_RDO_Collection>(bkgElementHash);
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),bkgElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle

    configureSCTOverlay();
    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<SCT_RDO_Container> outputDataHandle{"StoreGateSvc+SCT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    const SCT_RDO_Collection *outputCollection1 = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_TRUE( outputCollection1!=nullptr );
    ASSERT_TRUE( outputCollection1->empty() );
    const SCT_RDO_Collection *outputCollection2 = outputDataHandle->indexFindPtr(bkgElementHash);
    ASSERT_TRUE( outputCollection2!=nullptr );
    ASSERT_TRUE( outputCollection2->empty() );
  }

  TEST_F(SCTOverlay_test, containers_with_matching_collections_one_with_an_RDO) {
    EventContext ctx(0,0);
    ctx.setExtension( Atlas::ExtendedEventContext( m_sg, 0 ) );
    SG::WriteHandle<SCT_RDO_Container> inputSigDataHandle{"StoreGateSvc+SCT_RDOs_SIG"};
    const unsigned int containerSize(1188);
    const IdentifierHash sigElementHash(1);
    const IdentifierHash bkgElementHash(1);
    const int sigGroupSize(1);
    const int sigTimeBin(1 << SCTOverlay::CurrentBC);
    const int sigStrip(10);
    const int sigErrors(0);
    const unsigned int sigWord(sigGroupSize | (sigStrip << 11) | (sigTimeBin << 22) | (sigErrors << 25));
    std::vector<int> sigErrorVect{};
    inputSigDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> sigCollection = std::make_unique<SCT_RDO_Collection>(sigElementHash);
    //Add a SCT3_RawData object
    std::unique_ptr<SCT3_RawData> sigDigit = std::make_unique<SCT3_RawData>(Identifier(12),sigWord, &sigErrorVect);
    sigCollection->push_back(sigDigit.release());
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<SCT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+SCT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> bkgCollection = std::make_unique<SCT_RDO_Collection>(bkgElementHash);
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),bkgElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle

    configureSCTOverlay();
    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<SCT_RDO_Container> outputDataHandle{"StoreGateSvc+SCT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    const SCT_RDO_Collection *outputCollection1 = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_TRUE( outputCollection1!=nullptr );
    ASSERT_TRUE( outputCollection1->size()==1 );
    const SCT3_RawData* outputDigit1 = dynamic_cast<const SCT3_RawData*>(outputCollection1->at(0));
    ASSERT_TRUE( outputDigit1!=nullptr );
    ASSERT_TRUE( outputDigit1->getGroupSize()==sigGroupSize );
    ASSERT_TRUE( outputDigit1->getTimeBin()==sigTimeBin );
  }

  TEST_F(SCTOverlay_test, containers_with_different_collections_one_RDO_each) {
    EventContext ctx(0,0);
    ctx.setExtension( Atlas::ExtendedEventContext( m_sg, 0 ) );
    SG::WriteHandle<SCT_RDO_Container> inputSigDataHandle{"StoreGateSvc+SCT_RDOs_SIG"};
    const unsigned int containerSize(1188);
    const IdentifierHash sigElementHash(1);
    const IdentifierHash bkgElementHash(2);
    const int sigGroupSize(1);
    const int sigTimeBin(1 << SCTOverlay::CurrentBC);
    const int sigStrip(10);
    const int sigErrors(0);
    const unsigned int sigWord(sigGroupSize | (sigStrip << 11) | (sigTimeBin << 22) | (sigErrors << 25));
    std::vector<int> sigErrorVect{};
    const int bkgGroupSize(1);
    const int bkgTimeBin(1 << SCTOverlay::CurrentBC );
    const int bkgStrip(10);
    const int bkgErrors(0);
    const unsigned int bkgWord(bkgGroupSize | (bkgStrip << 11) | (bkgTimeBin << 22) | (bkgErrors << 25));
    std::vector<int> bkgErrorVect{};
    inputSigDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> sigCollection = std::make_unique<SCT_RDO_Collection>(sigElementHash);
    //Add a SCT3_RawData object
    std::unique_ptr<SCT3_RawData> sigDigit = std::make_unique<SCT3_RawData>(Identifier(12),sigWord, &sigErrorVect);
    sigCollection->push_back(sigDigit.release());
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<SCT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+SCT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> bkgCollection = std::make_unique<SCT_RDO_Collection>(bkgElementHash);
    //Add a SCT3_RawData object
    std::unique_ptr<SCT3_RawData> bkgDigit = std::make_unique<SCT3_RawData>(Identifier(12),bkgWord,&bkgErrorVect);
    bkgCollection->push_back(bkgDigit.release());
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),bkgElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle

    configureSCTOverlay();
    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<SCT_RDO_Container> outputDataHandle{"StoreGateSvc+SCT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    const SCT_RDO_Collection *outputCollection1 = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_TRUE( outputCollection1!=nullptr );
    ASSERT_TRUE( outputCollection1->size()==1 );
    const SCT3_RawData* outputDigit1 = dynamic_cast<const SCT3_RawData*>(outputCollection1->at(0));
    ASSERT_TRUE( outputDigit1!=nullptr );
    ASSERT_TRUE( outputDigit1->getGroupSize()==sigGroupSize );
    ASSERT_TRUE( outputDigit1->getTimeBin()==sigTimeBin );
    const SCT_RDO_Collection *outputCollection2 = outputDataHandle->indexFindPtr(bkgElementHash);
    ASSERT_TRUE( outputCollection2!=nullptr );
    ASSERT_TRUE( outputCollection2->size()==1 );
    const SCT3_RawData* outputDigit2 = dynamic_cast<const SCT3_RawData*>(outputCollection2->at(0));
    ASSERT_TRUE( outputDigit2!=nullptr );
    ASSERT_TRUE( outputDigit2->getGroupSize()==bkgGroupSize );
    ASSERT_TRUE( outputDigit2->getTimeBin()==bkgTimeBin );
  }

  TEST_F(SCTOverlay_test, containers_with_matching_collections_with_adjecent_RDOs) {
    EventContext ctx(0,0);
    ctx.setExtension( Atlas::ExtendedEventContext( m_sg, 0 ) );
    SG::WriteHandle<SCT_RDO_Container> inputSigDataHandle{"StoreGateSvc+SCT_RDOs_SIG"};
    const unsigned int containerSize(1188);
    const IdentifierHash sigElementHash(1);
    const IdentifierHash bkgElementHash(1);
    const int sigGroupSize(1);
    const int sigTimeBin(1 << SCTOverlay::CurrentBC);
    const int sigStrip(10);
    const int sigErrors(0);
    const unsigned int sigWord(sigGroupSize | (sigStrip << 11) | (sigTimeBin << 22) | (sigErrors << 25));
    std::vector<int> sigErrorVect{};
    const int bkgGroupSize(2);
    const int bkgTimeBin(1 << SCTOverlay::CurrentBC);
    const int bkgStrip(11);
    const int bkgErrors(0);
    const unsigned int bkgWord(bkgGroupSize | (bkgStrip << 11) | (bkgTimeBin << 22) | (bkgErrors << 25));
    std::vector<int> bkgErrorVect{};
    inputSigDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> sigCollection = std::make_unique<SCT_RDO_Collection>(sigElementHash);
    //Add a SCT3_RawData object
    std::unique_ptr<SCT3_RawData> sigDigit = std::make_unique<SCT3_RawData>(Identifier(12),sigWord,&sigErrorVect);
    sigCollection->push_back(sigDigit.release());
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<SCT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+SCT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> bkgCollection = std::make_unique<SCT_RDO_Collection>(bkgElementHash);
    //Add a SCT3_RawData object
    std::unique_ptr<SCT3_RawData> bkgDigit = std::make_unique<SCT3_RawData>(Identifier(13),bkgWord,&bkgErrorVect);
    bkgCollection->push_back(bkgDigit.release());
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),bkgElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle

    configureSCTOverlay();
    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<SCT_RDO_Container> outputDataHandle{"StoreGateSvc+SCT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    const SCT_RDO_Collection *outputCollection = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_TRUE( outputCollection!=nullptr );
    ASSERT_TRUE( outputCollection->size()==1 );
    const SCT3_RawData* outputDigit1 = dynamic_cast<const SCT3_RawData*>(outputCollection->at(0));
    ASSERT_TRUE( outputDigit1!=nullptr );
    ASSERT_TRUE( outputDigit1->getGroupSize()==sigGroupSize+bkgGroupSize ); // adjacent RDOs combined
    ASSERT_TRUE( outputDigit1->getTimeBin()==sigTimeBin ); // TimeBin matches signal RDO
  }

  TEST_F(SCTOverlay_test, containers_with_matching_collections_signal_overlapping_with_early_bkg) {
    EventContext ctx(0,0);
    ctx.setExtension( Atlas::ExtendedEventContext( m_sg, 0 ) );
    SG::WriteHandle<SCT_RDO_Container> inputSigDataHandle{"StoreGateSvc+SCT_RDOs_SIG"};
    const unsigned int containerSize(1188);
    const IdentifierHash sigElementHash(1);
    const IdentifierHash bkgElementHash(1);
    const int sigGroupSize(3);
    const int sigTimeBin(1 << SCTOverlay::CurrentBC);
    const int sigStrip(10);
    const int sigErrors(0);
    const unsigned int sigWord(sigGroupSize | (sigStrip << 11) | (sigTimeBin << 22) | (sigErrors << 25));
    std::vector<int> sigErrorVect{};
    const int bkgGroupSize(1);
    const int bkgTimeBin(1 << SCTOverlay::PreviousBC);
    const int bkgStrip(11);
    const int bkgErrors(0);
    const unsigned int bkgWord(bkgGroupSize | (bkgStrip << 11) | (bkgTimeBin << 22) | (bkgErrors << 25));
    std::vector<int> bkgErrorVect{};
    inputSigDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> sigCollection = std::make_unique<SCT_RDO_Collection>(sigElementHash);
    //Add a SCT3_RawData object
    std::unique_ptr<SCT3_RawData> sigDigit = std::make_unique<SCT3_RawData>(Identifier(12),sigWord,&sigErrorVect);
    sigCollection->push_back(sigDigit.release());
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<SCT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+SCT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> bkgCollection = std::make_unique<SCT_RDO_Collection>(bkgElementHash);
    //Add a SCT3_RawData object
    std::unique_ptr<SCT3_RawData> bkgDigit = std::make_unique<SCT3_RawData>(Identifier(13),bkgWord,&bkgErrorVect);
    bkgCollection->push_back(bkgDigit.release());
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),bkgElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle

    configureSCTOverlay();
    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<SCT_RDO_Container> outputDataHandle{"StoreGateSvc+SCT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    const SCT_RDO_Collection *outputCollection = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_TRUE( outputCollection!=nullptr );
    ASSERT_TRUE( outputCollection->size()==3 );
    const SCT3_RawData* outputDigit1 = dynamic_cast<const SCT3_RawData*>(outputCollection->at(0));
    ASSERT_TRUE( outputDigit1!=nullptr );
    ASSERT_TRUE( outputDigit1->getGroupSize()==1 ); // Signal RDO split by overlapping RDO with different TimeBin
    ASSERT_TRUE( outputDigit1->getTimeBin()==sigTimeBin ); // TimeBin matches signal RDO
    const SCT3_RawData* outputDigit2 = dynamic_cast<const SCT3_RawData*>(outputCollection->at(1));
    ASSERT_TRUE( outputDigit2!=nullptr );
    ASSERT_TRUE( outputDigit2->getGroupSize()==1 ); // Signal RDO split by overlapping RDO with different TimeBin
    ASSERT_TRUE( outputDigit2->getTimeBin()==bkgTimeBin+sigTimeBin ); // TimeBin is a sum of signal and background TimeBins
    const SCT3_RawData* outputDigit3 = dynamic_cast<const SCT3_RawData*>(outputCollection->at(2));
    ASSERT_TRUE( outputDigit3!=nullptr );
    ASSERT_TRUE( outputDigit3->getGroupSize()==1 ); // Signal RDO split by overlapping RDO with different TimeBin
    ASSERT_TRUE( outputDigit3->getTimeBin()==sigTimeBin ); // TimeBin matches background RDO
  }

  TEST_F(SCTOverlay_test, containers_with_matching_collections_monkey) {
    EventContext ctx(0,0);
    ctx.setExtension( Atlas::ExtendedEventContext( m_sg, 0 ) );
    SG::WriteHandle<SCT_RDO_Container> inputSigDataHandle{"StoreGateSvc+SCT_RDOs_SIG"};
    const unsigned int containerSize(1188);
    const IdentifierHash sigElementHash(1);
    const IdentifierHash bkgElementHash(1);
    const int sigGroupSize(1);
    const int sigTimeBin(1 << SCTOverlay::CurrentBC);
    const int sigStrip(10);
    const int sigErrors(0);
    const unsigned int sigWord(sigGroupSize | (sigStrip << 11) | (sigTimeBin << 22) | (sigErrors << 25));
    std::vector<int> sigErrorVect{};
    const int bkgGroupSize(1);
    const int bkgTimeBin(1 << SCTOverlay::CurrentBC);
    const int bkgStrip(40);
    const int bkgErrors(0);
    const unsigned int bkgWord(bkgGroupSize | (bkgStrip << 11) | (bkgTimeBin << 22) | (bkgErrors << 25));
    std::vector<int> bkgErrorVect{};
    inputSigDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> sigCollection = std::make_unique<SCT_RDO_Collection>(sigElementHash);
    //Add a SCT3_RawData object
    std::unique_ptr<SCT3_RawData> sigDigit = std::make_unique<SCT3_RawData>(Identifier(12),sigWord,&sigErrorVect);
    sigCollection->push_back(sigDigit.release());
    ASSERT_TRUE(inputSigDataHandle->addCollection(sigCollection.get(),sigElementHash).isSuccess());
    (void)sigCollection.release(); // Now owned by inputSigDataHandle
    SG::WriteHandle<SCT_RDO_Container> inputBkgDataHandle{"StoreGateSvc+SCT_RDOs_BKG"};
    inputBkgDataHandle = std::make_unique<SCT_RDO_Container>(containerSize);
    std::unique_ptr<SCT_RDO_Collection> bkgCollection = std::make_unique<SCT_RDO_Collection>(bkgElementHash);
    //Add a SCT3_RawData object
    std::unique_ptr<SCT3_RawData> bkgDigit = std::make_unique<SCT3_RawData>(Identifier(40),bkgWord,&bkgErrorVect);
    bkgCollection->push_back(bkgDigit.release());
    ASSERT_TRUE(inputBkgDataHandle->addCollection(bkgCollection.get(),bkgElementHash).isSuccess());
    (void)bkgCollection.release(); // Now owned by inputBkgDataHandle

    configureSCTOverlay();
    ASSERT_TRUE( m_alg->initialize().isSuccess() );
    ASSERT_TRUE( m_alg->execute(ctx).isSuccess() );
    // check output makes sense
    SG::ReadHandle<SCT_RDO_Container> outputDataHandle{"StoreGateSvc+SCT_RDOs"};
    ASSERT_TRUE( outputDataHandle.isValid() );
    const SCT_RDO_Collection *outputCollection = outputDataHandle->indexFindPtr(sigElementHash);
    ASSERT_TRUE( outputCollection!=nullptr );
    ASSERT_TRUE( outputCollection->size()==2 );
    const SCT3_RawData* outputDigit1 = dynamic_cast<const SCT3_RawData*>(outputCollection->at(0));
    ASSERT_TRUE( outputDigit1!=nullptr );
    ASSERT_TRUE( outputDigit1->getGroupSize()==sigGroupSize ); // RDOs in same collection sorted according to Identifier
    ASSERT_TRUE( outputDigit1->getTimeBin()==sigTimeBin ); // TimeBin matches signal RDO
    const SCT3_RawData* outputDigit2 = dynamic_cast<const SCT3_RawData*>(outputCollection->at(1));
    ASSERT_TRUE( outputDigit2!=nullptr );
    ASSERT_TRUE( outputDigit2->getGroupSize()==bkgGroupSize ); // RDOs in same collection sorted according to Identifier
    ASSERT_TRUE( outputDigit2->getTimeBin()==bkgTimeBin ); // TimeBin matches background RDO
  }

} // <-- namespace OverlayTesting


int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest( &argc, argv );
  ::testing::AddGlobalTestEnvironment( new OverlayTesting::GaudiEnvironment );
  return RUN_ALL_TESTS();
}
