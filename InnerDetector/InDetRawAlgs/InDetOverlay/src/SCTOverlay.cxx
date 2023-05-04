/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetIdentifier/SCT_ID.h"
#include "InDetOverlay/SCTOverlay.h"

#include "IDC_OverlayBase/IDC_OverlayHelpers.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "AthAllocators/DataPool.h"

namespace Overlay
{
  // Specialize copyCollection() for the SCT
  template<>
  std::unique_ptr<SCT_RDO_Collection> copyCollection(const IdentifierHash &hashId,
                                                     const SCT_RDO_Collection *collection,
                                                     DataPool<SCT3_RawData>& dataItems)
  {
    auto outputCollection = std::make_unique<SCT_RDO_Collection>(hashId);
    outputCollection->setIdentifier(collection->identify());

    // Elements created here are owned by the DataPool
    outputCollection->clear(SG::VIEW_ELEMENTS);
    outputCollection->reserve(collection->size());

    for (const SCT_RDORawData *existingDatum : *collection) {
      auto *oldDatum = dynamic_cast<const SCT3_RawData *>(existingDatum);
      if (not oldDatum) {
        throw std::runtime_error("Dynamic cast to SCT3_RawData failed in SCTOverlay.cxx, Overlay::copyCollection");
      }
      SCT3_RawData* datumCopy = dataItems.nextElementPtr();
      (*datumCopy) = SCT3_RawData(oldDatum->identify(), oldDatum->getWord(),
                                  &oldDatum->getErrorCondensedHit());
      outputCollection->push_back(datumCopy);
    }

    return outputCollection;
  }

  // Specialize mergeCollections() for the SCT
  // bkg,signal and output collections
  // are all VIEW containers.
  template<>
  void mergeCollections(SCT_RDO_Collection *bkgCollection,
                        SCT_RDO_Collection *signalCollection,
                        SCT_RDO_Collection *outputCollection,
                        const IDC_OverlayBase *algorithm,
                        DataPool<SCT3_RawData>& dataItems)
  {
    // We want to use the SCT_ID helper provided by SCTOverlay, thus the constraint
    const SCTOverlay *parent = dynamic_cast<const SCTOverlay *>(algorithm);
    if (!parent) {
      throw std::runtime_error("mergeCollections<SCT_RDORawData>() called by a wrong parent algorithm? Must be SCTOverlay.");
    }

    if (bkgCollection->identify() != signalCollection->identify()) {
      throw std::runtime_error("mergeCollections<SCT_RDO_Collection>(): collection Id mismatch");
    }

    outputCollection->reserve(
        std::max(bkgCollection->size(), signalCollection->size()));
    const Identifier idColl =
        parent->get_sct_id()->wafer_id(signalCollection->identifyHash());

    // Strip hit timing information for Next, Current, Previous and Any BCs
    // Prepare one more strip to create the last one. The additional strip has no hits.
    std::bitset<SCTOverlay::NumberOfStrips+1> stripInfo[SCTOverlay::NumberOfBitSets];
    // Process background and signal in the wafer
    for (unsigned source = SCTOverlay::BkgSource; source < SCTOverlay::NumberOfSources; source++) {
      SCT_RDO_Collection::const_iterator rdo;
      SCT_RDO_Collection::const_iterator rdoEnd;
      if (source == SCTOverlay::BkgSource) { // background
        rdo = bkgCollection->begin();
        rdoEnd = bkgCollection->end();
      } else { // signal
        rdo = signalCollection->begin();
        rdoEnd = signalCollection->end();
      }
      // Loop over all RDOs in the wafer
      for (; rdo!=rdoEnd; ++rdo) {
        // Here  we'll just go with SCT3.
        const SCT3_RawData* rdo3 = dynamic_cast<const SCT3_RawData*>(*rdo);
        if (!rdo3) {
          std::ostringstream os;
          const auto& elt = **rdo;
          os << "mergeCollection<SCT_RDO_Collection>(): wrong datum format. Only SCT3_RawData are produced by SCT_RodDecoder and supported by overlay."
             << "For the supplied datum  typeid(datum).name() = " << typeid(elt).name();
          throw std::runtime_error(os.str());
        }
        int strip = parent->get_sct_id()->strip(rdo3->identify());
        int stripEnd = strip + rdo3->getGroupSize();
        int timeBin = rdo3->getTimeBin();
        for (; strip < stripEnd && strip < SCTOverlay::NumberOfStrips; strip++) {
          // Fill timing information for each strips, loop over 3 BCs
          for (unsigned int bc = SCTOverlay::NextBC; bc < SCTOverlay::NumberOfBCs; bc++) {
            if (timeBin & (1 << bc)) stripInfo[bc].set(strip);
          }
        }
      }
    }
    // Get OR for AnyBC, loop over 3 BCs
    for (unsigned int bc = SCTOverlay::NextBC; bc < SCTOverlay::NumberOfBCs; bc++) {
      stripInfo[SCTOverlay::AnyBC] |= stripInfo[bc];
    }
    // Check if we need to use Expanded mode by checking if there is at least one hit in Next BC or Previous BC
    bool anyNextBCHits = stripInfo[SCTOverlay::NextBC].any();
    bool anyPreivousBCHits = stripInfo[SCTOverlay::PreviousBC].any();
    bool isExpandedMode = (anyNextBCHits or anyPreivousBCHits);
    // No error information is recorded because this information is not filled in data and no errors are assumed in MC.
    const int ERRORS = 0;
    const std::vector<int> errvec{};
    if (isExpandedMode) {
      // Expanded mode (record strip one by one)
      const int groupSize = 1;
      int tbin = 0;
      for (unsigned int strip = 0; strip < SCTOverlay::NumberOfStrips; strip++) {
        if (stripInfo[SCTOverlay::AnyBC][strip]) {
          tbin = 0;
          for (unsigned int bc = SCTOverlay::NextBC; bc < SCTOverlay::NumberOfBCs; bc++) {
            if (stripInfo[bc][strip]) {
              tbin |= (1 << bc);
            }
          }
          unsigned int SCT_Word = (groupSize | (strip << 11) | (tbin <<22) | (ERRORS << 25));
          Identifier rdoId = parent->get_sct_id()->strip_id(idColl, strip) ;
          // use the pool
          SCT3_RawData* datum = dataItems.nextElementPtr();
          (*datum) = SCT3_RawData(rdoId, SCT_Word, &errvec);
          outputCollection->push_back(datum);
        }
      }
    } else {
      // We can record consecutive hits into one RDO if all hits have timeBin of 010.
      unsigned int groupSize = 0;
      const int tbin = (1 << SCTOverlay::CurrentBC);
      for (unsigned int strip=0; strip<SCTOverlay::NumberOfStrips+1; strip++) { // Loop over one more strip to create the last one if any
        if (stripInfo[SCTOverlay::AnyBC][strip]) {
          groupSize++;
        } else {
          if (groupSize>0) {
            unsigned int firstStrip = strip - groupSize;
            unsigned int SCT_Word = (groupSize | (firstStrip << 11) | (tbin <<22) | (ERRORS << 25));
            Identifier rdoId = parent->get_sct_id()->strip_id(idColl, firstStrip) ;
            SCT3_RawData* datum = dataItems.nextElementPtr();
            (*datum) = SCT3_RawData(rdoId, SCT_Word, &errvec);
            outputCollection->push_back(datum);
            groupSize = 0;
          }
        }
      }
    }
  } // mergeCollections()
} // namespace Overlay


