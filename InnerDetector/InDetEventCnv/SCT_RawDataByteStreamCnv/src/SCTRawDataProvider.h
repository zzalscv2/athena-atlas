// -*- C++ -*-

/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SCT_RAWDATABYTESTREAMCNV_SCTRAWDATAPROVIDER_H
#define SCT_RAWDATABYTESTREAMCNV_SCTRAWDATAPROVIDER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "InDetRawData/InDetTimeCollection.h"
#include "InDetRawData/SCT_RDO_Container.h"
#include "InDetByteStreamErrors/InDetBSErrContainer.h"
#include "InDetByteStreamErrors/IDCInDetBSErrContainer.h"
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "IRegionSelector/IRegSelTool.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

class ISCTRawDataProviderTool;
class ISCT_CablingTool;
class SCT_ID;

/**
 * @class SCTRawDataProvider
 *
 * @brief Athena Algorithm to decode the SCT Byte Stream
 *
 * Gets vector of ROBFragments from the ROBDataProviderSvc
 * and uses AlgTools (SCTRawDataProviderTool and SCT_RodDecoder
 * to read the ByteStream and make RDOs.
 * Output is one RDO container (IDC) per event, which contains
 * one Collection per link (8176 in total), each of which contains
 * RDOs for hits (one RDO per strip in expanded mode, one per cluster
 * in condensed mode).
 *
 * Class based on TRT equivalent.
 */
class SCTRawDataProvider : public AthReentrantAlgorithm
{
 public:

  /** Constructor */
  SCTRawDataProvider(const std::string &name, ISvcLocator *pSvcLocator);

  /** Destructor */
  virtual ~SCTRawDataProvider() = default;

  /** Initialize */
  virtual StatusCode initialize() override;

  /** Execute */
  virtual StatusCode execute(const EventContext& ctx) const override;

  /** Make this algorithm clonable. */
  virtual bool isClonable() const override { return true; };

 private:

  /** ROB Data Provider for accessing ROB data. */
  ServiceHandle<IROBDataProviderSvc> m_robDataProvider{this,
                                                       "ROBDataProviderSvc",
                                                       "ROBDataProviderSvc"};

  /** Region Selector tool for Athena. */
  ToolHandle<IRegSelTool> m_regionSelector{this,
                                           "RegSelTool",
                                           "RegSelTool/RegSel_SCT"};

  /** Tool to fill Collections of SCT RDO Containers. */
  ToolHandle<ISCTRawDataProviderTool> m_rawDataTool{this,
                                                    "ProviderTool",
                                                    "SCTRawDataProviderTool",
                                                    "SCT  Raw Data Provider Tool"};

  /** Providing mappings of online and offline identifiers and also serial numbers. */
  ToolHandle<ISCT_CablingTool> m_cabling{this,
                                         "SCT_CablingTool",
                                         "SCT_CablingTool",
                                         "Tool to retrieve SCT Cabling"};

  /** Identifier helper class for the SCT subdetector that creates compact Identifier objects and
      IdentifierHash or hash IDs. Also allows decoding of these IDs. */
  const SCT_ID* m_sctID{nullptr};

  /** Boolean to determine if SCT Raw Data Provider should be run in RoI seeded mode. */
  BooleanProperty m_roiSeeded{this, "isRoI_Seeded", false, "Use RoI"};

  /** Boolean to Use DataPool with IDC online Cache */
  Gaudi::Property<bool> m_useDataPoolWithCache{
      this, "useDataPoolWithCache", false, "use DataPool With Cache"};

  /** Read handle for Trigger ROI descriptor collection. */
  SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey{this,
                                                                    "RoIs",
                                                                    "",
                                                                    "RoIs to read in"};

  /** Write handle for SCT RDO container. */
  SG::WriteHandleKey<SCT_RDO_Container> m_rdoContainerKey{this,
                                                          "RDOKey",
                                                          "SCT_RDOs",
                                                          "SCT RDO key"};

  /** Write handle for LVL 1 Inner Detector time collection. */
  SG::WriteHandleKey<InDetTimeCollection> m_lvl1CollectionKey{this,
                                                              "LVL1IDKey",
                                                              "SCT_LVL1ID",
                                                              "SCT LVL1ID key"};

  /** Write handle for BC ID Inner Detector time collection. */
  SG::WriteHandleKey<InDetTimeCollection> m_bcIDCollectionKey{this,
                                                              "BCIDKey",
                                                              "SCT_BCID",
                                                              "SCT BCID key"};

  /** Write handle for Inner Detector ByteStream error container. */  
  SG::WriteHandleKey<IDCInDetBSErrContainer> m_bsIDCErrContainerKey{this,
                                                                    "IDCByteStreamErrContainer",
                                                                    "SCT_ByteStreamErrs",
                                                                    "SCT BS error key for IDC variant"};


  /** Update handle for SCT RDO and Erorrs Cache. */
  SG::UpdateHandleKey<SCT_RDO_Cache> m_rdoContainerCacheKey;
  SG::UpdateHandleKey<IDCInDetBSErrContainer_Cache> m_bsErrContainerCacheKey;

};

#endif // SCT_RAWDATABYTESTREAMCNV_SCTRAWDATAPROVIDER_H
