# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
This file defines the factories of the algorithms to be used in a photon trigger sequence in athenaMT
These are inspired by the offline factories, alhtough modified so they reflect the configuration we need for these algorithms at the HLT. 
Offline configurations are available here:
  https://gitlab.cern.ch/atlas/athena/blob/master/Reconstruction/egamma/egammaAlgs/python/
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
from AthenaConfiguration.ComponentFactory import CompFactory

def PrecisionPhotonCaloIsoMonitorCfg(flags, name = 'PrecisionPhotonCaloIsoEgammaBuilderMon', ion=False):
    acc = ComponentAccumulator()
    TrigEgammaKeys = getTrigEgammaKeys(ion = ion)
    from TrigEgammaMonitoring.egammaMonitorPrecisionConfig import egammaMonitorPrecisionCfg
    monTool = egammaMonitorPrecisionCfg(flags, name)
    collectionIn = TrigEgammaKeys.precisionPhotonContainer
    PrecisionPhotonCaloIsoMonitor = CompFactory.egammaMonitorPhotonAlgorithm(
                    name = name,
                    PhotonKey = TrigEgammaKeys.precisionPhotonContainer,
                    IsoVarKeys = [ '%s.topoetcone20' % collectionIn, '%s.topoetcone40' % collectionIn],
                    MonTool = monTool)
    acc.addEventAlgo(PrecisionPhotonCaloIsoMonitor)
    return acc
