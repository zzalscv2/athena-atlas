/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef EVENT_SELECTOR_MISSINGETPLUSTRANSVERSEMASSSELECTORALG_H
#define EVENT_SELECTOR_MISSINGETPLUSTRANSVERSEMASSSELECTORALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

// Framework includes
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODEventInfo/EventInfo.h>

#include <EventSelectionAlgorithms/SignEnums.h>

namespace CP {

  /// \brief an algorithm to select an event with MET + W-boson mass compared
  /// to a specified MET+MWT value

  class MissingETPlusTransverseMassSelectorAlg final : public EL::AnaAlgorithm {

    /// \brief the standard constructor
    public:
      MissingETPlusTransverseMassSelectorAlg(const std::string &name, ISvcLocator *pSvcLocator);
      virtual StatusCode initialize() override;
      virtual StatusCode execute() override;

    private:
    
      /// \brief the MET+MWT threshold against which to compare
      Gaudi::Property<float> m_sumref {this, "refMETMWT", 0., "MET+MWT cut (in MeV)"};

      /// \brief the comparison (GT, LT, etc)
      Gaudi::Property<std::string> m_sign {this, "sign", "SetMe", "comparison sign to use"};

      /// \brief the operator version of the comparison (>, <, etc)
      SignEnum::ComparisonOperator m_signEnum;

      /// \brief the systematics
      CP::SysListHandle m_systematicsList {this};

      /// \brief the electron input handle
      CP::SysReadHandle<xAOD::ElectronContainer> m_electronsHandle {
        this, "electrons", "", "the electron container to use"
      };

      /// \brief the electrons selection
      CP::SysReadSelectionHandle m_electronSelection {
        this, "electronSelection", "", "the selection on the input electrons"
      };

      /// \brief the muon input handle
      CP::SysReadHandle<xAOD::MuonContainer> m_muonsHandle {
        this, "muons", "", "the muon container to use"
      };

      /// \brief the muon selection
      CP::SysReadSelectionHandle m_muonSelection {
        this, "muonSelection", "", "the selection on the input muons"
      };

      /// \brief the MET handle
      CP::SysReadHandle<xAOD::MissingETContainer> m_metHandle {
        this, "met", "SetMe", "the MET container to use"
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
        this, "decorationName", "SetMe", "decoration name for the MET+MWT selector"
      };

  }; // class
} // namespace CP

#endif // EVENT_SELECTOR_MISSINGETPLUSTRANSVERSEMASSSELECTORALG_H
