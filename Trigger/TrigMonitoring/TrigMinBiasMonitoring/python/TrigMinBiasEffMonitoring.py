#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''
@brief configuration for the trigger efficiency monitoring
'''
from TrigConfigSvc.TriggerConfigAccess import getHLTMenuAccess
from AthenaConfiguration.ComponentFactory import CompFactory


def _TrigEff(flags, triggerAndRef, algname='HLTMinBiasEffMonitoringAlg'):
    from AthenaMonitoring import AthMonitorCfgHelper
    monConfig = AthMonitorCfgHelper(flags, algname)

    alg = monConfig.addAlgorithm(
        CompFactory.HLTMinBiasEffMonitoringAlg, algname)

    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_LoosePrimary_Cfg
    trkSel = monConfig.resobj.popToolsAndMerge(InDetTrackSelectionTool_LoosePrimary_Cfg(flags))
    alg.TrackSelectionTool = trkSel

    alg.triggerList = [ el["chain"] for el in  triggerAndRef ]
    alg.refTriggerList = [ el["refchain"] for el in  triggerAndRef ]

    length = len(alg.triggerList)

    mainGroup = monConfig.addGroup(alg, 'TrigAll', topPath='HLT/MinBiasMon/Counts/')

    alreadyConfigured = set()
    for cdef in triggerAndRef:
        chain = cdef['chain']
        refchain = cdef['refchain']
        xmin  = cdef['xmin']
        xmax  = cdef['xmax']
        xbins = xmax-xmin
        effGroup = monConfig.addGroup(alg, chain+refchain, topPath='HLT/MinBiasMon/EffAll/')

        whichcounter='nTrkOffline'
        # if the chain cuts on higher pt (there is a few predefined chains) use different counter
        if '_sptrk_pt' in chain:
            effGroup.defineHistogram(f'EffPassed,leadingTrackPt;{chain}_ref_{refchain}_pt', type='TEfficiency',
                                      title=chain+';Leading track pt;Efficiency', xbins=50, xmin=0.0, xmax=10)

            # these chains have such name pattern: HLT_mb_sptrk_pt2_L1MBTS_2
            whichcounter += '_'+chain.split('_')[3]
        elif '_excl_' in chain:
            effGroup.defineHistogram(f'EffPassed,nTrkOffline;{chain}_ref_{refchain}_exclusivity', type='TEfficiency',
                                                 title=chain+';Offline Good nTrk (low pt);Efficiency', xbins=30, xmin=-0.5, xmax=30-0.5)
            effGroup.defineHistogram(f'EffPassed,leadingTrackPt;{chain}_ref_{refchain}_pt', type='TEfficiency',
                                                  title=chain+';Leading track pt;Efficiency', xbins=50, xmin=0.0, xmax=10)
            # these chains have such form: HLT_mb_excl_1trk5_pt4_L1RD0_FILLED
            whichcounter += '_'+chain.split('_')[4]

        if '_pusup' in chain or '_hmt_' in chain:
            whichcounter = 'nTrkOfflineVtx'
        effGroup.defineHistogram(f'EffPassed,{whichcounter};{chain}_ref_{refchain}', type='TEfficiency',
                                    title=f'{chain} ref: {refchain} ;Offline Good nTrk {whichcounter};Efficiency', xbins=xbins, xmin=xmin, xmax=xmax)



        if chain not in alreadyConfigured:
            alreadyConfigured.add(chain)
            # need this protection because we can measure efficiency with several reference trigger, but want counts irrespective of ref. triggers
            mainGroup.defineHistogram('nTrkOffline_counts_' + chain, type='TH1F',
                                      title=chain+';Offline Good nTrk;Events', xbins=xmax-xmin, xmin=xmin, xmax=xmax)

    mainGroup.defineHistogram('TrigCounts', title='Trigger counts;;Event rate',
                              xbins=length, xmin=0, xmax=len(alreadyConfigured), xlabels=list(alreadyConfigured))

    return monConfig.result()


def TrigMinBiasEff(flags):
    allChains = getHLTMenuAccess(flags)
    mbChains = [c for c in allChains if '_mb_' in c]
    if len(mbChains) == 0:
        return _TrigEff(flags, [])

    # here we generate config with detailed settings
    def _c(chain, refchain, **kwargs):
        conf = {"chain": chain, "refchain": refchain, "xmin": 0, "xmax": 20}
        conf.update(kwargs)
        return conf

    def _isMBSPTRK(chain):
        return "HLT_mb_sptrk_" in chain or "HLT_mb_sp_" in chain or "HLT_mb_mbts_" in chain

    def _isFilled(chain):
        return "_EMPTY" not in chain and "_UNPAIRED_ISO" not in chain

    mbsptrkChains = [chain for chain in mbChains if _isMBSPTRK(chain)]

    filledChains = [chain for chain in mbsptrkChains if _isFilled(chain)]
    emptyChains = [chain for chain in mbsptrkChains if "_EMPTY" in chain]
    unpairedChains = [chain for chain in mbsptrkChains if "_UNPAIRED_ISO" in chain]

    triggerAndRef = []

    # check all mb_sptrk chains w.r.t. random noalg
    triggerAndRef += [_c(chain, "HLT_noalg_L1RD0_FILLED") for chain in filledChains]
    triggerAndRef += [_c(chain, "HLT_noalg_L1RD0_EMPTY") for chain in emptyChains]
    triggerAndRef += [_c(chain, "HLT_noalg_L1RD0_UNPAIRED_ISO") for chain in unpairedChains]

    # for monitoring in MB stream
    triggerAndRef += [_c(chain, "HLT_noalg_mb_L1RD0_FILLED") for chain in filledChains]
    triggerAndRef += [_c(chain, "HLT_noalg_mb_L1RD0_EMPTY") for chain in emptyChains]
    triggerAndRef += [_c(chain, "HLT_noalg_mb_L1RD0_UNPAIRED_ISO") for chain in unpairedChains]

    # sptrk vs sp
    triggerAndRef += [_c("HLT_mb_sptrk_L1RD0_FILLED", "HLT_mb_sp_L1RD0_FILLED")]

    hmt = [c for c in mbChains if ('_hmt_' in c and '_pusup' not in c)]
    if len(hmt) != 0:
        # sort by trk threshold
        def _trk(chain):
            part = chain.split("_")
            for el in part:
                if el.startswith("trk"):
                    return int(el.strip("trk"))
            raise RuntimeError(f"Chain {chain} is not the hmt chain")

        hmt.sort(key=lambda c: int(_trk(c)))

        # monitor first hmt w.r.t sptrk
        triggerAndRef += [_c(hmt[0], "HLT_mb_sptrk_L1RD0_FILLED", xmax=_trk(hmt[0])+30)]

        # group set the ref for each trigger to be one of lower threshold : ordering of chains needs to be reviewed
        # triggerAndRef += [  _c(chain, ref, xmin=_trk(chain)-20, xmax=_trk(chain)+50) for chain,ref in zip(hmt[1:], hmt) ]
        triggerAndRef += [_c(chain, "HLT_mb_sptrk_L1RD0_FILLED", xmin=_trk(chain)-20, xmax=_trk(chain)+50) for chain in hmt[1:]]

        # pu suppressing trigger should be monitored using trigger of the same threshold w/o pu suppression
        pusup = [c for c in mbChains if '_hmt_' in c and '_pusup' in c]

        def _dropsup(chain):
            s = chain.split("_")
            return "_".join(s[:3]+s[4:])
        triggerAndRef += [_c(chain, _dropsup(chain),  xmin=_trk(chain)-20, xmax=_trk(chain)+50) for chain in pusup]

    # monitor exclusivity cut
    excl = [c for c in mbChains if ('_excl_' in c)]
    triggerAndRef += [_c(chain, 'HLT_mb_sptrk_L1RD0_FILLED') for chain in excl]
    triggerAndRef += [_c(chain, 'HLT_mb_sp_L1RD0_FILLED') for chain in excl]

    # monitor noalg MBTS chains
    mbtsNoAlg = [c for c in mbChains if 'noalg' in c and 'L1MBTS' in c]
    triggerAndRef += [_c(chain, 'HLT_mb_sptrk_L1RD0_FILLED') for chain in mbtsNoAlg]

    # L1 MBTS
    mbts = ["L1_MBTS_A", "L1_MBTS_C", "L1_MBTS_1", "L1_MBTS_2", "L1_MBTS_1_1"]
    triggerAndRef += [_c(chain, 'HLT_mb_sptrk_L1RD0_FILLED') for chain in mbts]

    # L1 transverse energy
    triggerAndRef += [_c("L1_TE{}".format(i), 'HLT_mb_sptrk_L1RD0_FILLED', xmin=0, xmax=100) for i in [3, 5, 10, 40]]

    # Pair HLT chain with its noalg version
    def _find_noalg(chain):
        pos = chain.find('L1')
        return 'HLT_noalg_' + chain[pos:]

    def _chains_with_noalg_ref():
        chains = []

        for c in filledChains:
            # Already added
            if 'L1RD0_FILLED' in c:
                continue

            noalg = _find_noalg(c)
            if noalg not in allChains:
                continue

            chains.append(_c(c, noalg))

        return chains

    triggerAndRef += _chains_with_noalg_ref()

    # HI chains
    hiChains = [c for c in mbChains if '_hi_' in c]

    # Pair MB+HI chains with MB only reference if it exists
    def _hi_chain_with_mb_ref():
        chains = []

        for c in hiChains:
            split = c.split('_hi_')
            if len(split) != 2:
                continue
            mb, hi_l1 = split

            split = hi_l1.split('L1')
            if len(split) != 2:
                continue
            hi, l1 = split

            ref = f'{mb}_L1{l1}'

            if ref not in mbChains:
                continue

            chains.append(_c(c, ref))

        return chains

    triggerAndRef += _hi_chain_with_mb_ref()

    # Add here all the special cases:
    # HI Fgap chains
    triggerAndRef.append(_c('HLT_mb_excl_1trk4_pt1_hi_FgapAC5_L12eEM1_VjTE200', 'HLT_mb_excl_1trk4_pt1_L12eEM1_VjTE200_GAP_AANDC'))

    return _TrigEff(flags, triggerAndRef)


if __name__ == '__main__':
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Output.HISTFileName = 'TestMinBiasMonitorOutput.root'
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    cfg.merge(TrigMinBiasEff(flags))


    #    cfg.getEventAlgo('HLTMinBiasEffMonitoringAlg').OutputLevel = DEBUG  # DEBUG
    cfg.printConfig(withDetails=True)  # set True for exhaustive info
    with open("cfg.pkl", "wb") as f:
        cfg.store(f)

    cfg.run()
    # to run:
    # python -m TrigMinBiasMonitoring.TrigMinBiasEffMonitoring --filesInput=filepath --evtMax=XYZ
