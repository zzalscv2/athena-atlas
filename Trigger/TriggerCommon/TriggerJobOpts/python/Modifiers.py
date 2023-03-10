# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

##############################################################
# Modifiers.py
#
#  Small classes that modify the setup for non-standard running
#  conditions and fixes that are not yet in the slice/detector jO
#
#  for now there are no options foreseen for the modifiers
#
#  Permanent fixes that are only applied online should be
#  put into runHLT_standalone.py
###############################################################

from AthenaCommon.AppMgr import theApp
from AthenaCommon.AppMgr import ServiceMgr as svcMgr

from AthenaCommon.Logging import logging
log = logging.getLogger('Modifiers.py')

_run_number = None   # set by runHLT_standalone
_lb_number = None   # set by runHLT_standalone

# Base class
class _modifier:
    def name(self):
        return self.__class__.__name__

    def __init__(self):
        log.warning('using modifier: %s', self.name())
        log.warning(self.__doc__)

    def preSetup(self, flags):
        pass #default is no action

    def postSetup(self, flags):
        pass #default is no action


###############################################################
# Detector maps and conditions
###############################################################

class BunchSpacing25ns(_modifier):
    """
    ID (and other settings) related to 25ns bunch spacing
    """
    def preSetup(self, flags):
        from AthenaCommon.BeamFlags import jobproperties
        jobproperties.Beam.bunchSpacing = 25
        flags.Beam.BunchSpacing = 25
        from InDetTrigRecExample.InDetTrigFlags import InDetTrigFlags
        InDetTrigFlags.InDet25nsec.set_Value_and_Lock(True)


class ForceMuonDataType(_modifier):
    """
    Hardcode muon data to be of type of atlas
      this determines which cabling service to use
    """
    def preSetup(self, flags):
        from MuonByteStream.MuonByteStreamFlags import muonByteStreamFlags
        muonByteStreamFlags.RpcDataType = 'atlas'
        muonByteStreamFlags.MdtDataType = 'atlas'
        muonByteStreamFlags.TgcDataType = 'atlas'


class useNewRPCCabling(_modifier):
    """
    Switch to new RPC cabling code
    """
    def preSetup(self, flags):
        from MuonCnvExample.MuonCnvFlags import muonCnvFlags
        if hasattr(muonCnvFlags,'RpcCablingMode'):
            muonCnvFlags.RpcCablingMode.set_Value_and_Lock('new')

class SolenoidOff(_modifier):
    """
    Turn solenoid field OFF
    """
    def postSetup(self, flags):
        from AthenaCommon.AlgSequence import AthSequencer
        condSeq = AthSequencer("AthCondSeq")
        condSeq.AtlasFieldMapCondAlg.MapSoleCurrent = 0

class ToroidsOff(_modifier):
    """
    Turn toroid fields OFF
    """
    def postSetup(self, flags):
        from AthenaCommon.AlgSequence import AthSequencer
        condSeq = AthSequencer("AthCondSeq")
        condSeq.AtlasFieldMapCondAlg.MapToroCurrent = 0

class BFieldFromDCS(_modifier):
    """
    Read B-field currents from DCS (also works for MC)
    """
    def postSetup(self, flags):
        from IOVDbSvc.CondDB import conddb
        conddb._SetAcc("DCS_OFL","COOLOFL_DCS")
        conddb.addFolder("DCS_OFL","/EXT/DCS/MAGNETS/SENSORDATA",className="CondAttrListCollection")
        from AthenaCommon.AlgSequence import AthSequencer
        condSeq = AthSequencer("AthCondSeq")
        # see ATLASRECTS-5604 for these settings:
        condSeq.AtlasFieldMapCondAlg.LoadMapOnStart = False
        condSeq.AtlasFieldMapCondAlg.UseMapsFromCOOL = True
        condSeq.AtlasFieldCacheCondAlg.UseDCS = True
        if hasattr(svcMgr,'HltEventLoopMgr'):
            svcMgr.HltEventLoopMgr.setMagFieldFromPtree = False

class BFieldAutoConfig(_modifier):
    """
    Read field currents from configuration ptree (athenaHLT)
    """
    def postSetup(self, flags):
        if hasattr(svcMgr,'HltEventLoopMgr'):
            svcMgr.HltEventLoopMgr.setMagFieldFromPtree = True

