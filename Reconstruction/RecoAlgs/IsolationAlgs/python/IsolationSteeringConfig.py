# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

__doc__ = """
          Instantiate the isolation
          """

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def IsolationSteeringCfg(flags, name = 'IsolationSteering'):

    mlog = logging.getLogger(name)
    mlog.info('Starting Isolation steering')

    acc = ComponentAccumulator()

    from IsolationAlgs.IsoDensityConfig import (
        EMTopoInputAlgCfg, NFlowInputAlgCfg, DensityForIsoAlgCfg)
    if flags.Reco.EnableEgamma or flags.Reco.EnableCombinedMuon:
        if flags.Detector.EnableCalo:
            # do not compute density if HI with subtracted clusters
            # since no pu correction in this case
            if not flags.HeavyIon.Egamma.doSubtractedClusters:
                acc.merge(EMTopoInputAlgCfg(flags))
                acc.merge(DensityForIsoAlgCfg(flags))
                acc.merge(DensityForIsoAlgCfg(flags,name='ForwardDensityForTopoIso'))
        # should be switch off also for HI, but if done, crash sowewhere else...
        if flags.Reco.EnablePFlow:
            acc.merge(NFlowInputAlgCfg(flags))
            acc.merge(DensityForIsoAlgCfg(flags,name='CentralDensityForNFlowIso'))
            acc.merge(DensityForIsoAlgCfg(flags,name='ForwardDensityForNFlowIso'))
            
    from IsolationAlgs.IsolationBuilderConfig import egIsolationCfg, muIsolationCfg
    if flags.Reco.EnableEgamma:
        acc.merge(egIsolationCfg(flags,name = 'photonIsolation'))
        acc.merge(egIsolationCfg(flags,name = 'electronIsolation'))
    if flags.Reco.EnableCombinedMuon:
        acc.merge(muIsolationCfg(flags,name = 'muonIsolation'))

    # Add density related containers to the output stream
    if flags.Output.doWriteESD or flags.Output.doWriteAOD:
        from IsolationAlgs.IsoOutputConfig import IsoOutputCfg
        acc.merge(IsoOutputCfg(flags))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.Output.doWriteESD = True  # To test the ESD parts
    flags.Output.doWriteAOD = True  # To test the AOD parts
    flags.lock()

    mlog = logging.getLogger("isolationConfigTest")
    mlog.info("Configuring isolation: ")

    acc = MainServicesCfg(flags)
    acc.merge(IsolationSteeringCfg(flags))
    acc.printConfig(withDetails=True,
                    printDefaults=True)
    printProperties(mlog,
                    acc.getEventAlgo('photonIsolationBuilder'),
                    nestLevel=1,
                    printDefaults=True)

    with open("isolationconfig.pkl", "wb") as f:
        acc.store(f)
