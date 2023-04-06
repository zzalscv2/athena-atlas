/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "FlavorTagDiscriminants/customGetter.h"
#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"

#include "xAODJet/JetContainer.h"
#include "xAODTracking/TrackParticle.h"

#include <optional>

namespace {
  // ______________________________________________________________________
  // Custom getters for jet-wise quantities
  //
  // this function is not at all optimized, but then it doesn't have
  // to be since it should only be called in the initialization stage.
  //
  std::function<double(const xAOD::Jet&)> customGetter(
    const std::string& name)
  {
    if (name == "pt") {
      return [](const xAOD::Jet& j) -> float {return j.pt();};
    }
    if (name == "log_pt") {
      return [](const xAOD::Jet& j) -> float {return std::log(j.pt());};
    }
    if (name == "eta") {
      return [](const xAOD::Jet& j) -> float {return j.eta();};
    }
    if (name == "abs_eta") {
      return [](const xAOD::Jet& j) -> float {return std::abs(j.eta());};
    }
    if (name == "energy") {
      return [](const xAOD::Jet& j) -> float {return j.e();};
    }
    if (name == "mass") {
      return [](const xAOD::Jet& j) -> float {return j.m();};
    }

    throw std::logic_error("no match for custom getter " + name);
  }


  // _______________________________________________________________________
  // Custom getters for track variables

  template <typename T>
  class TJGetter
  {
  private:
    T m_getter;
  public:
    TJGetter(T getter):
      m_getter(getter)
      {}
    std::vector<double> operator()(
      const xAOD::Jet& jet,
      const std::vector<const xAOD::TrackParticle*>& tracks) const {
      std::vector<double> sequence;
      sequence.reserve(tracks.size());
      for (const auto* track: tracks) {
        sequence.push_back(m_getter(*track, jet));
      }
      return sequence;
    }
  };

  std::optional<FlavorTagDiscriminants::SequenceFromTracks>
  sequenceWithIpDep(
    const std::string& name,
    const std::string& prefix)
  {

    using Tp = xAOD::TrackParticle;
    using Jet = xAOD::Jet;

    BTagTrackIpAccessor a(prefix);
    if (name == "IP3D_signed_d0_significance") {
      return TJGetter([a](const Tp& t, const Jet& j){
          return a.getSignedIp(t, j).ip3d_signed_d0_significance;
      });
    }
    if (name == "IP3D_signed_z0_significance") {
      return TJGetter([a](const Tp& t, const Jet& j){
        return a.getSignedIp(t, j).ip3d_signed_z0_significance;
      });
    }
    if (name == "IP2D_signed_d0") {
      return TJGetter([a](const Tp& t, const Jet& j){
        return a.getSignedIp(t, j).ip2d_signed_d0;
      });
    }
    if (name == "IP3D_signed_d0") {
      return TJGetter([a](const Tp& t, const Jet& j){
        return a.getSignedIp(t, j).ip3d_signed_d0;
      });
    }
    if (name == "IP3D_signed_z0") {
      return TJGetter([a](const Tp& t, const Jet& j){
        return a.getSignedIp(t, j).ip3d_signed_z0;
      });
    }
    if (name == "d0" || name == "btagIp_d0") {
      return TJGetter([a](const Tp& t, const Jet&){
        return a.d0(t);
      });
    }
    if (name == "z0SinTheta" || name == "btagIp_z0SinTheta") {
      return TJGetter([a](const Tp& t, const Jet&){
        return a.z0SinTheta(t);
      });
    }
    if (name == "d0Uncertainty") {
      return TJGetter([a](const Tp& t, const Jet&){
        return a.d0Uncertainty(t);
      });
    }
    if (name == "z0SinThetaUncertainty") {
      return TJGetter([a](const Tp& t, const Jet&){
        return a.z0SinThetaUncertainty(t);
      });
    }
    return std::nullopt;
  }

