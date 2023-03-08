#!/bin/env python

# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# file   ZDCRecConfig.py
# author Peter Steinberg <steinberg@bnl.gov>
# date   2022-07-31

# brief  A script that provides setup for ZDC reconstruction included in RecJobTransforms/RecoSteering.py. 
#        It also allows to run standalone ZDC reconstruction. To use it:
#           0. setup athena enviroment
#           1a. run this script:
#               $ python ZDCRecConfig.py
#           1b. feel free to change the input file:
#               $ python ZDCRecConfig.py --filesInput=/eos/atlas/atlascerngroupdisk/det-zdc/ZDCRuns/2021/data21_900GeV/main/data21_900GeV.00405396.physics_Main.daq.RAW/data21_900GeV.00405396.physics_Main.daq.RAW._lb0211._SFO-13._0001.data
#           1c. run the whole reconstruction with:
#               $ RecoSteeringTest.py --filesInput=/eos/atlas/atlascerngroupdisk/det-zdc/ZDCRuns/2021/data21_900GeV/main/data21_900GeV.00405396.physics_Main.daq.RAW/data21_900GeV.00405396.physics_Main.daq.RAW._lb0211._SFO-13._0001.data --RAW

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import Format

def ZDCRecOutputCfg(flags):
    """defines outputs for ESD and AOD; provides the same information as in ForwardRec/ZDC_Rec_OutputItemList_jobOptions.py"""
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc = ComponentAccumulator()
    
    ZDC_ItemList=[]

    if flags.Input.Format is Format.BS:
        # ZDC Silicon hits containers
        ZDC_ItemList.append("xAOD::ZdcModuleContainer#ZdcModules")
        ZDC_ItemList.append("xAOD::ZdcModuleAuxContainer#ZdcModulesAux.")

    if flags.Output.doWriteESD:
        acc.merge(OutputStreamCfg(flags, "ESD", ZDC_ItemList))
    if flags.Output.doWriteAOD:
        acc.merge(OutputStreamCfg(flags, "AOD", ZDC_ItemList))
    return acc


def ZDCRecCfg(flags):
    """defines ZDC reconstruction; provides the same setup as used to be in ForwardRec/ForwardRec_jobOptions.py"""
    acc = ComponentAccumulator()
    
    if flags.Input.Format is Format.BS:
        from AthenaConfiguration.ComponentFactory import CompFactory
        
        acc.addEventAlgo(CompFactory.ZdcByteStreamLucrodData("ZdcByteStreamLucrodData"))
        acc.addEventAlgo(CompFactory.ZdcRecRun3("ZdcRecRun3"))
    
    # Setup output
    if flags.Output.doWriteESD or flags.Output.doWriteAOD:
        acc.merge(ZDCRecOutputCfg(flags))
    
    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.EnableVerboseViews = True
    
    flags.Input.Files = ["/eos/atlas/atlascerngroupdisk/det-zdc/ZDCRuns/2021/data21_900GeV/main/data21_900GeV.00405396.physics_Main.daq.RAW/data21_900GeV.00405396.physics_Main.daq.RAW._lb0211._SFO-13._0001.data"]
    
    flags.Exec.MaxEvents=500
    flags.Concurrency.NumThreads=4
 
    flags.fillFromArgs() # enable unit tests to switch only parts of reco: python -m HIRecConfig.HIRecConfig HeavyIon.doGlobal = 0 and so on
    flags.lock()
    flags.dump()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    acc.getEventAlgo("SGInputLoader").FailIfNoProxy = True # enforce no missing data
    
    
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))

    acc.merge(ZDCRecCfg(flags))
    
    from AthenaCommon.Constants import DEBUG
    acc.foreach_component("*ZDC*").OutputLevel=DEBUG
    
    acc.printConfig(withDetails=True, summariseProps=True)
    
    status = acc.run()
    if status.isFailure():
        import sys
        sys.exit(-1)


