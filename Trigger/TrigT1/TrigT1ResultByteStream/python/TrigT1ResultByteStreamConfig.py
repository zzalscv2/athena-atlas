#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format
from TrigEDMConfig.TriggerEDMRun3 import recordable
from TrigEDMConfig.Utils import getEDMListFromWriteHandles
from libpyeformat_helper import SourceIdentifier, SubDetector

from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import eFexByteStreamToolCfg, jFexRoiByteStreamToolCfg, jFexInputByteStreamToolCfg, gFexByteStreamToolCfg, gFexInputByteStreamToolCfg
from L1TopoByteStream.L1TopoByteStreamConfig import L1TopoPhase1ByteStreamToolCfg
from TrigT1MuonRecRoiTool.TrigT1MuonRecRoiToolConfig import getRun3RPCRecRoiTool
from TrigT1MuonRecRoiTool.TrigT1MuonRecRoiToolConfig import getRun3TGCRecRoiTool
from TrigT1MuctpiPhase1.TrigT1MuctpiPhase1Config import getTrigThresholdDecisionTool

_log = logging.getLogger('TrigT1ResultByteStreamConfig')

def RoIBResultByteStreamToolCfg(name, flags, writeBS=False):
  tool = CompFactory.RoIBResultByteStreamTool(name)

  if not flags.Trigger.L1.doCTP:
    # disable CTP ByteStream decoding/encoding as part of RoIBResult
    tool.CTPModuleId = 0xFF

  if flags.Trigger.enableL1MuonPhase1 or not flags.Trigger.L1.doMuon:
    # disable legacy MUCTPI ByteStream decoding/encoding as part of RoIBResult
    tool.MUCTPIModuleId = 0xFF

  if not flags.Trigger.enableL1CaloLegacy or not flags.Trigger.L1.doCalo:
    # disable legacy L1Calo ByteStream decoding/encoding as part of RoIBResult
    tool.JetModuleIds = []
    tool.EMModuleIds = []

  if flags.Trigger.EDMVersion == 1 or not flags.Trigger.L1.doTopo:
    # disable legacy L1Topo ByteStream decoding/encoding as part of RoIBResult
    tool.L1TopoModuleIds = []

  if writeBS:
    # write BS == read RDO
    tool.RoIBResultReadKey="RoIBResult"
    tool.RoIBResultWriteKey=""
  else:
    # read BS == write RDO
    tool.RoIBResultReadKey=""
    tool.RoIBResultWriteKey="RoIBResult"
  return tool

def ExampleL1TriggerByteStreamToolCfg(name, writeBS=False):
  tool = CompFactory.ExampleL1TriggerByteStreamTool(name)
  muctpi_moduleid = 0
  muctpi_robid = int(SourceIdentifier(SubDetector.TDAQ_MUON_CTP_INTERFACE, muctpi_moduleid))
  tool.ROBIDs = [muctpi_robid]
  if writeBS:
    # write BS == read xAOD
    tool.MuonRoIContainerReadKey="LVL1MuonRoIs"
    tool.MuonRoIContainerWriteKey=""
    tool.L1TopoOutputLocID=""
  else:
    # read BS == write xAOD
    tool.MuonRoIContainerReadKey=""
    tool.MuonRoIContainerWriteKey=recordable("LVL1MuonRoIs")
  return tool

def MuonRoIByteStreamToolCfg(name, flags, writeBS=False):
  tool = CompFactory.MuonRoIByteStreamTool(name)
  muctpi_moduleid = 0  # No RoIB in Run 3, we always read the DAQ ROB
  muctpi_robid = int(SourceIdentifier(SubDetector.TDAQ_MUON_CTP_INTERFACE, muctpi_moduleid)) # 0x760000
  tool.ROBIDs = [muctpi_robid]
  tool.DoTopo = flags.Trigger.L1.doMuonTopoInputs

  from TrigT1ResultByteStream.TrigT1ResultByteStreamMonitoring import L1MuonBSConverterMonitoring
  tool.MonTool = L1MuonBSConverterMonitoring(name, flags, writeBS)

  # Build container names for each bunch crossing in the maximum readout window (size 5)
  containerBaseName = "LVL1MuonRoIs"
  containerNames = [
    containerBaseName + "BCm2",
    containerBaseName + "BCm1",
    containerBaseName,
    containerBaseName + "BCp1",
    containerBaseName + "BCp2",
  ]
  topocontainerBaseName = "L1MuCTPItoL1TopoLocation"
  topocontainerNames = [
    topocontainerBaseName + "-2",
    topocontainerBaseName + "-1",
    topocontainerBaseName,
    topocontainerBaseName + "1",
    topocontainerBaseName + "2",
  ]
  if writeBS:
    # write BS == read xAOD
    tool.MuonRoIContainerReadKeys += containerNames
  else:
    # read BS == write xAOD
    tool.MuonRoIContainerWriteKeys += [recordable(c) for c in containerNames]
    tool.L1TopoOutputLocID += topocontainerNames

  tool.RPCRecRoiTool = getRun3RPCRecRoiTool(name="RPCRecRoiTool",useRun3Config=flags.Trigger.enableL1MuonPhase1)
  tool.TGCRecRoiTool = getRun3TGCRecRoiTool(name="TGCRecRoiTool",useRun3Config=flags.Trigger.enableL1MuonPhase1)
  tool.TrigThresholdDecisionTool = getTrigThresholdDecisionTool(name="TrigThresholdDecisionTool")

  return tool

