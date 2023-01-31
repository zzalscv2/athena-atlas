# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ConfigurationError

def ExecCondAlgsAtPreFork(flags, cfg):
    """ Execute the entire conditions sequence during the PreFork step.
    This allows saving some memory in AthenaMP jobs, which is particularly
    useful in the derivation production.
    """
    log = logging.getLogger('ExecCondAlgsAtPreFork')
    try:
        cfg.getService('AthMpEvtLoopMgr').ExecAtPreFork=['AthCondSeq']
        log.info('AthCondSeq will be executed during PreFork')
    except ConfigurationError: # in this context this might not be an error
        log.info('AthCondSeq will NOT be executed during PreFork')
