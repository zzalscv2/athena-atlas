#include <tuple>
#include "xAODBase/IParticleContainer.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEgamma/EgammaContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODBTagging/BTaggingContainer.h"
#include "xAODBTagging/BTagVertexContainer.h"

namespace EleLinkThinningHelpers {
  struct TypeList {
    static constexpr std::tuple<
      xAOD::IParticleContainer,
      xAOD::TrackParticleContainer,
      xAOD::VertexContainer,
      xAOD::CaloClusterContainer,
      xAOD::EgammaContainer,
      xAOD::ElectronContainer,
      xAOD::PhotonContainer,
      xAOD::MuonContainer,
      xAOD::JetContainer,
      xAOD::BTaggingContainer,
      xAOD::BTagVertexContainer>* types = nullptr;
  };
}
