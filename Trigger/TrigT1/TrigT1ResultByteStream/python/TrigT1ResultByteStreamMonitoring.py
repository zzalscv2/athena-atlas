#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
from libpyeformat_helper import SourceIdentifier, SubDetector

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

def L1TriggerByteStreamDecoderMonitoring(name, flags, decoderTools):
    monTool = GenericMonitoringTool('MonTool')
    monTool.HistPath = f'HLTFramework/L1BSConverters/{name}'

    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F',
                            title='Time of the alg execute() method;Time [ms];N events',
                            xbins=100, xmin=0, xmax=100)

    # In hindsight, it would be better for RoIBResult decoder to have the same property interface as the other tools
    def getRobIds(decoder):
        if 'RoIBResult' not in decoder.getName():
            return decoder.ROBIDs
        robIds = []
        robIds += [int(SourceIdentifier(SubDetector.TDAQ_CTP, decoder.CTPModuleId))] if decoder.CTPModuleId!=0xff else []
        robIds += [int(SourceIdentifier(SubDetector.TDAQ_MUON_CTP_INTERFACE, decoder.MUCTPIModuleId))] if decoder.MUCTPIModuleId!=0xff else []
        robIds += [int(SourceIdentifier(SubDetector.TDAQ_CALO_CLUSTER_PROC_ROI, modId)) for modId in decoder.EMModuleIds]
        robIds += [int(SourceIdentifier(SubDetector.TDAQ_CALO_JET_PROC_ROI, modId)) for modId in decoder.JetModuleIds]
        robIds += [int(SourceIdentifier(SubDetector.TDAQ_CALO_TOPO_PROC, modId)) for modId in decoder.L1TopoModuleIds]
        return robIds

    allRobIds = []
    for decoder in decoderTools:
        decoderName = decoder.getName()
        allRobIds += getRobIds(decoder)
        monTool.defineHistogram(f'TIME_prepareROBs_{decoderName}', path='EXPERT', type='TH1F',
                                title=f'Time of preparing ROB inputs for {decoderName};Time [ms];N events',
                                xbins=100, xmin=0, xmax=100)
        monTool.defineHistogram(f'TIME_convert_{decoderName}', path='EXPERT', type='TH1F',
                                title=f'Time of the convertFromBS() method of {decoderName};Time [ms];N events',
                                xbins=100, xmin=0, xmax=100)
        monTool.defineHistogram(f'LumiBlock,MissingROBFraction_{decoderName};MissingROBFraction_{decoderName}', path='EXPERT', type='TProfile',
                                title=f'Fraction of missing ROBs requested by {decoderName} vs LBN;LumiBlock;N missing ROBs / N requested ROBs',
                                xbins=100, xmin=0, xmax=100, opt='kCanRebin')
        monTool.defineHistogram(f'LumiBlock,CorruptedROBFraction_{decoderName};CorruptedROBFraction_{decoderName}', path='EXPERT', type='TProfile',
                                title=f'Fraction of corrupted ROBs requested by {decoderName} vs LBN;LumiBlock;N corrupted ROBs / N retrieved ROBs',
                                xbins=100, xmin=0, xmax=100, opt='kCanRebin')
        monTool.defineHistogram(f'LumiBlock,ErroneousROBFraction_{decoderName};ErroneousROBFraction_{decoderName}', path='EXPERT', type='TProfile',
                                title=f'Fraction of erroneous ROBs requested by {decoderName} vs LBN;LumiBlock;N erroneous ROBs / N retrieved ROBs',
                                xbins=100, xmin=0, xmax=100, opt='kCanRebin')

    robIdLabels = [hex(id) for id in sorted(list(set(allRobIds)))]
    monTool.defineHistogram('MissingROB', path='EXPERT', type='TH1F',
                            title='Count of missing obligatory ROBs;ROB ID;N events with ROB missing',
                            xbins=len(robIdLabels), xmin=0, xmax=len(robIdLabels), xlabels=robIdLabels)
    monTool.defineHistogram('MissingROBAllowed', path='EXPERT', type='TH1F',
                            title='Count of missing optional ROBs;ROB ID;N events with ROB missing',
                            xbins=len(robIdLabels), xmin=0, xmax=len(robIdLabels), xlabels=robIdLabels)
    monTool.defineHistogram('CorruptedROB', path='EXPERT', type='TH1F',
                            title='Count of corrupted obligatory ROBs;ROB ID;N events with ROB errors',
                            xbins=len(robIdLabels), xmin=0, xmax=len(robIdLabels), xlabels=robIdLabels)
    monTool.defineHistogram('CorruptedROBAllowed', path='EXPERT', type='TH1F',
                            title='Count of corrupted optional ROBs;ROB ID;N events with ROB errors',
                            xbins=len(robIdLabels), xmin=0, xmax=len(robIdLabels), xlabels=robIdLabels)
    monTool.defineHistogram('ErroneousROB', path='EXPERT', type='TH1F',
                            title='Count of erroneous obligatory ROBs;ROB ID;N events with ROB errors',
                            xbins=len(robIdLabels), xmin=0, xmax=len(robIdLabels), xlabels=robIdLabels)
    monTool.defineHistogram('ErroneousROBAllowed', path='EXPERT', type='TH1F',
                            title='Count of erroneous optional ROBs;ROB ID;N events with ROB errors',
                            xbins=len(robIdLabels), xmin=0, xmax=len(robIdLabels), xlabels=robIdLabels)

    return monTool