SCTOverlay::SCTOverlay(const std::string &name, ISvcLocator *pSvcLocator)
  : IDC_OverlayBase(name, pSvcLocator)
{
}

StatusCode SCTOverlay::initialize()
{
  ATH_MSG_DEBUG("Initializing...");

  // Check and initialize keys
  ATH_CHECK( m_bkgInputKey.initialize(!m_bkgInputKey.key().empty()) );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_bkgInputKey);
  ATH_CHECK( m_signalInputKey.initialize() );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_signalInputKey);
  ATH_CHECK( m_outputKey.initialize() );
  ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_outputKey);

  // Retrieve SCT ID helper
  if (!detStore()->retrieve(m_sctId, "SCT_ID").isSuccess() || !m_sctId) {
    ATH_MSG_FATAL("Cannot retrieve SCT ID helper");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode SCTOverlay::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("execute() begin");

  // Reading the input RDOs
  ATH_MSG_VERBOSE("Retrieving input RDO containers");

  const SCT_RDO_Container *bkgContainerPtr = nullptr;
  if (!m_bkgInputKey.empty()) {
    SG::ReadHandle<SCT_RDO_Container> bkgContainer(m_bkgInputKey, ctx);
    if (!bkgContainer.isValid()) {
      ATH_MSG_ERROR("Could not get background SCT RDO container " << bkgContainer.name() << " from store " << bkgContainer.store());
      return StatusCode::FAILURE;
    }
    bkgContainerPtr = bkgContainer.cptr();

    ATH_MSG_DEBUG("Found background SCT RDO container " << bkgContainer.name() << " in store " << bkgContainer.store());
    ATH_MSG_DEBUG("SCT Background = " << Overlay::debugPrint(bkgContainer.cptr(), 50));
  }

  SG::ReadHandle<SCT_RDO_Container> signalContainer(m_signalInputKey, ctx);
  if (!signalContainer.isValid()) {
    ATH_MSG_ERROR("Could not get signal SCT RDO container " << signalContainer.name() << " from store " << signalContainer.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Found signal SCT RDO container " << signalContainer.name() << " in store " << signalContainer.store());
  ATH_MSG_DEBUG("SCT Signal     = " << Overlay::debugPrint(signalContainer.cptr(), 50));

  // Creating output RDO container
  SG::WriteHandle<SCT_RDO_Container> outputContainer(m_outputKey, ctx);


  ATH_CHECK(outputContainer.record(std::make_unique<SCT_RDO_Container>(signalContainer->size())));
  if (!outputContainer.isValid()) {
    ATH_MSG_ERROR("Could not record output SCT RDO container " << outputContainer.name() << " to store " << outputContainer.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Recorded output SCT RDO container " << outputContainer.name() << " in store " << outputContainer.store());

  // The DataPool, this is what will actually own the elements
  // we create during this algorithm. The containers are views.
  DataPool<SCT3_RawData> dataItemsPool(ctx);
  // It resizes but lets reserve already quite a few
  dataItemsPool.reserve(250000);

  ATH_CHECK(overlayContainer(bkgContainerPtr, signalContainer.cptr(), outputContainer.ptr(),dataItemsPool));
  ATH_MSG_DEBUG("SCT Result   = " << Overlay::debugPrint(outputContainer.ptr(), 50));

  ATH_MSG_DEBUG("execute() end");
  return StatusCode::SUCCESS;
}
