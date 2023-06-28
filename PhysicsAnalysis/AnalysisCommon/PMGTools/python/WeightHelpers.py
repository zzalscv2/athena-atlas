# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from ROOT import PMGTools
dummy = PMGTools.ROOT6_NamespaceAutoloadHook

generatorWeightsPrefix = PMGTools.generatorWeightsPrefix
weightNameCleanup = PMGTools.weightNameCleanup
weightNameWithPrefix = PMGTools.weightNameWithPrefix

__all__ = ['generatorWeightsPrefix', 'weightNameCleanup', 'weightNameWithPrefix']
