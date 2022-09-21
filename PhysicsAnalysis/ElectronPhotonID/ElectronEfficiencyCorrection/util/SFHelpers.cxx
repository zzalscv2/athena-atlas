/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "SFHelpers.h"
#include "ElectronEfficiencyCorrection/AsgElectronEfficiencyCorrectionTool.h"
#include "PATInterfaces/MakeSystematicsVector.h"
#include "PATInterfaces/SystematicsUtil.h"
#include <cmath>
#include <xAODEgamma/Electron.h>

#include "AsgMessaging/MessageCheck.h"
#include "AsgMessaging/MsgStream.h"

namespace asg {
ANA_MSG_HEADER(SFHelper)
ANA_MSG_SOURCE(SFHelper, "")
}

int
SFHelpers::result(AsgElectronEfficiencyCorrectionTool& tool,
                  const xAOD::Electron& el,
                  double& nominalSF,
                  double& totalPos,
                  double& totalNeg,
                  const bool isToys)
{
  using namespace asg::SFHelper;
  ANA_CHECK_SET_TYPE(int);
  setMsgLevel(MSG::INFO);

  CP::SystematicSet systs = tool.recommendedSystematics();
  if (!isToys) {
    /*
     * Split the variation in up and down
     * Do it before any loop
     * This is obviously more of a sanity check
     * as they are currently symmetric so is mainly
     * about inspecting them both
     */
    std::vector<CP::SystematicVariation> positiveVar{};
    std::vector<CP::SystematicVariation> negativeVar{};
    for (const auto& sys : systs) {
      float param = sys.parameter();
      if (param < 0) {
        negativeVar.push_back(sys);
      } else {
        positiveVar.push_back(sys);
      }
    }
    // Helper function as a lamda
    auto totalSyst =
      [&tool](const xAOD::Electron& el,
              const std::vector<CP::SystematicVariation>& variations,
              const double nominal) {
        double total2{};
        double systematic{};
        for (const auto& sys : variations) {
          if (tool.applySystematicVariation({ sys }) != StatusCode::SUCCESS ||
              tool.getEfficiencyScaleFactor(el, systematic) !=
                CP::CorrectionCode::Ok) {
            ANA_MSG_ERROR("Error in setting/getting " << sys.name());
            return -999.0;
          }
          total2 += (nominal - systematic) * (nominal - systematic);
        }
        return  std::sqrt(total2);
      };

    // Do the work
    // Empty variation is the nominal
    ANA_CHECK(tool.applySystematicVariation({}));
    ANA_CHECK(tool.getEfficiencyScaleFactor(el, nominalSF) ==
              CP::CorrectionCode::Ok);
    totalNeg = totalSyst(el, negativeVar, nominalSF);
    totalPos = totalSyst(el, positiveVar, nominalSF);
  } else {
    CP::MakeSystematicsVector sysVec;
    sysVec.addGroup("toys");
    sysVec.setToys(tool.getNumberOfToys());
    sysVec.calc(systs);
    std::vector<CP::SystematicSet> toys = sysVec.result("toys");
    std::vector<double> toysVal{};
    toysVal.reserve(toys.size());

    // Do the work
    for (const auto& sys : toys) {
      double systematic{};
      ANA_CHECK(tool.applySystematicVariation(sys) == StatusCode::SUCCESS &&
                tool.getEfficiencyScaleFactor(el, systematic) ==
                  CP::CorrectionCode::Ok);
      ANA_MSG_DEBUG(tool.appliedSystematics().name()
                    << " toy Result : " << systematic);
      toysVal.push_back(systematic);
    }
    /*
     *  B. P. Welford 1962
     *  Donald KnutArt of Computer Programming, Vol 2, page
     *  232, 3rd edition
     */
    // 1st element,initilize
    double meanK{ toysVal[0] };   // current mean
    double meanK_1{ toysVal[0] }; // next mean
    double s{ 0 };
    size_t k{ 1 };
    const size_t N = toysVal.size();
    // subsequent ones
    for (size_t i = 1; i != N; ++i) {
      const double x{ toysVal[i] };
      const double invk{ (1.0 / (++k)) };
      meanK_1 = meanK + (x - meanK) * invk;
      s += (x - meanK_1) * (x - meanK);
      meanK = meanK_1;
    }
    const double variance = s / (N - 1);
    nominalSF = meanK;
    totalNeg = sqrt(variance);
    totalPos = sqrt(variance);
  }
  return 0;
}
