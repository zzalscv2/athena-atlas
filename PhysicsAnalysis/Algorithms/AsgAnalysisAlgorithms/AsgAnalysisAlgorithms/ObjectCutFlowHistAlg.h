/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef ASG_ANALYSIS_ALGORITHMS__OBJECT_CUT_FLOW_HIST_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__OBJECT_CUT_FLOW_HIST_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SelectionHelpers/ISelectionNameSvc.h>
#include <SelectionHelpers/ISelectionReadAccessor.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODBase/IParticleContainer.h>

namespace CP
{
  /// \brief an algorithm for dumping the kinematics of an IParticle
  /// container into histograms
  ///
  /// This is mostly meant as a temporary helper algorithm to debug
  /// the common CP algorithms as they get developed.

  class ObjectCutFlowHistAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    ObjectCutFlowHistAlg (const std::string& name, 
                            ISvcLocator* pSvcLocator);


  public:
    StatusCode initialize () override;

  public:
    StatusCode execute () override;


    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the jet collection we run on
  private:
    SysReadHandle<xAOD::IParticleContainer> m_inputHandle {
      this, "input", "", "the input collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the pattern for histogram names
  private:
    std::string m_histPattern {"cutflow_%SYS%"};

    /// \brief the selection name service
  private:
    ServiceHandle<ISelectionNameSvc> m_selectionNameSvc {"SelectionNameSvc", "ObjectCutFlowHistAlg"};

    /// \brief the histogram title to use
  private:
    Gaudi::Property<std::string> m_histTitle {this, "histTitle", "object cut flow", "title for the created histograms"};


  private:
    std::vector<std::string> m_selection;

    /// the list of accessors and cut ignore list
  private:
    std::vector<std::pair<std::unique_ptr<ISelectionReadAccessor>,unsigned> > m_accessors;

    /// \brief the total number of cuts configured (needed to
    /// configure histograms)
  private:
    unsigned m_allCutsNum = 0;

    /// \brief the created histograms
  private:
    std::unordered_map<CP::SystematicSet,TH1*> m_hist;

    /// \brief histogram bin labels
  private:
    std::vector<std::string> m_labels;
  };
}

#endif
