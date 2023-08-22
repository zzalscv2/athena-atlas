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

class forceConditions(_modifier):
    """
    Force all conditions (except prescales) to match run from input file
    """
    def postSetup(self, flags):
        from TriggerJobOpts import PostExec
        assert _run_number and _lb_number, f'Run or LB number is undefined ({_run_number}, {_lb_number})'
        PostExec.forceConditions(_run_number, _lb_number, svcMgr.IOVDbSvc)


###############################################################
# Algorithm modifiers
###############################################################

class rewriteLVL1(_modifier):
    """
    Write LVL1 results to ByteStream output, usually used together with rerunLVL1
    """

    def preSetup(self, flags):
        log.warning('The rewriteLVL1 modifier is deprecated. LVL1 result writing is enabled by default '
                    'if flags.Trigger.doLVL1 (doL1Sim) is True.')


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


class doRuntimeNaviVal(_modifier):
    """
    Checks the validity of each Decision Object produced by a HypoAlg, including all of its
    parents all the way back to the HLT Seeding. Potentially CPU expensive.
    """
    def preSetup(self, flags):
        log.info("Enabling Runtime Trigger Navigation Validation")
        flags.Trigger.doRuntimeNaviVal = True
