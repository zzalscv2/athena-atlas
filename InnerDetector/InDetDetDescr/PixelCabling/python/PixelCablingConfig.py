# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr

def getPixelCablingSvc(name="PixelCablingSvc", **kwargs):
    from AthenaCommon.Logging import logging
    logger = logging.getLogger("PixelCablingSvc")
    from IOVDbSvc.CondDB import conddb
    from AthenaCommon.GlobalFlags import globalflags
    from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags as commonGeoFlags
    from AtlasGeoModel.InDetGMJobProperties import InDetGeometryFlags as geoFlags
    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags

    from AthenaCommon.AlgSequence import AthSequencer
    condSeq = AthSequencer("AthCondSeq")

    if not conddb.folderRequested("/PIXEL/ReadoutSpeed"):
        if not (globalflags.DataSource() == 'geant4'):
            conddb.addFolder("PIXEL", "/PIXEL/ReadoutSpeed", className="AthenaAttributeList")
        else:
            conddb.addFolderSplitMC("PIXEL","/PIXEL/ReadoutSpeed","/PIXEL/ReadoutSpeed", className="AthenaAttributeList")

    from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelReadoutSpeedAlg
    from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelCablingCondAlg
    pixelCablingArgs = {}
    pixelCablingArgs['UseConditions']=False
    pixelCablingArgs['MappingFile']="PixelCabling/Pixels_Atlas_IdMapping_2016.dat"
    pixelCablingArgs['RodIDForSingleLink40']=0
    pixelCablingArgs['DumpMapToFile']=False

    if (conddb.dbdata=="CONDBR2" or (conddb.dbmc=="OFLP200" and geoFlags.isIBL()==True)) and not conddb.folderRequested("/PIXEL/HitDiscCnfg"):
        conddb.addFolderSplitMC("PIXEL","/PIXEL/HitDiscCnfg","/PIXEL/HitDiscCnfg", className="AthenaAttributeList")
        from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelHitDiscCnfgAlg
        condSeq += PixelHitDiscCnfgAlg(name="PixelHitDiscCnfgAlg")

    # SIMULATION
    if (globalflags.DataSource() == 'geant4'):
        logger.debug("Running on simulation")

        pixelCablingArgs['UseConditions']=False
        pixelCablingArgs['RodIDForSingleLink40']=0

        # ITk:
        if geoFlags.isSLHC():
            IdMappingDat = "ITk_Atlas_IdMapping.dat"
            if "BrlIncl4.0_ref" == commonGeoFlags.GeoType():
                IdMappingDat = "ITk_Atlas_IdMapping_InclBrl4.dat"
            elif "IBrlExt4.0ref" == commonGeoFlags.GeoType():
                IdMappingDat = "ITk_Atlas_IdMapping_IExtBrl4.dat"
            elif "BrlExt4.0_ref" == commonGeoFlags.GeoType():
                IdMappingDat = "ITk_Atlas_IdMapping_ExtBrl4.dat"
            elif "BrlExt3.2_ref" == commonGeoFlags.GeoType():
                IdMappingDat = "ITk_Atlas_IdMapping_ExtBrl32.dat"
            logger.info("Using ITk pixel mapping: %s" % IdMappingDat)
            pixelCablingArgs['MappingFile']=IdMappingDat


        # No IBL
        elif (geoFlags.isIBL() == False):
            pixelCablingArgs['MappingFile']="PixelCabling/Pixels_Atlas_IdMapping.dat"

        else:
            # Planar IBL
            if (geoFlags.IBLLayout() == "planar"):

                # DBM or not
                if (geoFlags.isDBM() == True):
                    pixelCablingArgs['MappingFile']="PixelCabling/Pixels_Atlas_IdMapping_inclIBL_DBM.dat"
                else:
                    pixelCablingArgs['MappingFile']="PixelCabling/Pixels_Atlas_IdMapping_inclIBL.dat"

            # Hybrid IBL plus DBM
            elif (geoFlags.IBLLayout() == "3D"):
                pixelCablingArgs['MappingFile']="PixelCabling/Pixels_Atlas_IdMapping_Run2.dat"

            elif (geoFlags.IBLLayout() == "UNDEFINED"):
                logger.warning("Got GeometryFlags.isIBL = True, but IBLLayout is UNDEFINED")
                logger.warning("Pixel cabling map cannot be set at this point")


    # DATA
    elif (globalflags.DataSource == 'data'):

        from RecExConfig.AutoConfiguration import GetRunNumber
        runNum = GetRunNumber()
        logger.debug("Running on data, run number %d" % runNum)

        # For data older than run number 222222, use the appropriate text file
        if (runNum < 222222):
            pixelCablingArgs['UseConditions']=False
            pixelCablingArgs['MappingFile']="PixelCabling/Pixels_Atlas_IdMapping_May08.dat"
            pixelCablingArgs['RodIDForSingleLink40']=1300000

        # For Run-2 onwards, get cabling map from database
        else:
            if not conddb.folderRequested("/PIXEL/CablingMap"):
                conddb.addFolderSplitOnline("PIXEL", "/PIXEL/Onl/CablingMap","/PIXEL/CablingMap", className="AthenaAttributeList")
            pixelCablingArgs['UseConditions']=True
            pixelCablingArgs['RodIDForSingleLink40']=1300000

            # Even though we are reading from COOL, set the correct fallback map.
            if (runNum >= 344494):
                pixelCablingArgs['MappingFile']="PixelCabling/Pixels_Atlas_IdMapping_344494.dat"
            elif (runNum >= 314940 and runNum < 344494):
                pixelCablingArgs['MappingFile']="PixelCabling/Pixels_Atlas_IdMapping_314940.dat"
            elif (runNum >= 289350 and runNum < 314940): # 2016
                pixelCablingArgs['MappingFile']="PixelCabling/Pixels_Atlas_IdMapping_2016.dat"
            elif (runNum >= 222222 and runNum < 289350): # 2015
                pixelCablingArgs['MappingFile']="PixelCabling/Pixels_Atlas_IdMapping_Run2.dat"
            else:
                pixelCablingArgs['MappingFile']="PixelCabling/Pixels_Atlas_IdMapping_May08.dat"

    # Unknown input
    else:
        logger.warning("Unknown input source. Pixel cabling map cannot be set at this point")

    condSeq += PixelReadoutSpeedAlg(name="PixelReadoutSpeedAlg")
    condSeq += PixelCablingCondAlg(name="PixelCablingCondAlg", **pixelCablingArgs)

    return CfgMgr.PixelCablingSvc(name, **kwargs)
