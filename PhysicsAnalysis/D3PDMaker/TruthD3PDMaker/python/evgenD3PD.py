# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#
# @file TruthD3PDMaker/python/evgenD3PD.py
# @author Renaud Bruneliere <Renaud.Bruneliere@cern.ch>
# @date Apr, 2010
# @brief Construct an evgen D3PD.
#


import D3PDMakerCoreComps

from EventCommonD3PDMaker.EventInfoD3PDObject        import EventInfoD3PDObject
from TruthD3PDMaker.TruthJetD3PDObject               import TruthJetD3PDObject

from TruthD3PDMaker.GenEventD3PDObject               import GenEventD3PDObject
from TruthD3PDMaker.TruthParticleD3PDObject          import TruthParticleD3PDObject
from RecExConfig.RecFlags                            import rec

from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()


def evgenD3PD (file,
               tuplename = 'evgen',
               seq = topSequence,
               D3PDSvc = 'D3PD::RootD3PDSvc'):

    #--------------------------------------------------------------------------
    # Configuration
    #--------------------------------------------------------------------------
    if rec.doTruth():
        # compatibility with jets
        from JetRec.JetRecFlags import jobproperties as jobpropjet
        jobpropjet.JetRecFlags.inputFileType = "GEN"



    #--------------------------------------------------------------------------
    # Make the D3PD
    #--------------------------------------------------------------------------
    alg = D3PDMakerCoreComps.MakerAlg(tuplename, seq,
                                      file = file, D3PDSvc = D3PDSvc)
    alg += EventInfoD3PDObject (10)

    mysuffix = ''
    if rec.doTruth():
        alg += GenEventD3PDObject (1)
        alg += TruthParticleD3PDObject (1)
        alg += TruthJetD3PDObject (level=10, sgkey='AntiKt4Truth'+mysuffix+'Jets', prefix='jet_antikt4truth'+mysuffix+'jets_')
        alg += TruthJetD3PDObject (level=10, sgkey='AntiKt6Truth'+mysuffix+'Jets', prefix='jet_antikt6truth'+mysuffix+'jets_')
        alg += TruthJetD3PDObject (level=10, sgkey='AntiKt4TruthParton'+mysuffix+'Jets', prefix='jet_antikt4truthparton'+mysuffix+'jets_')
        alg += TruthJetD3PDObject (level=10, sgkey='AntiKt6TruthParton'+mysuffix+'Jets', prefix='jet_antikt6truthparton'+mysuffix+'jets_')

    return alg
