/*                                                                                                                                                     
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration                                                                              
*/

#include "TrigT1NSWSimTools/TriggerProcessorTool.h"

namespace NSWL1 {

  TriggerProcessorTool::TriggerProcessorTool(const std::string& type, const std::string& name, const IInterface* parent) :
    AthAlgTool(type,name,parent) 
  {
  }

  StatusCode TriggerProcessorTool::initialize() {

    ATH_MSG_DEBUG("In initialize()");
    return StatusCode::SUCCESS;
  }

  StatusCode TriggerProcessorTool::mergeRDO(const Muon::NSW_PadTriggerDataContainer* padTriggerContainer,
                                            const Muon::NSW_TrigRawDataContainer* stripTriggerContainer,
                                            const Muon::NSW_TrigRawDataContainer* MMTriggerContainer,
                                            Muon::NSW_TrigRawDataContainer* trigRdoContainer) {

    ATH_MSG_DEBUG("Pad Trigger Container size: " << padTriggerContainer->size());
    for ( const auto &padTriggerData : *padTriggerContainer ) {
      ATH_MSG_DEBUG("Pad Trigger data: " << *padTriggerData);

      char sectorSide = (padTriggerData->endcap() == Muon::NSW_PadTriggerData::Endcap::A) ? 'A' : 'C';
      auto sectorID = (padTriggerData->sectorSize() == Muon::NSW_PadTriggerData::SectorSize::SMALL) ? padTriggerData->sectorID()*2-1 : padTriggerData->sectorID()*2-2;
      Muon::NSW_TrigRawData* trigRawData = new Muon::NSW_TrigRawData(sectorID, sectorSide, padTriggerData->BCID());
      for ( const Muon::NSW_PadTriggerSegment* padTriggerSegment : *padTriggerData) {
        ATH_MSG_DEBUG("Pad Trigger segment: " << *padTriggerSegment);

        Muon::NSW_TrigRawDataSegment* trigRawDataSegment = new Muon::NSW_TrigRawDataSegment();
        uint8_t bandID = padTriggerSegment->bandID();
        uint8_t phiID  = padTriggerSegment->phiID();
        trigRawDataSegment->setRIndex(bandID);
        trigRawDataSegment->setPhiIndex(phiID);

        trigRawDataSegment->setPhiRes(true); // set sTGC bit
        trigRawDataSegment->setSpare(1);     // set spare bit to 1, to distinguish between strips (default, i.e. 0) and pads
        trigRawData->push_back(trigRawDataSegment);
      }
      trigRdoContainer->push_back(trigRawData);
    }
    ATH_MSG_DEBUG("After PadTrigger filling -> NSW Trigger RDO size: " << trigRdoContainer->size());

    for (const auto &rawData : *stripTriggerContainer) {
      Muon::NSW_TrigRawData* trigRawData = new Muon::NSW_TrigRawData(*rawData, true);
      trigRdoContainer->push_back(trigRawData);
    }
    ATH_MSG_DEBUG("After sTGC strip trigger filling -> NSW Trigger RDO size: " << trigRdoContainer->size());

    for (const auto &rawData : *MMTriggerContainer) {
      Muon::NSW_TrigRawData* trigRawData = new Muon::NSW_TrigRawData(*rawData, false);
      trigRdoContainer->push_back(trigRawData);
    }
    ATH_MSG_DEBUG("After MMTrigger filling -> NSW Trigger RDO size: " << trigRdoContainer->size());

    return StatusCode::SUCCESS;
  }
}
