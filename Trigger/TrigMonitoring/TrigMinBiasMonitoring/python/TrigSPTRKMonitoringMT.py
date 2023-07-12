#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

"""
@brief configuration for the min bias monitoring
"""
import math


def SPTRK_chains(configFlags):
    """ Add MinBias chains """
    from TrigConfigSvc.TriggerConfigAccess import getHLTMenuAccess
    allChains = getHLTMenuAccess(configFlags)

    tokens = ['_mb_sp_', '_mb_sptrk_', '_mb_excl_', '_hmt_']

    def isMinBias(chain):
        return any([t in chain for t in tokens])

    chains = [c for c in allChains if isMinBias(c)]

    return list(set(chains))  # Remove duplicates


def TrigSPTRK(configFlags, highGranularity=False):

    from AthenaMonitoring import AthMonitorCfgHelper

    monConfig = AthMonitorCfgHelper(configFlags, "HLTMBSPTRKMonAlg")

    from AthenaConfiguration.ComponentFactory import CompFactory

    alg = monConfig.addAlgorithm(CompFactory.HLTMinBiasTrkMonAlg, "HLTMBSPTRKMonAlg")

    from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_LoosePrimary_Cfg
    trkSel = monConfig.resobj.popToolsAndMerge(InDetTrackSelectionTool_LoosePrimary_Cfg(configFlags))
    alg.TrackSelectionTool = trkSel

    ZFinderCollection = 'HLT_vtx_z'
    if ZFinderCollection in configFlags.Input.Collections:
        print("Enabled z finder data reading")
        alg.zFinderDataKey = ZFinderCollection

    from TrigConfigSvc.TriggerConfigAccess import getHLTMenuAccess

    alg.triggerListTrackingMon = SPTRK_chains(configFlags)

    allChains = getHLTMenuAccess(configFlags)
    detailed = ["HLT_mb_sptrk_L1RD0_FILLED", "HLT_mb_sp_L1RD0_FILLED"]
    detailed += [c for c in allChains if 'HLT_mb_sp_pix' in c]
    detailed += [c for c in allChains if 'HLT_mb_sp_vpix' in c]
    detailed += [c for c in allChains if 'HLT_mb_sp' in c and '_hmt_' in c]
    alg.triggerListSpacePointsMon = detailed

    for chain in alg.triggerListTrackingMon:
        mbEffGroup = monConfig.addGroup(alg, chain + "_Tracking", topPath="HLT/MinBiasMon/Tracking/" + chain + "/")
        if chain in detailed:
            mbEffGroup.defineHistogram( "nTrkOffline;nTrkOfflineLowMult", title="Number of tracks reconstructed offline;track counts", xbins=50, xmin=-1, xmax=50 )

            mbEffGroup.defineHistogram( "nTrkOffline", title="Number of tracks reconstructed offline;track counts", xbins=200, xmin=-1, xmax=400 )
            mbEffGroup.defineHistogram( "nTrkOnline;nTrkOnlineLowMult", title="Number of tracks reconstructed online;track counts", xbins=50, xmin=-1, xmax=50 )
            nbins = 400 if highGranularity else 100
            mbEffGroup.defineHistogram( "nTrkOnline,nTrkOffline", type="TH2F", title=";N online tracks;N offline tracks", xbins=nbins, xmin=0, xmax=400, ybins=nbins, ymin=0, ymax=400 )
            mbEffGroup.defineHistogram( "nTrkOfflineVtx,nTrkOffline", type="TH2F", title=";N offline tracks with Vtx cut;N offline tracks", xbins=nbins, xmin=0, xmax=400, ybins=nbins, ymin=0, ymax=400 )
            mbEffGroup.defineHistogram( "nTrkOnline,zFinderWeight", type="TH2F", title=";N online tracks;ZFinder weight", xbins=nbins, xmin=0, xmax=400, ybins=nbins, ymin=0, ymax=400 )
            mbEffGroup.defineHistogram( "nTrkOffline,zFinderWeight", type="TH2F", title=";N online tracks;ZFinder weight", xbins=nbins, xmin=0, xmax=400, ybins=nbins, ymin=0, ymax=400 )
            mbEffGroup.defineHistogram( "nTrkOfflineVtx,zFinderWeight", type="TH2F", title=";N offline tracks with Vtx cut;ZFinder weight", xbins=nbins, xmin=0, xmax=400, ybins=nbins, ymin=0, ymax=400 )
            mbEffGroup.defineHistogram( "nTrkOfflineVtx,nTrkOnlineVtx", type="TH2F", title=";N offline tracks with Vtx cut;N online tracks with Vtx cut;", xbins=nbins, xmin=0, xmax=400, ybins=nbins, ymin=0, ymax=400 )
            mbEffGroup.defineHistogram( "onlineOfflineVtxDelta", title=";(offline - online) vertex z[mm]", xbins=200, xmin=-200, xmax=200 )

            mbEffGroup.defineHistogram( "nTrkOnline", title="Number of tracks reconstructed online;track counts", xbins=200, xmin=-1, xmax=400 )
            mbEffGroup.defineHistogram( "nTrkRatio", title="Number of tracks reconstructed online/offline;track counts online/offline", xbins=200, xmin=-1, xmax=4 )
            mbEffGroup.defineHistogram( "trkSelOfflineRatio", title="Number of tracks reconstructed offline(selected)/offline; N sel/all", xbins=200, xmin=0.1, xmax=1.9 )

            nbins = 400 if highGranularity else 50
            mbEffGroup.defineHistogram( "SctTot,nTrkOffline", type="TH2F", title=";Number of SP in whole SCT detector for all events;N offline tracks", xbins=nbins, xmin=0, xmax=4000, ybins=nbins, ymin=0, ymax=400 )
            mbEffGroup.defineHistogram( "SctTot,nTrkOfflineVtx", type="TH2F", title=";Number of SP in whole SCT detector for all events;N offline tracks wiht Vtx cut", xbins=nbins, xmin=0, xmax=4000, ybins=nbins, ymin=0, ymax=400 )
            mbEffGroup.defineHistogram( "SctTot,nTrkOnline", type="TH2F",  title=";Number of SP in whole SCT detector for all events;N online tracks", xbins=nbins, xmin=0, xmax=4000, ybins=nbins, ymin=0, ymax=400 )
            mbEffGroup.defineHistogram( "PixelCL,nTrkOffline", type="TH2F",  title=";Number of SP in whole Pixels detector for all events;N offline tracks", xbins=100, xmin=0, xmax=4000, ybins=100, ymin=0, ymax=400 )
            mbEffGroup.defineHistogram( "PixelCL,nTrkOnline", type="TH2F",  title=";Number of SP in whole Pixels detector for all events;N online tracks", xbins=100, xmin=0, xmax=4000, ybins=100, ymin=0, ymax=400 )

            mbEffGroup.defineHistogram( "L1sumEt", title=";L1 Total ET [GeV] ;Events", xbins=52, xmin=-2, xmax=50 )
            mbEffGroup.defineHistogram( "nTrkOffline,L1sumEt", type="TH2F", title=";Offline Ntrk;L1 Total ET [GeV]", xbins=100, xmin=-1, xmax=200, ybins=52, ymin=-2, ymax=50   )
            mbEffGroup.defineHistogram( "nTrkOnline,L1sumEt", type="TH2F", title=";Online Ntrk;L1 Total ET [GeV]",   xbins=100, xmin=-1, xmax=200, ybins=52, ymin=-2, ymax=50   )
            mbEffGroup.defineHistogram( "SctTot,L1sumEt", type="TH2F", title=";Number of SP in whole SCT detector for all events;L1 Total ET [GeV]", xbins=100, xmin=0, xmax=4000, ybins=52, ymin=-2, ymax=50   )
            mbEffGroup.defineHistogram( "PixelCL,L1sumEt", type="TH2F", title=";Number of SP in whole Pixels detector for all events;L1 Total ET [GeV]", xbins=100, xmin=0, xmax=4000, ybins=52, ymin=-2, ymax=50   )

            mbEffGroup.defineHistogram( "countsOnlineNames,countsOnline;onlineCounters", type="TProfile", title=";cuts;counts/evt", xbins=10, xmin=0, xmax=10, ybins=10, ymin=0, ymax=10)

        # 1D distributions
        mbEffGroup.defineHistogram( "trkPt", cutmask="trkMask", title="Offline selected tracks pt;p_{T} [GeV]", xbins=100, xmin=0, xmax=5)
        mbEffGroup.defineHistogram( "trkEta", cutmask="trkMask", title="Offline selected tracks eta;#eta", xbins=50, xmin=-2.5, xmax=2.5)
        mbEffGroup.defineHistogram( "trkPhi", cutmask="trkMask", title="Offline selected tracks phi;#phi", xbins=64, xmin=-math.pi, xmax=math.pi)
        mbEffGroup.defineHistogram( "trkHits", title="Offline selected tracks, hits per track;number of hits", xbins=15, xmin=-0.5, xmax=15-0.5)

        mbEffGroup.defineHistogram( "onlTrkPt", title="Online tracks pt;p_{T} [GeV]", xbins=100, xmin=0, xmax=5)
        mbEffGroup.defineHistogram( "onlTrkEta", title="Online tracks eta;#eta", xbins=50, xmin=-2.5, xmax=2.5)
        mbEffGroup.defineHistogram( "onlTrkPhi", title="Online tracks phi;#phi", xbins=64, xmin=-math.pi, xmax=math.pi)
        mbEffGroup.defineHistogram( "onlTrkHits", title="Online hits per track;number of hits", xbins=15, xmin=-0.5, xmax=15-0.5)
        mbEffGroup.defineHistogram( "onlTrkZ0", title="Online track z_{0};z_{0}[mm]", xbins=40, xmin=-200, xmax=200)
        mbEffGroup.defineHistogram( "onlTrkD0", title="Online track d_{0};d_{0}[mm]", xbins=40, xmin=-20, xmax=20)

        mbEffGroup.defineHistogram( "trkD0", cutmask="trkMask", title="Offline selected tracks D0;d_{0} [mm]", xbins=40, xmin=-20, xmax=20)
        mbEffGroup.defineHistogram( "trkZ0wrtPV", cutmask="trkMask", title="Offline selected tracks Z0 wrt PV;z_{0}[mm]", xbins=40, xmin=-20, xmax=20)
        mbEffGroup.defineHistogram( "trkZ0", cutmask="trkMask", title="Offline selected tracks Z0;z_{0}[mm]", xbins=40, xmin=-200, xmax=200)

        # 2D maps
        mbEffGroup.defineHistogram('trkEta,trkPt', cutmask='trkMask', type='TH2F', title='Offline selected tracks pT/eta correlation;#eta;p_{T} [GeV]', xbins=50, xmin=-2.5, xmax=2.5, ybins=50, ymin=0, ymax=5)
        mbEffGroup.defineHistogram('onlTrkEta,onlTrkPt', type='TH2F', title='Online tracks pT/eta correlation;#eta;p_{T} [GeV]', xbins=50, xmin=-2.5, xmax=2.5, ybins=50, ymin=0, ymax=5)

        mbEffGroup.defineHistogram('trkEta,trkPhi', cutmask='trkMask', type='TH2F', title='Offline selected tracks eta/phi correlation;#eta;#varphi', xbins=50, xmin=-2.5, xmax=2.5, ybins=50, ymin=-math.pi, ymax=math.pi)
        mbEffGroup.defineHistogram('onlTrkEta,onlTrkPhi', type='TH2F', title='Online tracks eta/phi correlation;#eta;#varphi', xbins=50, xmin=-2.5, xmax=2.5, ybins=50, ymin=-math.pi, ymax=math.pi)

    for chain in alg.triggerListSpacePointsMon:
        mbSpGroup = monConfig.addGroup(
            alg,
            chain + "_SpacePoints",
            topPath="HLT/MinBiasMon/SpacePoints/" + chain + "/",
        )
        mbSpGroup.defineHistogram( "PixelCL;PixelCLNarrowRange", title="Number of SP in whole Pixels detector for all events", xbins=200, xmin=0, xmax=100 )
        mbSpGroup.defineHistogram( "PixelCL;PixelCLWideRange", title="Number of SP in whole Pixels detector for all events", xbins=250, xmin=0, xmax=5000 )
        mbSpGroup.defineHistogram( "PixBarr_SP", title="Number of SP for all events in Barrel", xbins=250, xmin=0, xmax=5000 )
        mbSpGroup.defineHistogram( "PixECA_SP", title="Number of SP for all events in ECA", xbins=250, xmin=0, xmax=500 )
        mbSpGroup.defineHistogram( "PixECC_SP", title="Number of SP for all events in ECC", xbins=250, xmin=0, xmax=500 )
        mbSpGroup.defineHistogram( "SctTot", title="Number of SP in whole SCT detector for all events", xbins=250, xmin=0, xmax=5000 )
        mbSpGroup.defineHistogram( "SctBarr_SP", title="Number of SCT_SP for all events in Barrel", xbins=250, xmin=0, xmax=5000 )
        mbSpGroup.defineHistogram( "SctECA_SP", title="Number of SCT_SP for all events in ECA", xbins=250, xmin=0, xmax=5000 )
        mbSpGroup.defineHistogram( "SctECC_SP", title="Number of SCT_SP for all events in ECC", xbins=250, xmin=0, xmax=5000 )
        # expert plots
        nbins = 400 if highGranularity else 50
        mbSpGroup.defineHistogram( "SctECA_SP,SctECC_SP", type="TH2F", title=";SctECA_SP;SctECC_SP", xbins=nbins, xmin=0, xmax=4000, ybins=nbins, ymin=0, ymax=4000 )
        mbSpGroup.defineHistogram( "PixECA_SP,PixECC_SP", type="TH2F", title=";PixECA_SP;PixECC_SP", xbins=nbins, xmin=0, xmax=1000, ybins=nbins, ymin=0, ymax=1000 )
        mbSpGroup.defineHistogram( "SctBarr_SP,PixBarr_SP", type="TH2F", title=";SctBarr_SP;PixBarr_SP", xbins=nbins, xmin=0, xmax=4000, ybins=nbins, ymin=0, ymax=4000 )
        mbSpGroup.defineHistogram( "SctECA_SP,PixECA_SP", type="TH2F", title=";SctECA_SP;PixECA_SP", xbins=nbins, xmin=0, xmax=4000, ybins=nbins, ymin=0, ymax=1000 )
        mbSpGroup.defineHistogram( "SctECC_SP,PixECC_SP", type="TH2F", title=";SctECC_SP;PixECC_SP", xbins=nbins, xmin=0, xmax=6000, ybins=nbins, ymin=0, ymax=4000 )
        mbSpGroup.defineHistogram( "SctTot,PixelCL", type="TH2F", title=";Number of SP in whole SCT detector for all events;Number of SP in whole Pixels detector for all events", xbins=nbins, xmin=0, xmax=4000, ybins=nbins, ymin=0, ymax=4000 )

    return monConfig.result()


if __name__ == "__main__":
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.DQ.Environment = "AOD"
    flags.Concurrency.NumConcurrentEvents = 5

    flags.Output.HISTFileName = "TestMonitorOutput.root"
    flags.fillFromArgs()
    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)
    log.info("Input %s", str(flags.Input.Files))
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    cfg = MainServicesCfg(flags)

    cfg.merge(PoolReadCfg(flags))
    cfg.merge(TrigSPTRK(flags, highGranularity=True)) # for local testing have very granular histograms

    # If you want to turn on more detailed messages ...
    #    from AthenaCommon.Constants import DEBUG
    #    cfg.getEventAlgo("HLTMBSPTRKMonAlg").OutputLevel = DEBUG
    cfg.printConfig(withDetails=True)  # set True for exhaustive info
    with open("cfg.pkl", "wb") as f:
        cfg.store(f)

    cfg.run()  # use cfg.run(20) to only run on first 20 events
    # to run:
    # python -m TrigMinBiasMonitoring.TrigMinBiasMonitoringMT --filesInput=