class useOracle(_modifier):
    """
    Disable the use of SQLite for COOL and geometry
    """
    def postSetup(self, flags):
        if hasattr(svcMgr,'DBReplicaSvc'):
            svcMgr.DBReplicaSvc.UseCOOLSQLite = False
            svcMgr.DBReplicaSvc.UseCOOLFrontier = True
            svcMgr.DBReplicaSvc.UseGeomSQLite = False

class useOnlineLumi(_modifier):
    """
    Use online LuminosityTool
    """
    def preSetup(self, flags):
        from LumiBlockComps.LuminosityCondAlgDefault import LuminosityCondAlgOnlineDefault
        LuminosityCondAlgOnlineDefault()


class forceConditions(_modifier):
    """
    Force all conditions (except prescales) to match run from input file
    """
    def postSetup(self, flags):
        # Do not override these folders:
        ignore = ['/TRIGGER/HLT/PrescaleKey']   # see ATR-22143

        # All time-based folders (from IOVDbSvc DEBUG message, see athena!38274)
        timebased = ['/TDAQ/OLC/CALIBRATIONS',
                     '/TDAQ/Resources/ATLAS/SCT/Robins',
                     '/SCT/DAQ/Config/ChipSlim',
                     '/SCT/DAQ/Config/Geog',
                     '/SCT/DAQ/Config/MUR',
                     '/SCT/DAQ/Config/Module',
                     '/SCT/DAQ/Config/ROD',
                     '/SCT/DAQ/Config/RODMUR',
                     '/SCT/HLT/DCS/HV',
                     '/SCT/HLT/DCS/MODTEMP',
                     '/MUONALIGN/Onl/MDT/BARREL',
                     '/MUONALIGN/Onl/MDT/ENDCAP/SIDEA',
                     '/MUONALIGN/Onl/MDT/ENDCAP/SIDEC',
                     '/MUONALIGN/Onl/TGC/SIDEA',
                     '/MUONALIGN/Onl/TGC/SIDEC']

        assert _run_number and _lb_number, f'Run or LB number is undefined ({_run_number}, {_lb_number})'

        from TrigCommon.AthHLT import get_sor_params
        sor = get_sor_params(_run_number)
        timestamp = sor['SORTime'] // int(1e9)

        for i,f in enumerate(svcMgr.IOVDbSvc.Folders):
            if any(name in f for name in ignore):
                continue
            if any(name in f for name in timebased):
                svcMgr.IOVDbSvc.Folders[i] += f'<forceTimestamp>{timestamp:d}</forceTimestamp>'
            else:
                svcMgr.IOVDbSvc.Folders[i] += f'<forceRunNumber>{_run_number:d}</forceRunNumber> <forceLumiblockNumber>{_lb_number:d}</forceLumiblockNumber>'


###############################################################
# Algorithm modifiers
###############################################################

class rewriteLVL1(_modifier):
    """
    Write LVL1 results to ByteStream output, usually used together with rerunLVL1
    """
    # Example:
    # athenaHLT -c "setMenu='PhysicsP1_pp_run3_v1';rerunLVL1=True;rewriteLVL1=True;" --filesInput=input.data TriggerJobOpts/runHLT_standalone.py

    def preSetup(self, flags):
        from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
        from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamEncoderCfg
        flags1=flags.clone()
        flags1.lock()
        CAtoGlobalWrapper(L1TriggerByteStreamEncoderCfg, flags1)

    def postSetup(self, flags):
        if not flags.Output.doWriteBS:
            log.warning('rewriteLVL1 is True but flags.Output.doWriteBS is False')
        if not flags.Trigger.writeBS:
            log.warning('rewriteLVL1 is True but flags.Trigger.writeBS is False')

        if flags.Trigger.Online.isPartition:
            # online
            from AthenaCommon.AppMgr import ServiceMgr as svcMgr
            svcMgr.HltEventLoopMgr.RewriteLVL1 = True
            if flags.Trigger.enableL1MuonPhase1 or flags.Trigger.enableL1CaloPhase1:
                svcMgr.HltEventLoopMgr.L1TriggerResultRHKey = 'L1TriggerResult'
            if flags.Trigger.enableL1CaloLegacy or not flags.Trigger.enableL1MuonPhase1:
                svcMgr.HltEventLoopMgr.RoIBResultRHKey = 'RoIBResult'
        else:
            # offline
            from AthenaCommon.AlgSequence import AthSequencer
            from AthenaCommon.CFElements import findAlgorithm
            seq = AthSequencer('AthOutSeq')
            streamBS = findAlgorithm(seq, 'BSOutputStreamAlg')
            if flags.Trigger.enableL1MuonPhase1 or flags.Trigger.enableL1CaloPhase1:
                streamBS.ExtraInputs += [ ('xAOD::TrigCompositeContainer', 'StoreGateSvc+L1TriggerResult') ]
                streamBS.ItemList += [ 'xAOD::TrigCompositeContainer#L1TriggerResult' ]
            if flags.Trigger.enableL1CaloLegacy or not flags.Trigger.enableL1MuonPhase1:
                streamBS.ExtraInputs += [ ('ROIB::RoIBResult', 'StoreGateSvc+RoIBResult') ]
                streamBS.ItemList += [ 'ROIB::RoIBResult#RoIBResult' ]


