from __future__ import print_function
###############################################################
#
# Skeleton top job options for ESD->AOD
# Put here outputs that require rec.doAOD=True
#
# New version for revamped job transforms
#
# $Id: skeleton.ESDtoAOD_tf.py 700697 2015-10-15 09:48:11Z lerrenst $
#
#==============================================================

# Common job options disable most RecExCommon by default. Re-enable below on demand.
include("RecJobTransforms/CommonRecoSkeletonJobOptions.py")
rec.doAOD=True

#from AthenaCommon.Logging import logging
import logging
recoLog = logging.getLogger('esd_to_aod')
recoLog.info( '****************** STARTING ESD->AOD MAKING *****************' )

from AthenaCommon.AppMgr import ServiceMgr; import AthenaPoolCnvSvc.AthenaPool
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags

## Input
if hasattr(runArgs,"inputFile"): athenaCommonFlags.FilesInput.set_Value_and_Lock( runArgs.inputFile )
if hasattr(runArgs,"inputESDFile"):
    globalflags.InputFormat.set_Value_and_Lock('pool')
    rec.readESD.set_Value_and_Lock( True )
    rec.readRDO.set_Value_and_Lock( False )
    athenaCommonFlags.PoolESDInput.set_Value_and_Lock( runArgs.inputESDFile )

## Pre-exec
if hasattr(runArgs,"preExec"):
    recoLog.info("transform pre-exec")
    for cmd in runArgs.preExec:
        recoLog.info(cmd)
        exec(cmd)

## Pre-include
if hasattr(runArgs,"preInclude"): 
    for fragment in runArgs.preInclude:
        print("preInclude",fragment)
        include(fragment)

## Outputs
if hasattr(runArgs,"outputAODFile"):
    rec.doAOD.set_Value_and_Lock( True )
    rec.doWriteAOD.set_Value_and_Lock( True ) 
    athenaCommonFlags.PoolAODOutput.set_Value_and_Lock( runArgs.outputAODFile )
    # Begin temporary trigger block
    from RecExConfig.ObjKeyStore import objKeyStore
    if TriggerFlags.doMT():
        recoLog.info("Scheduling temporary ESDtoAOD propagation of Trigger MT EDM collections")
        recoLog.info("AOD content set according to the AODEDMSet flag: %s and EDM version (default setting) %d (for doMT() currently hardcoded to 3)", 
                     TriggerFlags.AODEDMSet(), TriggerFlags.EDMDecodingVersion())

        trigEDMListESD = {}
        trigEDMListAOD = {}

        from TrigEDMConfig.TriggerEDM import getTriggerEDMList
        # !!! this needs to be changed!! currently hardcoded to run version 3 corresponding to the Run 3 
        #trigEDMListESD.update(getTriggerEDMList(TriggerFlags.ESDEDMSet(),  TriggerFlags.EDMDecodingVersion()) )
        trigEDMListESD.update(getTriggerEDMList(TriggerFlags.ESDEDMSet(),  3) )
        objKeyStore.addManyTypesStreamESD( trigEDMListESD )

        # !!! this needs to be changed!! currently hardcoded to run version 3 corresponding to the Run 3 
        #trigEDMListAOD.update(getTriggerEDMList(TriggerFlags.AODEDMSet(),  TriggerFlags.EDMDecodingVersion()) )
        trigEDMListAOD.update(getTriggerEDMList(TriggerFlags.AODEDMSet(),  3) )
        objKeyStore.addManyTypesStreamAOD( trigEDMListAOD )

        notIncludedInAOD = [element for element in trigEDMListAOD if element not in trigEDMListESD]
        if (len(notIncludedInAOD)>0):
            recoLog.warning ("In AOD list but not in ESD list: ")
            recoLog.warning(notIncludedInAOD)
        else:
            recoLog.info("AOD EDM list is a subset of ESD list - good")


        # We also want to propagate the navigation to ESD and AOD. For now, unconditionally
        # Note: Not every TrigComposite collection is navigation, there are other use cases too.
        # So in future we should filter more heavily than this too.
        from PyUtils.MetaReaderPeeker import convert_itemList
        rawCollections = convert_itemList(layout='#join')
        for item in rawCollections:
            if item.startswith("xAOD::TrigComposite"):
                objKeyStore.addManyTypesStreamESD( [item] )
                objKeyStore.addManyTypesStreamAOD( [item] )
        if rec.doFileMetaData():
            metadataItems = [ "xAOD::TriggerMenuContainer#TriggerMenu",
                              "xAOD::TriggerMenuAuxContainer#TriggerMenuAux." ]
            objKeyStore.addManyTypesMetaData( metadataItems )
    else: # not TriggerFlags.doMT()
        pass # See TriggerJobOpts/python/TriggerGetter.py for Run 2. Called by RecExCommon


if hasattr(runArgs,"outputTAGFile"):
    # should be used as outputTAGFile_e2a=myTAG.root so that it does not trigger AODtoTAG
    # if writing TAG file, need AOD object in any case
    rec.doAOD.set_Value_and_Lock( True )
    rec.doWriteTAG.set_Value_and_Lock( True )
    athenaCommonFlags.PoolTAGOutput.set_Value_and_Lock( runArgs.outputTAGFile )

if hasattr(runArgs,"tmpAOD"):
    rec.doAOD.set_Value_and_Lock( True )
    rec.doWriteAOD.set_Value_and_Lock( True ) 
    athenaCommonFlags.PoolAODOutput.set_Value_and_Lock( runArgs.tmpAOD )

