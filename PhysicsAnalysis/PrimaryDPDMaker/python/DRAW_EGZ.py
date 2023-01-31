# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python
#====================================================================
# DRAW_EGZ.py
# This defines DRAW_EGZ, a skimmed DRAW format
# Z->ee, eey and mmy reduction for electron and photon ID and calibration
#====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import seqAND
from AthenaCommon.Logging import logging

# Main algorithm config
def DRAW_EGZKernelCfg(configFlags, name='DRAW_EGZKernel', **kwargs):
    """Configure DRAW_EGZ kerne"""

    mlog = logging.getLogger(name)
    mlog.info('Start configuration')
    
    acc = ComponentAccumulator()
    acc.addSequence(seqAND('DRAW_EGZSequence'))

    # The selections
    DRAWEGZSel = {}
    DRAWEGZSel['Zee'] = [
        'Electrons.pt > 20*GeV && Electrons.LHMedium',
        'eeMass1',
        '(count(eeMass1 > 55*GeV) >= 1)' ]
    DRAWEGZSel['Zeey'] = [
        'Electrons.pt > 15*GeV && Electrons.LHMedium',
        'eeMass2',
        '(count(eeMass2 > 20*GeV && eeMass2 < 90*GeV) >= 1 && count(Photons.pt > 7*GeV && Photons.Tight) >= 1)' ] 
    DRAWEGZSel['Zmmy'] = [
        'Muons.pt > 15*GeV',
        'mmMass',
        '(count(mmMass > 20*GeV && mmMass < 90*GeV) >= 1 && count(Photons.pt > 7*GeV && Photons.Tight) >= 1)' ] 

    # Augmentation tools for the di-lepton mass computations
    EventSels = []
    augmentationTools = []
    for key,sel in DRAWEGZSel.items():
        tool = CompFactory.DerivationFramework.InvariantMassTool(
            name               = f'llmassToolFor{key}',
            ContainerName      = 'Electrons' if key.find('Zee') >= 0 else 'Muons',
            ObjectRequirements = sel[0],
            MassHypothesis     = 0.511 if key.find('Zee') >= 0 else 105.66,
            StoreGateEntryName = sel[1])
        augmentationTools.append(tool)
        acc.addPublicTool(tool)
        EventSels.append(sel[2])
    draw_egz = " || ".join(EventSels)
    mlog.info('DRAW_EGZ selection '+draw_egz)

    # The skimming tool
    skimmingTool = CompFactory.DerivationFramework.xAODStringSkimmingTool(
        name       = 'DRAW_EGZSkimmingTool',
        expression = draw_egz)
    acc.addPublicTool(skimmingTool)

    # The main kernel algo
    DRAW_EGZKernel = CompFactory.DerivationFramework.DerivationKernel(
        name              = 'DRAW_EGZKernel',
        AugmentationTools = augmentationTools,
        SkimmingTools     = [skimmingTool])
        
    acc.addEventAlgo(DRAW_EGZKernel,sequenceName='DRAW_EGZSequence')  
    return acc

def DRAW_EGZCfg(configFlags):
    """Main config fragment for DRAW_EGZ"""
    acc = ComponentAccumulator()

    # Main algorithm (kernel)
    acc.merge(DRAW_EGZKernelCfg(configFlags,name = 'DRAW_EGZKernel'))

    # Output configuration
    # Inspired from ByteStreamConfig, adding a copy tool
    outCA = ComponentAccumulator(
        CompFactory.AthSequencer(
            name         = 'AthOutSeq',
            StopOverride = True))
    
    bsesoSvc = CompFactory.ByteStreamEventStorageOutputSvc(
        name           = "BSEventStorageOutputSvcDRAW_EGZ",
        MaxFileMB      = 15000,
        MaxFileNE      = 15000000,
        OutputDirectory= './',
        StreamType     = '',
        StreamName     = 'StreamDRAW_EGZ',
        SimpleFileName = configFlags.Output.DRAW_EGZFileName)
    outCA.addService(bsesoSvc)
    
    bsIS = CompFactory.ByteStreamEventStorageInputSvc('ByteStreamInputSvc')
    outCA.addService(bsIS)
            
    bsCopyTool = CompFactory.ByteStreamOutputStreamCopyTool(
        ByteStreamOutputSvc = bsesoSvc,
        ByteStreamInputSvc  = bsIS)

    bsCnvSvc = CompFactory.ByteStreamCnvSvc(
        ByteStreamOutputSvcList=[bsesoSvc.getName()])
    outCA.addService(bsCnvSvc)
        
    outCA.addEventAlgo(CompFactory.AthenaOutputStream(
        name             = 'BSOutputStreamAlgDRAW_EGZ',
        WritingTool      = bsCopyTool,
        EvtConversionSvc = bsCnvSvc.name,
        RequireAlgs      = ['DRAW_EGZKernel'],
        ExtraInputs      = [('xAOD::EventInfo','StoreGateSvc+EventInfo')]),
                           domain='IO', primary = True)
        
    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
    outCA.merge(IOVDbSvcCfg(configFlags))
    outCA.merge(MetaDataSvcCfg(configFlags,
        ["IOVDbMetaDataTool", "ByteStreamMetadataTool"]))

    acc.merge(outCA)

    return acc
