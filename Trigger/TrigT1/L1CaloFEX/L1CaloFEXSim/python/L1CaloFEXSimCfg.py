#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ReadSCellFromPoolFileCfg(flags, key='SCell'):
    '''Configure reading SCell container from a Pool file like RDO or ESD'''
    acc = ComponentAccumulator()

    # Ensure SCell container is in the input file
    # TODO this needs to be uncommented once all MC files used in tests contain SCells
    # e.g. test_trig_mc_v1DevHI_build.py
    # assert key in flags.Input.Collections or not flags.Input.Collections, 'MC input file is required to contain SCell container'

    # Need geometry and conditions for the SCell converter from POOL
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    return acc


def ReadSCellFromByteStreamCfg(flags, key='SCell', SCmask=True):
    acc=ComponentAccumulator()

    # Geometry, conditions and cabling setup
    from TileGeoModel.TileGMConfig import TileGMCfg
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from LArCabling.LArCablingConfig import LArLATOMEMappingCfg
    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    from LArCellRec.LArRAWtoSuperCellConfig import LArRAWtoSuperCellCfg
    acc.merge(TileGMCfg(flags))
    acc.merge(LArGMCfg(flags))
    acc.merge(LArLATOMEMappingCfg(flags))
    acc.merge(LArOnOffIdMappingSCCfg(flags))

    # Conversion from ByteStream to LArRawSCContainer
    decoderTool = CompFactory.LArLATOMEDecoder('LArLATOMEDecoder', ProtectSourceId = True)
    decoderAlg = CompFactory.LArRawSCDataReadingAlg('LArRawSCDataReadingAlg', LATOMEDecoder=decoderTool)
    acc.addEventAlgo(decoderAlg)

    acc.merge(LArRAWtoSuperCellCfg(flags,mask=SCmask, SCellContainerOut=key) )

    return acc

def eFEXTOBEtToolCfg(flags):
    """
    Configure the eFEX TOB Et Tool which recalculates isolation variables
    The tool requires eTowers as inputs (add eTowerMaker algorithm)
    """
    acc = ComponentAccumulator()

    eTowerMakerAlg = CompFactory.LVL1.eTowerMakerFromSuperCells('eTowerMakerFromSuperCells')
    acc.addEventAlgo(eTowerMakerAlg)

    eFEXTOBEtTool = CompFactory.LVL1.eFEXTOBEtTool
    acc.setPrivateTools(eFEXTOBEtTool())

    return acc

def TriggerTowersInputCfg(flags):
    '''Configuration to provide TriggerTowers as input to the Fex simulation'''
    if flags.Input.isMC:
        # For MC produce TT with R2TTMaker
        from TrigT1CaloSim.TrigT1CaloSimRun2Config import Run2TriggerTowerMakerCfg
        return Run2TriggerTowerMakerCfg(flags)
    else:
        # For data decode TT from ByteStream
        from TrigT1CaloByteStream.LVL1CaloRun2ByteStreamConfig import LVL1CaloRun2ReadBSCfg
        return LVL1CaloRun2ReadBSCfg(flags)


