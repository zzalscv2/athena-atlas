# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
from AthenaCommon.Logging import logging
_log = logging.getLogger( "TriggerRecoGetter.py" )

def TriggerRecoGetter(flags=None):
    if flags is None:
        from AthenaConfiguration.AllConfigFlags import ConfigFlags
        flags = ConfigFlags

    # setup configuration services
    from TriggerJobOpts.TriggerConfigGetter import TriggerConfigGetter
    cfg = TriggerConfigGetter(flags)  # noqa: F841

    # configure TrigDecisionTool
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    CAtoGlobalWrapper(TrigDecisionToolCfg, flags)

    if 'L1' in flags.Trigger.availableRecoMetadata:
        _log.info("configuring lvl1")
        from TriggerJobOpts.Lvl1ResultBuilderGetter import Lvl1ResultBuilderGetter
        lvl1 = Lvl1ResultBuilderGetter(flags)  # noqa: F841

    if 'HLT' in flags.Trigger.availableRecoMetadata:
        _log.info("configuring hlt")
        from TriggerJobOpts.HLTTriggerResultGetter import HLTTriggerResultGetter
        hlt = HLTTriggerResultGetter(flags)   # noqa: F841
