/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef ASG_ANALYSIS_ALGORITHMS__ASG_LEPTON_TRACK_SELECTION_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__ASG_LEPTON_TRACK_SELECTION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <PATCore/IAsgSelectionTool.h>
#include <SelectionHelpers/ISelectionNameSvc.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODBase/IParticleContainer.h>

namespace CP
{
  /// \brief an algorithm for performing track-vertex selection on
  /// leptons
  ///
  /// Originally I meant to implement this as an \ref
  /// IAsgSelectionTool, but since this needs other objects besides
  /// the lepton itself, I made it into an algorithm.  Technically
  /// this could also be addressed by retrieving those extra objects
  /// in the seleciton tool, but this seemed like potential overkill,
  /// given that the selection tools may be called very frequently and
  /// are not really doing any form of heavy lifting at all.  Still,
  /// at some point we may decide to change this into a selection tool
  /// instead (06 Aug 18).

  class AsgLeptonTrackSelectionAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    AsgLeptonTrackSelectionAlg (const std::string& name, 
                                   ISvcLocator* pSvcLocator);


  public:
    StatusCode initialize () override;

  public:
    StatusCode execute () override;


    /// algorithm properties
    /// \{

  private:
    float m_maxD0Significance {0};
    float m_maxDeltaZ0SinTheta {0};
    int m_nMinPixelHits{-1};
    int m_nMaxPixelHits{-1};
    int m_nMinSCTHits{-1};
    int m_nMaxSCTHits{-1};
    std::string m_eventInfo {"EventInfo"};
    std::string m_primaryVertices {"PrimaryVertices"};

    /// \}


    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the particle continer we run on
  private:
    SysReadHandle<xAOD::IParticleContainer> m_particlesHandle {
      this, "particles", "", "the asg collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the accessor for \ref m_selectionDecoration
  private:
    SysWriteSelectionHandle m_selectionHandle {
      this, "selectionDecoration", "trackSelection", "the decoration for the asg selection"};

    /// \brief the ISelectionNameSvc
  private:
    ServiceHandle<ISelectionNameSvc> m_nameSvc {"SelectionNameSvc", "AsgLeptonTrackSelectionAlg"};


    /// \brief the \ref asg::AcceptInfo we are using
  private:
    asg::AcceptInfo m_accept;
  };
}

#endif
