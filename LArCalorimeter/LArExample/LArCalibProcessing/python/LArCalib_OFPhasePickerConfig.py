# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory 
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from LArCalibProcessing.utils import FolderTagResolver
from IOVDbSvc.IOVDbSvcConfig import addFolders

def _OFPhasePickerCfg(flags, inputSuffix="4samples3bins17phases",outputSuffix="4samples1phase",keySuffix=""):

    result=ComponentAccumulator()
    FolderTagResolver._globalTag=flags.IOVDb.GlobalTag
    rs=FolderTagResolver()
    inputOFCTag=rs.getFolderTag(flags.LArCalib.OFCPhys.Folder+inputSuffix)
    inputShapeTag=rs.getFolderTag(flags.LArCalib.Shape.Folder+inputSuffix)

    outputOFCTag=rs.getFolderTag(flags.LArCalib.OFCPhys.Folder+outputSuffix)
    outputShapeTag=rs.getFolderTag(flags.LArCalib.Shape.Folder+outputSuffix)


    del rs #Close database

    from LArCalibProcessing.LArCalibBaseConfig import chanSelStr

    result.merge(addFolders(flags,flags.LArCalib.OFCPhys.Folder+inputSuffix,detDb=flags.LArCalib.Input.Database, 
                            tag=inputOFCTag, modifiers=chanSelStr(flags)+"<key>LArOFCIn"+keySuffix+"</key>"))
    result.merge(addFolders(flags,flags.LArCalib.Shape.Folder+inputSuffix,detDb=flags.LArCalib.Input.Database, 
                            tag=inputShapeTag, modifiers=chanSelStr(flags)+"<key>LArShapeIn"+keySuffix+"</key>"))


    LArOFPhasePick = CompFactory.LArOFPhasePicker("LArOFPhasePicker"+keySuffix)
    if not flags.LArCalib.isSC:
        LArOFPhasePick.KeyPhase = "LArOFCPhase"
    LArOFPhasePick.KeyOFC_new = "LArOFC"+keySuffix
    LArOFPhasePick.KeyOFC = "LArOFCIn"+keySuffix
    LArOFPhasePick.KeyShape_new = "LArShape"+keySuffix+"_uncorr" if flags.LArCalib.OFC.ShapeCorrection else  "LArShape"+keySuffix
    LArOFPhasePick.KeyShape = "LArShapeIn"+keySuffix
    LArOFPhasePick.GroupingType = flags.LArCalib.GroupingType
    LArOFPhasePick.DefaultPhase = 1
    LArOFPhasePick.TimeOffsetCorrection = 0
    LArOFPhasePick.KeyPhase = ""
    LArOFPhasePick.isSC = flags.LArCalib.isSC

    result.addEventAlgo(LArOFPhasePick)

    if flags.LArCalib.OFC.ShapeCorrection:
        result.merge(addFolders(flags,"/LAR/ElecCalibOfl/Shape/Residuals/5samples","LAR_OFL"))
        resShapeCorr=CompFactory.LArShapeCorrector("LArShapeCorr"+keySuffix)
        resShapeCorr.KeyShape= "LArShape"+keySuffix+"_uncorr" 
        resShapeCorr.KeyShape_newcorr="LArShape"+keySuffix
        result.addEventAlgo(resShapeCorr)

    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    result.merge(OutputConditionsAlgCfg(flags,
                                        outputFile=flags.LArCalib.Output.POOLFile,
                                        ObjectList=["LArOFCComplete#LArOFC"+keySuffix+"#"+flags.LArCalib.OFCPhys.Folder+outputSuffix,
                                                    "LArShapeComplete#LArShape"+keySuffix+"#"+flags.LArCalib.Shape.Folder+outputSuffix],
                                        IOVTagList=[outputOFCTag,outputShapeTag]))


    rootfile=flags.LArCalib.Output.ROOTFile
    if rootfile != "":
        OFC2Ntup=CompFactory.LArOFC2Ntuple("LArOFC2Ntuple"+keySuffix)
        OFC2Ntup.ContainerKey = "LArOFC"+keySuffix
        OFC2Ntup.NtupleName   = "OFC"+keySuffix
        OFC2Ntup.AddFEBTempInfo   = False   
        OFC2Ntup.isSC = flags.LArCalib.isSC
        result.addEventAlgo(OFC2Ntup)

        Shape2Ntup=CompFactory.LArShape2Ntuple("LArShape2Ntuple"+keySuffix)
        Shape2Ntup.ContainerKey="LArShape"+keySuffix
        Shape2Ntup.NtupleName="SHAPE"+keySuffix
        Shape2Ntup.AddFEBTempInfo   = False
        Shape2Ntup.isSC = flags.LArCalib.isSC
        result.addEventAlgo(Shape2Ntup)

    
    return result

def LArOFPhasePickerCfg(flags):

    #Get basic services and cond-algos
    from LArCalibProcessing.LArCalibBaseConfig import LArCalibBaseCfg
    result=LArCalibBaseCfg(flags)

    result.merge(_OFPhasePickerCfg(flags, inputSuffix="4samples3bins17phases",outputSuffix="4samples1phase",keySuffix=""))
    

    #RegistrationSvc    
    result.addService(CompFactory.IOVRegistrationSvc(RecreateFolders = False))
    result.getService("IOVDbSvc").DBInstance=""

    #Ntuple writing
    rootfile=flags.LArCalib.Output.ROOTFile
    if rootfile != "":
        import os
        if os.path.exists(rootfile):
            os.remove(rootfile)
        result.addService(CompFactory.NTupleSvc(Output = [ "FILE1 DATAFILE='"+rootfile+"' OPT='NEW'" ]))
        result.setAppProperty("HistogramPersistency","ROOT")
        pass # end if ROOT ntuple writing


    #MC Event selector since we have no input data file
    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    result.merge(McEventSelectorCfg(flags,
                                    RunNumber         = flags.LArCalib.Input.RunNumbers[0],
                                    EventsPerRun      = 1,
                                    FirstEvent	      = 1,
                                    InitialTimeStamp  = 0,
                                    TimeStampInterval = 1))

    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    result.merge(PerfMonMTSvcCfg(flags))
    

    
    return result
