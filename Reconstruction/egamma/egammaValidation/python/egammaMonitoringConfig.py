# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def egammaMonitoringCfg(flags, particleType = 'electron',
                            outputFileName = 'Nightly-monitoring.hist'):

    acc = ComponentAccumulator()

    # Preparing selection tools (ID and iso)
    from ROOT import LikeEnum
    from ElectronPhotonSelectorTools.EgammaPIDdefs import egammaPID

    eidWP = { 'Loose' : LikeEnum.Loose, 'Medium' : LikeEnum.Medium, 'Tight' : LikeEnum.Tight }
    gidWP = { 'Loose' : egammaPID.PhotonIDLoose, 'Tight' : egammaPID.PhotonIDTight }
    gisoWP = [ 'FixedCutTight', 'FixedCutLoose', 'TightCaloOnly' ]

    kwarg = {}
    from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import (
        AsgElectronLikelihoodToolCfg)
    for k,w in eidWP.items():
        t = AsgElectronLikelihoodToolCfg(flags, k+'LHSelector', w)
        kwarg[k+'LH'] = t.popPrivateTools()
        acc.merge(t)

    from ElectronPhotonSelectorTools.AsgPhotonIsEMSelectorsConfig import (
        AsgPhotonIsEMSelectorCfg)
    for k,w in gidWP.items():
        t = AsgPhotonIsEMSelectorCfg(flags, k+'_Photon', w)
        kwarg[k+'_Photon'] = t.popPrivateTools()
        acc.merge(t)

    from IsolationSelection.IsolationSelectionConfig import IsolationSelectionToolCfg
    for k in gisoWP:
        t = IsolationSelectionToolCfg(flags, 'PhIso'+k, PhotonWP=k)
        name = f'Iso{k}'
        kwarg[name] = t.popPrivateTools()
        acc.merge(t)

    # a Truth classifier
    from MCTruthClassifier.MCTruthClassifierConfig import MCTruthClassifierCfg
    MCClassifier = acc.popToolsAndMerge(MCTruthClassifierCfg(flags))

    # The monitoring alg
    egMon = CompFactory.EgammaMonitoring(
        name = 'egammaMonitoringAlg',
        sampleType = particleType,
        MCTruthClassifier = MCClassifier,
        **kwarg)
    acc.addEventAlgo(egMon)

    # Add the histogram service
    svc = CompFactory.THistSvc(name="THistSvc")
    svc.Output = [f"MONITORING DATAFILE='{outputFileName}.root' OPT='RECREATE'"]
    acc.addService(svc)

    return acc
