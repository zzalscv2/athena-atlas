# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from TrackToCalo.TrackToCaloConf import Trk__ParticleCaloExtensionTool
from MCTruthClassifier.MCTruthClassifierConf import (
    MCTruthClassifier as classifierTool)
from MCTruthClassifier.MCTruthClassifierConfig import firstSimCreatedBarcode
from AthenaCommon.AppMgr import ToolSvc
from AthenaCommon.Logging import logging

mlog = logging.getLogger('MCTruthClassifierBase.py::configure:')


def getMCTruthClassifierExtrapolator():
    # Configure the extrapolator, starting from the ATLAS one
    from TrkExTools.AtlasExtrapolator import AtlasExtrapolator
    theAtlasExtrapolator = AtlasExtrapolator(
        name='MCTruthClassifieExtrapolator')

    # all left to MaterialEffects/EnergyLossUpdators
    from TrkExTools.TrkExToolsConf import (
        Trk__MaterialEffectsUpdator as MaterialEffectsUpdator)

    NoElossMaterialEffectsUpdator = MaterialEffectsUpdator(
        name='NoElossMaterialEffectsUpdator')
    NoElossMaterialEffectsUpdator.EnergyLoss = False

    # We extrapolate truth to calo (mainly photons) with no-eloss
    # The 1st list is the updators we are to use
    MyUpdators = []
    MyUpdators += [NoElossMaterialEffectsUpdator]
    # And here for which part we are going to use them
    MySubUpdators = []
    MySubUpdators += [NoElossMaterialEffectsUpdator.name()]  # Global
    MySubUpdators += [NoElossMaterialEffectsUpdator.name()]  # ID
    MySubUpdators += [NoElossMaterialEffectsUpdator.name()]  # beampipe
    MySubUpdators += [NoElossMaterialEffectsUpdator.name()]  # calo
    MySubUpdators += [NoElossMaterialEffectsUpdator.name()]  # MS
    MySubUpdators += [NoElossMaterialEffectsUpdator.name()]  # cavern
    theAtlasExtrapolator.MaterialEffectsUpdators = MyUpdators
    theAtlasExtrapolator.SubMEUpdators = MySubUpdators

    return theAtlasExtrapolator


ClassifierParticleCaloExtensionTool = Trk__ParticleCaloExtensionTool(
    name="ClassifierParticleCaloExtensionTool",
    Extrapolator=getMCTruthClassifierExtrapolator())

MCTruthClassifier = classifierTool(
    name='MCTruthClassifier',
    barcodeG4Shift=firstSimCreatedBarcode(),
    ParticleCaloExtensionTool='')

ToolSvc += MCTruthClassifier

MCTruthClassifierCalo = classifierTool(
    name='MCTruthClassifierCalo',
    barcodeG4Shift=firstSimCreatedBarcode(),
    ParticleCaloExtensionTool=ClassifierParticleCaloExtensionTool)
