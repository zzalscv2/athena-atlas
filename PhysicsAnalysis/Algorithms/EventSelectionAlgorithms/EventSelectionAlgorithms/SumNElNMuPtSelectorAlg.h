/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef EVENT_SELECTOR_SUMNELNMUPTSELECTORALG_H
#define EVENT_SELECTOR_SUMNELNMUPTSELECTORALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>

// Framework includes
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEventInfo/EventInfo.h>

#include <EventSelectionAlgorithms/SignEnums.h>

namespace CP {

  /// \brief an algorithm to select an event with a specified number
  /// of electrons or muons compared to a transverse momentum value

  class SumNElNMuPtSelectorAlg final : public EL::AnaAlgorithm {

    /// \brief the standard constructor 
  public:
    SumNElNMuPtSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

  private:

    /// \brief the pT threshold on which to select
    Gaudi::Property<float> m_ptmin {this, "minPt", 0., "minimum pT (in MeV)"};

    /// \brief the sign against which to compare pT (GT, LT, etc)
    Gaudi::Property<std::string> m_sign {this, "sign", "SetMe", "comparison sign to use"};

    /// \brief the count of events desired
    Gaudi::Property<int> m_count {this, "count", 0, "count value"};

    /// \brief the operator version of the comparison (>, <, etc)
    SignEnum::ComparisonOperator m_signEnum;

    /// \brief the systematics list
    CP::SysListHandle m_systematicsList {this};

    /// \brief the electrons handle
    CP::SysReadHandle<xAOD::ElectronContainer> m_electronsHandle {
      this, "electrons", "", "the electron container to use"
    };

    /// \brief the electrons selection
    CP::SysReadSelectionHandle m_electronSelection {
      this, "electronSelection", "", "the selection on the input electrons"
    };

    /// \brief the muons handle
    CP::SysReadHandle<xAOD::MuonContainer> m_muonsHandle {
      this, "muons", "", "the muon container to use"
    };

    /// \brief the muons selection
    CP::SysReadSelectionHandle m_muonSelection {
      this, "muonSelection", "", "the selection on the input muons"
    };

    /// \brief the event info handle
    CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
      this, "eventInfo", "EventInfo", "the EventInfo container to read selection decisions from"
    };

    /// \brief the preselection
    CP::SysReadSelectionHandle m_preselection {
      this, "eventPreselection", "SetMe", "name of the preselection to check before applying this one"
    };

    /// \brief the output selection decoration 
    CP::SysWriteSelectionHandle m_decoration {
      this, "decorationName", "SetMe", "decoration name for the NObjects selector"
    };

  }; // class
} // namespace CP

#endif // EVENT_SELECTOR_SUMNELNMUPTSELECTORALG_H
