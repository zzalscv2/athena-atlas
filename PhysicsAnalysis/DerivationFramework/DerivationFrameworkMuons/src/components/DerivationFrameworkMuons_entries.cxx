/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkMuons/AnalysisMuonThinningAlg.h"
#include "DerivationFrameworkMuons/IDTrackCaloDepositsDecoratorAlg.h"
#include "DerivationFrameworkMuons/TrackIsolationDecorAlg.h"
#include "DerivationFrameworkMuons/CaloIsolationDecorAlg.h"
#include "DerivationFrameworkMuons/PflowIsolationDecorAlg.h"
#include "DerivationFrameworkMuons/MuonJetDrTool.h"
#include "DerivationFrameworkMuons/MuonTPExtrapolationAlg.h"
#include "DerivationFrameworkMuons/MuonTruthClassifierFallback.h"
#include "DerivationFrameworkMuons/MuonTruthIsolationDecorAlg.h"
#include "DerivationFrameworkMuons/DiMuonTaggingAlg.h"

DECLARE_COMPONENT(DerivationFramework::MuonTruthClassifierFallback)
DECLARE_COMPONENT(DerivationFramework::MuonTruthIsolationDecorAlg)
DECLARE_COMPONENT(DerivationFramework::MuonJetDrTool)
DECLARE_COMPONENT(DerivationFramework::DiMuonTaggingAlg)
DECLARE_COMPONENT(DerivationFramework::AnalysisMuonThinningAlg)
DECLARE_COMPONENT(DerivationFramework::IDTrackCaloDepositsDecoratorAlg)
DECLARE_COMPONENT(DerivationFramework::TrackIsolationDecorAlg)
DECLARE_COMPONENT(DerivationFramework::CaloIsolationDecorAlg)
DECLARE_COMPONENT(DerivationFramework::PflowIsolationDecorAlg)
DECLARE_COMPONENT(DerivationFramework::IDTrackCaloDepositsDecoratorAlg)
DECLARE_COMPONENT(DerivationFramework::MuonTPExtrapolationAlg)
