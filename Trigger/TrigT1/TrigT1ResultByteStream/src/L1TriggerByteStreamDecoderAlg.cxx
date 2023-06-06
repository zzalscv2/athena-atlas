/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "L1TriggerByteStreamDecoderAlg.h"
#include "eformat/DetectorMask.h"
#include "eformat/SourceIdentifier.h"
#include "eformat/Problem.h"
#include <algorithm>
#include <charconv>

namespace {
  std::string u32toHexString(uint32_t num) {
    std::string hexRobId("0x        ");
    std::to_chars(hexRobId.data()+2, hexRobId.data()+hexRobId.size(), num, 16);
    hexRobId.resize(std::min(10ul,hexRobId.find(' ')));
    return hexRobId;
  }
  /// RAII fraction vs LBN monitoring helper
  struct FractionPerLBNMonitor {
    FractionPerLBNMonitor(std::string&& name, EventIDBase::number_type lbn, float denom, const ToolHandle<GenericMonitoringTool>& monTool)
    : errorFraction(Monitored::Scalar<float>{std::move(name), 0}),
      errorFractionLBN(Monitored::Scalar<EventIDBase::number_type>{"LumiBlock", lbn}),
      denominator(denom),
      tool(monTool) {}
    ~FractionPerLBNMonitor() {
      if (denominator==0) {
        errorFraction = 0;
      } else {
        errorFraction /= denominator;
      }
      Monitored::Group{tool, errorFraction, errorFractionLBN};
    }
    void operator++() {++errorFraction;}
    Monitored::Scalar<float> errorFraction;
    Monitored::Scalar<EventIDBase::number_type> errorFractionLBN;
    float denominator{1};
    const ToolHandle<GenericMonitoringTool>& tool;
  };
  /// Get ROB problems vector and print string representation to a stream
  void printRobProblems(std::ostream& s, const IROBDataProviderSvc::ROBF* rob) {
      std::vector<eformat::FragmentProblem> problems;
      rob->problems(problems);
      s << problems.size() << " problems: [";
      bool firstProblem{true};
      for (eformat::FragmentProblem problem : problems) {
        if (firstProblem) {firstProblem=false;}
        else {s << ", ";}
        s << eformat::helper::FragmentProblemDictionary.string(problem);
      }
      s << "]";
  }
  std::string robIdsToString(const std::vector<uint32_t>& robIds) {
    std::ostringstream ss;
    bool first{true};
    ss << "[";
    for (const uint32_t robId : robIds) {
      if (first) {first=false;}
      else {ss << ", ";}
      ss << "0x" << std::hex << robId << std::dec;
    }
    ss << "]";
    return ss.str();
  }
}

// =============================================================================
// Standard constructor
// =============================================================================
L1TriggerByteStreamDecoderAlg::L1TriggerByteStreamDecoderAlg(const std::string& name, ISvcLocator* svcLoc)
: AthReentrantAlgorithm(name, svcLoc) {}