  std::optional<FlavorTagDiscriminants::SequenceFromTracks>
  sequenceNoIpDep(const std::string& name)
  {

    using Tp = xAOD::TrackParticle;
    using Jet = xAOD::Jet;

    if (name == "pt") {
      return TJGetter([](const Tp& t, const Jet&) {
        return t.pt();
      });
    }
    if (name == "log_pt") {
      return TJGetter([](const Tp& t, const Jet&) {
        return std::log(t.pt());
      });
    }
    if (name == "ptfrac") {
      return TJGetter([](const Tp& t, const Jet& j) {
        return t.pt() / j.pt();
      });
    }
    if (name == "log_ptfrac") {
      return TJGetter([](const Tp& t, const Jet& j) {
        return std::log(t.pt() / j.pt());
      });
    }

    if (name == "eta") {
      return TJGetter([](const Tp& t, const Jet&) {
        return t.eta();
      });
    }
    if (name == "deta") {
      return TJGetter([](const Tp& t, const Jet& j) {
        return t.eta() - j.eta();
      });
    }
    if (name == "abs_deta") {
      return TJGetter([](const Tp& t, const Jet& j) {
        return copysign(1.0, j.eta()) * (t.eta() - j.eta());
      });
    }

    if (name == "phi") {
      return TJGetter([](const Tp& t, const Jet&) {
        return t.phi();
      });
    }
    if (name == "dphi") {
      return TJGetter([](const Tp& t, const Jet& j) {
        return t.p4().DeltaPhi(j.p4());
      });
    }

    if (name == "dr") {
      return TJGetter([](const Tp& t, const Jet& j) {
        return t.p4().DeltaR(j.p4());
      });
    }
    if (name == "log_dr") {
      return TJGetter([](const Tp& t, const Jet& j) {
        return std::log(t.p4().DeltaR(j.p4()));
      });
    }
    if (name == "log_dr_nansafe") {
      return TJGetter([](const Tp& t, const Jet& j) {
        return std::log(t.p4().DeltaR(j.p4()) + 1e-7);
      });
    }

    if (name == "mass") {
      return TJGetter([](const Tp& t, const Jet&) {
        return t.m();
      });
    }
    if (name == "energy") {
      return TJGetter([](const Tp& t, const Jet&) {
        return t.e();
      });
    }

    if (name == "phiUncertainty") {
      return TJGetter([](const Tp& t, const Jet&) {
        return std::sqrt(t.definingParametersCovMatrixDiagVec().at(2));
      });
    }
    if (name == "thetaUncertainty") {
      return TJGetter([](const Tp& t, const Jet&) {
        return std::sqrt(t.definingParametersCovMatrixDiagVec().at(3));
      });
    }
    if (name == "qOverPUncertainty") {
      return TJGetter([](const Tp& t, const Jet&) {
        return std::sqrt(t.definingParametersCovMatrixDiagVec().at(4));
      });
    }
    if (name == "z0RelativeToBeamspot") {
      return TJGetter([](const Tp& t, const Jet&) {
        return t.z0();
      });
    }
    if (name == "log_z0RelativeToBeamspotUncertainty") {
      return TJGetter([](const Tp& t, const Jet&) {
        return std::log(std::sqrt(t.definingParametersCovMatrixDiagVec().at(1)));
      });
    }
    if (name == "z0RelativeToBeamspotUncertainty") {
      return TJGetter([](const Tp& t, const Jet&) {
        return std::sqrt(t.definingParametersCovMatrixDiagVec().at(1));
      });
    }

    if (name == "numberOfPixelHitsInclDead") {
      SG::AuxElement::ConstAccessor<unsigned char> pix_hits("numberOfPixelHits");
      SG::AuxElement::ConstAccessor<unsigned char> pix_dead("numberOfPixelDeadSensors");
      return TJGetter([pix_hits, pix_dead](const Tp& t, const Jet&) {
        return pix_hits(t) + pix_dead(t);
      });
    }
    if (name == "numberOfSCTHitsInclDead") {
      SG::AuxElement::ConstAccessor<unsigned char> sct_hits("numberOfSCTHits");
      SG::AuxElement::ConstAccessor<unsigned char> sct_dead("numberOfSCTDeadSensors");
      return TJGetter([sct_hits, sct_dead](const Tp& t, const Jet&) {
        return sct_hits(t) + sct_dead(t);
      });
    }
    if (name == "numberOfInnermostPixelLayerHits21p9") {
      SG::AuxElement::ConstAccessor<unsigned char> barrel_hits("numberOfInnermostPixelLayerHits");
      SG::AuxElement::ConstAccessor<unsigned char> endcap_hits("numberOfInnermostPixelLayerEndcapHits");
      return TJGetter([barrel_hits, endcap_hits](const Tp& t, const Jet&) {
        return barrel_hits(t) + endcap_hits(t);
      });
    }
    if (name == "numberOfNextToInnermostPixelLayerHits21p9") {
      SG::AuxElement::ConstAccessor<unsigned char> barrel_hits("numberOfNextToInnermostPixelLayerHits");
      SG::AuxElement::ConstAccessor<unsigned char> endcap_hits("numberOfNextToInnermostPixelLayerEndcapHits");
      return TJGetter([barrel_hits, endcap_hits](const Tp& t, const Jet&) {
        return barrel_hits(t) + endcap_hits(t);
      });
    }
    if (name == "numberOfInnermostPixelLayerSharedHits21p9") {
      SG::AuxElement::ConstAccessor<unsigned char> barrel_hits("numberOfInnermostPixelLayerSharedHits");
      SG::AuxElement::ConstAccessor<unsigned char> endcap_hits("numberOfInnermostPixelLayerSharedEndcapHits");
      return TJGetter([barrel_hits, endcap_hits](const Tp& t, const Jet&) {
        return barrel_hits(t) + endcap_hits(t);
      });
    }
    if (name == "numberOfInnermostPixelLayerSplitHits21p9") {
      SG::AuxElement::ConstAccessor<unsigned char> barrel_hits("numberOfInnermostPixelLayerSplitHits");
      SG::AuxElement::ConstAccessor<unsigned char> endcap_hits("numberOfInnermostPixelLayerSplitEndcapHits");
      return TJGetter([barrel_hits, endcap_hits](const Tp& t, const Jet&) {
        return barrel_hits(t) + endcap_hits(t);
      });
    }


    return std::nullopt;
  }

}

