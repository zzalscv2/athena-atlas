/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "eflowRec/eflowTrackCaloExtensionTool.h"
#include "eflowRec/eflowTrackCaloDummyExtensionTool.h"
#include "eflowRec/PFTrackClusterMatchingTool.h"
#include "eflowRec/eflowCellEOverPTool_Run2_mc20_JetETMiss.h"
#include "eflowRec/eflowCellEOverPTool_mc12_JetETMiss.h"
#include "eflowRec/eflowCellEOverPTool_mc12_HLLHC.h"
#include "eflowRec/eflowCellEOverPTool_mc12_LC.h"
#include "eflowRec/PFLeptonSelector.h"
#include "eflowRec/PFTrackSelector.h"
#include "eflowRec/PFClusterSelectorTool.h"
#include "eflowRec/PFAlgorithm.h"
#include "eflowRec/PFChargedFlowElementCreatorAlgorithm.h"
#include "eflowRec/PFNeutralFlowElementCreatorAlgorithm.h"
#include "eflowRec/PFLCNeutralFlowElementCreatorAlgorithm.h"
#include "eflowRec/PFSubtractionTool.h"
#include "eflowRec/PFMomentCalculatorTool.h"
#include "eflowRec/PFClusterCollectionTool.h"
#include "eflowRec/PFLCCalibTool.h"
#include "eflowRec/PFOChargedCreatorAlgorithm.h"
#include "eflowRec/PFONeutralCreatorAlgorithm.h"
#include "eflowRec/PFEGammaPFOAssoc.h"
#include "eflowRec/PFMuonFlowElementAssoc.h"
#include "eflowRec/PFEGamFlowElementAssoc.h"
#include "eflowRec/PFTauFlowElementAssoc.h"
#include "../PFTrackPreselAlg.h"
#include "../PFTrackMuonCaloTaggingAlg.h"
#include "../PFTrackMuonIsoTaggingAlg.h"

DECLARE_COMPONENT( PFLeptonSelector )
DECLARE_COMPONENT( PFEGammaPFOAssoc )
DECLARE_COMPONENT( PFClusterSelectorTool )
DECLARE_COMPONENT( PFTrackSelector )
DECLARE_COMPONENT( PFAlgorithm )
DECLARE_COMPONENT( PFChargedFlowElementCreatorAlgorithm)
DECLARE_COMPONENT( PFNeutralFlowElementCreatorAlgorithm)
DECLARE_COMPONENT( PFLCNeutralFlowElementCreatorAlgorithm)
DECLARE_COMPONENT( PFOChargedCreatorAlgorithm )
DECLARE_COMPONENT( PFONeutralCreatorAlgorithm )
DECLARE_COMPONENT( PFSubtractionTool )
DECLARE_COMPONENT( PFMomentCalculatorTool )
DECLARE_COMPONENT( PFClusterCollectionTool )
DECLARE_COMPONENT( PFLCCalibTool )
DECLARE_COMPONENT( eflowTrackCaloExtensionTool )
DECLARE_COMPONENT( eflowTrackCaloDummyExtensionTool )
DECLARE_COMPONENT( PFTrackClusterMatchingTool )
DECLARE_COMPONENT( eflowCellEOverPTool_Run2_mc20_JetETMiss)
DECLARE_COMPONENT( eflowCellEOverPTool_mc12_JetETMiss)
DECLARE_COMPONENT(  eflowCellEOverPTool_mc12_HLLHC)
DECLARE_COMPONENT( eflowCellEOverPTool_mc12_LC)
DECLARE_COMPONENT( PFMuonFlowElementAssoc )
DECLARE_COMPONENT( PFEGamFlowElementAssoc )
DECLARE_COMPONENT( PFTauFlowElementAssoc )
DECLARE_COMPONENT( PFTrackPreselAlg )
DECLARE_COMPONENT( PFTrackMuonCaloTaggingAlg )
DECLARE_COMPONENT( PFTrackMuonIsoTaggingAlg )