def L1CaloFEXSimCfg(flags, eFexTowerInputs = [],deadMaterialCorrections=False, eFEXDebug=False):
    acc = ComponentAccumulator()

    # Configure SCell inputs
    sCellType = "SCell"
    if flags.Input.isMC:
        # Read SCell directly from input RDO file
        acc.merge(ReadSCellFromPoolFileCfg(flags,sCellType))
    else:
        from AthenaConfiguration.Enums import LHCPeriod
        if flags.GeoModel.Run is LHCPeriod.Run2:
            # Run-2 data inputs, emulate SCells
            sCellType = "EmulatedSCell"
            from TrigT1CaloFexPerf.EmulationConfig import emulateSC_Cfg
            acc.merge(emulateSC_Cfg(flags,SCOut=sCellType))
        else:
            # Run-3+ data inputs, decode SCells from ByteStream
            acc.merge(ReadSCellFromByteStreamCfg(flags,key=sCellType))

    # Need also TriggerTowers as input
    acc.merge(TriggerTowersInputCfg(flags))

    if flags.Trigger.L1.doeFex:
        if eFexTowerInputs==[]:
            # no input specified, so use the old eTowerMaker
            eFEXInputs = CompFactory.LVL1.eTowerMakerFromSuperCells('eTowerMakerFromSuperCells',
               eSuperCellTowerMapperTool = CompFactory.LVL1.eSuperCellTowerMapper('eSuperCellTowerMapper', SCell=sCellType))
        else:
            eFEXInputs = CompFactory.LVL1.eTowerMakerFromEfexTowers('eTowerMakerFromEfexTowers')
            if 'L1_eFexEmulatedTowers' in eFexTowerInputs:
                acc.addEventAlgo( CompFactory.LVL1.eFexTowerBuilder("eFexTowerBuilder") ) # builds the emulated towers to use as secondary input to eTowerMaker
            eFEXInputs.InputTowers = eFexTowerInputs[0]
            eFEXInputs.SecondaryInputTowers = eFexTowerInputs[1] if len(eFexTowerInputs) > 1 else ""

        eFEX = CompFactory.LVL1.eFEXDriver('eFEXDriver')
        if eFEXDebug:
            from AthenaCommon.Constants import DEBUG
            eFEX.OutputLevel = DEBUG
        eFEX.eFEXSysSimTool = CompFactory.LVL1.eFEXSysSim('eFEXSysSimTool')
        eFEX.eFEXSysSimTool.eFEXSimTool = CompFactory.LVL1.eFEXSim('eFEXSimTool')
        eFEX.eFEXSysSimTool.eFEXSimTool.eFEXFPGATool = CompFactory.LVL1.eFEXFPGA('eFEXFPGATool')
        eFEX.eFEXSysSimTool.eFEXSimTool.eFEXFPGATool.eFEXegAlgoTool = CompFactory.LVL1.eFEXegAlgo('eFEXegAlgoTool',dmCorr=deadMaterialCorrections) # only dmCorrections in data for now
        eFEX.eFEXSysSimTool.eFEXSimTool.eFEXFPGATool.eFEXtauAlgoTool = CompFactory.LVL1.eFEXtauAlgo("eFEXtauAlgo")
        eFEX.eFEXSysSimTool.eFEXSimTool.eFEXFPGATool.eFEXtauBDTAlgoTool = CompFactory.LVL1.eFEXtauBDTAlgo("eFEXtauBDTAlgo")
        # load noise cuts and dm corrections when running on data
        if not flags.Input.isMC:
            from IOVDbSvc.IOVDbSvcConfig import addFolders#, addFoldersSplitOnline
            acc.merge(addFolders(flags,"/TRIGGER/L1Calo/V1/Calibration/EfexNoiseCuts","TRIGGER_ONL",className="CondAttrListCollection"))
            if len(eFexTowerInputs)>0: eFEXInputs.NoiseCutsKey = "/TRIGGER/L1Calo/V1/Calibration/EfexNoiseCuts"
            acc.merge(addFolders(flags,"/TRIGGER/L1Calo/V1/Calibration/EfexEnergyCalib","TRIGGER_ONL",className="CondAttrListCollection")) # dmCorr from DB!
            eFEX.eFEXSysSimTool.eFEXSimTool.eFEXFPGATool.eFEXegAlgoTool.DMCorrectionsKey = "/TRIGGER/L1Calo/V1/Calibration/EfexEnergyCalib"

        acc.addEventAlgo(eFEXInputs)
        acc.addEventAlgo(eFEX)

    if flags.Trigger.L1.dojFex:
        
        if not flags.Input.isMC:
            from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import jFexInputByteStreamToolCfg
            inputjFexTool = acc.popToolsAndMerge(jFexInputByteStreamToolCfg(flags, 'jFexInputBSDecoderTool'))
            
            maybeMissingRobs = []
            decoderTools = []
            
            for module_id in inputjFexTool.ROBIDs:
                maybeMissingRobs.append(module_id)

            decoderTools += [inputjFexTool]
            decoderAlg = CompFactory.L1TriggerByteStreamDecoderAlg(name="L1TriggerByteStreamDecoder", DecoderTools=[inputjFexTool], MaybeMissingROBs=maybeMissingRobs)
            acc.addEventAlgo(decoderAlg)
        
        jFEXInputs = CompFactory.LVL1.jTowerMakerFromJfexTowers('jTowerMakerFromJfexTowers')
        jFEXInputs.IsMC = flags.Input.isMC
        jFEXInputs.jSuperCellTowerMapperTool = CompFactory.LVL1.jSuperCellTowerMapper('jSuperCellTowerMapper', SCell=sCellType)
        jFEXInputs.jSuperCellTowerMapperTool.SCellMasking = not flags.Input.isMC
        jFEX = CompFactory.LVL1.jFEXDriver('jFEXDriver')
        jFEX.jFEXSysSimTool = CompFactory.LVL1.jFEXSysSim('jFEXSysSimTool')
        acc.addEventAlgo(jFEXInputs)
        acc.addEventAlgo(jFEX)

    if flags.Trigger.L1.dogFex:
        gFEX = CompFactory.LVL1.gFEXDriver('gFEXDriver')
        gFEX.gSuperCellTowerMapperTool = CompFactory.LVL1.gSuperCellTowerMapper('gSuperCellTowerMapper', SCell=sCellType)
        gFEX.gSuperCellTowerMapperTool.SCellMasking = not flags.Input.isMC
        gFEX.gFEXSysSimTool = CompFactory.LVL1.gFEXSysSim('gFEXSysSimTool')
        acc.addEventAlgo(gFEX)

    if flags.Trigger.doHLT:
        # Check the RoI EDM containers are registered in HLT outputs
        from TrigEDMConfig.TriggerEDMRun3 import recordable
        def check(key):
            assert key==recordable(key), f'recordable() check failed for {key}'
        if flags.Trigger.L1.doeFex:
            check(eFEX.eFEXSysSimTool.Key_eFexEMOutputContainer)
            check(eFEX.eFEXSysSimTool.Key_eFexTauOutputContainer)
            check(eFEX.eFEXSysSimTool.Key_eFexTauBDTOutputContainer)
        if flags.Trigger.L1.dojFex:
            check(jFEX.jFEXSysSimTool.Key_jFexSRJetOutputContainer)
            check(jFEX.jFEXSysSimTool.Key_jFexLRJetOutputContainer)
            check(jFEX.jFEXSysSimTool.Key_jFexTauOutputContainer)
            check(jFEX.jFEXSysSimTool.Key_jFexSumETOutputContainer)
            check(jFEX.jFEXSysSimTool.Key_jFexMETOutputContainer)
            check(jFEX.jFEXSysSimTool.Key_jFexFwdElOutputContainer)
        if flags.Trigger.L1.dogFex:
            check(gFEX.gFEXSysSimTool.Key_gFexSRJetOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gFexLRJetOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gFexRhoOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gScalarEJwojOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gMETComponentsJwojOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gMHTComponentsJwojOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gMSTComponentsJwojOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gMETComponentsNoiseCutOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gMETComponentsRmsOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gScalarENoiseCutOutputContainer)
            check(gFEX.gFEXSysSimTool.Key_gScalarERmsOutputContainer)
    else:
        # Rename outputs for monitoring resimulation to avoid clash with standard SG keys
        def getSimHandle(key):
            """
            Add 'Sim' to the standard handle path
            """
            keyPath = key.path()
            keyPath += "Sim"
            key.Path = keyPath
            return key

        if flags.Trigger.L1.doeFex:
            eFEX.eFEXSysSimTool.Key_eFexEMOutputContainer=getSimHandle(eFEX.eFEXSysSimTool.Key_eFexEMOutputContainer)
            eFEX.eFEXSysSimTool.Key_eFexTauOutputContainer=getSimHandle(eFEX.eFEXSysSimTool.Key_eFexTauOutputContainer)
            eFEX.eFEXSysSimTool.Key_eFexTauBDTOutputContainer=getSimHandle(eFEX.eFEXSysSimTool.Key_eFexTauBDTOutputContainer)
            eFEX.eFEXSysSimTool.Key_eFexEMxTOBOutputContainer=getSimHandle(eFEX.eFEXSysSimTool.Key_eFexEMxTOBOutputContainer)
            eFEX.eFEXSysSimTool.Key_eFexTauxTOBOutputContainer=getSimHandle(eFEX.eFEXSysSimTool.Key_eFexTauxTOBOutputContainer)
            eFEX.eFEXSysSimTool.Key_eFexTauBDTxTOBOutputContainer=getSimHandle(eFEX.eFEXSysSimTool.Key_eFexTauBDTxTOBOutputContainer)
        if flags.Trigger.L1.dojFex:
            jFEX.jFEXSysSimTool.Key_jFexSRJetOutputContainer=getSimHandle(jFEX.jFEXSysSimTool.Key_jFexSRJetOutputContainer)
            jFEX.jFEXSysSimTool.Key_jFexLRJetOutputContainer=getSimHandle(jFEX.jFEXSysSimTool.Key_jFexLRJetOutputContainer)
            jFEX.jFEXSysSimTool.Key_jFexTauOutputContainer=getSimHandle(jFEX.jFEXSysSimTool.Key_jFexTauOutputContainer)
            jFEX.jFEXSysSimTool.Key_jFexSumETOutputContainer=getSimHandle(jFEX.jFEXSysSimTool.Key_jFexSumETOutputContainer)
            jFEX.jFEXSysSimTool.Key_jFexMETOutputContainer=getSimHandle(jFEX.jFEXSysSimTool.Key_jFexMETOutputContainer)
            jFEX.jFEXSysSimTool.Key_jFexFwdElOutputContainer=getSimHandle(jFEX.jFEXSysSimTool.Key_jFexFwdElOutputContainer)
            jFEX.jFEXSysSimTool.Key_xTobOutKey_jJ=getSimHandle(jFEX.jFEXSysSimTool.Key_xTobOutKey_jJ)
            jFEX.jFEXSysSimTool.Key_xTobOutKey_jLJ=getSimHandle(jFEX.jFEXSysSimTool.Key_xTobOutKey_jLJ)
            jFEX.jFEXSysSimTool.Key_xTobOutKey_jTau=getSimHandle(jFEX.jFEXSysSimTool.Key_xTobOutKey_jTau)
            jFEX.jFEXSysSimTool.Key_xTobOutKey_jEM=getSimHandle(jFEX.jFEXSysSimTool.Key_xTobOutKey_jEM)
        if flags.Trigger.L1.dogFex:
            gFEX.gFEXSysSimTool.Key_gFexSRJetOutputContainer=getSimHandle(gFEX.gFEXSysSimTool.Key_gFexSRJetOutputContainer)
            gFEX.gFEXSysSimTool.Key_gFexLRJetOutputContainer=getSimHandle(gFEX.gFEXSysSimTool.Key_gFexLRJetOutputContainer)
            gFEX.gFEXSysSimTool.Key_gFexRhoOutputContainer=getSimHandle(gFEX.gFEXSysSimTool.Key_gFexRhoOutputContainer)
            gFEX.gFEXSysSimTool.Key_gScalarEJwojOutputContainer=getSimHandle(gFEX.gFEXSysSimTool.Key_gScalarEJwojOutputContainer)
            gFEX.gFEXSysSimTool.Key_gMETComponentsJwojOutputContainer=getSimHandle(gFEX.gFEXSysSimTool.Key_gMETComponentsJwojOutputContainer)
            gFEX.gFEXSysSimTool.Key_gMHTComponentsJwojOutputContainer=getSimHandle(gFEX.gFEXSysSimTool.Key_gMHTComponentsJwojOutputContainer)
            gFEX.gFEXSysSimTool.Key_gMSTComponentsJwojOutputContainer=getSimHandle(gFEX.gFEXSysSimTool.Key_gMSTComponentsJwojOutputContainer)
            gFEX.gFEXSysSimTool.Key_gMETComponentsNoiseCutOutputContainer=getSimHandle(gFEX.gFEXSysSimTool.Key_gMETComponentsNoiseCutOutputContainer)
            gFEX.gFEXSysSimTool.Key_gMETComponentsRmsOutputContainer=getSimHandle(gFEX.gFEXSysSimTool.Key_gMETComponentsRmsOutputContainer)
            gFEX.gFEXSysSimTool.Key_gScalarENoiseCutOutputContainer=getSimHandle(gFEX.gFEXSysSimTool.Key_gScalarENoiseCutOutputContainer)
            gFEX.gFEXSysSimTool.Key_gScalarERmsOutputContainer=getSimHandle(gFEX.gFEXSysSimTool.Key_gScalarERmsOutputContainer)

    return acc


