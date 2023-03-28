# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from METReconstruction.METRecoCfg import BuildConfig, METConfig,getMETRecoAlg
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def METCalo_Cfg(configFlags):

    components = ComponentAccumulator()
    ############################################################################
    # EMTopo

    cfg_emt = METConfig('EMTopo',configFlags,[BuildConfig('SoftClus','EMTopo')],
                    doRegions=True,
                    doOriginCorrClus=False
                    )
    components.merge(cfg_emt.accumulator)

    ############################################################################
    # LocHadTopo
    
    cfg_lht = METConfig('LocHadTopo',configFlags,[BuildConfig('SoftClus','LocHadTopo')],
                    doRegions=True,
                    doOriginCorrClus=False
                    )
    components.merge(cfg_lht.accumulator)


    ############################################################################
    # Calo regions
    #SWITCH OFF CELLS WHEN RUNNING ON AOD
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    components.merge(CaloNoiseCondAlgCfg(configFlags, 'totalNoise'))
    cfg_calo = METConfig('Calo',configFlags,
                     [BuildConfig('CaloReg')],
                     doCells=False
                     )
    components.merge(cfg_calo.accumulator)

    recoAlg_calo = getMETRecoAlg(algName='METRecoAlg_Calo',configs={"EMTopo":cfg_emt,"LocHadTopo":cfg_lht,"Calo":cfg_calo})
    components.addEventAlgo(recoAlg_calo)
    return components
