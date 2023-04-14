# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def ITkStripGeoModelCfg(flags,setGeometryAlignable=False,setAlignmentFolderName="/Indet/Align"):
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc = GeoModelCfg(flags)
    geoModelSvc = acc.getPrimary()

    from AthenaConfiguration.ComponentFactory import CompFactory
    ITkStripDetectorTool = CompFactory.ITk.StripDetectorTool()
    # ITkStripDetectorTool.useDynamicAlignFolders = flags.GeoModel.Align.Dynamic #Will we need to do dynamic alignment for ITk?
    ITkStripDetectorTool.Alignable = setGeometryAlignable # make this a flag? Set true as soon as decided on folder structure
    ITkStripDetectorTool.AlignmentFolderName = setAlignmentFolderName
    ITkStripDetectorTool.DetectorName = "ITkStrip"
    if flags.ITk.Geometry.StripLocal:
        # Setting this filename triggers reading from local file rather than DB
        ITkStripDetectorTool.GmxFilename = flags.ITk.Geometry.StripFilename
    if flags.ITk.Geometry.StripClobOutputName:
        ITkStripDetectorTool.ClobOutputName = flags.ITk.Geometry.StripClobOutputName
    geoModelSvc.DetectorTools += [ ITkStripDetectorTool ]
    return acc


def ITkStripAlignmentCfg(flags,setGeometryAlignable=False,setAlignmentFolderName="/Indet/Align"):
    if flags.GeoModel.Align.LegacyConditionsAccess:  # revert to old style CondHandle in case of simulation
        from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
        return addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/Align", setAlignmentFolderName)
    else:
        from SCT_ConditionsAlgorithms.ITkStripConditionsAlgorithmsConfig import ITkStripAlignCondAlgCfg
        return ITkStripAlignCondAlgCfg(flags,setGeometryAlignable=setGeometryAlignable,setAlignmentFolderName=setAlignmentFolderName)


def ITkStripSimulationGeometryCfg(flags,setGeometryAlignable=False,setAlignmentFolderName="/Indet/Align"):
    # main GeoModel config
    acc = ITkStripGeoModelCfg(flags,setGeometryAlignable=setGeometryAlignable,setAlignmentFolderName=setAlignmentFolderName)
    acc.merge(ITkStripAlignmentCfg(flags,setGeometryAlignable=setGeometryAlignable,setAlignmentFolderName=setAlignmentFolderName))
    return acc


def ITkStripReadoutGeometryCfg(flags,setGeometryAlignable=False,setAlignmentFolderName="/Indet/Align"):
    # main GeoModel config
    acc = ITkStripGeoModelCfg(flags,setGeometryAlignable=setGeometryAlignable,setAlignmentFolderName=setAlignmentFolderName)
    acc.merge(ITkStripAlignmentCfg(flags,setGeometryAlignable=setGeometryAlignable,setAlignmentFolderName=setAlignmentFolderName))
    from SCT_ConditionsAlgorithms.ITkStripConditionsAlgorithmsConfig import ITkStripDetectorElementCondAlgCfg
    acc.merge(ITkStripDetectorElementCondAlgCfg(flags,setGeometryAlignable=setGeometryAlignable,setAlignmentFolderName=setAlignmentFolderName))
    return acc
