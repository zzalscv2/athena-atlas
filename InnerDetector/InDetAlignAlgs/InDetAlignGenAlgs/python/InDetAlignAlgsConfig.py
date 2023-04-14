# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def CreateITkMisalignAlgCfg(flags, name="CreateITkMisalignAlg",SetITkPixelAlignable=False,SetITkStripAlignable=False,setAlignmentFolderName="/Indet/AlignITk", **kwargs):
    kwargs.setdefault("MisalignmentX",0.0)
    kwargs.setdefault("MisalignmentY",0.0)
    kwargs.setdefault("MisalignmentZ",0.0)
    kwargs.setdefault("MisalignmentAlpha",0.0)
    kwargs.setdefault("MisalignmentBeta",0.0)
    kwargs.setdefault("MisalignmentGamma",0.0)
    kwargs.setdefault("ScalePixelIBL",0.0)
    kwargs.setdefault("IBLBowingTshift",0.0)
    kwargs.setdefault("ScalePixelBarrel",1.0)
    kwargs.setdefault("ScalePixelEndcap",1.0)
    kwargs.setdefault("ScaleSCTBarrel",0.0)
    kwargs.setdefault("ScaleSCTEndcap",0.0)

    result = ComponentAccumulator()
    
    if flags.Detector.EnableITkPixel:
        from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
        result.merge(ITkPixelReadoutGeometryCfg(flags,setGeometryAlignable=SetITkPixelAlignable,setAlignmentFolderName=setAlignmentFolderName))
        kwargs.setdefault("PixelDetEleCollKey","ITkPixelDetectorElementCollection")
    else:
        kwargs.setdefault("PixelDetEleCollKey","")

    if flags.Detector.EnableITkStrip:    
        from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
        result.merge(ITkStripReadoutGeometryCfg(flags,setGeometryAlignable=SetITkStripAlignable,setAlignmentFolderName=setAlignmentFolderName))
        kwargs.setdefault("SCTDetEleCollKey","ITkStripDetectorElementCollection")
    else:
        kwargs.setdefault("SCTDetEleCollKey","")

    from InDetAlignGenTools.InDetAlignGenToolsConfig import ITkAlignDBTool
    dbTool = result.popToolsAndMerge(ITkAlignDBTool(flags,setAlignmentFolderName=setAlignmentFolderName))

    kwargs.setdefault("IDAlignDBTool",dbTool)
    kwargs.setdefault("TRTDetEleCollKey","")

    histoSvc = CompFactory.THistSvc(Output = ["IDENTIFIERTREE DATAFILE='ITkIdentifierTree.root' TYPE='ROOT' OPT='RECREATE'"])
    result.addService( histoSvc )
    ntupSvc = CompFactory.NTupleSvc(Output = ["CREATEMISALIGN DATAFILE='CreateMisalignmentITk.root' TYP='ROOT' OPT='NEW'"])
    result.addService( ntupSvc )
    result.setAppProperty("HistogramPersistency","ROOT")


    #from AthenaCommon.Constants import VERBOSE
    #kwargs.setdefault("OutputLevel",VERBOSE)
    result.addEventAlgo(CompFactory.InDetAlignment.CreateMisalignAlg(name,**kwargs))
    return result

def CreateInDetMisalignAlgCfg(flags, name="CreateInDetMisalignAlg",setAlignmentFolderName="/Indet/Align", **kwargs):
    result = ComponentAccumulator()
    
    if flags.Detector.EnablePixel:
        from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
        result.merge(PixelReadoutGeometryCfg(flags))
    else:
        kwargs.setdefault("PixelDetEleCollKey","")
    
    if flags.Detector.EnableSCT:    
        from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
        result.merge(SCT_ReadoutGeometryCfg(flags))
    else:
        kwargs.setdefault("SCTDetEleCollKey","")
    
    if flags.Detector.EnableTRT:    
        from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
        result.merge(TRT_ReadoutGeometryCfg(flags))
    else:
        kwargs.setdefault("TRTDetEleCollKey","")

    from InDetAlignGenTools.InDetAlignGenToolsConfig import InDetAlignDBTool
    dbTool = result.popToolsAndMerge(InDetAlignDBTool(flags,setAlignmentFolderName=setAlignmentFolderName))

    histoSvc = CompFactory.THistSvc(Output = ["IDENTIFIERTREE DATAFILE='InDetIdentifierTree.root' TYPE='ROOT' OPT='RECREATE'"])
    result.addService( histoSvc )
    ntupSvc = CompFactory.NTupleSvc(Output = ["CREATEMISALIGN DATAFILE='CreateMisalignmentInDet.root' TYP='ROOT' OPT='NEW'"])
    result.addService( ntupSvc )
    result.setAppProperty("HistogramPersistency","ROOT")

    kwargs.setdefault("IDAlignDBTool",dbTool)
    result.addEventAlgo(CompFactory.InDetAlignment.CreateMisalignAlg(name,**kwargs))
    return result