if hasattr(runArgs,"outputHIST_AOD_INTFile"):
    rec.doMonitoring.set_Value_and_Lock(True)
    from AthenaMonitoring.DQMonFlags import DQMonFlags
    DQMonFlags.histogramFile.set_Value_and_Lock( runArgs.outputHIST_AOD_INTFile )

if hasattr(runArgs,"outputNTUP_BTAGFile"):
    from BTagging.BTaggingFlags import BTaggingFlags
    BTaggingFlags.doJetTagNtuple = True
    BTaggingFlags.JetTagNtupleName = runArgs.outputNTUP_BTAGFile

if hasattr(runArgs, "outputNTUP_HIGHMULTFile"):
    from TrigMbD3PDMaker.TrigMbD3PDMakerFlags import trigMbD3PDflags
    trigMbD3PDflags.FileName=runArgs.outputNTUP_HIGHMULTFile
    include("TrigMbD3PDMaker/HighMultD3PD_jobOptions.py")

if hasattr(runArgs,"outputNTUP_ENHBIASFile"):
    from TrigCostAthena.TrigCostAthenaFlags import TrigCostAthenaFlags
    TrigCostAthenaFlags.StoreNtVerticesOutputFile.set_Value_and_Lock( runArgs.outputNTUP_ENHBIASFile )
    TrigCostAthenaFlags.DoStoreNtVertices.set_Value_and_Lock( True )
    if hasattr(runArgs,"inputESDFile") and not hasattr(runArgs,"inputFile"):
        athenaCommonFlags.FilesInput.set_Value_and_Lock( runArgs.inputESDFile )
    include("TrigCostAthena/ESDtoNTUP_ENHBIAS.py")

if hasattr(runArgs,"outputHIST_PHYSVALMONFile"):
    rec.doPhysValMonHists=True
    
    ## Setup the output file(s):
    from GaudiSvc.GaudiSvcConf import THistSvc
    svcMgr += THistSvc()
    output=svcMgr.THistSvc.Output
    svcMgr.THistSvc.Output+= ["PhysValMon DATAFILE='"+runArgs.outputHIST_PHYSVALMONFile+"' OPT='RECREATE'"]
    # now done in RecExCommon_topOption to ensure the right ordering of algs.
    # include("PhysValMon/PhysValMon_RecoOpt.py")
    
if hasattr(runArgs, 'outputXML_JiveXMLFile'):
    jp.Rec.doJiveXML.set_Value_and_Lock(True)

rec.OutputFileNameForRecoStep="ESDtoAOD"

#========================================================
# Central topOptions (this is one is a string not a list)
#========================================================
if hasattr(runArgs,"topOptions"): include(runArgs.topOptions)
else: include( "RecExCommon/RecExCommon_topOptions.py" )

# Remove unwanted back navigation to ESD when ESD is temporary
if hasattr(runArgs,"outputAODFile"):
    if hasattr(runArgs,"ESDFileIO") and runArgs.ESDFileIO == "temporary":
        try:
            StreamAOD.ExtendProvenanceRecord = False
        except:
            recoLog.info("StreamAOD was not defined, cannot set ExtendProvenanceRecord = False. Check your flags.")

#D3PDMaker outputs
if hasattr(runArgs,"outputNTUP_MINBIASFile"):
    from D3PDMakerConfig.D3PDProdFlags import prodFlags
    prodFlags.WriteMinBiasD3PD.FileName = runArgs.outputNTUP_MINBIASFile
    prodFlags.WriteMinBiasD3PD.set_Value_and_Lock( True )
    include( prodFlags.WriteMinBiasD3PD.DPDMakerScript )
    pass

if hasattr(runArgs,"outputNTUP_TRIGFile"):
    from D3PDMakerConfig.D3PDProdFlags import prodFlags
    prodFlags.WriteTriggerD3PD.FileName = runArgs.outputNTUP_TRIGFile
    prodFlags.WriteTriggerD3PD.set_Value_and_Lock( True )
    include( prodFlags.WriteTriggerD3PD.DPDMakerScript )
    pass

if hasattr(runArgs,"outputDESDM_BEAMSPOTFile"):
    #needs to be used with: preInclude=InDetBeamSpotFinder/BeamSpotRecoPreInclude_standard.py
    from InDetBeamSpotFinder import BeamSpotDPDFlags 
    primDPD.WriteDESDM_BEAMSPOTStream.FileName=runArgs.outputDESDM_BEAMSPOTFile
    primDPD.WriteDESDM_BEAMSPOTStream.set_Value_and_Lock( True )
    include("InDetBeamSpotFinder/DESDM_BEAMSPOTFragment.py")

#==========================================================
# Use LZIB for compression of temporary outputs of AthenaMP
#==========================================================
if hasattr(runArgs, "outputAODFile") and '_000' in runArgs.outputAODFile:
    ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += [ "DatabaseName = '" +  athenaCommonFlags.PoolAODOutput()+ "'; COMPRESSION_ALGORITHM = '1'" ]
    ServiceMgr.AthenaPoolCnvSvc.PoolAttributes += [ "DatabaseName = '" +  athenaCommonFlags.PoolAODOutput()+ "'; COMPRESSION_LEVEL = '1'" ]

## Post-include
if hasattr(runArgs,"postInclude"): 
    for fragment in runArgs.postInclude:
        include(fragment)

## Post-exec
if hasattr(runArgs,"postExec"):
    recoLog.info("transform post-exec")
    for cmd in runArgs.postExec:
        recoLog.info(cmd)
        exec(cmd)
