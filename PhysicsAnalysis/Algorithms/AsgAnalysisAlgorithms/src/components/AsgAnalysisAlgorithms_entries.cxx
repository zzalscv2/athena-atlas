/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#include <AsgAnalysisAlgorithms/AsgFlagSelectionTool.h>
#include <AsgAnalysisAlgorithms/AsgMaskSelectionTool.h>
#include <AsgAnalysisAlgorithms/AsgPtEtaSelectionTool.h>
#include <AsgAnalysisAlgorithms/AsgCutBookkeeperAlg.h>
#include <AsgAnalysisAlgorithms/AsgClassificationDecorationAlg.h>
#include <AsgAnalysisAlgorithms/AsgPriorityDecorationAlg.h>
#include <AsgAnalysisAlgorithms/AsgEventScaleFactorAlg.h>
#include <AsgAnalysisAlgorithms/AsgLeptonTrackSelectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgOriginalObjectLinkAlg.h>
#include <AsgAnalysisAlgorithms/AsgSelectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgShallowCopyAlg.h>
#include <AsgAnalysisAlgorithms/AsgUnionPreselectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgUnionSelectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgViewFromSelectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgxAODNTupleMakerAlg.h>
#include <AsgAnalysisAlgorithms/AsgxAODMetNTupleMakerAlg.h>
#include <AsgAnalysisAlgorithms/EventFlagSelectionAlg.h>
#include <AsgAnalysisAlgorithms/EventStatusSelectionAlg.h>
#include <AsgAnalysisAlgorithms/EventSelectionByObjectFlagAlg.h>
#include <AsgAnalysisAlgorithms/KinematicHistAlg.h>
#include <AsgAnalysisAlgorithms/ObjectCutFlowHistAlg.h>
#include <AsgAnalysisAlgorithms/OverlapRemovalAlg.h>
#include <AsgAnalysisAlgorithms/PileupReweightingAlg.h>
#include <AsgAnalysisAlgorithms/PMGTruthWeightAlg.h>
#include <AsgAnalysisAlgorithms/SysListDumperAlg.h>
#include <AsgAnalysisAlgorithms/TreeFillerAlg.h>
#include <AsgAnalysisAlgorithms/TreeMakerAlg.h>
#include "AsgAnalysisAlgorithms/SystObjectLinkerAlg.h"
#include "AsgAnalysisAlgorithms/SystObjectUnioniserAlg.h"

DECLARE_COMPONENT (CP::AsgFlagSelectionTool)
DECLARE_COMPONENT (CP::AsgMaskSelectionTool)
DECLARE_COMPONENT (CP::AsgPtEtaSelectionTool)
DECLARE_COMPONENT (CP::AsgClassificationDecorationAlg)
DECLARE_COMPONENT (CP::AsgPriorityDecorationAlg)
DECLARE_COMPONENT (CP::AsgCutBookkeeperAlg)
DECLARE_COMPONENT (CP::AsgEventScaleFactorAlg)
DECLARE_COMPONENT (CP::AsgLeptonTrackSelectionAlg)
DECLARE_COMPONENT (CP::AsgOriginalObjectLinkAlg)
DECLARE_COMPONENT (CP::AsgSelectionAlg)
DECLARE_COMPONENT (CP::AsgShallowCopyAlg)
DECLARE_COMPONENT (CP::AsgUnionPreselectionAlg)
DECLARE_COMPONENT (CP::AsgUnionSelectionAlg)
DECLARE_COMPONENT (CP::AsgViewFromSelectionAlg)
DECLARE_COMPONENT (CP::AsgxAODNTupleMakerAlg)
DECLARE_COMPONENT (CP::AsgxAODMetNTupleMakerAlg)
DECLARE_COMPONENT (CP::EventFlagSelectionAlg)
DECLARE_COMPONENT (CP::EventStatusSelectionAlg)
DECLARE_COMPONENT (CP::EventSelectionByObjectFlagAlg)
DECLARE_COMPONENT (CP::KinematicHistAlg)
DECLARE_COMPONENT (CP::ObjectCutFlowHistAlg)
DECLARE_COMPONENT (CP::OverlapRemovalAlg)
DECLARE_COMPONENT (CP::PileupReweightingAlg)
DECLARE_COMPONENT (CP::PMGTruthWeightAlg)
DECLARE_COMPONENT (CP::SysListDumperAlg)
DECLARE_COMPONENT (CP::TreeFillerAlg)
DECLARE_COMPONENT (CP::TreeMakerAlg)
DECLARE_COMPONENT (CP::SystObjectLinkerAlg)
// Concrete classes of SystObjectUnioniserAlg
DECLARE_COMPONENT (CP::SystJetUnioniserAlg)
DECLARE_COMPONENT (CP::SystElectronUnioniserAlg)
DECLARE_COMPONENT (CP::SystPhotonUnioniserAlg)
DECLARE_COMPONENT (CP::SystMuonUnioniserAlg)
DECLARE_COMPONENT (CP::SystTauUnioniserAlg)
DECLARE_COMPONENT (CP::SystDiTauUnioniserAlg)
