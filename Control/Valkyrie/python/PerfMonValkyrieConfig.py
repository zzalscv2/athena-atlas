"""Callgrind/Valkyrie profiler config

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ValkyrieProfilerServiceCfg(flags, **kwargs):
    """Configure Valkyrie/Valgrind profiler"""

    kwargs.setdefault("ProfiledAlgs", flags.PerfMon.Valgrind.ProfiledAlgs)
    from AthenaCommon.Constants import VERBOSE
    kwargs.setdefault("OutputLevel", VERBOSE)

    ValgrindSvc = CompFactory.ValgrindSvc
    acc = ComponentAccumulator()
    acc.addService(ValgrindSvc(**kwargs), create=True)
    acc.addService(CompFactory.AuditorSvc(), create=True)
    acc.setAppProperty("AuditAlgorithms", True)
    acc.setAppProperty("AuditTools", True)
    acc.setAppProperty("AuditServices", True)
    return acc
