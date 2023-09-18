
# block include of file, this is used by many packages
include.block ("InDetRecExample/InDetRecConditionsAccess.py")

from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from AthenaCommon.DetFlags import DetFlags

include("BeamSpotConditions/BeamCondAlgSetup.py")

#
# --- Load PixelConditionsTools
#
if DetFlags.pixel_on():
    # Load pixel conditions summary service
    from AthenaCommon.AppMgr import ToolSvc
    from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags as commonGeoFlags
    from AtlasGeoModel.InDetGMJobProperties import InDetGeometryFlags as geoFlags

    #################
    # Module status #
    #################
    if not hasattr(condSeq, "PixelConfigCondAlg"):
        from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelConfigCondAlg

        if (globalflags.DataSource()=='geant4'):
            from PixelDigitization.PixelDigitizationConfigLegacy import PixelConfigCondAlg_MC
            condSeq += PixelConfigCondAlg_MC()

        elif (globalflags.DataSource=='data'):
            from RecExConfig.AutoConfiguration import GetRunNumber
            runNum = GetRunNumber()
            IdMappingDat="PixelCabling/Pixels_Atlas_IdMapping_344494.dat"
            if (runNum is None):
                IdMappingDat="PixelCabling/Pixels_Atlas_IdMapping_344494.dat"
            elif (runNum<222222):
                IdMappingDat="PixelCabling/Pixels_Atlas_IdMapping_May08.dat"
            else:
                # Even though we are reading from COOL, set the correct fallback map.
                if (runNum >= 344494):
                    IdMappingDat="PixelCabling/Pixels_Atlas_IdMapping_344494.dat"
                elif (runNum >= 314940 and runNum < 344494):
                    IdMappingDat="PixelCabling/Pixels_Atlas_IdMapping_314940.dat"
                elif (runNum >= 289350 and runNum < 314940): # 2016
                    IdMappingDat="PixelCabling/Pixels_Atlas_IdMapping_2016.dat"
                elif (runNum >= 222222 and runNum < 289350): # 2015
                    IdMappingDat="PixelCabling/Pixels_Atlas_IdMapping_Run2.dat"
                else:
                    IdMappingDat="PixelCabling/Pixels_Atlas_IdMapping_344494.dat"

            # Data overlay: hard-scatter MC digitization + pileup data configuration
            if (globalflags.isOverlay()):
                from PixelDigitization.PixelDigitizationConfigLegacy import PixelConfigCondAlg_MC
                alg = PixelConfigCondAlg_MC()
                alg.CablingMapFileName=IdMappingDat
            else: 
                alg = PixelConfigCondAlg(name="PixelConfigCondAlg", 
                                         CablingMapFileName=IdMappingDat)

            if jobproperties.Beam.beamType() == 'cosmics':
                alg.BarrelTimeJitter=[25.0,25.0,25.0,25.0]
                alg.EndcapTimeJitter=[25.0,25.0,25.0]
                alg.DBMTimeJitter=[25.0,25.0,25.0]
                alg.BarrelNumberOfBCID=[8,8,8,8]
                alg.EndcapNumberOfBCID=[8,8,8]
                alg.DBMNumberOfBCID=[8,8,8]
                alg.BarrelTimeOffset=[100.0,100.0,100.0,100.0]
                alg.EndcapTimeOffset=[100.0,100.0,100.0]
                alg.DBMTimeOffset=[100.0,100.0,100.0]

            condSeq += alg

    if not conddb.dbdata=='COMP200':
        if not conddb.folderRequested("/PIXEL/PixelModuleFeMask"):
            conddb.addFolderSplitOnline("PIXEL", "/PIXEL/Onl/PixelModuleFeMask", "/PIXEL/PixelModuleFeMask", className="CondAttrListCollection")

    if not hasattr(condSeq, "PixelDeadMapCondAlg"):
        from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelDeadMapCondAlg
        alg = PixelDeadMapCondAlg(name="PixelDeadMapCondAlg")
        if conddb.dbdata=='COMP200':
            alg.ReadKey = ''
        condSeq += alg

    if not hasattr(condSeq, "PixelDCSCondStateAlg"):
        from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelDCSCondStateAlg
        alg = PixelDCSCondStateAlg(name="PixelDCSCondStateAlg")
        if athenaCommonFlags.isOnline() or globalflags.DataSource!='data' or globalflags.isOverlay() or not InDetFlags.usePixelDCS():
            alg.ReadKeyState = ''
        else :
            if not conddb.folderRequested("/PIXEL/DCS/FSMSTATE"):
                conddb.addFolder("DCS_OFL", "/PIXEL/DCS/FSMSTATE", className="CondAttrListCollection")
            alg.ReadKeyState = '/PIXEL/DCS/FSMSTATE'
        condSeq += alg

    if not hasattr(condSeq, "PixelDCSCondStatusAlg"):
        from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelDCSCondStatusAlg
        alg = PixelDCSCondStatusAlg(name="PixelDCSCondStatusAlg")
        if athenaCommonFlags.isOnline() or globalflags.DataSource!='data' or globalflags.isOverlay() or not InDetFlags.usePixelDCS():
            alg.ReadKeyStatus = ''
        else :
            if not conddb.folderRequested("/PIXEL/DCS/FSMSTATUS"):
                conddb.addFolder("DCS_OFL", "/PIXEL/DCS/FSMSTATUS", className="CondAttrListCollection")
            alg.ReadKeyStatus = '/PIXEL/DCS/FSMSTATUS'
        condSeq += alg

    #####################
    # Calibration Setup #
    #####################
    from Digitization.DigitizationFlags import digitizationFlags
    if commonGeoFlags.Run()=="RUN3" and 'UseOldIBLCond' not in digitizationFlags.experimentalDigi():
        if not conddb.folderRequested("/PIXEL/ChargeCalibration"):
            conddb.addFolderSplitOnline("PIXEL", "/PIXEL/Onl/ChargeCalibration", "/PIXEL/ChargeCalibration", className="CondAttrListCollection")
            
        if not hasattr(condSeq, 'PixelChargeLUTCalibCondAlg'):
            from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelChargeLUTCalibCondAlg
            condSeq += PixelChargeLUTCalibCondAlg(name="PixelChargeLUTCalibCondAlg", ReadKey="/PIXEL/ChargeCalibration")
    else:
        PixCalibFolder = 'ChargeCalibration'
        if not conddb.folderRequested("/PIXEL/"+PixCalibFolder) and commonGeoFlags.Run() != "RUN1":
            conddb.addFolderSplitOnline("PIXEL", "/PIXEL/Onl/"+PixCalibFolder, "/PIXEL/"+PixCalibFolder, className="CondAttrListCollection")
        if not hasattr(condSeq, 'PixelChargeCalibCondAlg'):
            from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelChargeCalibCondAlg
            condSeq += PixelChargeCalibCondAlg(name="PixelChargeCalibCondAlg", ReadKey="/PIXEL/"+PixCalibFolder if commonGeoFlags.Run() == "RUN2" else "")

    if not athenaCommonFlags.isOnline():
        if not conddb.folderRequested('/PIXEL/PixdEdx'):
            conddb.addFolder("PIXEL_OFL", "/PIXEL/PixdEdx", className="AthenaAttributeList")

        if not conddb.folderRequested("/Indet/PixelDist"):
            conddb.addFolder("INDET", "/Indet/PixelDist", className="DetCondCFloat")

    if not hasattr(condSeq, 'PixelOfflineCalibCondAlg'):
        if not conddb.folderRequested("/PIXEL/PixReco") and not conddb.folderRequested("/PIXEL/Onl/PixReco") :
            conddb.addFolderSplitOnline("PIXEL", "/PIXEL/Onl/PixReco", "/PIXEL/PixReco", className="DetCondCFloat")

        from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelOfflineCalibCondAlg
        condSeq += PixelOfflineCalibCondAlg(name="PixelOfflineCalibCondAlg", ReadKey="/PIXEL/PixReco", InputSource=2)
        
    if not hasattr(ToolSvc, "PixelLorentzAngleTool"):
        from SiLorentzAngleTool.PixelLorentzAngleToolSetup import PixelLorentzAngleToolSetup
        pixelLorentzAngleToolSetup = PixelLorentzAngleToolSetup()

    if not hasattr(condSeq, 'PixelDistortionAlg'):
        from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelDistortionAlg
        condSeq += PixelDistortionAlg(name="PixelDistortionAlg")

    if not hasattr(condSeq, 'PixeldEdxAlg'):
        from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixeldEdxAlg
        dedxAlg = PixeldEdxAlg(name="PixeldEdxAlg")
        if not athenaCommonFlags.isOnline():
            dedxAlg.ReadFromCOOL = True
        else:
            dedxAlg.ReadFromCOOL = False
            if (globalflags.DataSource=='data'):
                dedxAlg.CalibrationFile="dtpar_signed_234.txt"
            else:
                dedxAlg.CalibrationFile="mcpar_signed_234.txt"
        condSeq += dedxAlg

    ################
    # RUN-1 legacy #
    ################
    if commonGeoFlags.Run()=="RUN1" and athenaCommonFlags.isOnline():
        if not conddb.folderRequested("/TDAQ/Resources/ATLAS/PIXEL/Modules"):
            conddb.addFolder("TDAQ_ONL", "/TDAQ/Resources/ATLAS/PIXEL/Modules", className="CondAttrListCollection")
        if not hasattr(condSeq, "PixelTDAQCondAlg"):
            from PixelConditionsAlgorithms.PixelConditionsAlgorithmsConf import PixelTDAQCondAlg
            condSeq += PixelTDAQCondAlg(name="PixelTDAQCondAlg", ReadKey="/TDAQ/Resources/ATLAS/PIXEL/Modules")

