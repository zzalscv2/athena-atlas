#include "TrigT1NSWSimTools/PadTriggerAdapter.h"

namespace NSWL1 {
namespace PadTriggerAdapter {
Muon::NSW_PadTriggerSegment segment(const NSWL1::PadTrigger& data) {
    return Muon::NSW_PadTriggerSegment{
        static_cast<uint8_t>(data.bandId()),
        static_cast<uint8_t>(data.phiId()),
        {static_cast<uint8_t>(data.trgSelectedLayersInner().size()),
        static_cast<uint8_t>(data.trgSelectedLayersOuter().size())}
    };
}

// TODO should probably be a tool, and then we can retrieve event data from
// evtStore
StatusCode
fillContainer(const std::unique_ptr<Muon::NSW_PadTriggerDataContainer> &out,
              const std::vector<std::unique_ptr<NSWL1::PadTrigger>> &triggers,
              const uint32_t l1id) {
  using TriggerList = std::vector<const NSWL1::PadTrigger *>;
  using TriggerMap = std::map<uint32_t, TriggerList>;
  TriggerMap triggerMap;
  // Filter by sector:
  for (const auto &pt : triggers) {
    // `sector` in range [0, 16)
    auto sector = pt->triggerSectorNumber() - 1;
    auto endcap = pt->sideId();
    if (sector == -1 || endcap == -1) {
      return StatusCode::FAILURE;
    }
    // Calculate hash, range [0, 32)
    const uint32_t hash = 16 * endcap + sector;
    auto it = triggerMap.find(hash);
    if (it == triggerMap.end()) {
        it = triggerMap.insert(std::make_pair(hash, TriggerList())).first;
    }
    it->second.push_back(pt.get());
  }
  for (const auto &item : triggerMap) {
    const TriggerList &triggerList = item.second;
    const auto pt = triggerList[0];
    const bool sideA = static_cast<bool>(pt->sideId());
    auto newCollection = new Muon::NSW_PadTriggerData(sideA, pt->triggerSectorNumber() - 1, pt->bctag(), l1id);
    for (const auto& pt: triggerList) {
      newCollection->addTrigger(pt->bandId(), pt->phiId(), 0);
    }
    if (out->addCollection(newCollection, out->numberOfCollections()).isFailure()) {
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}
} // namespace PadTriggerAdapter
} // namespace NSWL1