class DisableMdtT0Fit(_modifier):
    """
    Disable MDT T0 re-fit and use constants from COOL instead
    """
    def preSetup(self, flags):
        if flags.Trigger.doMuon:
            from MuonRecExample.MuonRecFlags import muonRecFlags
            muonRecFlags.doSegmentT0Fit.set_Value_and_Lock(False)
            flags.Muon.doSegmentT0Fit=False

###############################################################
# Monitoring and misc.
###############################################################


class doCosmics(_modifier):
    """
    set beamType flag to cosmics data taking
    """
    def preSetup(self, flags):
       from AthenaCommon.BeamFlags import jobproperties
       jobproperties.Beam.beamType.set_Value_and_Lock('cosmics')
       from AthenaConfiguration.Enums import BeamType
       flags.Beam.Type = BeamType.Cosmics


class enableALFAMon(_modifier):
    """
    turn on ALFA monitoring in the HLT
    """
    def postSetup(self, flags):
        from TrigOnlineMonitor.TrigOnlineMonitorConfig import TrigALFAROBMonitor
        from AthenaCommon.AlgSequence import AlgSequence
        topSequence = AlgSequence()
        topSequence += TrigALFAROBMonitor(flags)

class fpeAuditor(_modifier):
    """
    Turn on FPEAuditor
    """
    def postSetup(self, flags):
        import os
        platform = os.environ.get(os.environ.get('AtlasProject','')+'_PLATFORM','')
        if 'x86_64' not in platform:
            log.info('The fpeAuditor Modifier is ignored because FPEAuditor doesn\'t support the platform "%s". It only supports x86_64', platform)
            return
        from AthenaCommon import CfgMgr
        theApp.AuditAlgorithms = True
        theApp.AuditServices = True
        theApp.AuditTools = True
        svcMgr.AuditorSvc += CfgMgr.FPEAuditor()
        svcMgr.AuditorSvc.FPEAuditor.NStacktracesOnFPE=1

class enableSchedulerMon(_modifier):
    """
    Enable SchedulerMonSvc
    """
    def preSetup(self, flags):
        if not flags.Trigger.Online.isPartition:
            log.debug('SchedulerMonSvc currently only works with athenaHLT / online partition. Skipping setup.')
            return

        from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
        from TrigSteerMonitor.TrigSteerMonitorConfig import SchedulerMonSvcCfg
        flags1=flags.clone()
        flags1.lock()
        CAtoGlobalWrapper(SchedulerMonSvcCfg, flags1)
    
    def postSetup(self, flags):
        if flags.Trigger.Online.isPartition:
            from AthenaCommon.AppMgr import ServiceMgr as svcMgr
            svcMgr.HltEventLoopMgr.MonitorScheduler = True

class enableCountAlgoMiss(_modifier):
    """
    Enable monitoring of non-reentrant algorithms that scheduler is waiting for
    """
    def postSetup(self, flags):
        from AthenaCommon.AppMgr import ServiceMgr as svcMgr
        svcMgr.AlgResourcePool.CountAlgorithmInstanceMisses=True

class doValidation(_modifier):
    """
    Enable validation mode (e.g. extra histograms)
    """
    def preSetup(self, flags):
        flags.Trigger.doValidationMonitoring = True

class useDynamicAlignFolders(_modifier):
    """
    enable the new (2016-) alignment scheme
    """
    def preSetup(self, flags):
        from AtlasGeoModel.InDetGMJobProperties import InDetGeometryFlags
        InDetGeometryFlags.useDynamicAlignFolders.set_Value_and_Lock(True)


class doRuntimeNaviVal(_modifier):
    """
    Checks the validity of each Decision Object produced by a HypoAlg, including all of its
    parents all the way back to the HLT Seeding. Potentially CPU expensive.
    """
    def preSetup(self, flags):
        log.info("Enabling Runtime Trigger Navigation Validation")
        flags.Trigger.doRuntimeNaviVal = True
