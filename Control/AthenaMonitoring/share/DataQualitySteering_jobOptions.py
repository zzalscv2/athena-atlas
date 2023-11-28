# ********************************************************************************
# Offline AthenaMonitoring configuration
#
# ********************************************************************************

import six

# disable TRTEleMon until further notice
# - PUEO 20140401
TRTELEMON=False

local_logger = logging.getLogger('DataQualitySteering_jobOptions')

if DQMonFlags.doMonitoring():
   def doOldStylePreSetup():
      # don't set up lumi access if in MC or enableLumiAccess == False
      if globalflags.DataSource.get_Value() != 'geant4' and DQMonFlags.enableLumiAccess():
         if athenaCommonFlags.isOnline:
            local_logger.debug("luminosity tool not found, importing online version")
            from LumiBlockComps.LuminosityCondAlgDefault import LuminosityCondAlgOnlineDefault
            LuminosityCondAlgOnlineDefault()
         else:
            local_logger.debug("luminosity tool not found, importing offline version")
            from LumiBlockComps.LuminosityCondAlgDefault import LuminosityCondAlgDefault 
            LuminosityCondAlgDefault()

         from LumiBlockComps.LBDurationCondAlgDefault import LBDurationCondAlgDefault 
         LBDurationCondAlgDefault()

         from LumiBlockComps.TrigLiveFractionCondAlgDefault import TrigLiveFractionCondAlgDefault 
         TrigLiveFractionCondAlgDefault()

      if DQMonFlags.monManEnvironment() == 'AOD':
         from LumiBlockComps.BunchCrossingCondAlgDefault import BunchCrossingCondAlgDefault
         BunchCrossingCondAlgDefault()

         from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
         from AthenaConfiguration.AllConfigFlags import ConfigFlags

         from AthenaMonitoring.AthenaMonitoringAODRecoCfg import AthenaMonitoringAODRecoCfg
         CAtoGlobalWrapper(AthenaMonitoringAODRecoCfg, ConfigFlags)

   def doOldStylePostSetup(addlSequences=[]):
      #------------------------#
      # Trigger chain steering #
      #------------------------#
      trigMap = DQMonFlags.triggerChainMap.get_Value()
      for toolName, trigChain in six.iteritems(trigMap):
         local_logger.debug("Applying trigger %s to %s", trigChain, toolName)
         if hasattr(ToolSvc,toolName):
            tool = getattr(ToolSvc,toolName)
            tool.TriggerChain = trigChain
         else:
            local_logger.debug("%s is not found in ToolSvc. Cannot set trigger chain!", toolName)

      # force conditions update because the converter can't do it
      if DQMonFlags.monManEnvironment in ('tier0ESD', 'AOD'):
         from AthenaCommon.AlgSequence import AthSequencer
         from AthenaMonitoring.AthenaMonitoringConf import ForceIDConditionsAlg
         asq = AthSequencer("AthBeginSeq") 
         asq += [ForceIDConditionsAlg()]

      local_logger.debug('DQ Post-Setup Configuration')
      import re
      from AthenaMonitoring.EventFlagFilterTool import GetEventFlagFilterTool
      from AthenaMonitoring.AtlasReadyFilterTool import GetAtlasReadyFilterTool

      # now the DQ tools are private, extract them from the set of monitoring algorithms
      toolset = set()
      from AthenaMonitoring.AthenaMonitoringConf import AthenaMonManager
      local_logger.info(f'Configuring LWHist ROOT backend flag to {jobproperties.ConcurrencyFlags.NumThreads()>0} because NumThreads is {jobproperties.ConcurrencyFlags.NumThreads()}')

      for sq in [topSequence] + addlSequences:
         for _ in sq:
            if isinstance(_, AthenaMonManager):
               toolset.update(_.AthenaMonTools)

               # in MT the initialization can be in random order ... force all managers to have common setup
               _.FileKey             = DQMonFlags.monManFileKey()
               _.ManualDataTypeSetup = DQMonFlags.monManManualDataTypeSetup()
               _.DataType            = DQMonFlags.monManDataType()
               _.Environment         = DQMonFlags.monManEnvironment()
               _.LBsInLowStatInterval = DQMonFlags.monManLBsInLowStatInterval()
               _.LBsInMediumStatInterval = DQMonFlags.monManLBsInMediumStatInterval()
               _.LBsInHighStatInterval = DQMonFlags.monManLBsInHighStatInterval()
               _.ManualRunLBSetup    = DQMonFlags.monManManualRunLBSetup()
               _.Run                 = DQMonFlags.monManRun()
               _.LumiBlock           = DQMonFlags.monManLumiBlock()
               _.ROOTBackend         = (jobproperties.ConcurrencyFlags.NumThreads()>0)

      for tool in toolset:
         # stop lumi access if we're in MC or enableLumiAccess == False
         if 'EnableLumi' in dir(tool):
            if globalflags.DataSource.get_Value() == 'geant4' or not DQMonFlags.enableLumiAccess():
               tool.EnableLumi = False
            else:
               tool.EnableLumi = True
         # if we have the FilterTools attribute, assume this is in fact a
         # monitoring tool
         if hasattr(tool, 'FilterTools'):
            # if express: use ATLAS Ready filter
            local_logger.debug('Setting up filters for tool %s', tool)
            if rec.triggerStream()=='express':
               local_logger.info('Stream is express and we will add ready tool for %s', tool)
               tool.FilterTools += [GetAtlasReadyFilterTool()]
            # if requested: configure a generic event cleaning tool
            if not athenaCommonFlags.isOnline() and any(re.match(_, tool.name()) for _ in DQMonFlags.includeInCleaning()):
               if tool.name() in DQMonFlags.specialCleaningConfiguration():
                  config_ = DQMonFlags.specialCleaningConfiguration()[tool.name()].copy()
                  for _ in config_:
                     try:
                        config_[_] = bool(config_[_])
                     except:
                        local_logger.error('Unable to enact special event cleaning configuration for tool %s; cannot cast %s=%s to bool', tool.name(), _, config_[_])
                  config_['name'] = 'DQEventFlagFilterTool_%s' % tool.name()
                  tool.FilterTools += [GetEventFlagFilterTool(**config_)]
                  del config_
                  local_logger.info('Configurating special event cleaning for tool %s', tool)
               else:
                  local_logger.info('Configuring generic event cleaning for tool %s', tool)
                  tool.FilterTools += [GetEventFlagFilterTool('DQEventFlagFilterTool')]
            else:
               local_logger.info('NOT configuring event cleaning for tool %s', tool)

            # give all the tools the trigger translator
            if DQMonFlags.useTrigger():
               tool.TrigDecisionTool = getattr(ToolSvc, DQMonFlags.nameTrigDecTool())

            if DQMonFlags.monToolPostExec():
               postprocfunc = eval(DQMonFlags.monToolPostExec())
               local_logger.debug('Applying postexec transform to  ===> %s', tool)
               postprocfunc(tool)
               del postprocfunc

   if not DQMonFlags.doNewMonitoring():
      if DQMonFlags.useTrigger():
         from AthenaConfiguration.AllConfigFlags import ConfigFlags
         from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
         from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
         CAtoGlobalWrapper(TrigDecisionToolCfg, ConfigFlags)

      doOldStylePreSetup()

      #---------------------------#
      # Inner detector monitoring #
      #---------------------------#
      if DQMonFlags.doPixelMon() or DQMonFlags.doSCTMon() or DQMonFlags.doTRTMon() or \
            DQMonFlags.doInDetGlobalMon() or DQMonFlags.doInDetAlignMon():
         try:
            include("InDetRecExample/InDetMonitoring.py")
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up inner detector monitoring")

      #----------------------------#
      # Global combined monitoring #
      #----------------------------#
      if DQMonFlags.doDataFlowMon() or DQMonFlags.doGlobalMon():
         try:
            include("DataQualityTools/DataQualityMon_jobOptions.py")
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up global monitoring")
      
      #--------------------#
      # Trigger monitoring #
      #--------------------#

      if DQMonFlags.doCTPMon():
         local_logger.warning("The legacy Run-2 CTP monitoring is no longer supported")

      if DQMonFlags.doLVL1CaloMon():
         local_logger.warning("The legacy L1Calo monitoring is no longer supported")

      if DQMonFlags.doHLTMon():
         local_logger.warning("The legacy Run-2 HLT monitoring is no longer supported")

      #--------------#
      # TRT electron #
      #--------------#
      if DQMonFlags.doTRTElectronMon() and TRTELEMON:
         try:
            include("InDetPerformanceMonitoring/TRT_Monitoring_RecExCommonAddOn_jobOptions.py") 
            TRTElMonMan = topSequence.TRTElectronMonManager
            TRTElMonMan.FileKey     = DQMonFlags.monManFileKey()
            TRTElMonMan.Environment = DQMonFlags.monManEnvironment()
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up TRT electron monitoring")

      #----------------#
      # ID performance #
      #----------------#
      if DQMonFlags.doInDetPerfMon():
         try:
            include("InDetPerformanceMonitoring/IDPerfMon_jobOptions.py")
            IDPerfMonManager = topSequence.IDPerfMonManager
            IDPerfMonManager.FileKey          = DQMonFlags.monManFileKey()
            IDPerfMonManager.Environment      = DQMonFlags.monManEnvironment()
            include("InDetDiMuonMonitoring/InDetDiMuMon_jobOptions.py")
            InDetDiMuMonManager = topSequence.InDetDiMuMonManager
            InDetDiMuMonManager.FileKey          = DQMonFlags.monManFileKey()
            InDetDiMuMonManager.Environment      = DQMonFlags.monManEnvironment()
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up InDetPerformance/InDetDiMuon monitoring")

      #--------------------#
      # TileCal monitoring #
      #--------------------#
      if DQMonFlags.doTileMon():
         try:
            include("TileMonitoring/TileMon_jobOptions.py")
      
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up Tile monitoring")

      #------------------#
      # LAr monitoring   #
      #------------------#
      if DQMonFlags.doLArMon():
         from LArMonTools.LArMonFlags import LArMonFlags
         if LArMonFlags.doLArCollisionTimeMon():
            #Schedule algorithms producing collision timing
            include("LArClusterRec/LArClusterCollisionTime_jobOptions.py")
            include("LArCellRec/LArCollisionTime_jobOptions.py")
         try:
            LArMon = AthenaMonManager(name="LArMonManager",
                           FileKey             = DQMonFlags.monManFileKey(),
                           Environment         = DQMonFlags.monManEnvironment(),
                           ManualDataTypeSetup = DQMonFlags.monManManualDataTypeSetup(),
                           DataType            = DQMonFlags.monManDataType())
            topSequence += LArMon
            include("LArMonTools/LArAllMonitoring_jobOptions.py")
            include("LArMonitoring/LArMonitoring_jobOption.py")
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up LAr monitoring")

      #-------------------------------------------------------------------------#
      # Calo monitoring - cells and clusters independent of LAr or Tile origin  #
      #-------------------------------------------------------------------------#
      if DQMonFlags.doCaloMon():
         try:
            include("CaloMonitoring/CaloAllMonitoring_jobOptions.py")
            include("CaloMonitoring/CaloNewMonitoring_jobOptions.py")
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up Calo monitoring")

      #-------------------#
      # Egamma monitoring #
      #-------------------#
      if DQMonFlags.doEgammaMon():
         try:
            include("egammaPerformance/egammaMonitoring_jobOptions.py")
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up Egamma monitoring")

      #----------------#
      # MET monitoring #
      #----------------#
      if DQMonFlags.doMissingEtMon():
         try:
            include("MissingETMonitoring/MissingET_Monitoring.py")
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up ETmiss monitoring")

      #----------------#
      # Jet monitoring #
      #----------------#
      if DQMonFlags.doJetMon():
         try:
            include("JetMonitoring/JetMonitoring_jobOptions.py")
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up Jet monitoring")

      #----------------#
      # Tau monitoring #
      #----------------#
      if DQMonFlags.doTauMon():
         local_logger.warning("The legacy TauCP monitoring is no longer supported")

      #--------------------#
      # Jet tag monitoring #
      #--------------------#
      if DQMonFlags.doJetTagMon():
         try:
            include("JetTagMonitoring/JetTagMonitoring_jobOptions.py")
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up jet-tag monitoring")

      #-----------------------------#
      # MuonSpectrometer monitoring #
      #-----------------------------#
      if (DQMonFlags.doMuonRawMon() or DQMonFlags.doMuonSegmentMon()
         or DQMonFlags.doMuonTrackMon() or DQMonFlags.doMuonAlignMon()
         or DQMonFlags.doMuonTrkPhysMon() or DQMonFlags.doMuonPhysicsMon()):
         try:
            include("MuonDQAMonitoring/MuonDetMonitoring.py")
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up Muon detector monitoring")
   
      #------------------#
      # LUCID monitoring #
      #------------------#
      if DQMonFlags.doLucidMon():
         try:
            include("LUCID_Monitoring/LUCIDMon_jobOptions.py")
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up LUCID monitoring")

      #------------------#                                                                                                                       
      # AFP monitoring   #                                                                                                                       
      #------------------#                                                                                                                       
      if DQMonFlags.doAFPMon():
         try:
            include("AFP_Monitoring/AFPMonitoring_jobOptions.py")
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up AFP monitoring")


      #---------------------#
      # HeavyIon monitoring #
      #---------------------#
      if DQMonFlags.doHIMon():
         try:
            include("HIMonitoring/HIMonitoringSteering_jo.py")
         except Exception:
            treatException("DataQualitySteering_jobOptions.py: exception when setting up HI monitoring")

      #--------------------------#
      # Post-setup configuration #
      #--------------------------#
      doOldStylePostSetup()

   else:
      from AthenaConfiguration.AllConfigFlags import ConfigFlags
      ConfigFlags.dump()

      from AthenaConfiguration import ComponentAccumulator
      from AthenaMonitoring.AthenaMonitoringCfg import AthenaMonitoringCfg
      ComponentAccumulator.CAtoGlobalWrapper(AthenaMonitoringCfg, ConfigFlags)

      if len([_ for _ in AthSequencer("AthCondSeq") if _.getName()=="PixelDetectorElementCondAlg"]) > 0:
         local_logger.info("DQ: force ID conditions loading")
         from AthenaMonitoring import AthenaMonitoringConf
         asq = AthSequencer("AthBeginSeq")
         asq += AthenaMonitoringConf.ForceIDConditionsAlg("ForceIDConditionsAlg")

   if DQMonFlags.doPostProcessing():
      from AthenaConfiguration.AllConfigFlags import ConfigFlags
      asq = AthSequencer("AthEndSeq")
      from DataQualityUtils.DQPostProcessingAlg import DQPostProcessingAlg
      ppa = DQPostProcessingAlg("DQPostProcessingAlg")
      ppa.ExtraInputs = [( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' )]
      ppa.Interval = DQMonFlags.postProcessingInterval()
      if ConfigFlags.Common.isOnline:
         ppa.FileKey = ((ConfigFlags.DQ.FileKey + '/') if not ConfigFlags.DQ.FileKey.endswith('/') 
                        else ConfigFlags.DQ.FileKey) 
      asq += ppa

del local_logger
