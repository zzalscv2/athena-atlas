########################################################################
# Offline AthenaMonitoring configuration
# set DQMonFlags; defaults are in DataQualityTools/python/DQMonFlags.py
########################################################################

local_logger = logging.getLogger('DQMonFlagsConfig_jobOptions')

if not 'DQMonFlags' in dir():
   local_logger.debug("DQMonFlags not yet imported - I import them now")
   from AthenaMonitoring.DQMonFlags import DQMonFlags

from AthenaConfiguration.AllConfigFlags import ConfigFlags
ConfigFlags.DQ.isReallyOldStyle = True

if not 'rec' in dir():
   from RecExConfig.RecFlags import rec

from RecExConfig.RecAlgsFlags import recAlgs

# if we are not in MT mode, do not run new-style monitoring
# if you really want to override, use set_Value_and_Lock(True) on the relevant flags below
if jobproperties.ConcurrencyFlags.NumThreads() == 0:
   DQMonFlags.doNewMonitoring=False

# Set the data type based on beamType/HI flag
if globalflags.DataSource.get_Value() == 'geant4':
   DQMonFlags.monManDataType = 'monteCarlo'
elif (rec.doHeavyIon() or rec.doHIP()):
   DQMonFlags.monManDataType = 'heavyioncollisions'
   DQMonFlags.doHIMon = True
elif jobproperties.Beam.beamType()   == 'cosmics':
   DQMonFlags.monManDataType = 'cosmics'
elif jobproperties.Beam.beamType() == 'collisions':
   DQMonFlags.monManDataType = 'collisions'
elif jobproperties.Beam.beamType() == 'singlebeam':
   ## There is no singlebeam in AthenaMonitoring
   DQMonFlags.monManDataType = 'collisions'
else:
   local_logger.warning("invalid jobproperties.Beam.beamType(): %s, using default (%s)", 
                        jobproperties.Beam.beamType(), DQMonFlags.monManDataType)


# First, test to see if this is being run online
if athenaCommonFlags.isOnline:
   DQMonFlags.monManEnvironment='online'

# Running offline; set the monMan environment based on monType
elif DQMonFlags.monType=='BSonly':
   DQMonFlags.monManEnvironment='tier0Raw'
   if rec.readESD(): 
      DQMonFlags.doMonitoring=False

elif DQMonFlags.monType=='ESDonly':  
   DQMonFlags.monManEnvironment='tier0ESD' 
   if rec.readRDO(): 
      DQMonFlags.doMonitoring=False

elif (DQMonFlags.monType=='BS_ESD'):
   if rec.readRDO():
      DQMonFlags.monManEnvironment='tier0Raw'
   if rec.readESD(): 
      DQMonFlags.monManEnvironment='tier0ESD'
   if rec.readAOD():
      DQMonFlags.monManEnvironment='AOD'

elif DQMonFlags.monType=='BSall':
   DQMonFlags.monManEnvironment='tier0'
   if rec.readESD(): 
      DQMonFlags.doMonitoring=False
else:
   local_logger.warning("invalid DQMonFlags.monType: %s, using default monManEnvironment", DQMonFlags.monType())

# the meaning of this flag has changed in MT
if (rec.doTrigger() == False and 
    not (ConfigFlags.Trigger.EDMVersion == 3 and DQMonFlags.monManEnvironment=='tier0ESD' and DQMonFlags.useTrigger())):
   DQMonFlags.useTrigger=False     # steers trigger-awareness
   DQMonFlags.doLVL1CaloMon=False
   DQMonFlags.doCTPMon=False
   DQMonFlags.doHLTMon=False
elif rec.doTrigger() and ConfigFlags.Trigger.EDMVersion < 3 and not DQMonFlags.doNewMonitoring():
   # CTP/L1Calo monitoring currently not supported on old data (ATR-24262)
   DQMonFlags.doLVL1CaloMon=False
   DQMonFlags.doCTPMon=False