def doRoIBResult(flags):
  '''
  Helper function returning a logic combination of flags deciding
  whether the RoIBResult decoding/encoding is required in the job
  '''
  if flags.Trigger.L1.doCalo and flags.Trigger.enableL1CaloLegacy:
    # Only needed for legacy (Run-2) L1Calo system
    return True
  if flags.Trigger.L1.doMuon and not flags.Trigger.enableL1MuonPhase1:
    # Only needed for legacy (Run-2) MUCTPI data
    return True
  if flags.Trigger.L1.doTopo:
    # Currently only RoIBResult path implemented for L1Topo
    return True
  if flags.Trigger.L1.doCTP:
    # Currently only RoIBResult path implemented for CTP
    return True
  # Otherwise don't need RoIBResult
  return False

def L1TriggerByteStreamDecoderCfg(flags, returnEDM=False):

  decoderTools = []
  maybeMissingRobs = []

  ########################################
  # Legacy decoding via RoIBResult
  ########################################
  if not flags.Trigger.doLVL1: #if we rerun L1, don't decode the original RoIBResult
    if doRoIBResult(flags):
      roibResultTool = RoIBResultByteStreamToolCfg(name="RoIBResultBSDecoderTool", flags=flags, writeBS=False)
      decoderTools += [roibResultTool]
      if flags.Trigger.EDMVersion == 2:
        # L1Topo may be missing in some runs of Run 2 when it was under commissioning
        for module_id in roibResultTool.L1TopoModuleIds:
          maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_TOPO_PROC, module_id)))
      if flags.Trigger.EDMVersion == 2 and not flags.Trigger.doHLT:
        # L1Calo occasional readout errors weren't caught by HLT in 2015 - ignore these in offline reco, see ATR-24493
        for module_id in roibResultTool.JetModuleIds:
          maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_JET_PROC_ROI, module_id)))
        for module_id in roibResultTool.EMModuleIds:
          maybeMissingRobs.append(int(SourceIdentifier(SubDetector.TDAQ_CALO_CLUSTER_PROC_ROI, module_id)))

  ########################################
  # Run-3 L1Muon decoding (only when running HLT - offline we read it from HLT result)
  ########################################
  if flags.Trigger.L1.doMuon and flags.Trigger.enableL1MuonPhase1 and flags.Trigger.doHLT:
    muonRoiTool = MuonRoIByteStreamToolCfg(name="L1MuonBSDecoderTool",
                                           flags=flags,
                                           writeBS=False)
    decoderTools += [muonRoiTool]

  ########################################
  # Run-3 L1Calo decoding
  ########################################
  if flags.Trigger.L1.doCalo and flags.Trigger.enableL1CaloPhase1:
    # online case in HLT
    if flags.Trigger.doHLT:
      eFexByteStreamTool = eFexByteStreamToolCfg("eFexBSDecoderTool",
                                                 flags=flags,
                                                 writeBS=False,
                                                 multiSlice=False)
      jFexRoiByteStreamTool = jFexRoiByteStreamToolCfg("jFexBSDecoderTool",
                                                       flags=flags,
                                                       writeBS=False)
      gFexByteStreamTool = gFexByteStreamToolCfg("gFexBSDecoderTool",
                                                 flags=flags,
                                                 writeBS=False)
      decoderTools += [eFexByteStreamTool, jFexRoiByteStreamTool, gFexByteStreamTool]
      # During commissioning of the phase-1 L1Calo (2022), allow the data to be missing
      maybeMissingRobs += eFexByteStreamTool.ROBIDs
      maybeMissingRobs += jFexRoiByteStreamTool.ROBIDs
      maybeMissingRobs += gFexByteStreamTool.ROBIDs

    # reco/monitoring case (either online but downstream from HLT, or at Tier-0)
    else:
      # Decode eFex and jFex xTOBs (TOBs are already decoded in HLT, gFex doesn't have xTOBs)
      eFexByteStreamTool = eFexByteStreamToolCfg("eFexBSDecoderTool",
                                                 flags=flags,
                                                 writeBS=False,
                                                 TOBs=False,
                                                 xTOBs=True,
                                                 multiSlice=True,
                                                 decodeInputs=flags.Trigger.L1.doCaloInputs)
      jFexRoiByteStreamTool = jFexRoiByteStreamToolCfg("jFexBSDecoderTool",
                                                       flags=flags,
                                                       writeBS=False,
                                                       xTOBs=True)
      decoderTools += [eFexByteStreamTool, jFexRoiByteStreamTool]

      if flags.Trigger.L1.doCaloInputs:
        # Decode jFex and gFex inputs (towers), eFex inputs already included in the tool above
        jFexInputByteStreamTool = jFexInputByteStreamToolCfg('jFexInputBSDecoderTool',
                                                             flags=flags,
                                                             writeBS=False)
        gFexInputByteStreamTool = gFexInputByteStreamToolCfg('gFexInputByteStreamTool',
                                                             flags=flags,
                                                             writeBS=False)
        decoderTools += [jFexInputByteStreamTool, gFexInputByteStreamTool]

      # During commissioning of the phase-1 L1Calo (2022), allow the data to be missing
      maybeMissingRobs += eFexByteStreamTool.ROBIDs
      maybeMissingRobs += jFexRoiByteStreamTool.ROBIDs
      if flags.Trigger.L1.doCaloInputs:
        maybeMissingRobs += jFexInputByteStreamTool.ROBIDs
        maybeMissingRobs += gFexInputByteStreamTool.ROBIDs

  ########################################
  # Run-3 L1Topo decoding
  ########################################
  if flags.Trigger.L1.doTopo and flags.Trigger.enableL1CaloPhase1 and flags.Trigger.doHLT:
    topoByteStreamTool = L1TopoPhase1ByteStreamToolCfg("L1TopoBSDecoderTool",
                                                       flags=flags,
                                                       writeBS=False)
    decoderTools += [topoByteStreamTool]
    # During commissioning of the phase-1 L1Topo (2022), allow the data to be missing
    maybeMissingRobs += topoByteStreamTool.ROBIDs

  decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder",
                                                         DecoderTools=decoderTools,
                                                         MaybeMissingROBs=maybeMissingRobs)

  acc = ComponentAccumulator()
  acc.addEventAlgo(decoderAlg, primary=True)

  # The decoderAlg needs to load ByteStreamMetadata for the detector mask
  from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
  readBSAcc = ByteStreamReadCfg(flags)
  readBSAcc.getEventAlgo('SGInputLoader').Load += [
    ('ByteStreamMetadataContainer', 'InputMetaDataStore+ByteStreamMetadata')]
  acc.merge(readBSAcc)

  # In reconstruction/monitoring jobs add the decoders' output EDM to the output file
  if not flags.Trigger.doHLT:
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    outputEDM = getEDMListFromWriteHandles([tool for tool in decoderAlg.DecoderTools if 'RoIBResult' not in tool.getName()])
    _log.info('Adding the following output EDM to ItemList: %s', outputEDM)
    acc.merge(addToESD(flags, outputEDM))
    acc.merge(addToAOD(flags, outputEDM))

  # Return outputEDM as a second object to be used for compatibility with RecExCommon output configuration,
  # because the above calls to addToESD/addtoAOD are no-op when this fragment is wrapped in RecExCommon.
  # See discussions in https://gitlab.cern.ch/atlas/athena/-/merge_requests/55891#note_5912844
  if returnEDM:
    return acc, outputEDM
  return acc

