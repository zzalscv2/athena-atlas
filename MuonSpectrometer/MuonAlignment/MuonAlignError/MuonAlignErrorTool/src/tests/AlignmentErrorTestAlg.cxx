#include "AlignmentErrorTestAlg.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "MuonAlignErrorBase/AlignmentRotationDeviation.h"
#include "MuonAlignErrorBase/AlignmentTranslationDeviation.h"
#include "TrkToolInterfaces/ITrkAlignmentDeviationTool.h"

#include <fmt/format.h>

namespace MuonAlign {

AlignmentErrorTestAlg::AlignmentErrorTestAlg(const std::string& name,
                                             ISvcLocator* pSvcLocator)
    : AthAlgorithm(name, pSvcLocator) {}

StatusCode AlignmentErrorTestAlg::initialize() {
  ATH_CHECK(m_alignmentErrorTool.retrieve());
  ATH_CHECK(m_trackCollection.initialize());
  return StatusCode::SUCCESS;
}

StatusCode AlignmentErrorTestAlg::execute() {
  const EventContext& ctx = getContext();
  SG::ReadHandle<::TrackCollection> trackCollection{m_trackCollection, ctx};
  if (!trackCollection.isValid()) {
    throw std::runtime_error("Cannot retrieve track collection");
  }

  ATH_MSG_INFO("Retrieved track collection with " << trackCollection->size()
                                                  << " entries");
  int itrack = 0;
  for (const Trk::Track* track : *trackCollection) {
    ATH_MSG_INFO("Track " << itrack++);
    std::vector<Trk::AlignmentDeviation*> deviations;
    m_alignmentErrorTool->makeAlignmentDeviations(*track, deviations);
    for (const Trk::AlignmentDeviation* np : deviations) {
      if (const auto* tdev = dynamic_cast<const MuonAlign::AlignmentTranslationDeviation*>(np)) {
        Amg::Vector3D u = tdev->getU();
        double sigma = tdev->getSigma();
        ATH_MSG_INFO(fmt::format("TranslationNP U = {:14.6f} {:14.6f} {:14.6f} sigma = {:8.6f}", u.x(), u.y(), u.z(), sigma));
      } else if (const auto* rdev = dynamic_cast<const MuonAlign::AlignmentRotationDeviation*>(np)) {
        Amg::Vector3D c = rdev->getCenter();
        Amg::Vector3D a = rdev->getAxis();
        double sigma = rdev->getSigma();
        ATH_MSG_INFO(fmt::format("RotationNP    C = {:14.6f} {:14.6f} {:14.6f} A = {:14.6f} {:14.6f} {:14.6f} sigma = {:11.9f}",
                                 c.x(), c.y(), c.z(), a.x(), a.y(), a.z(), sigma));
      } else {
        throw std::runtime_error("Deviation type is not implemented");
      }
    }
  }

  return StatusCode::SUCCESS;
}

}  // namespace MuonAlign
