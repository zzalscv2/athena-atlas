# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#
# read alignment constants from DB to update MuonGeoModel
#
from AthenaCommon.GlobalFlags import globalflags

from AthenaCommon.AlgSequence import AthSequencer
from MuonCondAlg.MuonCondAlgConf import MuonAlignmentErrorDbAlg

from MuonRecExample.MuonRecUtils import logMuon
from IOVDbSvc.CondDB import conddb

from .MuonAlignFlags import muonAlignFlags

# defaults have to be re-set maybe 
muonAlignFlags.setDefaults()

from AthenaCommon.AppMgr import ServiceMgr as svcMgr
if not hasattr(svcMgr, "MuonIdHelperSvc"):
    from MuonIdHelpers.MuonIdHelpersConfigLegacy import MuonIdHelperSvc
    svcMgr += MuonIdHelperSvc("MuonIdHelperSvc")
###############################################################
# There are 3 types of problems in some old global tags (already locked)
# Pb 1: no TGC alignment folders exist
# Pb 2: Information of tags is wrong causing crashes (example endcap alignment corrections on COMCOND-ES1C-000-00)
# Pb 3: No association of tags to the global tag is done for the alignment folder
# 
# To avoid crashes, a check on old, locked global tags is done and action is taken not to read some alignment folders
###############################################################

####
# A very grumpy but helpful comment to my dear successor or a future developer, if you're faced to scheduling problems in serial athena
# The TrackingGeometryCondAlg/python/AtlasTrackingGeometryCondAlg reshuffles all the geometry algortihms. 
# If you ever add an extra dependency to the MuonDetectorManager please have a look there and check that the alg
# is also considered properly by this reshuffling!

logMuon.info("Reading alignment constants from DB")
if not conddb.folderRequested('/MUONALIGN/Onl/MDT/BARREL') and not conddb.folderRequested('/MUONALIGN/MDT/BARREL'):
    conddb.addFolderSplitOnline('MUONALIGN','/MUONALIGN/Onl/MDT/BARREL','/MUONALIGN/MDT/BARREL',className='CondAttrListCollection')
    conddb.addFolderSplitOnline('MUONALIGN','/MUONALIGN/Onl/MDT/ENDCAP/SIDEA','/MUONALIGN/MDT/ENDCAP/SIDEA',className='CondAttrListCollection')
    conddb.addFolderSplitOnline('MUONALIGN','/MUONALIGN/Onl/MDT/ENDCAP/SIDEC','/MUONALIGN/MDT/ENDCAP/SIDEC',className='CondAttrListCollection')
if not conddb.folderRequested('/MUONALIGN/Onl/TGC/SIDEA') and not conddb.folderRequested('/MUONALIGN/TGC/SIDEA'):
    conddb.addFolderSplitOnline('MUONALIGN','/MUONALIGN/Onl/TGC/SIDEA','/MUONALIGN/TGC/SIDEA',className='CondAttrListCollection')
    conddb.addFolderSplitOnline('MUONALIGN','/MUONALIGN/Onl/TGC/SIDEC','/MUONALIGN/TGC/SIDEC',className='CondAttrListCollection')

from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags

from AtlasGeoModel.MuonGM import GeoModelSvc
MuonDetectorTool = GeoModelSvc.DetectorTools[ "MuonDetectorTool" ]
condSequence = AthSequencer("AthCondSeq")

from MuonCondAlg.MuonCondAlgConf import MuonAlignmentCondAlg
from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags
MuonAlignAlg = MuonAlignmentCondAlg()
MuonAlignAlg.ParlineFolders = ["/MUONALIGN/MDT/BARREL",
                               "/MUONALIGN/MDT/ENDCAP/SIDEA",
                               "/MUONALIGN/MDT/ENDCAP/SIDEC",
                               "/MUONALIGN/TGC/SIDEA",
                               "/MUONALIGN/TGC/SIDEC"]
if conddb.dbdata != 'COMP200' and conddb.dbmc != 'COMP200' and \
   'HLT' not in globalflags.ConditionsTag() and not conddb.isOnline :
    MuonAlignAlg.IsData = False
condSequence+=MuonAlignAlg

MuonDetectorTool.FillCacheInitTime = 0 # We do not need to fill cache for the MuonGeoModel MuonDetectorTool, just for the condAlg

# Condition DB is needed only if A-lines or B-lines are requested
if not (muonAlignFlags.UseAlines=='none' and muonAlignFlags.UseBlines=='none'):
    MuonDetectorTool.UseConditionDb = 1
    # here define to what extent B-lines are enabled
    if muonAlignFlags.UseBlines=='none':
        MuonDetectorTool.EnableMdtDeformations = 0
    elif muonAlignFlags.UseBlines=='all':
        MuonDetectorTool.EnableMdtDeformations = 1
    elif muonAlignFlags.UseBlines=='barrel':
        MuonDetectorTool.EnableMdtDeformations = 2
    elif muonAlignFlags.UseBlines=='endcaps':
        MuonDetectorTool.EnableMdtDeformations = 3

