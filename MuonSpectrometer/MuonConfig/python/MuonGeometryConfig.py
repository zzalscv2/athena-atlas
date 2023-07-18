# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.Enums import ProductionStep
from AtlasGeoModel.GeoModelConfig import GeoModelCfg
from AthenaConfiguration.Enums import LHCPeriod

def MuonIdHelperSvcCfg(flags):
    acc = ComponentAccumulator()
    acc.addService( CompFactory.Muon.MuonIdHelperSvc("MuonIdHelperSvc",
        HasCSC=flags.Detector.GeometryCSC,
        HasSTGC=flags.Detector.GeometrysTGC,
        HasMM=flags.Detector.GeometryMM,
        HasMDT=flags.Detector.GeometryMDT,
        HasRPC=flags.Detector.GeometryRPC,
        HasTGC=flags.Detector.GeometryTGC), primary=True )
    return acc


def MuonDetectorToolCfg(flags):
    acc = ComponentAccumulator()
    detTool = CompFactory.MuonDetectorTool(
        HasCSC=flags.Detector.GeometryCSC,
        HasSTgc=flags.Detector.GeometrysTGC,
        HasMM=flags.Detector.GeometryMM
        )
    detTool.UseConditionDb = 1
    detTool.UseIlinesFromGM = False

    if flags.Muon.enableAlignment:
        # Condition DB is needed only if A-lines or B-lines are requested
        if not (not flags.Muon.Align.UseALines and flags.Muon.Align.UseBLines=='none'):
            detTool.UseConditionDb = 1
        
        # here define if I-lines (CSC internal alignment) are enabled
        if flags.Muon.Align.UseILines and flags.Detector.GeometryCSC:
            if 'HLT' in flags.IOVDb.GlobalTag:
                #logMuon.info("Reading CSC I-Lines from layout - special configuration for COMP200 in HLT setup.")
                detTool.UseIlinesFromGM = True
                detTool.EnableCscInternalAlignment = False
            else :
                #logMuon.info("Reading CSC I-Lines from conditions database.")
                if (flags.Common.isOnline and not flags.Input.isMC):
                    detTool.EnableCscInternalAlignment = True
                else:
                    detTool.UseIlinesFromGM = False
                    detTool.EnableCscInternalAlignment = True
    else:
        detTool.UseConditionDb = 0
        detTool.UseAsciiConditionData = 0
        if flags.Common.ProductionStep == ProductionStep.Simulation:
            detTool.FillCacheInitTime = 0

    if not flags.GeoModel.SQLiteDB:
        ## Additional material in the muon system
        AGDD2Geo = CompFactory.AGDDtoGeoSvc()
        muonAGDDTool = CompFactory.MuonAGDDTool("MuonSpectrometer", BuildNSW=False)
        AGDD2Geo.Builders += [ muonAGDDTool ]
        if (flags.Detector.GeometrysTGC and flags.Detector.GeometryMM):
            nswAGDDTool = CompFactory.NSWAGDDTool("NewSmallWheel", Locked=False)
            nswAGDDTool.Volumes = ["NewSmallWheel"]
            nswAGDDTool.DefaultDetector = "Muon"
            AGDD2Geo.Builders += [ nswAGDDTool ]

        #create=True is needed for the service to be initialised in the new style
        acc.addService(AGDD2Geo, create=True)

    # call fill cache of MuonDetectorTool such that all MdtReadoutElement caches are filled
    # already during initialize() -> this will increase memory -> needs to be measured
    detTool.FillCacheInitTime = 1 

    # turn on/off caching of MdtReadoutElement surfaces
    detTool.CachingFlag = 1
    acc.merge(MuonIdHelperSvcCfg(flags))
    acc.setPrivateTools(detTool)
    return acc

