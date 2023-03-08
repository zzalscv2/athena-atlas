# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__doc__ = "ToolFactories to configure egammaAlgs to be used at the HLT" 
__author__ = "Fernando Monticelli"
"""
This file defines the factories of the algorithms to be used in a photon trigger sequence in athenaMT
These are inspired by the offline factories, alhtough modified so they reflect the configuration we need for these algorithms at the HLT. 
Offline configurations are available here:
    https://gitlab.cern.ch/atlas/athena/blob/master/Reconstruction/egamma/egammaAlgs/python/


"""
from egammaAlgs import egammaAlgsConf
from egammaRec.Factories import AlgFactory,  FcnWrapper
# Tools and funtions from TrigEgammaFactories
from TriggerMenuMT.HLT.Egamma.TrigEgammaFactories import TrigEMClusterTool, TrigEMShowerBuilder_HI, TrigEMShowerBuilder, TrigEgammaDecorationTools, TrigPhotonDecorationTools
# Load TrigEgammaKeys where we store the container names and other TrigEgamma configuration values
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys


# Decoration tools for egamma and photon objects:
# Copying from https://gitlab.cern.ch/atlas/athena/blob/master/Reconstruction/egamma/egammaRec/python/topoEgammaGetter.py#L28

TrigEgammaKeys = getTrigEgammaKeys()


#Factory for photons
TrigTopoEgammaPhotons_HI = AlgFactory( egammaAlgsConf.xAODEgammaBuilder,
        name = 'TrigTopoEgammaPhotons_HI',
        InputElectronRecCollectionName = TrigEgammaKeys.precisionElectronSuperClusterRecCollection,
        InputPhotonRecCollectionName = TrigEgammaKeys.precisionPhotonSuperClusterRecCollection,
        ElectronOutputName = TrigEgammaKeys.precisionElectronContainer,
        PhotonOutputName = TrigEgammaKeys.precisionPhotonContainer,
        EMClusterTool = TrigEMClusterTool("photon"),
        EMShowerTool=TrigEMShowerBuilder_HI,
        egammaTools = FcnWrapper(TrigEgammaDecorationTools),
        PhotonTools = FcnWrapper(TrigPhotonDecorationTools),
        doAdd = False,
        doPhotons = True,
        doElectrons = False,
        )

TrigTopoEgammaPhotons = AlgFactory( egammaAlgsConf.xAODEgammaBuilder, 
        name = 'TrigTopoEgammaPhotons',
        InputElectronRecCollectionName = TrigEgammaKeys.precisionElectronSuperClusterRecCollection,
        InputPhotonRecCollectionName = TrigEgammaKeys.precisionPhotonSuperClusterRecCollection,
        ElectronOutputName = TrigEgammaKeys.precisionElectronContainer,
        PhotonOutputName = TrigEgammaKeys.precisionPhotonContainer,  
        EMClusterTool = TrigEMClusterTool("photon"),
        EMShowerTool=TrigEMShowerBuilder,
        egammaTools = FcnWrapper(TrigEgammaDecorationTools),
        PhotonTools = FcnWrapper(TrigPhotonDecorationTools),
        doAdd = False,
        doPhotons = True,
        doElectrons = False,
        )

def PrecisionPhotonCaloIsoMonitorCfg(flags, name = 'PrecisionPhotonCaloIsoMonitoring'):
    
    from TrigEgammaMonitoring import TrigEgammaMonitoringConf
    from TrigEgammaMonitoring.egammaMonitorPrecisionConfig import egammaMonitorPrecisionCfg
    monTool = egammaMonitorPrecisionCfg(flags, name)

    PrecisionPhotonCaloIsoMonitor = AlgFactory( TrigEgammaMonitoringConf.egammaMonitorPhotonAlgorithm,
            name = name,
            doAdd = False,
            PhotonKey = TrigEgammaKeys.precisionPhotonContainer,
            MonTool = monTool
            )

    return PrecisionPhotonCaloIsoMonitor()

def PrecisionPhotonTopoMonitorCfg(flags, name = 'PrecisionPhotonTopoMonitoring'):
    
    from TrigEgammaMonitoring import TrigEgammaMonitoringConf
    from TrigEgammaMonitoring.egammaMonitorPrecisionConfig import egammaMonitorPrecisionCfg
    monTool = egammaMonitorPrecisionCfg(flags, name)

    PrecisionPhotonTopoMonitor = AlgFactory( TrigEgammaMonitoringConf.egammaMonitorPhotonAlgorithm,
            name = name,
            doAdd = False,
            PhotonKey = TrigEgammaKeys.precisionPhotonContainer,
            MonTool = monTool
            )

    return PrecisionPhotonTopoMonitor()

def PrecisionPhotonSuperClusterMonitorCfg(flags, name = 'PrecisionPhotonSuperClusterMonitoring'):
    
    from TrigEgammaMonitoring import TrigEgammaMonitoringConf
    from TrigEgammaMonitoring.egammaMonitorPrecisionConfig import egammaMonitorSuperClusterCfg
    monTool = egammaMonitorSuperClusterCfg(flags, name)

    PrecisionPhotonSuperClusterMonitor = AlgFactory( TrigEgammaMonitoringConf.egammaMonitorSuperClusterAlgorithm,
            name = name,
            doAdd = False,
            InputEgammaRecContainerName = TrigEgammaKeys.precisionEgammaRecCollection,
            MonTool = monTool
            )

    return PrecisionPhotonSuperClusterMonitor()
