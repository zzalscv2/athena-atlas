/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/VRJetOverlapDecoratorTool.h"
#include "FlavorTagDiscriminants/HbbTagTool.h"
#include "FlavorTagDiscriminants/DL2Tool.h"
#include "FlavorTagDiscriminants/GNNTool.h"
#include "FlavorTagDiscriminants/MultifoldGNNTool.h"
#include "FlavorTagDiscriminants/BTagAugmenterTool.h"
#include "FlavorTagDiscriminants/BTagMuonAugmenterTool.h"
#include "FlavorTagDiscriminants/BTagDecoratorAlg.h"
#include "FlavorTagDiscriminants/JetTagDecoratorAlg.h"
#include "FlavorTagDiscriminants/JetTagConditionalDecoratorAlg.h"
#include "FlavorTagDiscriminants/BTagToJetLinkerAlg.h"
#include "FlavorTagDiscriminants/JetToBTagLinkerAlg.h"
#include "FlavorTagDiscriminants/BTagTrackLinkCopyAlg.h"
#include "FlavorTagDiscriminants/BTaggingBuilderAlg.h"
#include "FlavorTagDiscriminants/PoorMansIpAugmenterAlg.h"
#include "FlavorTagDiscriminants/TrackLeptonDecoratorAlg.h"
#include "FlavorTagDiscriminants/TruthParticleDecoratorAlg.h"
#include "FlavorTagDiscriminants/TrackTruthDecoratorAlg.h"
#include "FlavorTagDiscriminants/SoftElectronDecoratorAlg.h"
#include "FlavorTagDiscriminants/SoftElectronTruthDecoratorAlg.h"
#include <FlavorTagDiscriminants/TrackClassifier.h>

#include "src/FoldDecoratorAlg.h"

using namespace FlavorTagDiscriminants;

DECLARE_COMPONENT(VRJetOverlapDecoratorTool)
DECLARE_COMPONENT(HbbTagTool)
DECLARE_COMPONENT(DL2Tool)
DECLARE_COMPONENT(GNNTool)
DECLARE_COMPONENT(MultifoldGNNTool)
DECLARE_COMPONENT(BTagAugmenterTool)
DECLARE_COMPONENT(BTagMuonAugmenterTool)
DECLARE_COMPONENT(BTagDecoratorAlg)
DECLARE_COMPONENT(JetTagDecoratorAlg)
DECLARE_COMPONENT(JetTagConditionalDecoratorAlg)
DECLARE_COMPONENT(BTagToJetLinkerAlg)
DECLARE_COMPONENT(JetToBTagLinkerAlg)
DECLARE_COMPONENT(BTagTrackLinkCopyAlg)
DECLARE_COMPONENT(BTaggingBuilderAlg)
DECLARE_COMPONENT(PoorMansIpAugmenterAlg)
DECLARE_COMPONENT(TrackLeptonDecoratorAlg)
DECLARE_COMPONENT(TruthParticleDecoratorAlg)
DECLARE_COMPONENT(TrackTruthDecoratorAlg)
DECLARE_COMPONENT(SoftElectronDecoratorAlg)
DECLARE_COMPONENT(SoftElectronTruthDecoratorAlg)
DECLARE_COMPONENT(TrackClassifier)
DECLARE_COMPONENT(FoldDecoratorAlg)