if __name__ == '__main__':
    ##################################################
    # Add an argument parser
    ##################################################
    import argparse
    p = argparse.ArgumentParser()
    p.add_argument('-i', '--input',
                   metavar='KEY',
                   default='ttbar',
                   help='Key of the input from TrigValInputs to be used, default=%(default)s')
    p.add_argument('-e', '--execute',
                   action='store_true',
                   help='After building the configuration, also process a few events')
    p.add_argument('-n', '--nevents',
                   metavar='N',
                   type=int,
                   default=25,
                   help='Number of events to process if --execute is used, default=%(default)s')
    p.add_argument('-d', '--efexdebug',
                   action='store_true',
                   help='Activate DEBUG mode for eFEX driver for unit tests')

    args = p.parse_args()

    ##################################################
    # Configure all the flags
    ##################################################
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from TrigValTools.TrigValSteering import Input

    flags = initConfigFlags()
    flags.Input.Files = Input.get_input(args.input).paths
    flags.Output.AODFileName = 'AOD.pool.root'
    flags.Common.isOnline = not flags.Input.isMC
    flags.Exec.MaxEvents = args.nevents
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataFlow = True
    flags.Trigger.EDMVersion = 3
    flags.Trigger.doLVL1 = True
    flags.Trigger.enableL1CaloPhase1 = True
    flags.Trigger.triggerConfig = 'FILE'

    from AthenaConfiguration.Enums import LHCPeriod
    if not flags.Input.isMC and flags.GeoModel.Run is LHCPeriod.Run2:
        flags.GeoModel.AtlasVersion = 'ATLAS-R2-2016-01-00-01'

    # Enable only calo for this test
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(flags, ['LAr','Tile','MBTS'], toggle_geometry=True)

    flags.lock()

    ##################################################
    # Set up central services: Main + Input reading + L1Menu + Output writing
    ##################################################
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaConfiguration.Enums import Format
    if flags.Input.Format == Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        acc.merge(PoolReadCfg(flags))
    else:
        from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
        acc.merge(ByteStreamReadCfg(flags))

    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg, generateL1Menu, createL1PrescalesFileFromMenu
    acc.merge(L1ConfigSvcCfg(flags))
    generateL1Menu(flags)
    createL1PrescalesFileFromMenu(flags)

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    FexEDMList = [
        'xAOD::eFexEMRoIContainer#L1_eEMRoI','xAOD::eFexEMRoIAuxContainer#L1_eEMRoIAux.',
        'xAOD::eFexTauRoIContainer#L1_eTauRoI','xAOD::eFexTauRoIAuxContainer#L1_eTauRoIAux.',
        'xAOD::jFexTauRoIContainer#L1_jFexTauRoI','xAOD::jFexTauRoIAuxContainer#L1_jFexTauRoIAux.',
        'xAOD::jFexSRJetRoIContainer#L1_jFexSRJetRoI','xAOD::jFexSRJetRoIAuxContainer#L1_jFexSRJetRoIAux.',
        'xAOD::jFexLRJetRoIContainer#L1_jFexLRJetRoI','xAOD::jFexLRJetRoIAuxContainer#L1_jFexLRJetRoIAux.',
        'xAOD::jFexMETRoIContainer#L1_jFexMETRoI','xAOD::jFexMETRoIAuxContainer#L1_jFexMETRoIAux.',
        'xAOD::jFexSumETRoIContainer#L1_jFexSumETRoI','xAOD::jFexSumETRoIAuxContainer#L1_jFexSumETRoIAux.',
        'xAOD::gFexJetRoIContainer#L1_gFexSRJetRoI','xAOD::gFexJetRoIAuxContainer#L1_gFexSRJetRoIAux.',
        'xAOD::gFexJetRoIContainer#L1_gFexLRJetRoI','xAOD::gFexJetRoIAuxContainer#L1_gFexLRJetRoIAux.',
        'xAOD::gFexJetRoIContainer#L1_gFexRhoRoI','xAOD::gFexJetRoIAuxContainer#L1_gFexRhoRoIAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gScalarEJwoj','xAOD::gFexGlobalRoIAuxContainer#L1_gScalarEJwojAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMETComponentsJwoj','xAOD::gFexGlobalRoIAuxContainer#L1_gMETComponentsJwojAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMHTComponentsJwoj','xAOD::gFexGlobalRoIAuxContainer#L1_gMHTComponentsJwojAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMSTComponentsJwoj','xAOD::gFexGlobalRoIAuxContainer#L1_gMSTComponentsJwojAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMETComponentsNoiseCut','xAOD::gFexGlobalRoIAuxContainer#L1_gMETComponentsNoiseCutAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gMETComponentsRms','xAOD::gFexGlobalRoIAuxContainer#L1_gMETComponentsRmsAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gScalarENoiseCut','xAOD::gFexGlobalRoIAuxContainer#L1_gScalarENoiseCutAux.',
        'xAOD::gFexGlobalRoIContainer#L1_gScalarERms','xAOD::gFexGlobalRoIAuxContainer#L1_gScalarERmsAux.',

    ]
    acc.merge(OutputStreamCfg(flags, 'AOD', ItemList=FexEDMList))

    ##################################################
    # The configuration fragment to be tested
    ##################################################
    acc.merge(L1CaloFEXSimCfg(flags, eFEXDebug=args.efexdebug))

    ##################################################
    # Save and optionally run the configuration
    ##################################################
    with open("L1Sim.pkl", "wb") as f:
        acc.store(f)
        f.close()

    if args.execute:
        sc = acc.run()
        if sc.isFailure():
            exit(1)
