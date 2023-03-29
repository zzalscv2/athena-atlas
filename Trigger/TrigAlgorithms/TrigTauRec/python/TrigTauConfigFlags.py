# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from tauRec.TauConfigFlags import createTauConfigFlags

def createTrigTauConfigFlags():
    flags = AthConfigFlags()
    flags.join(createTauConfigFlags(), prefix='Trigger.Offline')

    flags.Trigger.Offline.Tau.tauRecToolsCVMFSPath = 'TrigTauRec/00-11-02'

    # deprecated and should be phased out
    flags.Trigger.Offline.Tau.CalibrateLCConfig = 'TES2016_LC_online_inc.root'

    flags.Trigger.Offline.Tau.MvaTESConfig = 'OnlineMvaTES_BRT_MC23a_v2.weights.root'

    flags.Trigger.Offline.Tau.TauJetRNNConfig = ['DeepSetID_MC23_v2_0p.json',
                                                 'DeepSetID_MC23_v2_1p.json',
                                                 'DeepSetID_MC23_v2_mp.json']

    flags.Trigger.Offline.Tau.TauJetRNNWPConfig = ['DeepSetID_MC23_v2_newPerf_flat_0p.root',
                                                   'DeepSetID_MC23_v2_newPerf_flat_1p.root',
                                                   'DeepSetID_MC23_v2_newPerf_flat_mp.root']

    # these flags only exists in the trigger, but 'cloneAndReplace' in 'addFlagsCategory'
    # assumes a 'Trigger.Offline.Tau' structure
    flags.addFlag("Trigger.Offline.Tau.FTFTauCoreBDTConfig", 'FTF_tauCore_BDT_v1.root')

    flags.addFlag("Trigger.Offline.Tau.TauJetRNNTargetEff", [ [0.98,  0.90, 0.65,  0.50],    # 0p WPs: VL, L, M, T
                                                              [0.992, 0.99, 0.97,  0.94],    # 1p WPs: VL, L, M, T
                                                              [0.99,  0.94, 0.895, 0.80] ] ) # mp WPs: VL, L, M, T

    flags.addFlag("Trigger.Offline.Tau.TauJetRNNConfigLLP", ['llpdev/net_experimental_llz_0p.json',
                                                             'llpdev/net_experimental_llz_1p.json',
                                                             'llpdev/net_experimental_llz_mp.json'])

    flags.addFlag("Trigger.Offline.Tau.TauJetRNNWPConfigLLP", ['llpdev/rnnid_flat_llp_llz0p_050621-v1.root',
                                                               'llpdev/rnnid_flat_llp_llz1p_050621-v1.root',
                                                               'llpdev/rnnid_flat_llp_llzmp_050621-v1.root'])

    flags.addFlag("Trigger.Offline.Tau.TauJetRNNLLPTargetEff", [ [0.98,  0.90, 0.65,  0.50],    # 0p WPs: VL, L, M, T
                                                                 [0.992, 0.99, 0.965, 0.94],    # 1p WPs: VL, L, M, T
                                                                 [0.99,  0.98, 0.865, 0.80] ] ) # mp WPs: VL, L, M, T


    return flags


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2

    flags.lock()
    flags.Tau.CalibrateLCConfig
    flags.Trigger.doLVL1
    flags.dump("Tau|Trigger")

    assert flags.Tau.CalibrateLCConfig != flags.Trigger.Offline.Tau.CalibrateLCConfig, "No difference between trigger customization"
    flags.dump("Tau|Trigger")
