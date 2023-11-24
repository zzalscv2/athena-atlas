# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import math

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoring import AthMonitorCfgHelper
from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
from AthenaCommon.Logging import logging

from .utils import getMinBiasChains

log = logging.getLogger('TrigFwdAFPMonitoring')

jet_containers = {
    'Topo': 'AntiKt4EMTopoJets',
    'PFlow': 'AntiKt4EMPFlowJets',
    'HLTTopo': 'HLT_AntiKt4EMTopoJets_subjesIS',
    'HLTPFlow': 'HLT_AntiKt4EMPFlowJets_subjesIS_ftf'
}


def TrigFwdAFPMonitoringCfg(flags):
    """ Configure general AFP chains monitoring algs """

    monConfig = AthMonitorCfgHelper(flags, 'FwdAFPMonitoringAlgs')

    monAccess = getHLTMonitoringAccess(flags)
    afp_chains = getMinBiasChains(monAccess, '(AFP|afp)')

    # Counting alg for all AFP triggers
    algCount = monConfig.addAlgorithm(CompFactory.FwdAFPCountMonitoringAlg, 'FwdAFPCountMonitoringAlg')
    algCount.chains = [name for name, _ in afp_chains]

    log.info(f'Monitoring {len(afp_chains)} AFP chains')

    afpCountGroup = monConfig.addGroup(algCount, 'AFPCount', topPath='HLT/FwdAFP/')
    afpCountGroup.defineHistogram('counts', title='Trigger counts;;Counts', xbins=len(algCount.chains),
                                  xmin=0, xmax=len(algCount.chains), xlabels=list(algCount.chains))

    return monConfig.result()


def TrigFwdAFPJetMonitoringCfg(flags):
    """ Configure AFP+jet chains monitoring algs """

    monConfig = AthMonitorCfgHelper(flags, 'FwdAFPJetMonitoringAlgs')
    monAccess = getHLTMonitoringAccess(flags)

    # Select non-noalg AFP chains seeded from L1 jet
    chains_afp = getMinBiasChains(monAccess, '(afpdijet)')
    ref_chains = ['HLT_mb_sp_L1RD0_FILLED', 'HLT_mb_sptrk_L1RD0_FILLED', 'HLT_noalg_L1RD0_FILLED',
                  'HLT_noalg_L1AFP_A_OR_C', 'HLT_noalg_L1AFP_A_OR_C']

    log.info(f'Monitoring {len(chains_afp)} AFP+DiJet chains')
    log.debug([name for name, _ in chains_afp])

    # Jet monitoring algs for different jet containers
    for jet, container in jet_containers.items():
        algEff = monConfig.addAlgorithm(CompFactory.FwdAFPJetMonitoringAlg, 'FwdAFP' + jet + 'JetMonitoringAlg')

        algEff.chains = [name for name, _ in chains_afp]
        algEff.jetContainer = container

        for chain, level in chains_afp:
            afpJetGroup = monConfig.addGroup(algEff, f'{chain}_{container}', topPath=f'HLT/FwdAFP/{level}/Jet/{chain}/{container}/')
            afpJetGroup.defineHistogram('jetPt', title=f'{jet} jet pT;Jet pT [GeV];Entries', xbins=100, xmin=0, xmax=200)
            afpJetGroup.defineHistogram('jetEta', title=f'{jet} jet eta;Jet #eta;Entries', xbins=100, xmin=-4.9, xmax=4.9)
            afpJetGroup.defineHistogram('jetPhi', title=f'{jet} jet phi;Jet #varphi;Entries', xbins=100, xmin=-math.pi, xmax=math.pi)
            afpJetGroup.defineHistogram('jetEta,jetPhi', type='TH2F', title=f'{jet} jet eta vs phi;Jet #eta;Jet #varphi;Entries',
                                        xbins=40, xmin=-4.9, xmax=4.9, ybins=40, ymin=-math.pi, ymax=math.pi)
            afpJetGroup.defineHistogram('jetEta,jetPt', type='TH2F', title=f'{jet} jet eta vs phi;Jet #eta;Jet pT [GeV];Entries',
                                        xbins=40, xmin=-4.9, xmax=4.9, ybins=40, ymin=0, ymax=200)

            afpJetGroup.defineHistogram('leadingJetPt', title=f'Leading {jet} jet pT;Leading jet pT [GeV];Entries',
                                        xbins=100, xmin=0, xmax=200)
            afpJetGroup.defineHistogram('leadingJetEta', title=f'Leading {jet} jet eta;Leading jet #eta;Entries',
                                        xbins=100, xmin=-4.9, xmax=4.9)
            afpJetGroup.defineHistogram('leadingJetPhi', title=f'Leading {jet} jet phi;Leading jet #varphi;Entries',
                                        xbins=100, xmin=-math.pi, xmax=math.pi)
            afpJetGroup.defineHistogram('leadingJetEta,leadingJetPhi', type='TH2F',
                                        title=f'Leading {jet} jet eta vs phi;Leading jet #eta;Leading jet #varphi;Entries',
                                        xbins=40, xmin=-4.9, xmax=4.9, ybins=40, ymin=-math.pi, ymax=math.pi)
            afpJetGroup.defineHistogram('leadingJetEta,leadingJetPt', type='TH2F',
                                        title=f'Leading {jet} jet eta vs phi;Leading jet #eta;Leading jet pT [GeV];Entries',
                                        xbins=40, xmin=-4.9, xmax=4.9, ybins=40, ymin=0, ymax=200)

    # Efficiency alg
    algEff = monConfig.addAlgorithm(CompFactory.FwdAFPJetEffMonitoringAlg, 'FwdAFPJetEffMonitoringAlg')
    algEff.chains = [name for name, _l in chains_afp for _r in range(len(ref_chains))]
    algEff.references = ref_chains * len(chains_afp)

    for chain, level in chains_afp:
        for ref in ref_chains:
            afpJetEffGroup = monConfig.addGroup(algEff, f'{chain}_{ref}', topPath=f'HLT/FwdAFP/{level}/Jet/Eff/')
            afpJetEffGroup.defineHistogram(f'effPassed,leadingJetPt;{chain}_vs_{ref}', type='TEfficiency',
                                           title=f'{chain} vs {ref};Leading jet pT [GeV];Efficiency', xbins=100, xmin=0, xmax=200)

    return monConfig.result()


def TrigFwdAFPAllMonitoringCfg(flags):
    """ Collect all configured AFP chains algorithms """

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    acc.merge(TrigFwdAFPMonitoringCfg(flags))
    acc.merge(TrigFwdAFPJetMonitoringCfg(flags))

    return acc


if __name__ == '__main__':
    """ Test AFP+jet monitoring by running: python -m TrigMinBiasMonitoring.TrigFwdAFPMonitoring --filesInput=... """

    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Output.HISTFileName = 'TestFwdAfpMonitorOutput.root'
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    cfg.merge(TrigFwdAFPAllMonitoringCfg(flags))

    cfg.printConfig(withDetails=True)
    with open('cfg.pkl', 'wb') as f:
        cfg.store(f)

    cfg.run()
