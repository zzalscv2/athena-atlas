#!/bin/env python

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# file   testAFPDB.py
# author Petr Balek <petr.balek@cern.ch> (with a lot of inspiration from Tomasz Bold)
# date   2021-03-22

# brief  A script that will test reading from the local DB file. In order to test it:
#           0. setup athena enviroment
#           1. have the local database; either get it from somewhere else (will be done in future), or create a new one with AFPLocalAlignDBCreate.py:
#               $ python AFPLocalAlignDBCreate.py
#           2. run this script as (feel free to change the input file):
#               $ python testAFPDB.py --filesInput=/afs/cern.ch/user/p/pbalek/workspace/public/data17_13TeV.00338480.physics_Main.daq.RAW/data17_13TeV.00338480.physics_Main.daq.RAW._lb0275._SFO-7._0007.data
#           2a. for another alignment constants, try this input file (n.b.: local constants are the same, only global are different):
#               $ python testAFPDB.py --filesInput=/afs/cern.ch/user/p/pbalek/workspace/public/data17_13TeV.00333380.physics_Main.daq.RAW/data17_13TeV.00333380.physics_Main.daq.RAW._lb0163._SFO-7._0001.data
#           3. the script will read the files and print out alignment variables for the events in the input file (based on run number and LB)

#           footnote: for the python setup with TopLocRecSeq, see AFP_LocReco/AFP_LocReco.py

from AthenaConfiguration.ComponentAccumulator import CompFactory, ComponentAccumulator

def testAFPDBCfg(flags):
    acc = ComponentAccumulator()

    from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
    from IOVDbSvc.IOVDbSvcConfig import addFolders

    # these are two randomly picked conditions, checked for comparison
    acc.merge(addFolders(flags, '/CALO/HadCalibration2/CaloEMFrac', 'CALO_ONL', className='CaloLocalHadCoeff', db='CONDBR2'))
    acc.merge(addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/Beampos", "/Indet/Beampos", className="AthenaAttributeList"))

    
    # set from where to read the local information - from DB
#     acc.merge(addFolders(flags, "/FWD/Onl/AFP/Align/Local", 'FWD_ONL', className='CondAttrListCollection', tag='AFPAlignLoc-02', db="CONDBR2"))
#     acc.merge(addFolders(flags, "/FWD/Onl/AFP/Align/Global", 'FWD_ONL', className='CondAttrListCollection', tag='AFPAlignGlob-01', db="CONDBR2"))
    
    # set from where to read the local information - from local file
#     schema = "<db>sqlite://;schema=Example.db;dbname=CONDBR2</db>"
#     locFolder = "/FWD/Onl/AFP/ToFParameters/Local"
#     locTag = "<tag>AFPToFLoc-01</tag>"
#     vtxFolder = "/FWD/Onl/AFP/ToFParameters/Vertex"
#     vtxTag = "<tag>AFPToFVtx-01</tag>"
#     acc.merge(addFolders(flags, schema+locFolder+locTag, className='CondAttrListCollection', db='CONDBR2' ))
#     acc.merge(addFolders(flags, schema+vtxFolder+vtxTag, className='CondAttrListCollection', db='CONDBR2' ))  
#     
#     acc.addCondAlgo(CompFactory.AFPDBTester())

    
    # this will read from DB for MC (note that the source file is still data that provides run and LB, thus this is really for testing only)
    schema = "<db>sqlite://;schema=ExampleMC_Align.db;dbname=OFLP200</db>"
    locFolder = "/FWD/AFP/Align/Local"
    locTag = "<tag>AFPMCAlignLoc-329484-02</tag>"
    globFolder = "/FWD/AFP/Align/Global"
    globTag = "<tag>AFPMCAlignGlob-331020-01</tag>"
    acc.merge(addFolders(flags, schema+locFolder+locTag, className='CondAttrListCollection', db='OFLP200' ))
    acc.merge(addFolders(flags, schema+globFolder+globTag, className='CondAttrListCollection', db='OFLP200' )) 
    
    schema2 = "<db>sqlite://;schema=ExampleMC_ToF.db;dbname=OFLP200</db>"
    acc.merge(addFolders(flags, schema2+"/FWD/AFP/ToFParameters/Local", className='CondAttrListCollection', tag='AFPMCToFLoc-ideal-01', db='OFLP200' ))
    acc.merge(addFolders(flags, schema2+"/FWD/AFP/ToFParameters/Vertex", className='CondAttrListCollection', tag='AFPMCToFVtx-ideal-01', db='OFLP200' )) 
    
    
    acc.addCondAlgo(CompFactory.AFPDBTester("AFPDBTester", locshiftXkey="/FWD/AFP/Align/Local", globshiftXkey="/FWD/AFP/Align/Global", locToFkey="/FWD/AFP/ToFParameters/Local", vtxToFkey="/FWD/AFP/ToFParameters/Vertex"))

    

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    #flags.Input.Files = [] # can hardcode file
    # if you have issues wiht data dependencies and want to debug it
    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.EnableVerboseViews = True
        
    # more threads and more concurent events; change both to 1 if something goes wrong
    flags.Concurrency.NumThreads = 3
    flags.Concurrency.NumConcurrentEvents = 5

    flags.Exec.MaxEvents = 500

    # AFP align constants are not included in these (yet)
    flags.IOVDb.GlobalTag = "CONDBR2-BLKPA-2017-16"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2

    acc = MainServicesCfg(flags)
    parser = flags.getArgumentParser()
    args = flags.fillFromArgs(parser=parser)

    flags.lock()


    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))

    acc.merge(testAFPDBCfg(flags))
    from AthenaCommon.Constants import DEBUG, VERBOSE
    acc.foreach_component("*AFP*").OutputLevel=VERBOSE
    acc.foreach_component("AFPDBTester").OutputLevel=DEBUG
    acc.printConfig(withDetails=True, summariseProps=True)
    acc.run()
