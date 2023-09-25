/* -*- C++ -*- */
/*
 * Copyright (C) 2023 CERN for the benefit of the ATLAS collaboration.
 */
#ifndef PILEUPMT_ISKIPEVENTIDXSVC_H
#define PILEUPMT_ISKIPEVENTIDXSVC_H

#include <GaudiKernel/IService.h>
#include <fmt/format.h>

#include <cstdint>

class ISkipEventIdxSvc : virtual public IService {
 public:
  DeclareInterfaceID(ISkipEventIdxSvc, 1, 0);
  struct EvtId {
    std::uint32_t runNum = 0;
    std::uint32_t lbNum = 0;
    std::uint64_t evtNum = 0;
    std::uint64_t evtIdx = 0;
  };

  using EvtIter = std::vector<EvtId>::const_iterator;

  template <
      typename Fn,
      std::enable_if_t<std::is_invocable_r_v<StatusCode, Fn, EvtIter, EvtIter>>>
  StatusCode registerCallback(Fn&& callback) {
    return this->registerCallback(
        std::function<StatusCode(EvtIter, EvtIter)>(callback));
  }
  virtual StatusCode registerCallback(
      std::function<StatusCode(EvtIter, EvtIter)>&& callback) = 0;
};

template <>
struct fmt::formatter<ISkipEventIdxSvc::EvtId> : formatter<string_view> {
  template <typename FormatContext>
  auto format(const ISkipEventIdxSvc::EvtId& evtId, FormatContext& ctx) {
    auto out = memory_buffer();
    format_to(std::back_inserter(out), "[Run: {}, LB: {}, Evt: {} ({})]",
              evtId.runNum, evtId.lbNum, evtId.evtNum, evtId.evtIdx);
    const string_view str(out.data(), out.size());
    return formatter<string_view>::format(str, ctx);
  }
};
#endif  // PILEUPMT_ISKIPEVENTIDXSVC_H