if not DQMonFlags.doMonitoring():
   local_logger.info("monitoring globally switched off")
   DQMonFlags.doGlobalMon=False
   DQMonFlags.doLVL1CaloMon=False
   DQMonFlags.doCTPMon=False
   DQMonFlags.doHLTMon=False
   DQMonFlags.doPixelMon=False
   DQMonFlags.doSCTMon=False
   DQMonFlags.doTRTMon=False
   DQMonFlags.doTRTElectronMon=False
   DQMonFlags.doInDetGlobalMon=False
   DQMonFlags.doInDetAlignMon=False
   DQMonFlags.doInDetPerfMon=False
   DQMonFlags.doLArMon=False
   DQMonFlags.doCaloMon=False
   DQMonFlags.doTileMon=False
   DQMonFlags.doEgammaMon=False
   DQMonFlags.doMissingEtMon=False
   DQMonFlags.doJetMon=False
   DQMonFlags.doTauMon=False
   DQMonFlags.doJetTagMon=False
   DQMonFlags.doMuonRawMon=False
   DQMonFlags.doMuonSegmentMon=False
   DQMonFlags.doMuonTrackMon=False
   DQMonFlags.doMuonAlignMon=False
   DQMonFlags.doMuonPhysicsMon=False
   DQMonFlags.doMuonTrkPhysMon=False
   DQMonFlags.doMuonCombinedMon=False
   DQMonFlags.doLucidMon=False
   DQMonFlags.doHIMon=False
else:
   local_logger.info("monitoring environment set to %s", DQMonFlags.monManEnvironment())

   # while we're in mixed mode for the following packages, drive these even if in new-style monitoring
   # AOD monitoring
   if DQMonFlags.monManEnvironment == 'AOD':
      DQMonFlags.histogramFile='MonitorAOD.root'
      DQMonFlags.doCTPMon=False
      DQMonFlags.doLVL1CaloMon=False
   # ESD monitoring: switch off DQ monitoring packages which are not yet migrated:
   elif DQMonFlags.monManEnvironment == 'tier0ESD':
      DQMonFlags.histogramFile='MonitorESD.root'
      DQMonFlags.doCTPMon=False

   # new-style monitoring drives this internally so skip this section
   if not DQMonFlags.doNewMonitoring:

      # AOD monitoring
      if DQMonFlags.monManEnvironment == 'AOD':
         DQMonFlags.histogramFile='MonitorAOD.root'
         DQMonFlags.doCaloMon=False
         DQMonFlags.doLArMon=False
         DQMonFlags.doTileMon=False
         DQMonFlags.doCTPMon=False
         DQMonFlags.doPixelMon=False
         DQMonFlags.doSCTMon=False
         DQMonFlags.doTRTMon=False
         DQMonFlags.doTRTElectronMon=False
         DQMonFlags.doInDetGlobalMon=False
         DQMonFlags.doInDetAlignMon=False

         DQMonFlags.doLVL1CaloMon=False
         DQMonFlags.doEgammaMon=False
         DQMonFlags.doMuonRawMon=False
         DQMonFlags.doLucidMon=False

      # ESD monitoring: switch off DQ monitoring packages which are not yet migrated:
      elif DQMonFlags.monManEnvironment == 'tier0ESD':
         DQMonFlags.histogramFile='MonitorESD.root'
         DQMonFlags.doCTPMon=False
         DQMonFlags.doPixelMon=False
         DQMonFlags.doSCTMon=False
         DQMonFlags.doTRTMon=False
         DQMonFlags.doTRTElectronMon=False
         DQMonFlags.doInDetGlobalMon=False
         DQMonFlags.doInDetAlignMon=False
      # ESD monitoring: packages which use only ESD: disable when running over BS
      elif DQMonFlags.monManEnvironment == 'tier0Raw':
         DQMonFlags.doInDetPerfMon=False
         DQMonFlags.doMissingEtMon=False
         DQMonFlags.doTauMon=False
         DQMonFlags.doMuonTrackMon=False
         DQMonFlags.doMuonAlignMon=False
         DQMonFlags.doMuonPhysicsMon=False
         DQMonFlags.doMuonSegmentMon=False
         DQMonFlags.doMuonTrkPhysMon=False
         DQMonFlags.doMuonCombinedMon=False
         DQMonFlags.doLucidMon=False
         DQMonFlags.doJetTagMon=False
         DQMonFlags.doJetMon=False
   
   # no HLT monitoring if new monitoring and input is Run 2 EDM
   if DQMonFlags.doNewMonitoring() and ConfigFlags.Trigger.EDMVersion == 2:
      DQMonFlags.doHLTMon = False
      
   # switch off monitoring if reco is off during BS reading
   if rec.readRDO() and not 'DetFlags' in dir():
      from AthenaCommon.DetFlags import DetFlags

   if rec.readRDO():
      if not DetFlags.detdescr.ID_on():
         local_logger.info("ID reco is off, switching off ID monitoring")
         DQMonFlags.doPixelMon=False
         DQMonFlags.doSCTMon=False
         DQMonFlags.doTRTMon=False
         DQMonFlags.doTRTElectronMon=False
         DQMonFlags.doInDetGlobalMon=False
         DQMonFlags.doInDetAlignMon=False
         DQMonFlags.doInDetPerfMon=False

      if not DetFlags.detdescr.LAr_on():
         local_logger.info("LAr reco is off, switching off LAr monitoring")
         DQMonFlags.doLArMon=False

      if not DetFlags.detdescr.Tile_on():
         local_logger.info("Tile reco is off, switching off Tile monitoring")
         DQMonFlags.doTileMon=False

      if (not DetFlags.detdescr.LAr_on()) and (not DetFlags.detdescr.Tile_on()) :
         local_logger.info("LAr and Tile reco are off, switching off Calo monitoring")
         DQMonFlags.doCaloMon=False

      if not DetFlags.detdescr.Muon_on():
         local_logger.info("Muon reco is off, switching off muon monitoring")
         DQMonFlags.doMuonRawMon=False
         DQMonFlags.doMuonSegmentMon=False
         DQMonFlags.doMuonTrackMon=False
         DQMonFlags.doMuonAlignMon=False
         DQMonFlags.doMuonPhysicsMon=False
         DQMonFlags.doMuonTrkPhysMon=False
         DQMonFlags.doMuonCombinedMon=False