#
# --- Load SCT Conditions Services
#
if DetFlags.haveRIO.SCT_on():
    # Set up SCT cabling
    include( 'InDetRecExample/InDetRecCabling.py' )

    # Load conditions summary tool
    from SCT_ConditionsTools.SCT_ConditionsSummaryToolSetup import SCT_ConditionsSummaryToolSetup
    sct_ConditionsSummaryToolSetup = SCT_ConditionsSummaryToolSetup()
    sct_ConditionsSummaryToolSetup.setup()
    InDetSCT_ConditionsSummaryTool = sct_ConditionsSummaryToolSetup.getTool()
    if (InDetFlags.doPrintConfigurables()):
        printfunc (InDetSCT_ConditionsSummaryTool)

    from SCT_ConditionsTools.SCT_ConditionsToolsHelper import getSCT_ConfigurationConditionsTool, getSCT_ByteStreamErrorsTool
    InDetSCT_ConfigurationConditionsTool = getSCT_ConfigurationConditionsTool()
    if (InDetFlags.doPrintConfigurables()):
        printfunc (InDetSCT_ConfigurationConditionsTool)

    # Load calibration conditions tool
    from SCT_ConditionsTools.SCT_ReadCalibDataToolSetup import SCT_ReadCalibDataToolSetup
    sct_ReadCalibDataToolSetup = SCT_ReadCalibDataToolSetup()
    sct_ReadCalibDataToolSetup.setup()
    InDetSCT_ReadCalibDataTool = sct_ReadCalibDataToolSetup.getTool()
    if (InDetFlags.doPrintConfigurables()):
        printfunc (InDetSCT_ReadCalibDataTool)

    # Load flagged condition tool
    from SCT_ConditionsTools.SCT_FlaggedConditionToolSetup import SCT_FlaggedConditionToolSetup
    sct_FlaggedConditionToolSetup = SCT_FlaggedConditionToolSetup()
    sct_FlaggedConditionToolSetup.setup()
    InDetSCT_FlaggedConditionTool = sct_FlaggedConditionToolSetup.getTool()
    if (InDetFlags.doPrintConfigurables()):
        printfunc (InDetSCT_FlaggedConditionTool)
    
    # Load conditions Monitoring tool
    if not athenaCommonFlags.isOnline():
        from SCT_ConditionsTools.SCT_MonitorConditionsToolSetup import SCT_MonitorConditionsToolSetup
        sct_MonitorConditionsToolSetup = SCT_MonitorConditionsToolSetup()
        sct_MonitorConditionsToolSetup.setOutputLevel(INFO)
        sct_MonitorConditionsToolSetup.setup()
        InDetSCT_MonitorConditionsTool = sct_MonitorConditionsToolSetup.getTool()
        if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetSCT_MonitorConditionsTool)

    if InDetFlags.doSCTModuleVeto():
        from SCT_ConditionsTools.SCT_ModuleVetoToolSetup import SCT_ModuleVetoToolSetup
        sct_ModuleVetoToolSetup = SCT_ModuleVetoToolSetup()
        sct_ModuleVetoToolSetup.setup()
        InDetSCT_ModuleVetoTool = sct_ModuleVetoToolSetup.getTool()
        if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetSCT_ModuleVetoTool)

    SCT_ByteStreamErrorsTool = getSCT_ByteStreamErrorsTool()
    if (InDetFlags.doPrintConfigurables()):
        printfunc (SCT_ByteStreamErrorsTool)

    if InDetFlags.useSctDCS():
        from SCT_ConditionsTools.SCT_DCSConditionsToolSetup import SCT_DCSConditionsToolSetup
        sct_DCSConditionsToolSetup = SCT_DCSConditionsToolSetup()

        # For HLT and online monitoring
        if athenaCommonFlags.isOnline():
            sct_DCSConditionsToolSetup.setReadAllDBFolders(False)
            if globalflags.DataSource() == "data":
                sct_DCSConditionsToolSetup.setDbInstance("SCT")
                dcs_folder="/SCT/HLT/DCS"
                sct_DCSConditionsToolSetup.setStateFolder(dcs_folder+"/CHANSTAT")
                sct_DCSConditionsToolSetup.setHVFolder(dcs_folder+"/HV")
                sct_DCSConditionsToolSetup.setTempFolder(dcs_folder+"/MODTEMP")

        sct_DCSConditionsToolSetup.setup()
        InDetSCT_DCSConditionsTool = sct_DCSConditionsToolSetup.getTool()
        if InDetFlags.useHVForSctDCS():
            sct_DCSConditionsToolSetup.getStateAlg().UseDefaultHV = True  #Hack to use ~20V cut for SCT DCS rather than ChanStat for startup
        if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetSCT_DCSConditionsTool)
    
    if (globalflags.DataSource() == 'data'):       
        printfunc ("Conditions db instance is ", conddb.dbdata)
        # Load Tdaq enabled tools for data only and add some to summary tool for data only
        tdaqFolder = '/TDAQ/EnabledResources/ATLAS/SCT/Robins'
        if (conddb.dbdata == "CONDBR2"):
            tdaqFolder = '/TDAQ/Resources/ATLAS/SCT/Robins'
        # Load TdaqEnabled tool
        from SCT_ConditionsTools.SCT_TdaqEnabledToolSetup import SCT_TdaqEnabledToolSetup
        sct_TdaqEnabledToolSetup = SCT_TdaqEnabledToolSetup()
        sct_TdaqEnabledToolSetup.setFolder(tdaqFolder)
        sct_TdaqEnabledToolSetup.setup()
        InDetSCT_TdaqEnabledTool = sct_TdaqEnabledToolSetup.getTool()
        if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetSCT_TdaqEnabledTool)
        
        # Configure summary tool
        InDetSCT_ConditionsSummaryTool.ConditionsTools= [ InDetSCT_ConfigurationConditionsTool,
                                                          InDetSCT_FlaggedConditionTool,
                                                          SCT_ByteStreamErrorsTool,
                                                          InDetSCT_ReadCalibDataTool,
                                                          InDetSCT_TdaqEnabledTool ]
        if not athenaCommonFlags.isOnline():
            InDetSCT_ConditionsSummaryTool.ConditionsTools += [ InDetSCT_MonitorConditionsTool ]

        if InDetFlags.useSctDCS():
            InDetSCT_ConditionsSummaryTool.ConditionsTools += [ InDetSCT_DCSConditionsTool ]
      
    else :
        InDetSCT_ConditionsSummaryTool.ConditionsTools= [ InDetSCT_ConfigurationConditionsTool,
                                                          InDetSCT_FlaggedConditionTool,
                                                          InDetSCT_ReadCalibDataTool,
                                                          InDetSCT_MonitorConditionsTool ]
        if InDetFlags.useSctDCS():
            InDetSCT_ConditionsSummaryTool.ConditionsTools += [ InDetSCT_DCSConditionsTool ]

    if InDetFlags.doSCTModuleVeto():
        InDetSCT_ConditionsSummaryTool.ConditionsTools += [ InDetSCT_ModuleVetoTool ]


    if (InDetFlags.doPrintConfigurables()):
        printfunc (InDetSCT_ConditionsSummaryTool)

    # Conditions summary tool without InDetSCT_FlaggedConditionTool
    sct_ConditionsSummaryToolSetupWithoutFlagged = SCT_ConditionsSummaryToolSetup("InDetSCT_ConditionsSummaryToolWithoutFlagged")
    sct_ConditionsSummaryToolSetupWithoutFlagged.setup()
    InDetSCT_ConditionsSummaryToolWithoutFlagged = sct_ConditionsSummaryToolSetupWithoutFlagged.getTool()    
    condTools = []
    for condToolHandle in InDetSCT_ConditionsSummaryTool.ConditionsTools:
        condTool = condToolHandle
        if condTool not in condTools:
            if condTool != InDetSCT_FlaggedConditionTool:
                condTools.append(condTool)
    InDetSCT_ConditionsSummaryToolWithoutFlagged.ConditionsTools = condTools

    # @TODO fix this temporary hack to make the configguration of the InDetSCT_ConditionsSummaryTool accessible to TrackingCommon
    import InDetRecExample.TrackingCommon as TrackingCommon
    TrackingCommon.def_InDetSCT_ConditionsSummaryTool=InDetSCT_ConditionsSummaryTool
    TrackingCommon.def_InDetSCT_ConditionsSummaryToolWithoutFlagged=InDetSCT_ConditionsSummaryToolWithoutFlagged

    # Setup Lorentz angle tool.
    from SiLorentzAngleTool.SCTLorentzAngleToolSetup import SCTLorentzAngleToolSetup

    forceUseDB = False
    forceUseGeoModel = False
    if InDetFlags.useSctDCS() or athenaCommonFlags.isOnline():
        # Force Lorentz angle calculation to use DCS for data
        # (Not actually using DCS yet but rather temperature and voltage from joboptions.)
        if (globalflags.DataSource() == 'data'):
            forceUseDB = True
    else:
        forceUseGeoModel = True

    sctLorentzAngleToolSetup = SCTLorentzAngleToolSetup(forceUseDB=forceUseDB, forceUseGeoModel=forceUseGeoModel)
    SCTLorentzAngleTool = sctLorentzAngleToolSetup.SCTLorentzAngleTool
            