@AccumulatorCache
def MuonAlignmentCondAlgCfg(flags, name="MuonAlignmentCondAlg", **kwargs):
    acc = ComponentAccumulator()
    acc.merge(MuonGeoModelToolCfg(flags))

    # This is all migrated from MuonSpectrometer/MuonReconstruction/MuonRecExample/python/MuonAlignConfig.py

    from IOVDbSvc.IOVDbSvcConfig import addFolders
    MuonAlignmentCondAlg=CompFactory.MuonAlignmentCondAlg
    if (flags.Common.isOnline and not flags.Input.isMC):
        acc.merge(addFolders( flags, ['/MUONALIGN/Onl/MDT/BARREL'], 'MUONALIGN', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/Onl/MDT/ENDCAP/SIDEA'], 'MUONALIGN', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/Onl/MDT/ENDCAP/SIDEC'], 'MUONALIGN', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/Onl/TGC/SIDEA'], 'MUONALIGN', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/Onl/TGC/SIDEC'], 'MUONALIGN', className='CondAttrListCollection'))
    else:
        acc.merge(addFolders( flags, ['/MUONALIGN/MDT/BARREL'], 'MUONALIGN_OFL', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/MDT/ENDCAP/SIDEA'], 'MUONALIGN_OFL', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/MDT/ENDCAP/SIDEC'], 'MUONALIGN_OFL', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/TGC/SIDEA'], 'MUONALIGN_OFL', className='CondAttrListCollection'))
        acc.merge(addFolders( flags, ['/MUONALIGN/TGC/SIDEC'], 'MUONALIGN_OFL', className='CondAttrListCollection'))

    MuonAlign = MuonAlignmentCondAlg()


    MuonAlign.ParlineFolders = ["/MUONALIGN/MDT/BARREL",
                                "/MUONALIGN/MDT/ENDCAP/SIDEA",
                                "/MUONALIGN/MDT/ENDCAP/SIDEC",
                                "/MUONALIGN/TGC/SIDEA",
                                "/MUONALIGN/TGC/SIDEC"]

    # here define if I-lines (CSC internal alignment) are enabled
    if flags.Muon.Align.UseILines and flags.Detector.GeometryCSC:
        if 'HLT' in flags.IOVDb.GlobalTag:
            #logMuon.info("Reading CSC I-Lines from layout - special configuration for COMP200 in HLT setup.")
            MuonAlign.ILinesFromCondDB = False
        else:
            #logMuon.info("Reading CSC I-Lines from conditions database.")
            if (flags.Common.isOnline and not flags.Input.isMC):
                acc.merge(addFolders( flags, ['/MUONALIGN/Onl/CSC/ILINES'], 'MUONALIGN', className='CondAttrListCollection'))
            else:
                acc.merge(addFolders( flags, ['/MUONALIGN/CSC/ILINES'], 'MUONALIGN_OFL', className='CondAttrListCollection'))
            MuonAlign.ParlineFolders += ["/MUONALIGN/CSC/ILINES"]
            MuonAlign.ILinesFromCondDB = True

    # here define if As-Built (MDT chamber alignment) are enabled
    if flags.Muon.Align.UseAsBuilt:
        if flags.IOVDb.DatabaseInstance == 'COMP200' or \
                'HLT' in flags.IOVDb.GlobalTag or flags.Common.isOnline :
            #logMuon.info("No MDT As-Built parameters applied.")
            pass
        else :
            #logMuon.info("Reading As-Built parameters from conditions database")
            acc.merge(addFolders( flags, '/MUONALIGN/MDT/ASBUILTPARAMS' , 'MUONALIGN_OFL', className='CondAttrListCollection'))
            MuonAlign.ParlineFolders += ["/MUONALIGN/MDT/ASBUILTPARAMS"]
            acc.merge(NswAsBuiltCondAlgCfg(flags))
            pass

    acc.addCondAlgo(MuonAlign)
    return acc


def MuonAlignmentErrorDbAlgCfg(flags):
    acc = ComponentAccumulator()
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    acc.merge(addFolders(flags, "/MUONALIGN/ERRS", "MUONALIGN_OFL", className="CondAttrListCollection"))
    acc.addCondAlgo(CompFactory.MuonAlignmentErrorDbAlg("MuonAlignmentErrorDbAlg"))
    return acc

def NswAsBuiltCondAlgCfg(flags, name = "NswAsBuiltCondAlg", **kwargs):
    result = ComponentAccumulator()
    #### Do not apply the as-built correction if not activated
    if flags.GeoModel.Run < LHCPeriod.Run3:
        return result
    ##TODO: remove hard-coded tag once the global tag is ready
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    result.merge(addFolders( flags, '/MUONALIGN/ASBUILTPARAMS/MM'  , 'MUONALIGN_OFL', className='CondAttrListCollection', tag='MuonAlignAsBuiltParamsMm-RUN3-01-00'))
    ### Disable the STGC as-built parameters (Keep the path if we want to add later fully validated As-built)
    result.merge(addFolders( flags, '/MUONALIGN/ASBUILTPARAMS/STGC', 'MUONALIGN_OFL', className='CondAttrListCollection', tag='MUONALIGN_STG_ASBUILT-001-03'))
    kwargs.setdefault("ReadSTgcAsBuiltParamsKey", "")
    the_alg = CompFactory.NswAsBuiltCondAlg(name, **kwargs)
    result.addCondAlgo(the_alg, primary = True)     
    return result


def MuonDetectorCondAlgCfg(flags, name = "MuonDetectorCondAlg", **kwargs):
    result = ComponentAccumulator()

    result.merge(MuonAlignmentCondAlgCfg(flags))
    kwargs.setdefault("applyMmPassivation", flags.Muon.applyMMPassivation)

    if kwargs["applyMmPassivation"]:
        from MuonConfig.MuonCondAlgConfig import NswPassivationDbAlgCfg
        result.merge(NswPassivationDbAlgCfg(flags))
    
    kwargs.setdefault("applyALines", len([alg for alg in result.getCondAlgos() if alg.name == "MuonAlignmentCondAlg"])>0)
    kwargs.setdefault("applyBLines", len([alg for alg in result.getCondAlgos() if alg.name == "MuonAlignmentCondAlg"])>0)
    kwargs.setdefault("applyNswAsBuilt", len([alg for alg in result.getCondAlgos() if alg.name == "NswAsBuiltCondAlg"])>0)
    kwargs.setdefault("applyMdtAsBuilt", flags.Muon.Align.UseAsBuilt and not (flags.IOVDb.DatabaseInstance == 'COMP200' or \
                                         'HLT' in flags.IOVDb.GlobalTag or flags.Common.isOnline or flags.Input.isMC))

    kwargs.setdefault("MuonDetectorTool", result.popToolsAndMerge(MuonDetectorToolCfg(flags)))
    
    MuonDetectorManagerCond = CompFactory.MuonDetectorCondAlg(name, **kwargs)

    result.addCondAlgo(MuonDetectorManagerCond, primary = True)
    return result


def MuonGeoModelToolCfg(flags):
    acc = GeoModelCfg(flags)
    gms = acc.getPrimary()
    detTool = acc.popToolsAndMerge(MuonDetectorToolCfg(flags))
    detTool.FillCacheInitTime = 0 # We do not need to fill cache for the MuonGeoModel MuonDetectorTool, just for the condAlg
    gms.DetectorTools += [ detTool ]
    return acc


def MuonGeoModelCfg(flags, forceDisableAlignment=False):
    if flags.Muon.setupGeoModelXML:
        from MuonGeoModelR4.MuonGeoModelConfig import MuonGeoModelCfg as MuonGeoModelCfgR4
        return MuonGeoModelCfgR4(flags)
    
    result = ComponentAccumulator()
    result.merge(MuonGeoModelToolCfg(flags))

    if flags.Muon.enableAlignment and not forceDisableAlignment:
        result.merge(MuonDetectorCondAlgCfg(flags))

    result.merge(MuonIdHelperSvcCfg(flags)) # This line can be removed once the configuration methods for all 258 components which directly use this service are updated!!
    return result
