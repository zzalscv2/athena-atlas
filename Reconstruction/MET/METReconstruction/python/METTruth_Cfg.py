# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from METReconstruction.METRecoCfg import BuildConfig, METConfig,getMETRecoAlg
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def METTruth_Cfg(configFlags):

    components = ComponentAccumulator()
    ## Simple truth terms
    cfg_truth = METConfig('Truth',configFlags,
                          [BuildConfig('NonInt'),
                           BuildConfig('Int'),
                           BuildConfig('IntOut'),
                           BuildConfig('IntMuons')],
                          doRegions=True
                          )
    components.merge(cfg_truth.accumulator)

    recoAlg=getMETRecoAlg(algName='METRecoAlg_Truth',configs={"Truth":cfg_truth})
    components.addEventAlgo(recoAlg)
    return components
