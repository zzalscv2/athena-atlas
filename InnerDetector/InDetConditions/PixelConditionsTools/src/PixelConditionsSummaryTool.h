/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PixelConditionsTools/PixelConditionsSummaryTool.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date November, 2019
 * @brief Return pixel module/FE status.
 */

#ifndef PIXELCONDITIONSSERVICES_PIXELCONDITIONSSUMMARYTOOL_H
#define PIXELCONDITIONSSERVICES_PIXELCONDITIONSSUMMARYTOOL_H



#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetConditionsSummaryService/IDetectorElementStatusTool.h"
#include "InDetConditionsSummaryService/IInDetConditionsTool.h"

#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"

#include "AthenaKernel/SlotSpecificObj.h"

#include "InDetByteStreamErrors/IDCInDetBSErrContainer.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "InDetIdentifier/PixelID.h"

#include "PixelConditionsData/PixelDeadMapCondData.h"
#include "PixelConditionsData/PixelDCSStateData.h"
#include "PixelConditionsData/PixelDCSStatusData.h"
#include "PixelConditionsData/PixelTDAQData.h"

#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetReadoutGeometry/SiDetectorElementStatus.h"

#include <string>
#include <mutex>
#include <tuple>
#include <memory>
#include <cstdint>//uint64_t
#include <vector>
#include <bitset>
#include <limits>

class PixelConditionsSummaryTool: public AthAlgTool, virtual public IDetectorElementStatusTool,virtual public IInDetConditionsTool {
  public:
    static InterfaceID& interfaceID();

    PixelConditionsSummaryTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~PixelConditionsSummaryTool();
    virtual StatusCode initialize() override;

    virtual bool isActive(const Identifier& elementId, const InDetConditions::Hierarchy h, const EventContext& ctx) const override final;
    virtual bool isActive(const IdentifierHash& moduleHash, const EventContext& ctx) const override final;
    virtual bool isActive(const IdentifierHash& moduleHash, const Identifier& elementId, const EventContext& ctx)  const override final;
    virtual double activeFraction(const IdentifierHash& moduleHash, const Identifier & idStart, const Identifier & idEnd, const EventContext& ctx)  const override final;

    virtual bool isGood(const Identifier& elementId, const InDetConditions::Hierarchy h, const EventContext& ctx) const override final;
    virtual bool isGood(const IdentifierHash& moduleHash, const EventContext& ctx) const override final;
    virtual bool isGood(const IdentifierHash& moduleHash, const Identifier& elementId, const EventContext& ctx) const override final;
    virtual double goodFraction(const IdentifierHash & moduleHash, const Identifier & idStart, const Identifier & idEnd, const EventContext& ctx) const override final;

    virtual std::unique_ptr<InDet::SiDetectorElementStatus>
    getDetectorElementStatus(const EventContext& ctx,
                             SG::WriteCondHandle<InDet::SiDetectorElementStatus>* whandle) const override;

    virtual bool hasBSError(const IdentifierHash& moduleHash, const EventContext& ctx) const override final;
    virtual bool hasBSError(const IdentifierHash& moduleHash, Identifier pixid, const EventContext& ctx) const override final;
    virtual uint64_t getBSErrorWord(const IdentifierHash& moduleHash, const EventContext& ctx) const override final;
    virtual uint64_t getBSErrorWord(const IdentifierHash& moduleHash, const int index, const EventContext& ctx) const override final;

    bool checkChipStatus(IdentifierHash moduleHash, Identifier pixid, const EventContext& ctx) const;

  private:
    const PixelID* m_pixelID{};

    std::vector<std::string> m_isActiveStatus;
    std::vector<std::string> m_isActiveStates;
    std::vector<int> m_activeState;
    std::vector<int> m_activeStatus;
    unsigned int m_activeStateMask{};  ///< mask in which each state is represented by a bit and for states which are cnsidered active the corresponding bit is set;
    unsigned int m_activeStatusMask{}; ///< mask in which each status is represented by a bit and for status values which are cnsidered active the corresponding bit is set;

    Gaudi::Property<bool> m_useByteStreamFEI4
    {this, "UseByteStreamFEI4", false, "Switch of the ByteStream error for FEI4"};

    Gaudi::Property<bool> m_useByteStreamFEI3
    {this, "UseByteStreamFEI3", false, "Switch of the ByteStream error for FEI3"};

    Gaudi::Property<bool> m_useByteStreamRD53
    {this, "UseByteStreamRD53", false, "Switch of the ByteStream error for RD53"};

    SG::ReadCondHandleKey<PixelDCSStateData> m_condDCSStateKey
    {this, "PixelDCSStateCondData", "PixelDCSStateCondData", "Pixel FSM state key"};

