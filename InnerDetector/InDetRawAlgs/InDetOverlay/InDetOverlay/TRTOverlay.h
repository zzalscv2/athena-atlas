/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETOVERLAY_TRTOVERLAY_H
#define INDETOVERLAY_TRTOVERLAY_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
//
#include "InDetRawData/TRT_RDO_Container.h"
#include "InDetRawData/TRT_LoLumRawData.h"
#include "InDetSimData/InDetSimDataCollection.h"
#include "TRT_ConditionsData/StrawStatusData.h"
//
#include "TRT_ElectronPidTools/ITRT_LocalOccupancy.h"
//
#include "AthenaKernel/IAthRNGSvc.h"
#include "AthAllocators/DataPool.h"
#include "GaudiKernel/ServiceHandle.h"


class TRT_ID;

namespace CLHEP {
class HepRandomEngine;
}

namespace OverlayTesting {
class TRTOverlay_test;
}

class TRTOverlay : public AthReentrantAlgorithm {
 public:
  friend class OverlayTesting::TRTOverlay_test;

  TRTOverlay(const std::string &name, ISvcLocator *pSvcLocator);

  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext &ctx) const override final;

 private:
  StatusCode overlayContainer(
      const EventContext &ctx, const TRT_RDO_Container *bkgContainer,
      const TRT_RDO_Container *signalContainer,
      TRT_RDO_Container *outputContainer,
      const InDetSimDataCollection *signalSDOCollection,
      DataPool<TRT_LoLumRawData>& dataItemsPool) const;

  void mergeCollections(TRT_RDO_Collection *bkgCollection,
                        TRT_RDO_Collection *signalCollection,
                        TRT_RDO_Collection *outputCollection, 
                        double occupancy,
                        const InDetSimDataCollection *signalSDOCollection,
                        const TRTCond::StrawStatusData *strawStatusHT,
                        CLHEP::HepRandomEngine *rndmEngine) const;

  const TRT_ID *m_trtId{};

  Gaudi::Property<bool> m_sortBkgInput{this, "SortBkgInput", false,
                                       "Sort background input RDOs"};

  SG::ReadHandleKey<TRT_RDO_Container> m_bkgInputKey{
      this, "BkgInputKey", "Bkg_TRT_RDOs",
      " ReadHandleKey for Background Input TRT_RDO_Container"};
  SG::ReadHandleKey<TRT_RDO_Container> m_signalInputKey{
      this, "SignalInputKey", "Sig_TRT_RDOs",
      "ReadHandleKey for Signal Input TRT_RDO_Container"};
  SG::WriteHandleKey<TRT_RDO_Container> m_outputKey{
      this, "OutputKey", "TRT_RDOs",
      "WriteHandleKey for Output TRT_RDO_Container"};
  SG::ReadHandleKey<InDetSimDataCollection> m_signalInputSDOKey{
      this, "SignalInputSDOKey", "Sig_TRT_SDO_Map",
      "ReadHandleKey for Signal Input InDetSimDataCollection for TRT"};
  SG::ReadCondHandleKey<TRTCond::StrawStatusData> m_strawStatusHTKey{
      this, "StrawStatusHT", "StrawStatusHTData", "StrawStatusHT key"};

  // Following tools, services and configurables are there only for the correct
  // treatment of HT hits
  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc",
                                      "Random Number Service"};

  Gaudi::Property<double> m_HTOccupancyCorrectionB{
      this, "TRT_HT_OccupancyCorrectionBarrel", 0.110, ""};
  Gaudi::Property<double> m_HTOccupancyCorrectionEC{
      this, "TRT_HT_OccupancyCorrectionEndcap", 0.090, ""};
  Gaudi::Property<double> m_HTOccupancyCorrectionB_noE{
      this, "TRT_HT_OccupancyCorrectionBarrelNoE", 0.060, ""};
  Gaudi::Property<double> m_HTOccupancyCorrectionEC_noE{
      this, "TRT_HT_OccupancyCorrectionEndcapNoE", 0.050, ""};
  Gaudi::Property<double> m_HTOccupancyCorrectionB_Ar{
      this, "TRT_HT_OccupancyCorrectionBarrelAr", 0.040, ""};
  Gaudi::Property<double> m_HTOccupancyCorrectionEC_Ar{
      this, "TRT_HT_OccupancyCorrectionEndcapAr", 0.041, ""};
  Gaudi::Property<double> m_HTOccupancyCorrectionB_Ar_noE{
      this, "TRT_HT_OccupancyCorrectionBarrelArNoE", 0.033, ""};
  Gaudi::Property<double> m_HTOccupancyCorrectionEC_Ar_noE{
      this, "TRT_HT_OccupancyCorrectionEndcapArNoE", 0.042, ""};
  ToolHandle<InDet::ITRT_LocalOccupancy> m_TRT_LocalOccupancyTool{
      this, "TRT_LocalOccupancyTool", "TRT_LocalOccupancy", ""};
};

#endif  // INDETOVERLAY_TRTOVERLAY_H