namespace FlavorTagDiscriminants {
  namespace internal {

    // ________________________________________________________________
    // Interface functions
    //
    // As long as we're giving lwtnn pair<name, double> objects, we
    // can't use the raw getter functions above (which only return a
    // double). Instead we'll wrap those functions in another function,
    // which returns the pair we wanted.
    //
    // Case for jet variables
    std::function<std::pair<std::string, double>(const xAOD::Jet&)>
    customGetterAndName(const std::string& name) {
      auto getter = customGetter(name);
      return [name, getter](const xAOD::Jet& j) {
               return std::make_pair(name, getter(j));
             };
    }

    // Case for track variables
    std::pair<std::function<std::pair<std::string, std::vector<double>>(
      const xAOD::Jet&,
      const std::vector<const xAOD::TrackParticle*>&)>,
      std::set<std::string>>
    customNamedSeqGetterWithDeps(const std::string& name,
                                 const std::string& prefix) {
      auto [getter, deps] = customSequenceGetterWithDeps(name, prefix);
      return {
        [n=name, g=getter](const xAOD::Jet& j,
                       const std::vector<const xAOD::TrackParticle*>& t) {
          return std::make_pair(n, g(j, t));
        },
        deps
      };
    }
  }
  // ________________________________________________________________________
  // Master track getter list
  //
  // These functions are wrapped by the customNamedSeqGetter function
  // below to become the ones that are actually used in DL2.
  //
  std::pair<SequenceFromTracks, std::set<std::string>>
  customSequenceGetterWithDeps(const std::string& name,
                               const std::string& prefix) {

    if (auto getter = sequenceWithIpDep(name, prefix)) {
      auto deps = BTagTrackIpAccessor(prefix).getTrackIpDataDependencyNames();
      return {*getter, deps};
    }

    if (auto getter = sequenceNoIpDep(name)) {
      return {*getter, {}};
    }
    throw std::logic_error("no match for custom getter " + name);
  }

}

