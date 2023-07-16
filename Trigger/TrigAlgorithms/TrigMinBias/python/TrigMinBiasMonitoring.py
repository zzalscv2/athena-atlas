# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def SpCountMonitoring(flags):

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram('pixCL_1', path='EXPERT', type='TH1I', title='pixCL_1',xbins=100, xmin=0, xmax=6000)
    monTool.defineHistogram('pixCLBeforeCuts', path='EXPERT', type='TH1I', title='totPixBeforeCuts', xbins = 250, xmin=0, xmax=6000)
    monTool.defineHistogram('pixCL_2', path='EXPERT', type='TH1I', title='pixCL_2',xbins=100, xmin=0, xmax=6000)
    monTool.defineHistogram('pixCLmin3', path='EXPERT', type='TH1I', title='pixCLmin3',xbins=100, xmin=0, xmax=6000)
    monTool.defineHistogram('pixCL', path='EXPERT', type='TH1I', title='pixCL', xbins = 500, xmin=0, xmax=6000)
    monTool.defineHistogram('pixCLnoToT', path='EXPERT', type='TH1I', title='pixCLnoToT', xbins = 500, xmin=0, xmax=6000)
    monTool.defineHistogram('pixCLBarrel', path='EXPERT', type='TH1I', title='pixClBarrel', xbins = 500, xmin=0, xmax=6000)
    monTool.defineHistogram('pixCLEndcapA', path='EXPERT', type='TH1I', title='pixClEndcapA', xbins = 500, xmin=0, xmax=3000)
    monTool.defineHistogram('pixCLEndcapC', path='EXPERT', type='TH1I', title='pixClEndcapC', xbins = 500, xmin=0, xmax=3000)
    monTool.defineHistogram('pixModulesOverThreshold', path='EXPERT', type='TH1I', title='Pixels ModulesOverThreshold', xbins = 100, xmin=1, xmax=200)
    monTool.defineHistogram('sctSP', path='EXPERT', type='TH1I', title='sctSP', xbins = 500, xmin=0, xmax=6000)
    monTool.defineHistogram('sctSPEndcapC', path='EXPERT', type='TH1I', title='sctSPEndcapC', xbins = 500, xmin=0, xmax=3000)
    monTool.defineHistogram('sctSPBarrel', path='EXPERT', type='TH1I', title='sctSPBarrel', xbins = 500, xmin=0, xmax=6000)
    monTool.defineHistogram('sctSPEndcapA', path='EXPERT', type='TH1I', title='sctSPEndcapA', xbins = 500, xmin=0, xmax=3000)
    monTool.defineHistogram('pixCL, sctSP', path='EXPERT', type='TH2I', title='SP ; pix SP after ToT cut; sct SP', xbins = 50, xmin=0, xmax=6000, ybins = 50, ymin=0, ymax=6000)
    monTool.defineHistogram('sctModulesOverThreshold', path='EXPERT', type='TH1I', title='SCT ModulesOverThreshold', xbins = 100, xmin=1, xmax=200)

    return monTool

__MBTSXTitle="channel (0-15 A side, 16-31 C side)"
def MbtsFexMonitoring(flags):
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags,'MonTool')
    monTool.defineHistogram('triggerEnergies', path='EXPERT', type='TH1D', title='triggerEnergies',xbins=50, xmin=-5, xmax=45)
    monTool.defineHistogram('triggerTimes', path='EXPERT', type='TH1I', title='triggerTimes',xbins=100, xmin=-50, xmax=50)
    monTool.defineHistogram('channelID, triggerEnergies', path='EXPERT', title=f'signal per channel;{__MBTSXTitle}; energy[pC]', type='TH2F', xbins=32, xmin=-0.5, xmax=31.5, ybins=20, ymin=-5,ymax=45)
    monTool.defineHistogram('channelID, triggerEnergies;triggerEnergies_vs_channelID_zoom', path='EXPERT', title=f'signal per channel;{__MBTSXTitle}; energy[pC]', type='TH2F', xbins=32, xmin=-0.5, xmax=31.5, ybins=50, ymin=-2, ymax=3)
    monTool.defineHistogram('channelID, triggerTimes', path='EXPERT', title=f'times per channel;{__MBTSXTitle}; time[ns]', type='TH2F', xbins=32, xmin=-0.5, xmax=31.5, ybins=20, ymin=-50, ymax=50)
    monTool.defineHistogram('channelID, triggerTimes;triggerTimes_vs_channelID_zoom', path='EXPERT', title=f'times per channel;{__MBTSXTitle}; time[ns]', type='TH2F', xbins=32, xmin=-0.5, xmax=31.5, ybins=20, ymin=-5, ymax=5)
    monTool.defineHistogram('timeDelta', path='EXPERT', type='TH1F', title='MBTS time delta;time[ns]', xbins=80, xmin=-40, xmax=40)

    return monTool

def MbtsHypoToolMonitoring(flags):
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram('Counts', path='EXPERT', title=f'MBTS counts per channel;{__MBTSXTitle};counts', type='TH1F', xbins=32, xmin=-0.5, xmax=31.5)

    return monTool

def TrackCountMonitoring(flags, hypoAlg):
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram('ntrks', path='EXPERT', type='TH1I', title='ntrks', xbins=200, xmin=0, xmax=200)
    for i in range(len(hypoAlg.minPt)):
        monTool.defineHistogram('countsSelection{}'.format(i),
                                path='EXPERT', type='TH1I', title='counts for min pT and max z0 cut',
                                xbins=200, xmin=0, xmax=200)
    monTool.defineHistogram( "trkPt", path='EXPERT', type='TH1I', title="Tracks pt (low pt part);p_{T} [GeV]", xbins=100, xmin=0, xmax=10)
    monTool.defineHistogram( "trkEta", path='EXPERT', type='TH1I', title="Tracks eta;#eta", xbins=50, xmin=-2.5, xmax=2.5)

    return monTool
