# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def BPHY_InDetV0FinderToolCfg(flags, derivation,
                              V0ContainerName, KshortContainerName,
                              LambdaContainerName, LambdabarContainerName):
    from InDetConfig.InDetV0FinderConfig import InDetV0FinderToolCfg
    return InDetV0FinderToolCfg(flags,
                                name = derivation + "_InDetV0FinderTool",
                                V0ContainerName=V0ContainerName,
                                KshortContainerName=KshortContainerName,
                                LambdaContainerName=LambdaContainerName,
                                LambdabarContainerName= LambdabarContainerName)
