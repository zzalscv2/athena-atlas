/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "DerivationFrameworkHiggs/DiphotonVertexDecorator.h"
#include "DerivationFrameworkHiggs/FourLeptonVertexingAlgorithm.h"
#include "DerivationFrameworkHiggs/MergedElectronDetailsDecorator.h"
#include "DerivationFrameworkHiggs/SkimmingToolHIGG1.h"
#include "DerivationFrameworkHiggs/SkimmingToolHIGG2.h"
#include "DerivationFrameworkHiggs/SkimmingToolHIGG5VBF.h"
#include "DerivationFrameworkHiggs/TruthCategoriesDecorator.h"
#include "DerivationFrameworkHiggs/ZeeVertexRefittingTool.h"
using namespace DerivationFramework;

DECLARE_COMPONENT(SkimmingToolHIGG1)
DECLARE_COMPONENT(SkimmingToolHIGG2)
DECLARE_COMPONENT(SkimmingToolHIGG5VBF)
DECLARE_COMPONENT(TruthCategoriesDecorator)
DECLARE_COMPONENT(DiphotonVertexDecorator)
DECLARE_COMPONENT(MergedElectronDetailsDecorator)
DECLARE_COMPONENT(ZeeVertexRefittingTool)
DECLARE_COMPONENT(FourLeptonVertexingAlgorithm)