# here define if I-lines (CSC internal alignment) are enabled
if muonAlignFlags.UseIlines and MuonGeometryFlags.hasCSC(): 
    if 'HLT' in globalflags.ConditionsTag() :
        logMuon.info("Reading CSC I-Lines from layout - special configuration for COMP200 in HLT setup.")
        MuonDetectorTool.UseIlinesFromGM = True
        MuonDetectorTool.EnableCscInternalAlignment = False
    else :
        logMuon.info("Reading CSC I-Lines from conditions database.")
        conddb.addFolderSplitOnline('MUONALIGN','/MUONALIGN/Onl/CSC/ILINES','/MUONALIGN/CSC/ILINES',className='CondAttrListCollection')
        MuonDetectorTool.UseIlinesFromGM = False
        MuonDetectorTool.EnableCscInternalAlignment = True
        MuonAlignAlg.ParlineFolders += ["/MUONALIGN/CSC/ILINES"]
        MuonAlignAlg.ILinesFromCondDB = True

# here define if As-Built (MDT chamber alignment) are enabled
applyNswAsBuilt = False
if muonAlignFlags.UseAsBuilt:
    if conddb.dbdata == 'COMP200' or conddb.dbmc == 'COMP200' or \
       'HLT' in globalflags.ConditionsTag() or conddb.isOnline or conddb.isMC:
        logMuon.info("No MDT As-Built parameters applied.")
        MuonDetectorTool.EnableMdtAsBuiltParameters = 0        
    else :
        logMuon.info("Reading As-Built parameters from conditions database")
        MuonDetectorTool.EnableMdtAsBuiltParameters = 1        
        conddb.addFolder('MUONALIGN_OFL','/MUONALIGN/MDT/ASBUILTPARAMS' ,className='CondAttrListCollection')
        MuonAlignAlg.ParlineFolders += ["/MUONALIGN/MDT/ASBUILTPARAMS"]
        if CommonGeometryFlags.Run not in ["RUN1","RUN2"]: 
            applyNswAsBuilt = True
            # TODO: remove hard-coded tag once the global tag is ready
            conddb.addFolderWithTag('MUONALIGN_OFL','/MUONALIGN/ASBUILTPARAMS/MM' , tag='MuonAlignAsBuiltParamsMm-RUN3-01-00', className='CondAttrListCollection')
            conddb.addFolderWithTag('MUONALIGN_OFL','/MUONALIGN/ASBUILTPARAMS/STGC',tag='MUONALIGN_STG_ASBUILT-001-03',        className='CondAttrListCollection')
            from MuonCondAlg.MuonCondAlgConf import NswAsBuiltCondAlg
            if not hasattr(condSequence, "NswAsBuiltCondAlg"):
                condSequence+=NswAsBuiltCondAlg("NswAsBuiltCondAlg",
                                            ReadSTgcAsBuiltParamsKey="")

# nuisance parameter used during track fit to account for alignment uncertainty
if conddb.dbdata != 'COMP200' and conddb.dbmc != 'COMP200' and \
   'HLT' not in globalflags.ConditionsTag() and not conddb.isOnline :

    conddb.addFolder("MUONALIGN_OFL","/MUONALIGN/ERRS",className='CondAttrListCollection')
    condSequence += MuonAlignmentErrorDbAlg("MuonAlignmentErrorDbAlg")

if muonAlignFlags.applyMMPassivation() and not hasattr(condSequence, "NswPassivationDbAlg"):
    from MuonCondAlg.MuonTopCondAlgConfigRUN2 import NswPassivationDbAlg
    NswPassAlg = NswPassivationDbAlg("NswPassivationDbAlg")
    condSequence += NswPassAlg

if not hasattr(condSequence, "MuonDetectorCondAlg"):
    from MuonGeoModel.MuonGeoModelConf import MuonDetectorCondAlg
    MuonDetectorManagerCond = MuonDetectorCondAlg("MuonDetectorCondAlg")
    MuonDetectorManagerCond.applyMmPassivation = muonAlignFlags.applyMMPassivation()
    MuonDetectorManagerCond.MuonDetectorTool = MuonDetectorTool
    MuonDetectorManagerCond.applyNswAsBuilt = applyNswAsBuilt
    MuonDetectorManagerCond.MuonDetectorTool.FillCacheInitTime = 1 # CondAlg cannot update itself later - not threadsafe

    if conddb.dbdata != 'COMP200' and conddb.dbmc != 'COMP200' and \
    'HLT' not in globalflags.ConditionsTag() and not conddb.isOnline :
        MuonDetectorManagerCond.IsData = False
    condSequence+=MuonDetectorManagerCond
