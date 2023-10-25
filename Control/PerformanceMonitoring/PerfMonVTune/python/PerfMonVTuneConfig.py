#!/usr/bin/env python
"""VTune profiler config

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def VTuneProfilerServiceCfg(flags, **kwargs):
    """Configure VTune profiler"""
    kwargs.setdefault("ResumeEvent", flags.Concurrency.NumThreads + 1)
    kwargs.setdefault("ProfiledAlgs", flags.PerfMon.VTune.ProfiledAlgs)

    VTuneProfilerService = CompFactory.VTuneProfilerService
    acc = ComponentAccumulator()
    acc.addService(VTuneProfilerService(**kwargs), create=True)
    acc.setAppProperty("AuditAlgorithms", True)
    acc.setAppProperty("AuditTools", True)
    acc.setAppProperty("AuditServices", True)
    return acc
