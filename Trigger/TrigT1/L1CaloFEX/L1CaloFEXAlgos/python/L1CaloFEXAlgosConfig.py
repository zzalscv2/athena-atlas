#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import jFexInputByteStreamToolCfg
from AthenaConfiguration.Enums import Format

def L1CaloFEXDecoratorCfg(name):
    
    acc=ComponentAccumulator()
    
    decorator = CompFactory.LVL1.jFexTower2SCellDecorator(name)
    acc.addEventAlgo(decorator)

    return acc


if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
    from AthenaCommon.Logging import logging
    import glob
    import sys

    import argparse
    parser = argparse.ArgumentParser(prog='python -m L1CaloFEXTools.L1CaloFEXToolsConfig',
                                   description="""Decorator tool for FEX towers athena script.\n\n
                                   Example: python -m L1CaloFEXTools.L1CaloFEXToolsConfig --filesInput "data22*" --evtMax 10 --outputs eTOBs """)
    parser.add_argument('--evtMax',type=int,default=-1,help="number of events")
    parser.add_argument('--filesInput',nargs='+',help="input files",required=True)
    parser.add_argument('--outputLevel',default="WARNING",choices={ 'INFO','WARNING','DEBUG','VERBOSE'})
    parser.add_argument('--outputs',nargs='+',choices={"jTowers"},required=True, help="What data to decode and output.")
    args = parser.parse_args()


    log = logging.getLogger('L1CaloFEXToolsConfig')
    log.setLevel(logging.DEBUG)

    from AthenaCommon import Constants
    algLogLevel = getattr(Constants,args.outputLevel)

    if any(["data22" in f for f in args.filesInput]):
        flags.Trigger.triggerConfig='DB'

    flags.Exec.OutputLevel = algLogLevel
    flags.Exec.MaxEvents = args.evtMax
    flags.Input.Files = [file for x in args.filesInput for file in glob.glob(x)]
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
  
    if any(["data22" in f for f in args.filesInput]):
        s=args.filesInput[0].replace('*','').replace('.data','')
        flags.Output.AODFileName = "AOD."+(s.split("/")[-1]).split('_SFO')[0]+"pool.root"
    else:
        flags.Output.AODFileName = 'AOD.pool.root'  

    flags.Trigger.EDMVersion = 3
    flags.Trigger.doLVL1 = True
    flags.Trigger.enableL1CaloPhase1 = True
    if flags.Common.isOnline:
        flags.IOVDb.GlobalTag = flags.Trigger.OnlineCondTag

    if not flags.Input.isMC and flags.Input.RunNumber[0] > 400000:
        flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-02-00-00'

    # Enable only calo for this test
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorsFromList
    setupDetectorsFromList(flags,['LAr','Tile','MBTS'],True)

    flags.lock()

    
    # Set up the main service "acc"
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    
    # The decoderAlg needs to load ByteStreamMetadata for the detector mask
    from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))
    
    
    # Generate run3 L1 menu
    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg
    acc.merge(L1ConfigSvcCfg(flags))

    decoderTools = []
    outputEDM = []
    maybeMissingRobs = []

    def addEDM(edmType, edmName):
        auxType = edmType.replace('Container','AuxContainer')
        return [f'{edmType}#{edmName}',
            f'{auxType}#{edmName}Aux.']


    ########################################
    # jFEX input Data
    ########################################
    if 'jTowers' in args.outputs:
        inputjFexTool = jFexInputByteStreamToolCfg('jFexInputBSDecoder', flags)
        for module_id in inputjFexTool.ROBIDs:
            maybeMissingRobs.append(module_id)

        decoderTools += [inputjFexTool]
        # saving/adding the jTower xAOD container
        outputEDM += addEDM('xAOD::jFexTowerContainer', inputjFexTool.jTowersWriteKey.Path)

    decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder",
                                                         DecoderTools=decoderTools, OutputLevel=algLogLevel, MaybeMissingROBs=maybeMissingRobs)

    acc.addEventAlgo(decoderAlg, sequenceName='AthAlgSeq')

    ########################################
    # Decorators   
    ########################################
    from L1CaloFEXSim.L1CaloFEXSimCfg import ReadSCellFromByteStreamCfg
    if any(["data22_cos" in f for f in args.filesInput]):
        acc.merge(ReadSCellFromByteStreamCfg(flags,keyIn="SC_ET_ID"))
    else:
        acc.merge(ReadSCellFromByteStreamCfg(flags))
    
    DecoratorAlgo = L1CaloFEXDecoratorCfg('FEX2SCellDecorator')   
    acc.merge(DecoratorAlgo)

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    log.debug('Adding the following output EDM to ItemList: %s', outputEDM)
    acc.merge(OutputStreamCfg(flags, 'AOD', ItemList=outputEDM))

    acc.getEventAlgo("EventInfoTagBuilder").PropagateInput = (flags.Input.Format != Format.BS)

    if acc.run().isFailure():
        sys.exit(1)
