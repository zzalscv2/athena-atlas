/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**   @file SCT_Clusterization.cxx
 *   Implementation file for the SCT_Clusterization class.
 *   @author Paul Bell, Tommaso Lari, Shaun Roe, Carl Gwilliam
 *   @date 08 July 2008
 */

#include "InDetPrepRawDataFormation/SCT_Clusterization.h"

#include "AtlasDetDescr/AtlasDetectorID.h"
#include "Identifier/IdentifierHash.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetPrepRawData/SiClusterContainer.h"
#include "InDetRawData/SCT_RDORawData.h"
#include "SCT_ConditionsData/SCT_FlaggedCondEnum.h"
#include "StoreGate/WriteHandle.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include "AthAllocators/DataPool.h"
#include <limits>
#include <unordered_map>

namespace InDet {
  using namespace InDet;

  // Constructor with parameters:
  SCT_Clusterization::SCT_Clusterization(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator)
  {
    // Get parameter values from jobOptions file
    declareProperty("ClusterContainerCacheKey", m_clusterContainerCacheKey="");
    declareProperty("FlaggedCondCacheKey", m_flaggedCondCacheKey="");
  }

  // Initialize method:
  StatusCode SCT_Clusterization::initialize() {
    ATH_MSG_INFO("SCT_Clusterization::initialize()!");

    // Get the conditions summary service (continue anyway, just check the pointer
    // later and declare everything to be 'good' if it is nullptr)
    ATH_CHECK( m_pSummaryTool.retrieve( DisableTool{!m_checkBadModules.value() || (!m_sctDetElStatus.empty() && !VALIDATE_STATUS_ARRAY_ACTIVATED)} ) );

    m_clusterContainerLinkKey = m_clusterContainerKey.key();

    ATH_CHECK(m_rdoContainerKey.initialize());
    ATH_CHECK(m_clusterContainerKey.initialize());
    ATH_CHECK(m_clusterContainerLinkKey.initialize());
    ATH_CHECK(m_clusterContainerCacheKey.initialize(not m_clusterContainerCacheKey.key().empty()));
    ATH_CHECK(m_flaggedCondDataKey.initialize());
    ATH_CHECK(m_flaggedCondCacheKey.initialize(not m_flaggedCondCacheKey.key().empty()));

    ATH_CHECK(m_sctDetElStatus.initialize( !m_sctDetElStatus.empty() ));

    // Get the clustering tool
    ATH_CHECK(m_clusteringTool.retrieve());

    // Get the SCT ID helper
    ATH_CHECK(detStore()->retrieve(m_idHelper, "SCT_ID"));

    if (m_roiSeeded.value()) {
      ATH_CHECK(m_roiCollectionKey.initialize());
      ATH_CHECK(m_regionSelector.retrieve());
    } else {
      ATH_CHECK(m_roiCollectionKey.initialize(false));
      m_regionSelector.disable();
    }

    if ( !m_monTool.empty() ) {
       ATH_CHECK(m_monTool.retrieve() );
    }
    else {
       ATH_MSG_INFO("Monitoring tool is empty");
    }

    return StatusCode::SUCCESS;
  }

