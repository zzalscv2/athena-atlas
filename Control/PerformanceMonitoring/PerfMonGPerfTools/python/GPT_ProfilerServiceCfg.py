"""GPerfTools profiler config

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def GPT_ProfilerServiceCfg(flags, **kwargs):
    """Configure GPerfTools CPU profiler"""

    ProfilerService = CompFactory.GPT.ProfilerService
    kwargs.setdefault("ProfileFileName", "gpt-execute.profile")
    kwargs.setdefault("InitEvent", 1)
    
    acc = ComponentAccumulator()
    acc.addService(ProfilerService(**kwargs), create=True)
    acc.setAppProperty("AuditAlgorithms", True)
    acc.setAppProperty("AuditTools", True)
    acc.setAppProperty("AuditServices", True)
    return acc


#Brief example of running profiler
if __name__ == "__main__":
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthExHelloWorld.HelloWorldConfig import HelloWorldCfg
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    ConfigFlags.Exec.MaxEvents=10
    cfg=MainServicesCfg(ConfigFlags)
    cfg.merge(HelloWorldCfg())
    cfg.merge(GPT_ProfilerServiceCfg(ConfigFlags))
    cfg.run()