# turn monitoring off if rec.do<System>=False
# add more protections against reco off and running over bytestream
#
# From David Rousseau (4 Nov 2009): https://savannah.cern.ch/bugs/?59075
# rec.doCalo=False means: "do not do anything wrt to calo"
# CaloRecFlags.Enabled=False means: switch off just the calo reco
# (and similar flags for other systems)

if rec.readRDO():
   from CaloRec.CaloRecFlags import jobproperties
   from InDetRecExample.InDetJobProperties import InDetFlags
   from MuonRecExample.MuonRecFlags import muonRecFlags
   from MuonCombinedRecExample.MuonCombinedRecFlags import muonCombinedRecFlags
   from tauRec.tauRecFlags import jobproperties

from JetRec.JetRecFlags import jobproperties

if (not rec.doCalo()) or (rec.readRDO() and not jobproperties.CaloRecFlags.Enabled()):
   DQMonFlags.doCaloMon=False
   DQMonFlags.doTileMon=False
   DQMonFlags.doLArMon=False

if (not rec.doInDet()) or (rec.readRDO() and not jobproperties.InDetJobProperties.Enabled()):
   DQMonFlags.doPixelMon=False
   DQMonFlags.doSCTMon=False
   DQMonFlags.doTRTMon=False
   DQMonFlags.doTRTElectronMon=False
   DQMonFlags.doInDetGlobalMon=False
   DQMonFlags.doInDetAlignMon=False
   DQMonFlags.doInDetPerfMon=False

if (not rec.doMuon()) or (rec.readRDO() and not jobproperties.MuonRec.Enabled()):
   DQMonFlags.doMuonRawMon=False
   DQMonFlags.doMuonSegmentMon=False
   DQMonFlags.doMuonTrackMon=False
   DQMonFlags.doMuonAlignMon=False
   DQMonFlags.doMuonPhysicsMon=False
   DQMonFlags.doMuonTrkPhysMon=False
   DQMonFlags.doMuonCombinedMon=False

if (not rec.doMuonCombined()) or (rec.readRDO() and not jobproperties.MuonCombinedRec.Enabled()):
   DQMonFlags.doMuonCombinedMon=False

if (not rec.doEgamma()) or (rec.readRDO() and not ConfigFlags.Reco.EnableEgamma):
   DQMonFlags.doEgammaMon=False