def L1TriggerByteStreamEncoderCfg(flags):
  acc = ComponentAccumulator()

  # Legacy encoding via RoIBResult
  if doRoIBResult(flags):
    roibResultTool = RoIBResultByteStreamToolCfg(name="RoIBResultBSEncoderTool", flags=flags, writeBS=True)
    acc.addPublicTool(roibResultTool)
    # Special - in BS->BS job without L1Sim, need to decode extra data from input
    # for encoding the CTP information back to BS
    if flags.Input.Format is Format.BS and not flags.Trigger.doLVL1 and roibResultTool.CTPModuleId != 0xFF:
      from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
      acc.merge(ByteStreamReadCfg(flags, type_names=['CTP_RDO/CTP_RDO']))

  # Run-3 L1Muon encoding
  if flags.Trigger.L1.doMuon and flags.Trigger.enableL1MuonPhase1:
    muonRoiTool = MuonRoIByteStreamToolCfg(name="L1MuonBSEncoderTool",
                                           flags=flags,
                                           writeBS=True)
    acc.addPublicTool(muonRoiTool)

  # TODO: Run-3 L1Calo, L1Topo, CTP

  return acc


if __name__ == '__main__':
  from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
  import glob
  import sys

  import argparse
  parser = argparse.ArgumentParser(prog='python -m TrigT1ResultByteStream.TrigT1ResultByteStreamConfig',
                                   description="""Bytestream decoder athena script.\n\n
                                   Example: python -m TrigT1ResultByteStream.TrigT1ResultByteStreamConfig --filesInput "data22*" --evtMax 10 --outputs eTOBs exTOBs """)
  parser.add_argument('--evtMax',type=int,default=-1,help="number of events to process (-1 = til end of files)")
  parser.add_argument('--skipEvents',type=int,default=0,help="number of events to skip")
  parser.add_argument('--filesInput',nargs='+',help="input files",required=True)
  parser.add_argument('--outputLevel',default="WARNING",choices={ 'INFO','WARNING','DEBUG','VERBOSE'})
  parser.add_argument('--outputs',nargs='+',choices={"eTOBs","exTOBs","eTowers","jTOBs","jTowers","gTOBs","gCaloTowers","Topo","legacy"},required=True,
                      help="What data to decode and output.")
  args = parser.parse_args()

  _log.setLevel(logging.DEBUG)

  from AthenaCommon import Constants
  algLogLevel = getattr(Constants,args.outputLevel)

  if any(["data22" in f for f in args.filesInput]):
    flags.Trigger.triggerConfig='DB'
    from AthenaConfiguration.Enums import LHCPeriod
    flags.GeoModel.Run = LHCPeriod.Run3 # needed for LArGMConfig

  flags.Exec.OutputLevel = algLogLevel
  flags.Exec.MaxEvents = args.evtMax
  flags.Exec.SkipEvents = args.skipEvents
  flags.Input.Files = [file for x in args.filesInput for file in glob.glob(x)]
  flags.Concurrency.NumThreads = 1
  flags.Concurrency.NumConcurrentEvents = 1

  if any(["data22" in f for f in args.filesInput]):
    flags.Output.AODFileName = "AOD."+(args.filesInput[0].split("/")[-1]).split('_SFO')[0]+"pool.root"
  else:
    flags.Output.AODFileName = 'AOD.pool.root'

  flags.Trigger.enableL1CaloLegacy = 'legacy' in args.outputs
  flags.lock()

  from AthenaConfiguration.MainServicesConfig import MainServicesCfg
  from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
  acc = MainServicesCfg(flags)
  acc.merge(ByteStreamReadCfg(flags)) # configure reading bytestream


  # Generate run3 L1 menu
  from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg,generateL1Menu
  acc.merge(L1ConfigSvcCfg(flags))

  if not any(["data22" in f for f in args.filesInput]):
    generateL1Menu(flags)


  decoderTools = []
  outputEDM = []
  maybeMissingRobs = []


  def addEDM(edmType, edmName):
    auxType = edmType.replace('Container','AuxContainer')
    return [f'{edmType}#{edmName}',
            f'{auxType}#{edmName}Aux.']


  ########################################
  # Legacy decoding via RoIBResult
  ########################################
  if 'legacy' in args.outputs:
    # Produce xAOD L1 RoIs from RoIBResult. RoIB readout only
    from AnalysisTriggerAlgs.AnalysisTriggerAlgsCAConfig import RoIBResultToxAODCfg
    xRoIBResultAcc, xRoIBResultOutputs = RoIBResultToxAODCfg(flags)
    acc.merge(xRoIBResultAcc)

    roibResultTool = RoIBResultByteStreamToolCfg(name="RoIBResultBSDecoderTool", flags=flags, writeBS=False)
    decoderTools += [roibResultTool]

    outputEDM += addEDM('xAOD::JetEtRoI'         , 'LVL1JetEtRoI')
    outputEDM += addEDM('xAOD::JetRoIContainer'  , 'LVL1JetRoIs')
    outputEDM += addEDM('xAOD::EmTauRoIContainer', 'LVL1EmTauRoIs')
    outputEDM += addEDM('xAOD::EnergySumRoI'     , 'LVL1EnergySumRoI')

    # CPM and JEM RoIs (independent from above). Both readouts
    from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
    type_names = [
      # ===== CPM ================================================================
      "xAOD::CPMTowerContainer/CPMTowers",
      "xAOD::CPMTowerAuxContainer/CPMTowersAux.",
      "xAOD::CPMTowerContainer/CPMTowersOverlap",
      "xAOD::CPMTowerAuxContainer/CPMTowersOverlapAux.",
      # ===== CPMTOBROIS =========================================================
      "xAOD::CPMTobRoIContainer/CPMTobRoIs",
      "xAOD::CPMTobRoIAuxContainer/CPMTobRoIsAux.",
      "xAOD::CPMTobRoIContainer/CPMTobRoIsRoIB",
      "xAOD::CPMTobRoIAuxContainer/CPMTobRoIsRoIBAux.",
      # ===== JEMTOBROIS =========================================================
      "xAOD::JEMTobRoIContainer/JEMTobRoIs",
      "xAOD::JEMTobRoIAuxContainer/JEMTobRoIsAux.",
      "xAOD::JEMTobRoIContainer/JEMTobRoIsRoIB",
      "xAOD::JEMTobRoIAuxContainer/JEMTobRoIsRoIBAux.",
    ]
    acc.merge(ByteStreamReadCfg(flags, type_names=type_names))

    outputEDM += [item.replace('/','#') for item in type_names]

  ########################################
  # jFEX ROIs
  ########################################
  if 'jTOBs' in args.outputs:
    jFexTool = jFexRoiByteStreamToolCfg('jFexBSDecoder', flags)
    for module_id in jFexTool.ROBIDs:
        maybeMissingRobs.append(module_id)

    decoderTools += [jFexTool]

  ########################################
  # jFEX input Data
  ########################################
  if 'jTowers' in args.outputs:
    inputjFexTool = jFexInputByteStreamToolCfg('jFexInputBSDecoder', flags)
    for module_id in inputjFexTool.ROBIDs:
        maybeMissingRobs.append(module_id)

    decoderTools += [inputjFexTool]

  ########################################
  # eFEX ROIs and Input data
  ########################################
  if any( [x in args.outputs for x in ['eTOBs','exTOBs','eTowers']] ):
    eFexTool = eFexByteStreamToolCfg('eFexBSDecoder', flags,TOBs='eTOBs' in args.outputs,xTOBs='exTOBs' in args.outputs,decodeInputs='eTowers' in args.outputs)
    # eFexTool_xTOBs = eFexByteStreamToolCfg('eFexBSDecoder_xTOBs', flags,xTOBs=True)
    decoderTools += [eFexTool]

    # allow for missing ROBs for eFEX decoding:
    maybeMissingRobs += eFexTool.ROBIDs

  ########################################
  # gFEX ROIs
  ########################################
  if 'gTOBs' in args.outputs:
    gFexTool = gFexByteStreamToolCfg('gFexBSDecoder', flags)
    decoderTools += [gFexTool]

  ########################################
  # gFEX input Data
  ########################################
  if 'gCaloTowers' in args.outputs:
    inputgFexTool = gFexInputByteStreamToolCfg('gFexInputBSDecoder', flags)
    decoderTools += [inputgFexTool]

  ########################################
  # Topo data
  ########################################
  if 'Topo' in args.outputs:
    l1topoBSTool = L1TopoPhase1ByteStreamToolCfg("L1TopoBSDecoderTool", flags)
    decoderTools += [l1topoBSTool]
    # allow for missing Topo ROBs:
    maybeMissingRobs += l1topoBSTool.ROBIDs

  decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder",
                                                         DecoderTools=decoderTools, OutputLevel=algLogLevel, MaybeMissingROBs=maybeMissingRobs)

  acc.addEventAlgo(decoderAlg, sequenceName='AthAlgSeq')

  from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
  outputEDM += getEDMListFromWriteHandles([tool for tool in decoderAlg.DecoderTools if 'RoIBResult' not in tool.getName()])
  _log.debug('Adding the following output EDM to ItemList: %s', outputEDM)
  acc.merge(OutputStreamCfg(flags, 'AOD', ItemList=outputEDM))

  # get rid of warning about propagating input attribute list ... since there is none
  # note it's odd that the AthenaCommon.globalflags input format property doesn't get updated appropriately by flags??
  acc.getEventAlgo("EventInfoTagBuilder").PropagateInput = (flags.Input.Format != Format.BS)



  if acc.run().isFailure():
    sys.exit(1)
