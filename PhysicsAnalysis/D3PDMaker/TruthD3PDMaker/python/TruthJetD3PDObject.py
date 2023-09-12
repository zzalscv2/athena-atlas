# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import EventCommonD3PDMaker
from D3PDMakerCoreComps.D3PDObject      import make_SGDataVector_D3PDObject
from D3PDMakerConfig.D3PDMakerFlags     import D3PDMakerFlags

TruthJetD3PDObject = make_SGDataVector_D3PDObject ('DataVector<xAOD::Jet_v1>',
                                                   D3PDMakerFlags.JetSGKey(),
                                                   'truthjet_',
                                                   'TruthJetD3PDObject')

TruthJetD3PDObject.defineBlock(0, 'Kinematics',
                               EventCommonD3PDMaker.FourMomFillerTool,
                               WriteE  = True)