#if (not rec.doJetRec()) or (rec.readRDO() and not jobproperties.JetRecFlags.Enabled()):
if (not rec.doJetMissingETTag() or (rec.readRDO() and not jobproperties.JetRecFlags.Enabled())):
   DQMonFlags.doJetMon=False
   DQMonFlags.doMissingEtMon=False
   DQMonFlags.doJetTagMon=False

#if (not rec.doTau()) or (rec.readRDO() and not jobproperties.TauRecFlags.Enabled()):
if (not rec.doTau()):
   DQMonFlags.doTauMon=False

if not rec.doAFP() or DQMonFlags.monManDataType == 'monteCarlo':
   DQMonFlags.doAFPMon=False


#
# Stream Aware Monitoring
# Turn off tools on a stream basis to save cpu
#

if DQMonFlags.doStreamAwareMon:
   local_logger.info("Setting stream-aware monitoring for stream %s", rec.triggerStream())

   from LArMonTools.LArMonFlags import LArMonFlags

   # Some LAr monitoring is only done on a couple of streams for cpu sake
   # So set them as false by default. Turn them on as needed
   LArMonFlags.doLArRODMonTool=False            # savannah bug report #83390
   LArMonFlags.doLArRawMonitorSignal=False
   LArMonFlags.doLArRawChannelMon=False
   LArMonFlags.doLArCollisionTimeMon=False
   LArMonFlags.doLArAffectedRegions=False
   LArMonFlags.doLArHVCorrectionMonTool=False
   LArMonFlags.doLArCoverage=False
   doCaloCellVecMon=False

   # The following are ON except for certain streams
   LArMonFlags.doLArDigitMon=True
   LArMonFlags.doLArNoisyROMon=True

   # All monitoring turned on for express stream (except LArRawChannelMon)
   # HIP/HI runs will use the express settings for MinBias, MinBiasOverlay, HardProbes, bulk, and UPC
   if (rec.triggerStream()=='express' or rec.triggerStream()=='Main' or
       ((rec.doHIP() or rec.doHeavyIon())
        and rec.triggerStream() in ['MinBias', 'MinBiasOverlay', 'HardProbes', 'bulk', 'UPC'])):
      LArMonFlags.doLArCollisionTimeMon=True
      LArMonFlags.doLArAffectedRegions=True
      LArMonFlags.doLArHVCorrectionMonTool=True
      LArMonFlags.doLArCoverage=True
      LArMonFlags.doLArRODMonTool=True          # savannah bug report #83390
      if (rec.triggerStream()=='express' or rec.triggerStream()=='Main'):
         doCaloCellVecMon=True

   elif (rec.triggerStream()=='CosmicCalo'):
      LArMonFlags.doLArRawChannelMon=True
      LArMonFlags.doLArRawMonitorSignal=True
      LArMonFlags.doLArAffectedRegions=True
      LArMonFlags.doLArHVCorrectionMonTool=True
      LArMonFlags.doLArCoverage=True
      LArMonFlags.doLArDigitMon=False
      doCaloCellVecMon=True
      DQMonFlags.doTauMon=False
      DQMonFlags.doPixelMon=False
      DQMonFlags.doMuonRawMon=False
      DQMonFlags.doMuonTrackMon=False
      DQMonFlags.doMuonAlignMon=False
      DQMonFlags.doMuonCombinedMon=False
      DQMonFlags.doMuonSegmentMon=False
      DQMonFlags.doMuonPhysicsMon=False
      DQMonFlags.doMuonTrkPhysMon=False
      DQMonFlags.doTRTMon=False
      DQMonFlags.doJetTagMon=False
   elif (rec.triggerStream()=='JetTauEtmiss'):
      DQMonFlags.doEgammaMon=False
      DQMonFlags.doPixelMon=False
      DQMonFlags.doSCTMon=False
      DQMonFlags.doTRTMon=False
      LArMonFlags.doLArRawChannelMon=False
   elif (rec.triggerStream()=='Egamma'):
      DQMonFlags.doJetMon=False
      DQMonFlags.doMissingEtMon=False
      DQMonFlags.doTauMon=False
      DQMonFlags.doMuonRawMon=False
      DQMonFlags.doMuonTrackMon=False
      DQMonFlags.doMuonAlignMon=False
      DQMonFlags.doMuonCombinedMon=False
      DQMonFlags.doMuonSegmentMon=False
      DQMonFlags.doMuonPhysicsMon=False
      DQMonFlags.doMuonTrkPhysMon=False
      DQMonFlags.doTileMon=False
      DQMonFlags.doPixelMon=False
      DQMonFlags.doSCTMon=False
      DQMonFlags.doJetTagMon=False
      LArMonFlags.doLArRawChannelMon=False
   elif (rec.triggerStream()=='Muons'):
      DQMonFlags.doEgammaMon=False
      DQMonFlags.doJetMon=False
      DQMonFlags.doMissingEtMon=False
      DQMonFlags.doTauMon=False
      DQMonFlags.doTileMon=False
      DQMonFlags.doPixelMon=False
      DQMonFlags.doSCTMon=False
      DQMonFlags.doTRTMon=False
      DQMonFlags.doCaloMon=False
      DQMonFlags.doJetTagMon=False
      LArMonFlags.doLArRawChannelMon=False
      LArMonFlags.doLArCollisionTimeMon=False
      LArMonFlags.doLArAffectedRegions=False
      #LArMonFlags.doLArFEBMon=False
      LArMonFlags.doLArHVCorrectionMonTool=False
      LArMonFlags.doLArCoverage=False
      LArMonFlags.doLArDigitMon=False
      LArMonFlags.doLArNoisyROMon=False
   elif (rec.triggerStream()=='IDCosmic'):
      DQMonFlags.doEgammaMon=False
      DQMonFlags.doJetMon=False
      DQMonFlags.doMissingEtMon=False
      DQMonFlags.doTauMon=False
      DQMonFlags.doTileMon=False
      DQMonFlags.doMuonRawMon=False
      DQMonFlags.doMuonTrackMon=False
      DQMonFlags.doMuonAlignMon=False
      DQMonFlags.doMuonSegmentMon=False
      DQMonFlags.doMuonPhysicsMon=False
      DQMonFlags.doCaloMon=False
      DQMonFlags.doJetTagMon=False
      #LArMonFlags.doLArFEBMon=False
      #LArMonFlags.doLArDigitMon=False
      #LArMonFlags.doLArNoisyROMon=False
   elif (rec.triggerStream()=='ZeroBias'):
      DQMonFlags.doTauMon=False
      DQMonFlags.doPixelMon=True
      DQMonFlags.doMuonRawMon=False
      DQMonFlags.doMuonTrackMon=False
      DQMonFlags.doMuonAlignMon=False
      DQMonFlags.doMuonCombinedMon=False
      DQMonFlags.doMuonSegmentMon=False
      DQMonFlags.doMuonPhysicsMon=False
      DQMonFlags.doMuonTrkPhysMon=False
      DQMonFlags.doTRTMon=False
      DQMonFlags.doJetTagMon=False
      #LArMonFlags.doLArRawChannelMon=False
      #LArMonFlags.doLArCollisionTimeMon=False
      #LArMonFlags.doLArAffectedRegions=False
      #LArMonFlags.doLArFEBMon=False
      #LArMonFlags.doLArHVCorrectionMonTool=False
      #LArMonFlags.doLArCoverage=False
      #LArMonFlags.doLArDigitMon=False
      #LArMonFlags.doLArNoisyROMon=False
      #LArMonFlags.doLArRODMonTool=True          # savannah bug report #83390

   elif (rec.triggerStream()=='L1Calo' or rec.triggerStream()=='L1Topo'):
      DQMonFlags.doPixelMon=False
      DQMonFlags.doMuonRawMon=False
      DQMonFlags.doMuonTrackMon=False
      DQMonFlags.doMuonAlignMon=False
      DQMonFlags.doMuonCombinedMon=False
      DQMonFlags.doMuonSegmentMon=False
      DQMonFlags.doMuonPhysicsMon=False
      DQMonFlags.doMuonTrkPhysMon=False
      DQMonFlags.doTRTMon=False
      DQMonFlags.doJetTagMon=False

   # Default stream-aware settings for unspecified streams
   # Only run a select few set of tools
   else:
      #LArMonFlags.doLArRawChannelMon=False
      DQMonFlags.doEgammaMon=False
      DQMonFlags.doJetMon=False
      DQMonFlags.doMissingEtMon=False
      DQMonFlags.doTauMon=False
      DQMonFlags.doMuonTrackMon=False
      DQMonFlags.doMuonAlignMon=False
      DQMonFlags.doMuonCombinedMon=False
      DQMonFlags.doMuonSegmentMon=False
      DQMonFlags.doMuonPhysicsMon=False
      DQMonFlags.doMuonTrkPhysMon=False
      DQMonFlags.doCaloMon=False
      DQMonFlags.doJetTagMon=False

