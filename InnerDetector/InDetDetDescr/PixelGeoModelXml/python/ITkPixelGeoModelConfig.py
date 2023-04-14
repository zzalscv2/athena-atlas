# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def ITkPixelGeoModelCfg(flags,setGeometryAlignable=False,setAlignmentFolderName="/Indet/Align"):
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    acc = GeoModelCfg(flags)
    geoModelSvc = acc.getPrimary()

    from AthenaConfiguration.ComponentFactory import CompFactory
    ITkPixelDetectorTool = CompFactory.ITk.PixelDetectorTool()
    # ITkPixelDetectorTool.useDynamicAlignFolders = flags.GeoModel.Align.Dynamic
    ITkPixelDetectorTool.Alignable = setGeometryAlignable # make this a flag? Set true as soon as decided on folder structure
    ITkPixelDetectorTool.AlignmentFolderName = setAlignmentFolderName
    ITkPixelDetectorTool.DetectorName = "ITkPixel"
    if flags.ITk.Geometry.PixelLocal:
        # Setting this filename triggers reading from local file rather than DB
        ITkPixelDetectorTool.GmxFilename = flags.ITk.Geometry.PixelFilename
    if flags.ITk.Geometry.PixelClobOutputName:
        ITkPixelDetectorTool.ClobOutputName = flags.ITk.Geometry.PixelClobOutputName
    geoModelSvc.DetectorTools += [ ITkPixelDetectorTool ]
    return acc


def ITkPixelAlignmentCfg(flags,setGeometryAlignable=False,setAlignmentFolderName="/Indet/Align"):
    if flags.GeoModel.Align.LegacyConditionsAccess:  # revert to old style CondHandle in case of simulation
        from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline
        return addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/Align", setAlignmentFolderName)
    else:
        from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelAlignCondAlgCfg
        return ITkPixelAlignCondAlgCfg(flags,setAlignmentFolderName=setAlignmentFolderName)


def ITkPixelSimulationGeometryCfg(flags,setGeometryAlignable=False,setAlignmentFolderName="/Indet/Align"):
    # main GeoModel config
    acc = ITkPixelGeoModelCfg(flags,setGeometryAlignable=setGeometryAlignable,setAlignmentFolderName=setAlignmentFolderName)
    acc.merge(ITkPixelAlignmentCfg(flags,setGeometryAlignable=setGeometryAlignable,setAlignmentFolderName=setAlignmentFolderName))
    return acc


def ITkPixelReadoutGeometryCfg(flags,setGeometryAlignable=False,setAlignmentFolderName="/Indet/Align"):
    # main GeoModel config
    acc = ITkPixelGeoModelCfg(flags,setGeometryAlignable=setGeometryAlignable,setAlignmentFolderName=setAlignmentFolderName)
    acc.merge(ITkPixelAlignmentCfg(flags,setGeometryAlignable=setGeometryAlignable,setAlignmentFolderName=setAlignmentFolderName))
    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import ITkPixelDetectorElementCondAlgCfg
    acc.merge(ITkPixelDetectorElementCondAlgCfg(flags,setAlignmentFolderName=setAlignmentFolderName))
    return acc
