#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def L1MuonBSConverterMonitoring(name, flags, encoder=False):
    tool = GenericMonitoringTool('MonTool')
    tool.HistPath = f'HLTFramework/L1BSConverters/{name}'
    tool.defineHistogram('NumWordsInROD', path='EXPERT', type='TH1F',
                         title='Size of the MUCTPI ROD payload;N words;N events',
                         xbins=100, xmin=0, xmax=100)
    tool.defineHistogram('WordType,WordTypeCount;WordTypeCounts', path='EXPERT', type='TH2F',
                         title='Counts of each word type in MUCTPI ROD payload;;Count per event',
                         xbins=6, xmin=0, xmax=6,
                         ybins=150, ymin=0, ymax=150,  # max candidate word count per time slice in hardware is 352
                         xlabels=['Undefined', 'Timeslice', 'Multiplicity', 'Candidate', 'Topo', 'Status'])
    tool.defineHistogram('BCIDOffsetsWrtROB', path='EXPERT', type='TH1F',
                         title='BCID difference between timeslice header and ROB header;BCID difference;N time slices',
                         xbins=201, xmin=-100.5, xmax=100.5)
    tool.defineHistogram('SubsysID', path='EXPERT', type='TH1F',
                         title='RoI candidate subsys ID;;N RoIs',
                         xbins=4, xmin=0, xmax=4,
                         xlabels=['Undefined','Barrel', 'Forward', 'Endcap'])
    if flags.Trigger.L1.doMuonTopoInputs:
        tool.defineHistogram('topoSubsysID', path='EXPERT', type='TH1F',
                            title='Topo TOB subsys ID;;N RoIs',
                            xbins=4, xmin=0, xmax=4,
                            xlabels=['Undefined','Barrel', 'Forward', 'Endcap'])


    if not encoder:
        tool.defineHistogram('BCOffset,NumOutputRoIs;NumOutputRoIs', path='EXPERT', type='TH2F',
                            title='Number of output xAOD::MuonRoI objects in each time slice per event;Time slice;N RoIs',
                            xbins=5, xmin=-2, xmax=3,
                            xlabels=[str(n) for n in range(-2,3)],
                            ybins=100, ymin=0, ymax=100)  # max candidate word count per time slice in hardware is 352
        tool.defineHistogram('DataStatusWordErrors', path='EXPERT', type='TH1F',
                            title='Error bits set in data status word;Bit number;N errors',
                            xbins=16, xmin=0, xmax=16)
        if flags.Trigger.L1.doMuonTopoInputs:
            tool.defineHistogram('BCOffset,NumOutputTopoTOBs;NumOutputTopoTOBs', path='EXPERT', type='TH2F',
                                title='Number of output LVL1::MuCTPIL1TopoCandidate objects in each time slice per event;Time slice;N TOBs',
                                xbins=5, xmin=-2, xmax=3,
                                xlabels=[str(n) for n in range(-2,3)],
                                ybins=100, ymin=0, ymax=100)
            tool.defineHistogram('BCOffset,NumOutputDiffRoITopo;NumOutputDiffRoITopo', path='EXPERT', type='TProfile',
                                title='Average difference between the number of output RoIs and Topo TOBs;Time slice;N RoIs - N TOBs',
                                xbins=5, xmin=-2, xmax=3,
                                xlabels=[str(n) for n in range(-2,3)])

    for subsysName in ['Barrel', 'Forward', 'Endcap']:
        tool.defineHistogram(f'roiEta_{subsysName}', path='EXPERT', type='TH1F',
                             title=f'Eta of output RoIs in the {subsysName} subsystem;eta;N RoIs',
                             xbins=60, xmin=-3, xmax=3)
        tool.defineHistogram(f'roiPhi_{subsysName}', path='EXPERT', type='TH1F',
                             title=f'Phi of output RoIs in the {subsysName} subsystem;phi;N RoIs',
                             xbins=64, xmin=-3.2, xmax=3.2)
        if flags.Trigger.L1.doMuonTopoInputs:
            tool.defineHistogram(f'topoEta_{subsysName}', path='EXPERT', type='TH1F',
                                title=f'Eta of output Topo TOBs in the {subsysName} subsystem;eta;N TOBs',
                                xbins=60, xmin=-3, xmax=3)
            tool.defineHistogram(f'topoPhi_{subsysName}', path='EXPERT', type='TH1F',
                                title=f'Phi of output Topo TOBs in the {subsysName} subsystem;phi;N TOBs',
                                xbins=64, xmin=-3.2, xmax=3.2)
            tool.defineHistogram(f'topoPtThreshold_{subsysName}', path='EXPERT', type='TH1F',
                                 title=f'pT threshold [GeV] of output Topo TOBs in the {subsysName} subsystem;pT threshold [GeV];N TOBs',
                                xbins=50, xmin=0, xmax=50)

    return tool