// =============================================================================
// Implementation of AthReentrantAlgorithm::initialize
// =============================================================================
StatusCode L1TriggerByteStreamDecoderAlg::initialize() {
  ATH_MSG_DEBUG("Initialising " << name());
  ATH_CHECK(m_decoderTools.retrieve());
  ATH_CHECK(m_robDataProviderSvc.retrieve());
  if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());
  ATH_CHECK(m_bsMetaDataContRHKey.initialize(SG::AllowEmpty));
  
  renounce(m_bsMetaDataContRHKey);

  // Build a list of unique ROB IDs to request in each event
  for (const auto& decoderTool : m_decoderTools) {
    const std::vector<uint32_t>& ids = decoderTool->robIds();
    m_robIds.insert(m_robIds.end(), ids.begin(), ids.end());
  }
  std::sort(m_robIds.begin(),m_robIds.end());
  m_robIds.erase(std::unique(m_robIds.begin(),m_robIds.end()), m_robIds.end());
  ATH_MSG_INFO("Will request " << m_robIds.size() << " ROBs per event: " << robIdsToString(m_robIds));

  // Parse properties
  m_maybeMissingRobs.insert(m_maybeMissingRobsProp.value().begin(), m_maybeMissingRobsProp.value().end());

  auto setRobCheckBehaviour = [this](const Gaudi::Property<std::string>& prop, ROBCheckBehaviour& out) -> StatusCode {
    if (prop.value() == "None") {out=ROBCheckBehaviour::None;}
    else if (prop.value() == "Warning") {out=ROBCheckBehaviour::Warning;}
    else if (prop.value() == "Error") {out=ROBCheckBehaviour::Error;}
    else if (prop.value() == "Fatal") {out=ROBCheckBehaviour::Fatal;}
    else {
      ATH_MSG_ERROR(prop.name() << " is set to \"" << prop.value()
                    << "\" but needs to be one of [\"None\", \"Warning\", \"Error\", \"Fatal\"]");
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  };

  ATH_CHECK(setRobCheckBehaviour(m_robStatusCheckLevel,m_robStatusCheck));
  ATH_CHECK(setRobCheckBehaviour(m_robFormatCheckLevel,m_robFormatCheck));

  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of AthReentrantAlgorithm::start
// =============================================================================
StatusCode L1TriggerByteStreamDecoderAlg::start() {
  // Skip detector mask checks if ByteStreamMetaData not available
  if (m_bsMetaDataContRHKey.key().empty()) {
    return StatusCode::SUCCESS;
  }

  // Get the detector mask
  auto bsmdc = SG::makeHandle(m_bsMetaDataContRHKey);
  ATH_CHECK(bsmdc.isValid());
  ATH_CHECK(!bsmdc->empty());
  const ByteStreamMetadata* metadata = bsmdc->front();
  const eformat::helper::DetectorMask detectorMask{metadata->getDetectorMask(), metadata->getDetectorMask2()};

  // Check if the requested ROBs should be expected to be present or may be missing
  for (const uint32_t robId : m_robIds) {
    const eformat::helper::SourceIdentifier sid{robId};
    if (!detectorMask.is_set(sid.subdetector_id())) {
      if (m_maybeMissingRobs.insert(robId).second) {
        ATH_MSG_WARNING("ROB ID 0x" << MSG::hex << robId << MSG::dec << " was requested for decoding, "
                        << "but the SubDetector " << sid.human_detector() << " is disabled in "
                        << "the detector mask. Will not require this ROB ID to be present in every event.");
      }
    }
  }

  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of AthReentrantAlgorithm::finalize
// =============================================================================
StatusCode L1TriggerByteStreamDecoderAlg::finalize() {
  ATH_MSG_DEBUG("Finalising " << name());
  ATH_CHECK(m_robDataProviderSvc.release());
  ATH_CHECK(m_decoderTools.release());
  return StatusCode::SUCCESS;
}

// =============================================================================
// Implementation of AthReentrantAlgorithm::execute
// =============================================================================
StatusCode L1TriggerByteStreamDecoderAlg::execute(const EventContext& eventContext) const {
  ATH_MSG_DEBUG("Executing " << name());
  auto monTimeExec = Monitored::Timer<std::chrono::duration<float, std::milli>>("TIME_execute");

  // Retrieve the BS data for all tools in one request to parallelise DCM->ROS network requests
  IROBDataProviderSvc::VROBFRAG vrobf;
  m_robDataProviderSvc->getROBData(eventContext, m_robIds, vrobf, name());

  // Filter ROB list for each tool and call the conversion
  for (const auto& decoderTool : m_decoderTools) {
    std::string toolName{decoderTool->name().substr(decoderTool->name().rfind('.')+1)};
    auto monTimePrep = Monitored::Timer<std::chrono::duration<float, std::milli>>("TIME_prepareROBs_"+toolName);
    IROBDataProviderSvc::VROBFRAG vrobfForTool;
    ATH_CHECK(filterRobs(vrobf, vrobfForTool, decoderTool->robIds(), toolName, eventContext));
    ATH_CHECK(checkRobs(vrobfForTool, toolName, eventContext));
    monTimePrep.stop();
    auto monTimeConv = Monitored::Timer<std::chrono::duration<float, std::milli>>("TIME_convert_"+toolName);
    try {
      ATH_CHECK(decoderTool->convertFromBS(vrobfForTool, eventContext));
    } catch (const std::exception& ex) {
      ATH_MSG_ERROR("Exception in " << toolName << "::convertFromBS: " << ex.what());
      return StatusCode::FAILURE;
    }
    // Note: time histograms not filled if any ATH_CHECK above fails
    Monitored::Group(m_monTool, monTimePrep, monTimeConv);
  }

  Monitored::Group(m_monTool, monTimeExec);
  return StatusCode::SUCCESS;
}

// =============================================================================
StatusCode L1TriggerByteStreamDecoderAlg::filterRobs(const IROBDataProviderSvc::VROBFRAG& in,
                                                     IROBDataProviderSvc::VROBFRAG& out,
                                                     const std::vector<uint32_t>& ids,
                                                     std::string_view toolName,
                                                     const EventContext& eventContext) const {
  std::copy_if(in.cbegin(), in.cend(), std::back_inserter(out),
               [&ids](const IROBDataProviderSvc::ROBF* rob){
                 return (std::find(ids.cbegin(),ids.cend(),rob->source_id()) != ids.cend());
               });

  std::string missingFractionName{"MissingROBFraction_"};
  missingFractionName.append(toolName);
  FractionPerLBNMonitor monMissingFraction{std::move(missingFractionName), eventContext.eventID().lumi_block(), static_cast<float>(ids.size()), m_monTool};

  // Check if all ROBs were found and report errors if not
  if (ids.size() != out.size()) {
    bool fail{false};
    for (const uint32_t id : ids) {
      auto eqRobId = [&id](const IROBDataProviderSvc::ROBF* rob){return (rob->source_id() == id);};
      if (std::find_if(out.cbegin(),out.cend(),eqRobId) == out.cend()) {
        ++monMissingFraction;
        std::string hexRobId = u32toHexString(id);
        if (std::find(m_maybeMissingRobs.cbegin(),m_maybeMissingRobs.cend(),id) != m_maybeMissingRobs.cend()) {
          ATH_MSG_DEBUG("Missing ROBFragment with ID " << hexRobId << " ("
                        << eformat::helper::SourceIdentifier(id).human() << ") requested by " << toolName
                        << " but it is allowed to be missing due to the detector mask or the "
                        << m_maybeMissingRobsProp.name() << " property");
          Monitored::Scalar<std::string> monMissing{"MissingROBAllowed", hexRobId};
          Monitored::Group{m_monTool, monMissing};
        } else {
          ATH_MSG_ERROR("Missing ROBFragment with ID " << hexRobId << " ("
                        << eformat::helper::SourceIdentifier(id).human() << ") requested by " << toolName);
          fail = true;
          Monitored::Scalar<std::string> monMissing{"MissingROB", hexRobId};
          Monitored::Group{m_monTool, monMissing};
        }
      }
    }
    if (fail) {
      return StatusCode::FAILURE;
    }
  }
  ATH_MSG_DEBUG("Found " << out.size() << " out of " << ids.size() << " ROBFragments requested by " << toolName);
  return StatusCode::SUCCESS;
}

// =============================================================================
StatusCode L1TriggerByteStreamDecoderAlg::checkRobs(const IROBDataProviderSvc::VROBFRAG& robs,
                                                    std::string_view toolName,
                                                    const EventContext& eventContext) const {
  StatusCode sc{StatusCode::SUCCESS};
  if (m_robStatusCheck == ROBCheckBehaviour::None && m_robFormatCheck == ROBCheckBehaviour::None) {
    return sc;
  }
  std::string corruptedFractionName{"CorruptedROBFraction_"};
  std::string errorFractionName{"ErroneousROBFraction_"};
  corruptedFractionName.append(toolName);
  errorFractionName.append(toolName);
  FractionPerLBNMonitor monCorruptedFraction{std::move(corruptedFractionName), eventContext.eventID().lumi_block(), static_cast<float>(robs.size()), m_monTool};
  FractionPerLBNMonitor monErrorFraction{std::move(errorFractionName), eventContext.eventID().lumi_block(), static_cast<float>(robs.size()), m_monTool};
  auto printCheckMsg = [this](std::ostringstream& ss, ROBCheckBehaviour checkBehaviour, bool maybeMissing, StatusCode& sc) {
      if (maybeMissing) {
        ss << " but it is allowed due to the detector mask or the " << m_maybeMissingRobsProp.name() << " property";
        ATH_MSG_DEBUG(ss.str());
      }
      else if (checkBehaviour==ROBCheckBehaviour::Warning) {ATH_MSG_WARNING(ss.str());}
      else if (checkBehaviour==ROBCheckBehaviour::Error) {ATH_MSG_ERROR(ss.str());}
      else if (checkBehaviour==ROBCheckBehaviour::Fatal) {
        ATH_MSG_ERROR(ss.str());
        sc = StatusCode::FAILURE;
      }
  };

  for (const IROBDataProviderSvc::ROBF* rob : robs) {
    // Format check
    if (m_robFormatCheck!=ROBCheckBehaviour::None && !rob->check_noex()) {
      ++monCorruptedFraction;
      const uint32_t id{rob->rob_source_id()}; // there's a small chance ID is also corrupted, but usually corruption affects latter words
      std::string hexRobId = u32toHexString(id);
      const bool maybeMissing{std::find(m_maybeMissingRobs.cbegin(),m_maybeMissingRobs.cend(),id) != m_maybeMissingRobs.cend()};

      Monitored::Scalar<std::string> monCorrupted{(maybeMissing ? "CorruptedROBAllowed" : "CorruptedROB"), hexRobId};
      Monitored::Group{m_monTool, monCorrupted};

      std::ostringstream ss;
      ss << "ROBFragment with ID " << hexRobId << " (" << eformat::helper::SourceIdentifier(id).human()
         << ") requested by " << toolName << " has corrupted data with ";
      printRobProblems(ss, rob);
      printCheckMsg(ss, m_robFormatCheck, maybeMissing, sc);
    }
    // Status check
    if (m_robStatusCheck!=ROBCheckBehaviour::None && rob->nstatus()>0 && rob->status()[0]!=0) {
      ++monErrorFraction;
      const uint32_t id{rob->rob_source_id()};
      std::string hexRobId = u32toHexString(id);
      const bool maybeMissing{std::find(m_maybeMissingRobs.cbegin(),m_maybeMissingRobs.cend(),id) != m_maybeMissingRobs.cend()};

      Monitored::Scalar<std::string> monErroneous{(maybeMissing ? "ErroneousROBAllowed" : "ErroneousROB"), hexRobId};
      Monitored::Group{m_monTool, monErroneous};

      std::ostringstream ss;
      ss << "ROBFragment with ID " << hexRobId << " (" << eformat::helper::SourceIdentifier(id).human()
         << ") requested by " << toolName << " has non-zero status 0x" << std::hex << rob->status()[0] << std::dec;
      printCheckMsg(ss, m_robStatusCheck, maybeMissing, sc);
    }
  }
  return sc;
}
