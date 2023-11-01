/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PhotonVertexSelection_PhotonPointingTool_h
#define PhotonVertexSelection_PhotonPointingTool_h

// Framework includes
#include "AsgTools/AsgMetadataTool.h"

// Local includes
#include "PhotonVertexSelection/IPhotonPointingTool.h"

// EDM includes
#include "xAODEgamma/EgammaContainer.h"

// Data handles
#include "AsgTools/CurrentContext.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "AsgDataHandles/ReadHandleKey.h"

// Forward declarations
class TH1F;
namespace CP { class ShowerDepthTool; }

namespace CP {

  /// Implementation for the photon pointing tool
  ///
  /// Takes photon shower shape and/or conversion vertex
  /// and extrapolates back to beamline
  ///
  /// @author Christopher Meyer <chris.meyer@cern.ch>
  ///
  class PhotonPointingTool : public virtual IPhotonPointingTool,
                             public asg::AsgMetadataTool {

    /// Create a proper constructor for Athena
    ASG_TOOL_CLASS(PhotonPointingTool, CP::IPhotonPointingTool)

  private:
    /// Correction histogram
    TH1F *m_zCorrection;

    SG::ReadHandleKey<xAOD::EventInfo> m_evtInfo{
      this,
      "EventInfo",
      "EventInfo",
      "SG key of xAOD::EventInfo"
    };

    //Write decoration handle keys
    SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_zvertex{
      this,
      "zvertex",
      "Photons.zvertex"
      "z vertex"
    };
    SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_errz{
      this,
      "errz",
      "Photons.errz"
      "error in Z"
    };
    SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_HPV_zvertex{
      this,
      "HPV_zvertex",
      "Photons.HPV_zvertex",
      "HPV z vertex"
    };
    SG::WriteDecorHandleKey<xAOD::EgammaContainer> m_HPV_errz{
      this,
      "HPV_errz",
      "Photons.HPV_errz",
      "HPV error in z"
    };

  private:
    ///
    float getCorrectedZ(float zPointing, float etas2) const;
    bool m_isMC{};
    std::string m_zOscFileMC, m_zOscFileData;
    std::string m_ContainerName;

  public:
    PhotonPointingTool(const std::string &name);
    virtual ~PhotonPointingTool();

    /// @name Function(s) implementing the asg::IAsgTool interface
    /// @{

    /// Function initialising the tool
    virtual StatusCode initialize();

    /// @}

    /// @name Function(s) implementing the IPhotonPointingTool interface
    /// @{

    /// Add calo and conversion (HPV) pointing variables
    StatusCode updatePointingAuxdata(const xAOD::EgammaContainer &egammas) const ;

    /// Return calo pointing variables
    std::pair<float, float> getCaloPointing(const xAOD::Egamma *egamma) const ;

    /// Return conversion (HPV) pointing variables
    std::pair<float, float> getConvPointing(const xAOD::Photon *photon) const ;
    /// @}

  }; // class PhotonPointingTool

} // namespace CP


#endif // PhotonVertexSelection_PhotonPointingTool_H