#
# --- Load necessary TRT conditions folders
#
if DetFlags.haveRIO.TRT_on():
    # Compression table
    if (globalflags.DataSource() == 'data'): 
        if not conddb.folderRequested('/TRT/Onl/ROD/Compress'):
            conddb.addFolder("TRT_ONL","/TRT/Onl/ROD/Compress",className='CondAttrListCollection')


# Rt calibration coinstants
    if not conddb.folderRequested('/TRT/Calib/RT'):
        conddb.addFolderSplitOnline("TRT","/TRT/Onl/Calib/RT","/TRT/Calib/RT",className='TRTCond::RtRelationMultChanContainer')

    if not conddb.folderRequested('/TRT/Calib/T0'):
        conddb.addFolderSplitOnline("TRT","/TRT/Onl/Calib/T0","/TRT/Calib/T0",className='TRTCond::StrawT0MultChanContainer')

    if not conddb.folderRequested('/TRT/Calib/errors2d'):
        TRTErrorsFolder = conddb.addFolderSplitOnline ("TRT","/TRT/Onl/Calib/errors2d","/TRT/Calib/errors2d",className='TRTCond::RtRelationMultChanContainer')

    if not conddb.folderRequested('/TRT/Calib/slopes'):
        TRTSlopesFolder = conddb.addFolderSplitOnline ("TRT","/TRT/Onl/Calib/slopes","/TRT/Calib/slopes",className='TRTCond::RtRelationMultChanContainer')


    # Dead/Noisy Straw Lists
    if not conddb.folderRequested('/TRT/Cond/Status'):
        conddb.addFolderSplitOnline("TRT","/TRT/Onl/Cond/Status","/TRT/Cond/Status",className='TRTCond::StrawStatusMultChanContainer')

    if not conddb.folderRequested('/TRT/Cond/StatusPermanent'):
        conddb.addFolderSplitOnline("TRT","/TRT/Onl/Cond/StatusPermanent","/TRT/Cond/StatusPermanent",className='TRTCond::StrawStatusMultChanContainer')

    # Argon straw list
    if not conddb.folderRequested('/TRT/Cond/StatusHT'):
        conddb.addFolderSplitOnline("TRT","/TRT/Onl/Cond/StatusHT","/TRT/Cond/StatusHT",className='TRTCond::StrawStatusMultChanContainer')

    # TRT PID tools        
    if not conddb.folderRequested( "/TRT/Calib/PID_vector" ):
        conddb.addFolderSplitOnline("TRT","/TRT/Onl/Calib/PID_vector", "/TRT/Calib/PID_vector",className='CondAttrListVec')
    if not conddb.folderRequested( "/TRT/Calib/ToT/ToTVectors"):
       conddb.addFolderSplitOnline( "TRT", "/TRT/Onl/Calib/ToT/ToTVectors", "/TRT/Calib/ToT/ToTVectors",className='CondAttrListVec')

    if not conddb.folderRequested( "/TRT/Calib/ToT/ToTValue"):
       conddb.addFolderSplitOnline( "TRT", "/TRT/Onl/Calib/ToT/ToTValue", "/TRT/Calib/ToT/ToTValue",className='CondAttrListCollection')

    if InDetFlags.doTRTPIDNN():
        if not conddb.folderRequested( "/TRT/Calib/PID_NN"):
            conddb.addFolderSplitOnline( "TRT", "/TRT/Onl/Calib/PID_NN", "/TRT/Calib/PID_NN",className='CondAttrListCollection')
        # FIXME: need to force an override for the online DB until this folder has been added to the latest tag
        conddb.addOverride("/TRT/Onl/Calib/PID_NN", "TRTCalibPID_NN_v2")

    #
    # now do the services
    #
    InDetTRT_DAQ_ConditionsSvc = None
    if (InDetFlags.doMonitoringTRT() and globalflags.DataSource() == 'data'):
        tdaqFolder = '/TDAQ/EnabledResources/ATLAS/TRT/Robins'
        if (conddb.dbdata == "CONDBR2"):
            tdaqFolder = '/TDAQ/Resources/ATLAS/TRT/Robins'
        # TDAQ Enabled Service (only for monitoring on data)
        conddb.addFolder('TDAQ_ONL',tdaqFolder,className='CondAttrListCollection')
        from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_DAQ_ConditionsSvc
        InDetTRT_DAQ_ConditionsSvc = TRT_DAQ_ConditionsSvc( name = "InDetTRT_DAQ_ConditionsSvc" )
        ServiceMgr += InDetTRT_DAQ_ConditionsSvc
        if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetTRT_DAQ_ConditionsSvc)
    
    #
    # Load and Configure TRT Conditions Services
    #
    InDetTRTConditionsServices=[]

    # Dead/Noisy Straw Service
    useOldStyle = False
    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()
    if DetFlags.simulate.any_on() or hasattr(topSequence,"OutputConditionsAlg"):
         useOldStyle = True

    # Straw status tool
    from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_StrawStatusSummaryTool
    InDetTRTStrawStatusSummaryTool = TRT_StrawStatusSummaryTool(name = "TRT_StrawStatusSummaryTool",
                                                            isGEANT4 = useOldStyle)
    # CalDb tool
    from TRT_ConditionsServices.TRT_ConditionsServicesConf import TRT_CalDbTool
    InDetTRTCalDbTool = TRT_CalDbTool(name = "TRT_CalDbTool")

    # straw status  algorithm
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTStrawStatusCondAlg
    TRTStrawStatusCondAlg = TRTStrawStatusCondAlg(name = "TRTStrawStatusCondAlg")

    # Alive straws algorithm
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTStrawCondAlg
    TRTStrawCondAlg = TRTStrawCondAlg(name = "TRTStrawCondAlg")
    # Active Fraction algorithm
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTActiveCondAlg
    TRTActiveCondAlg = TRTActiveCondAlg(name = "TRTActiveCondAlg",
                                      TRTStrawStatusSummaryTool = InDetTRTStrawStatusSummaryTool)

    # HT probability algorithm
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTHTCondAlg
    TRTHTCondAlg = TRTHTCondAlg(name = "TRTHTCondAlg")

    # PID NN
    if InDetFlags.doTRTPIDNN():
        from TRT_ConditionsNN.TRT_ConditionsNNConf import TRTPIDNNCondAlg
        TRTPIDNNCondAlg = TRTPIDNNCondAlg(name = "TRTPIDNNCondAlg")

    # dEdx probability algorithm
    from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTToTCondAlg
    TRTToTCondAlg = TRTToTCondAlg(name = "TRTToTCondAlg")

    if InDetFlags.doCosmics() :
        # Average T0 CondAlg
        from TRT_ConditionsAlgs.TRT_ConditionsAlgsConf import TRTPhaseCondAlg
        TRTPhaseCondAlg = TRTPhaseCondAlg(name = "TRTPhaseCondAlg",
                                          TRTCalDbTool = InDetTRTCalDbTool)
        condSeq += TRTPhaseCondAlg

    # Condition algorithms for straw conditions
    if not hasattr(condSeq, "TRTStrawStatusCondAlg"):
        condSeq += TRTStrawStatusCondAlg
    if not hasattr(condSeq, "TRTStrawCondAlg"):
        condSeq += TRTStrawCondAlg
    if not hasattr(condSeq, "TRTActiveCondAlg"):
        condSeq += TRTActiveCondAlg

    # Condition algorithms for Pid
    if not hasattr(condSeq, "TRTHTCondAlg"):
        condSeq += TRTHTCondAlg
    if (InDetFlags.doTRTPIDNN() and not hasattr(condSeq, "TRTPIDNNCondAlg")):
        condSeq += TRTPIDNNCondAlg
    if not hasattr(condSeq, "TRTToTCondAlg"):
        condSeq += TRTToTCondAlg