    SG::ReadCondHandleKey<PixelDCSStatusData> m_condDCSStatusKey
    {this, "PixelDCSStatusCondData", "PixelDCSStatusCondData", "Pixel FSM status key"};

    SG::ReadCondHandleKey<PixelTDAQData> m_condTDAQKey
    {this, "PixelTDAQCondData", "", "Pixel TDAQ conditions key"}; //Default empty - legacy option

    SG::ReadCondHandleKey<PixelDeadMapCondData> m_condDeadMapKey
    {this, "PixelDeadMapCondData", "PixelDeadMapCondData", "Pixel deadmap conditions key"};

    ServiceHandle<InDetDD::IPixelReadoutManager> m_pixelReadout
    {this, "PixelReadoutManager", "PixelReadoutManager", "Pixel readout manager" };

    SG::ReadHandleKey<IDCInDetBSErrContainer>  m_BSErrContReadKey
    {this, "PixelByteStreamErrs", "PixelByteStreamErrs", "PixelByteStreamErrs container key"};

    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey
    {this, "PixelDetEleCollKey", "PixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};

    SG::ReadHandleKey<InDet::SiDetectorElementStatus> m_pixelDetElStatusEventKey
       {this, "PixelDetElStatusEventDataBaseKey", "", "Optional event data key of an input SiDetectorElementStatus on which the newly created object will be based."};
    SG::ReadCondHandleKey<InDet::SiDetectorElementStatus> m_pixelDetElStatusCondKey
       {this, "PixelDetElStatusCondDataBaseKey", "" , "Optional conditions data key of an input SiDetectorElementStatus on which the newly created object will be based."};

    Gaudi::Property< bool> m_activeOnly
       {this, "ActiveOnly", false, "Module and chip status will only reflect whether the modules or chips are active not necessarily whether the signals are good."};

    const uint64_t m_missingErrorInfo{std::numeric_limits<uint64_t>::max()-3000000000};

    mutable SG::SlotSpecificObj<std::mutex> m_cacheMutex ATLAS_THREAD_SAFE;

    struct IDCCacheEntry {
      EventContext::ContextEvt_t eventId = EventContext::INVALID_CONTEXT_EVT; // invalid event ID for the start
      const IDCInDetBSErrContainer_Cache* IDCCache = nullptr;

      void reset( EventContext::ContextEvt_t evtId, const IDCInDetBSErrContainer_Cache* cache) {
        eventId = evtId;
        IDCCache   = cache;
      }

      bool needsUpdate( const EventContext& ctx) const {
        return eventId != ctx.evt() or eventId == EventContext::INVALID_CONTEXT_EVT;
      }

    };
    mutable SG::SlotSpecificObj<IDCCacheEntry> m_eventCache ATLAS_THREAD_SAFE; // Guarded by m_cacheMutex

    /**
     * Obtains container form the SG, if it is missing it will complain (hard-coded 3 times per job) and return nullptr
     **/
    [[nodiscard]] const IDCInDetBSErrContainer* getContainer(const EventContext& ctx) const;

    /**
     * Return cache for the current event
     * If, for current slot, the cache is outdated it is retrieved from the IDC collection.
     * If the IDC is missing nullptr is returned.
     **/
    [[nodiscard]] IDCCacheEntry* getCacheEntry(const EventContext& ctx) const;

   /** Create a new detector element status element container.
    * Depending on the properties the container may be a copy of an event data or conditions data element status container.
    */
    std::unique_ptr<InDet::SiDetectorElementStatus>
    createDetectorElementStatus(const EventContext& ctx,
                                SG::WriteCondHandle<InDet::SiDetectorElementStatus>* whandle) const;

};

inline InterfaceID& PixelConditionsSummaryTool::interfaceID(){
  static InterfaceID IID_PixelConditionsSummaryTool("PixelConditionsSummaryTool", 1, 0);
  return IID_PixelConditionsSummaryTool;
}

inline bool PixelConditionsSummaryTool::checkChipStatus(IdentifierHash moduleHash, Identifier pixid, const EventContext& ctx) const {
  std::bitset<16> chipStatus(SG::ReadCondHandle<PixelDeadMapCondData>(m_condDeadMapKey, ctx)->getChipStatus(moduleHash));
  if (chipStatus.any()) {
    Identifier moduleID = m_pixelID->wafer_id(pixid);
    std::bitset<16> circ; 
    circ.set(m_pixelReadout->getFE(pixid,moduleID));
    if ((chipStatus&circ).any()) { return false; }
  }
  return true;
}

#endif