else:
   local_logger.info("Stream-Aware monitoring is turned OFF")

# If data type is '*comm' disable ATLAS Ready filter by default
if (rec.projectName.get_Value().endswith('comm') and 
    not DQMonFlags.disableAtlasReadyFilter()
    ):
   local_logger.info("This is a commissioning project tag, will attempt to disable ATLAS Ready filter for monitoring tools. To really enable it, use DQMonFlags.disableAtlasReadyFilter.set_Value_and_Lock(False).")
   DQMonFlags.disableAtlasReadyFilter=True

DQMonFlags.lock_JobProperties()
DQMonFlags.print_JobProperties()

local_logger.info("DQ: setting up ConfigFlags")

if DQMonFlags.configureFromOldStyleFlags():
   ConfigFlags.DQ.doMonitoring=DQMonFlags.doMonitoring()
   ConfigFlags.DQ.FileKey=DQMonFlags.monManFileKey()
   ConfigFlags.DQ.Environment=DQMonFlags.monManEnvironment()
   ConfigFlags.DQ.useTrigger=DQMonFlags.useTrigger()
   ConfigFlags.DQ.triggerDataAvailable=DQMonFlags.useTrigger()

   Steering = ConfigFlags.DQ.Steering
   Steering.doDataFlowMon=DQMonFlags.doDataFlowMon()
   Steering.doGlobalMon=DQMonFlags.doGlobalMon()
   Steering.doHLTMon=DQMonFlags.doHLTMon()
   Steering.doLVL1CaloMon=DQMonFlags.doLVL1CaloMon()
   Steering.doCTPMon=DQMonFlags.doCTPMon()
   Steering.doPixelMon=DQMonFlags.doPixelMon()
   Steering.doSCTMon=DQMonFlags.doSCTMon()
   Steering.doTRTMon=DQMonFlags.doTRTMon()
   Steering.InDet.doGlobalMon=DQMonFlags.doInDetGlobalMon()
   Steering.InDet.doAlignMon=DQMonFlags.doInDetAlignMon()
   Steering.doInDetMon = Steering.InDet.doGlobalMon or Steering.InDet.doAlignMon
   Steering.doLArMon=DQMonFlags.doLArMon()
   Steering.doTileMon=DQMonFlags.doTileMon()
   Steering.doCaloGlobalMon=DQMonFlags.doCaloMon()
   Steering.Muon.doRawMon=DQMonFlags.doMuonRawMon()
   Steering.Muon.doTrackMon=DQMonFlags.doMuonTrackMon()
   Steering.doMuonMon=(Steering.Muon.doRawMon or Steering.Muon.doTrackMon)
   Steering.doLucidMon=DQMonFlags.doLucidMon()
   Steering.doAFPMon=DQMonFlags.doAFPMon()
   Steering.doHIMon=DQMonFlags.doHIMon()
   Steering.doEgammaMon=DQMonFlags.doEgammaMon()
   Steering.doJetMon=DQMonFlags.doJetMon()
   Steering.doMissingEtMon=DQMonFlags.doMissingEtMon()
   Steering.doTauMon=DQMonFlags.doTauMon()
   Steering.doJetTagMon=DQMonFlags.doJetTagMon()
   Steering.doJetInputsMon=DQMonFlags.doJetInputsMon()

if DQMonFlags.doNewMonitoring():
   ConfigFlags.DQ.isReallyOldStyle = False
   ConfigFlags.DQ.Environment = DQMonFlags.monManEnvironment()  # unfortunately cannot break this dependency

del local_logger
