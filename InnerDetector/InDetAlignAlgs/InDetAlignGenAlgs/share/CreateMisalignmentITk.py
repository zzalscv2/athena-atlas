#!/usr/bin/env python

#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#####################################################################
# JobOptions for CreateMisalignAlg which creates a misaligned copy
# of the geometry database to introduce misalignments at reconstruction level
# This is configured to run with ITk geometries
##############################################################

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg    
from InDetAlignGenAlgs.InDetAlignAlgsConfig import CreateITkMisalignAlgCfg
from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
from InDetAlignGenTools.InDetAlignGenToolsConfig import ITkAlignDBTool
import sys
from AthenaConfiguration.ComponentFactory import CompFactory

def getFlags(**kwargs):
    flags=initConfigFlags()
    flags.Input.RunNumber = 2222222 # Set to either MC DSID or MC Run Number

    ## Just enable ID for the moment.
    flags.Input.isMC             = True

    flags.Input.Files = []
    flags.ITk.Geometry.AllLocal = False
    detectors = [
    "ITkPixel",
    "ITkStrip",
    "Bpipe"
    ]
    setupDetectorFlags(flags, custom_list=detectors, toggle_geometry=True)
    flags.TrackingGeometry.MaterialSource = "Input"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    flags.GeoModel.Align.Dynamic = False

    #Define the output database file name and add it to the flags
    if 'MisalignMode' not in kwargs.keys():
        MisalignMode = 11 # Radial
    else:
        MisalignMode=int(kwargs.get('MisalignMode',11))
    databaseFilename     = 'MisalignmentSet%s.db' % (MisalignMode)
    flags.IOVDb.DBConnection="sqlite://;schema=%s;dbname=OFLCOND" % (databaseFilename) 
    flags.IOVDb.GlobalTag = "OFLCOND-MC21-SDR-RUN4-01"

    # This should run serially for the moment.
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.lock()

    return flags

def CreateMis(flags,name="CreateITkMisalignAlg",**kwargs):
    #
    #--------------------------------------------------------------
    # Geometry section
    #--------------------------------------------------------------
    misalignmentOnTopOfExistingSet = False
    readDBPoolFile = False

    createFreshDB = not(readDBPoolFile or misalignmentOnTopOfExistingSet)

    MisalignMode = kwargs.pop('MisalignMode',11)

    shiftInMicrons = 100

    if MisalignMode in [11, 12, 31]:
        shiftInMicrons = 500
    outFiles = 'MisalignmentSet%s' % (MisalignMode)
    misalignModeMap = {0:'no Misalignment',
                    1: 'misalignment by 6 parameters',
                    2: 'random misalignment',
                    3: 'IBL-stave temperature dependent bowing',
                    11: 'R deltaR (radial expansion)', 12: 'Phi deltaR (ellipse)',13: 'Z deltaR (funnel)',
                    21: 'R deltaPhi (curl)', 22: 'Phi deltaPhi (clamshell) ',23:'Z deltaPhi (twist)',
                    31: 'R deltaZ (telescope)',32:'Phi deltaZ (skew)',33:'Z deltaZ (z-expansion)'}
    ####################################################################################################################

    acc=MainServicesCfg(flags)
    print ("\n CreateMisalignAlg: Creation of misalignment mode %s: %s \n" % (int(MisalignMode),misalignModeMap.get(int(MisalignMode),'unknown')))
    kwargs.setdefault("ASCIIFilenameBase",outFiles)
    kwargs.setdefault("SQLiteTag",'MisalignmentMode_'+str(misalignModeMap.get(int(MisalignMode),'unknown')))
    kwargs.setdefault("MisalignMode",int(MisalignMode))
    kwargs.setdefault("MaxShift",shiftInMicrons)
    kwargs.setdefault("CreateFreshDB",createFreshDB)
    #Create and configure the AlignDB tool
    itkAlignFolder="/Indet/AlignITk"
    AlignFolder="/Indet/Align"
    writeDBPoolFile=True   #Activate or deactivate writing to outFiles + '.pool.root'
    kargsTool={}
    kargsTool.setdefault("SCTTwoSide",True)
    kargsTool.setdefault("DBRoot",itkAlignFolder)
    kargsTool.setdefault("DBKey",itkAlignFolder)
    kargsTool.setdefault("forceUserDBConfig",True)
    if writeDBPoolFile:
        InDetCondStream=CompFactory.AthenaOutputStreamTool(OutputFile = outFiles+'.pool.root')
        InDetCondStream.PoolContainerPrefix="<type>"
        InDetCondStream.TopLevelContainerName=""
        InDetCondStream.SubLevelBranchName="<key>"
        kargsTool.setdefault("CondStream",InDetCondStream)
        kargsTool.setdefault("DBRoot",AlignFolder)
        kargsTool.setdefault("DBKey",AlignFolder)
    dbTool = acc.popToolsAndMerge(ITkAlignDBTool(flags,**kargsTool))

    kwargs.setdefault("IDAlignDBTool",dbTool)

    cfg=CreateITkMisalignAlgCfg(flags,name=name,SetITkPixelAlignable=True,SetITkStripAlignable=True,setAlignmentFolderName=AlignFolder,**kwargs)
    acc.merge(cfg)
    if writeDBPoolFile:
        print("To be writen DB pool File")
    return acc

if __name__ == "__main__":
    if(len(sys.argv[1:])):
        kwargs=dict(arg.split('=') for arg in sys.argv[1:])
        print(kwargs)
    else:
        kwargs={}
        print("no args")
    #Get the basic configuration flags
    flags=getFlags(**kwargs)
    flags.dump()
    #Add the tools and the algorithm to the accumulator
    acc=CreateMis(flags,**kwargs)
    acc.printConfig()
    #run
    sc=acc.run(10)
    if sc.isFailure():
        print("Failed to run the Misalignment Algorithm")
        sys.exit(-1)