  // Execute method:
  StatusCode SCT_Clusterization::execute(const EventContext& ctx) const {
    //Monitoring Tool Configuration
    auto mnt_timer_Total                 = Monitored::Timer<std::chrono::milliseconds>("TIME_Total");
    auto mnt_timer_SummaryTool           = Monitored::Timer<std::chrono::milliseconds>("TIME_SummaryTool");
    auto mnt_timer_Clusterize            = Monitored::Timer<std::chrono::milliseconds>("TIME_Clusterize");
    // Register the IdentifiableContainer into StoreGate
    SG::WriteHandle<SCT_ClusterContainer> clusterContainer{m_clusterContainerKey, ctx};
    if (m_clusterContainerCacheKey.key().empty()) {
      ATH_CHECK(clusterContainer.record(std::make_unique<SCT_ClusterContainer>(m_idHelper->wafer_hash_max(), EventContainers::Mode::OfflineFast)));
    } else {
      SG::UpdateHandle<SCT_ClusterContainerCache> clusterContainercache{m_clusterContainerCacheKey, ctx};
      ATH_CHECK(clusterContainer.record(std::make_unique<SCT_ClusterContainer>(clusterContainercache.ptr())));
    }
    ATH_MSG_DEBUG("Container '" << clusterContainer.name() << "' initialised");

    ATH_CHECK(clusterContainer.symLink(m_clusterContainerLinkKey));
    ATH_CHECK(clusterContainer.isValid());
    ATH_MSG_DEBUG("SCT clusters '" << clusterContainer.name() << "' symlinked in StoreGate");

    SG::WriteHandle<IDCInDetBSErrContainer> flaggedCondData{m_flaggedCondDataKey, ctx};
    if (m_flaggedCondCacheKey.key().empty()) {
      ATH_CHECK(flaggedCondData.record( std::make_unique<IDCInDetBSErrContainer>(m_idHelper->wafer_hash_max(), std::numeric_limits<IDCInDetBSErrContainer::ErrorCode>::min())));
      ATH_MSG_DEBUG("Created IDCInDetBSErrContainer w/o using external cache");
    } else {
      SG::UpdateHandle<IDCInDetBSErrContainer_Cache> flaggedCondCacheHandle(m_flaggedCondCacheKey, ctx);
      ATH_CHECK(flaggedCondCacheHandle.isValid() );
      ATH_CHECK(flaggedCondData.record( std::make_unique<IDCInDetBSErrContainer>(flaggedCondCacheHandle.ptr())) );
      ATH_MSG_DEBUG("Created SCT IDCInDetBSErrContainer using external cache");
    }
    std::unordered_map<IdentifierHash, IDCInDetBSErrContainer::ErrorCode> flaggedCondMap; // temporary store of flagged condition error

    // First, we have to retrieve and access the container, not because we want to
    // use it, but in order to generate the proxies for the collections, if they
    // are being provided by a container converter.
    SG::ReadHandle<SCT_RDO_Container> rdoContainer{m_rdoContainerKey, ctx};
    ATH_CHECK(rdoContainer.isValid());

    // Anything to dereference the DataHandle will trigger the converter
    SCT_RDO_Container::const_iterator rdoCollections{rdoContainer->begin()};
    SCT_RDO_Container::const_iterator rdoCollectionsEnd{rdoContainer->end()};
    bool dontDoClusterization{false};
    //new code to remove large numbers of hits (what is large?)
    if (m_maxTotalOccupancyPercent.value()!=100) {
      constexpr unsigned int totalNumberOfChannels{6279168};
      const unsigned int maxAllowableStrips{(totalNumberOfChannels*m_maxTotalOccupancyPercent.value())/100};//integer arithmetic, should be ok
      unsigned int totalFiredStrips{0};
      for (; rdoCollections != rdoCollectionsEnd; ++rdoCollections) {
        for (const SCT_RDORawData* rdo: **rdoCollections) {
          totalFiredStrips += rdo->getGroupSize();
        }
      } //iterator is now at the end
      //reset the iterator
      rdoCollections = rdoContainer->begin();
      if (totalFiredStrips > maxAllowableStrips) {
        ATH_MSG_WARNING("This event has too many hits in the SCT: " << totalFiredStrips << " > " << maxAllowableStrips);
        dontDoClusterization = true;
      }
    }
    SG::ReadHandle<InDet::SiDetectorElementStatus> sctDetElStatus;
    if (!m_sctDetElStatus.empty()) {
       sctDetElStatus=SG::ReadHandle<InDet::SiDetectorElementStatus>(m_sctDetElStatus, ctx);
       ATH_CHECK( sctDetElStatus.isValid() ? StatusCode::SUCCESS : StatusCode::FAILURE);
    }

    if (not dontDoClusterization) {
       std::unique_ptr<DataPool<SCT_Cluster>> dataItemsPool = nullptr;
       const bool hasExternalCache = rdoContainer->hasExternalCache();
       if (!hasExternalCache) {
        dataItemsPool = std::make_unique<DataPool<SCT_Cluster>>(ctx);
        dataItemsPool->reserve(20000);  // Some large default size
       } else if (m_useDataPoolWithCache) {
        dataItemsPool = std::make_unique<DataPool<SCT_Cluster>>(ctx);
        // this is  per view so let it expand on its own in blocks
       }
       // cache to avoid re-allocation inside the loop.
       // Trying to re-use the vector capacities.
       // we clear them inside the methods we call

       SCTClusteringCache cache;
       cache.currentVector.reserve(32);
       cache.idGroups.reserve(16);
       cache.tbinGroups.reserve(16);
       if (not m_roiSeeded.value()) {  // Full-scan mode

        for (; rdoCollections != rdoCollectionsEnd; ++rdoCollections) {
          const InDetRawDataCollection<SCT_RDORawData>* rd{*rdoCollections};
          ATH_MSG_DEBUG("RDO collection size=" << rd->size() << ", Hash="
                                               << rd->identifyHash());
          SCT_ClusterContainer::IDC_WriteHandle lock{
              clusterContainer->getWriteHandle(rdoCollections.hashId())};
          if (lock.OnlineAndPresentInAnotherView()) {
            ATH_MSG_DEBUG(
                "Item already in cache , Hash=" << rd->identifyHash());
            continue;
          }
          bool goodModule{
              m_checkBadModules.value()
                  ? (!m_sctDetElStatus.empty()
                         ? sctDetElStatus->isGood(rd->identifyHash())
                         : m_pSummaryTool->isGood(rd->identifyHash(), ctx))
                  : true};
          VALIDATE_STATUS_ARRAY(
              m_checkBadModules.value() && !m_sctDetElStatus.empty(),
              sctDetElStatus->isGood(rd->identifyHash()),
              m_pSummaryTool->isGood(rd->identifyHash(), ctx));

          if (!goodModule) {
            ATH_MSG_DEBUG(" module status is bad");
          }
          // Check the RDO is not empty and that the wafer is good according to
          // the conditions
          if ((not rd->empty()) and goodModule) {
            // If more than a certain number of RDOs set module to bad
            if (m_maxFiredStrips.value()) {
              unsigned int nFiredStrips{0};
              for (const SCT_RDORawData* rdo : *rd) {
                nFiredStrips += rdo->getGroupSize();
              }
              if (nFiredStrips > m_maxFiredStrips.value()) {
                // This should work in the case of a new code or existing, since
                // the default init is 0
                constexpr int value =
                    (1 << SCT_FlaggedCondEnum::ExceedMaxFiredStrips);
                auto [pPair, inserted] =
                    flaggedCondMap.insert({rd->identifyHash(), value});
                if (not inserted) {
                  pPair->second |= value;
                }
                continue;
              }
            }
            // Use one of the specific clustering AlgTools to make clusters
            std::unique_ptr<SCT_ClusterCollection> clusterCollection{
                m_clusteringTool->clusterize(
                    *rd, *m_idHelper,
                    !m_sctDetElStatus.empty() ? sctDetElStatus.cptr() : nullptr,
                    cache, dataItemsPool.get(), ctx)};
            if (clusterCollection) {
              if (not clusterCollection->empty()) {
                const IdentifierHash hash{clusterCollection->identifyHash()};
                ATH_CHECK(lock.addOrDelete(std::move(clusterCollection)));
                ATH_MSG_DEBUG("Clusters with key '"
                              << hash << "' added to Container\n");
              } else {
                ATH_MSG_DEBUG("Don't write empty collections\n");
              }
            } else {
              ATH_MSG_DEBUG("Clustering algorithm found no clusters\n");
            }
          }
        }
       } else {  // enter RoI-seeded mode
        SG::ReadHandle<TrigRoiDescriptorCollection> roiCollection{
            m_roiCollectionKey, ctx};
        ATH_CHECK(roiCollection.isValid());
        TrigRoiDescriptorCollection::const_iterator roi{roiCollection->begin()};
        TrigRoiDescriptorCollection::const_iterator roiE{roiCollection->end()};
        std::vector<IdentifierHash> listOfSCTIds;
        for (; roi != roiE; ++roi) {
          listOfSCTIds.clear();  // Prevents needless memory reallocations
          m_regionSelector->HashIDList(**roi, listOfSCTIds);
          ATH_MSG_VERBOSE(**roi);
          ATH_MSG_VERBOSE("REGTEST: SCT : Roi contains " << listOfSCTIds.size()
                                                         << " det. Elements");
          for (size_t i{0}; i < listOfSCTIds.size(); i++) {
            IdentifierHash id = listOfSCTIds[i];
            const InDetRawDataCollection<SCT_RDORawData>* RDO_Collection{
                rdoContainer->indexFindPtr(id)};
            if (RDO_Collection == nullptr){
              continue;
            }
            bool goodModule;
            {
              Monitored::ScopedTimer time_SummaryTool(mnt_timer_SummaryTool);
              goodModule = {m_checkBadModules.value()
                                ? (!m_sctDetElStatus.empty()
                                       ? sctDetElStatus->isGood(id)
                                       : m_pSummaryTool->isGood(id, ctx))
                                : true};
              VALIDATE_STATUS_ARRAY(
                  m_checkBadModules.value() && !m_sctDetElStatus.empty(),
                  sctDetElStatus->isGood(id), m_pSummaryTool->isGood(id));
              if (!goodModule){
                ATH_MSG_VERBOSE("module status flagged as BAD");
              }
            }
            // Check the RDO is not empty and that the wafer is good according
            // to the conditions
            if ((not RDO_Collection->empty()) and goodModule) {
              // If more than a certain number of RDOs set module to bad
              if (m_maxFiredStrips.value()) {
                unsigned int nFiredStrips{0};
                for (const SCT_RDORawData* rdo : *RDO_Collection){
                  nFiredStrips += rdo->getGroupSize();
                }
                if (nFiredStrips > m_maxFiredStrips.value()) {
                  // This should work in the case of a new code or existing,
                  // since the default init is 0
                  constexpr int value =
                      (1 << SCT_FlaggedCondEnum::ExceedMaxFiredStrips);
                  auto [pPair, inserted] = flaggedCondMap.insert({id, value});
                  if (not inserted) {
                    pPair->second |= value;
                  }
                  continue;
                }
              }
            }

            SCT_ClusterContainer::IDC_WriteHandle lock{
                clusterContainer->getWriteHandle(listOfSCTIds[i])};
            if (lock.OnlineAndPresentInAnotherView()) {
              ATH_MSG_DEBUG("Item already in cache , Hash=" << listOfSCTIds[i]);
              continue;
            }

            // Use one of the specific clustering AlgTools to make clusters
            {
              Monitored::ScopedTimer time_Clusterize(mnt_timer_Clusterize);
              std::unique_ptr<SCT_ClusterCollection> clusterCollection{
                  m_clusteringTool->clusterize(
                      *RDO_Collection, *m_idHelper,
                      !m_sctDetElStatus.empty() ? sctDetElStatus.cptr()
                                                : nullptr,
                      cache, dataItemsPool.get(), ctx)};
              if (clusterCollection and (not clusterCollection->empty())) {
                ATH_MSG_VERBOSE("REGTEST: SCT : clusterCollection contains " << clusterCollection->size() << " clusters");
                ATH_CHECK(lock.addOrDelete(std::move(clusterCollection)));
              } else {
                ATH_MSG_DEBUG("No SCTClusterCollection to write");
              }
            }
          }
        }
       }
    }
    // Set container to const
    ATH_CHECK(clusterContainer.setConst());

    // Fill flaggedCondData
    for (auto [hash, error] : flaggedCondMap) {
      flaggedCondData->setOrDrop(hash, error);
    }
    auto monTime = Monitored::Group(m_monTool, mnt_timer_Total, mnt_timer_Clusterize, mnt_timer_SummaryTool);
    return StatusCode::SUCCESS;
  }

  // Finalize method:
  StatusCode SCT_Clusterization::finalize()
  {
    return StatusCode::SUCCESS;
  }
}
