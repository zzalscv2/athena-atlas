# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MaxCellDecoratorCfg(ConfigFlags,**kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("SGKey_electrons", ConfigFlags.Egamma.Keys.Output.Electrons)
    kwargs.setdefault("SGKey_photons", ConfigFlags.Egamma.Keys.Output.Photons)
    acc.setPrivateTools(CompFactory.DerivationFramework.MaxCellDecorator(**kwargs))
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
    acc.merge(LArOnOffIdMappingCfg(ConfigFlags))
    return acc

def GainDecoratorCfg(ConfigFlags,**kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("SGKey_electrons", ConfigFlags.Egamma.Keys.Output.Electrons)
    kwargs.setdefault("SGKey_photons", ConfigFlags.Egamma.Keys.Output.Photons)
    kwargs.setdefault("name", "GainDecor")
    acc.setPrivateTools(CompFactory.DerivationFramework.GainDecorator(**kwargs))
    return acc

def EgammaCoreCellRecoveryCfg(ConfigFlags,**kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGammaClusterCoreCellRecovery(**kwargs))
    return acc

def CaloFillRectangularClusterCfg(ConfigFlags,**kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("cells_name", ConfigFlags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("fill_cluster", True)
    acc.setPrivateTools(CompFactory.CaloFillRectangularCluster(**kwargs))
    return acc

def ClusterEnergyPerLayerDecoratorCfg(ConfigFlags,**kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("SGKey_electrons", ConfigFlags.Egamma.Keys.Output.Electrons)
    kwargs.setdefault("SGKey_photons", ConfigFlags.Egamma.Keys.Output.Photons)
    kwargs.setdefault("SGKey_caloCells", ConfigFlags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("neta", 5)
    kwargs.setdefault("nphi", 5)
    toolArgs = {}
    toolArgs.update({"eta_size": kwargs["neta"]})
    toolArgs.update({"phi_size": kwargs["nphi"]})
    kwargs.setdefault("CaloFillRectangularClusterTool", acc.popToolsAndMerge(CaloFillRectangularClusterCfg(ConfigFlags,**toolArgs) ) )
    acc.setPrivateTools(CompFactory.DerivationFramework.ClusterEnergyPerLayerDecorator(**kwargs))
    return acc

def CaloClusterThinningCfg(ConfigFlags,**kwargs):
    acc = ComponentAccumulator()
    CaloClusterThinning = CompFactory.DerivationFramework.CaloClusterThinning
    acc.addPublicTool(CaloClusterThinning(**kwargs),primary=True)
    return acc


#### 
# additional utilities to return the list of decorations
# added by the tools

from egammaRec.Factories import getPropertyValue
from egammaRec import egammaKeys

def getGainLayerNames( tool ):
    """getGainLayerNames( tool ) -> return a list of names of the decorations added to the
    egamma tool, given the GainDecorator tool"""
    return [getPropertyValue(tool, "decoration_pattern").format(info=info, layer=layer, gain=gain) \
        for info in ["E", "nCells"] \
        for layer in getPropertyValue(tool, "layers") \
        for gain in getPropertyValue(tool, "gain_names").values() ]

def getGainDecorations(acc, kernel,
    collections=[egammaKeys.outputElectronKey(), egammaKeys.outputPhotonKey()],
    info = ["E", "nCells"] ):
    """getGainDecorations( acc, kernel collections=["Electrons", "Photons"] ) -> 
    Return a list with the 'ExtraContent' to be added to the decorations to save the gain
    information per layer"""

    GainDecoratorTool = None
    for toolStr in (acc.getEventAlgo(kernel).AugmentationTools):
        toolStr  = f'{toolStr}'
        splitStr = toolStr.split('/')
        tool =  acc.getPublicTool(splitStr[1])
        if splitStr[0] == 'DerivationFramework::GainDecorator':
            GainDecoratorTool = tool

    if GainDecoratorTool : 
        return ["{part}.{info}".format(part=part, info=info) for part in collections \
                for info in getGainLayerNames(GainDecoratorTool) ]
    else:
        return ""


def getClusterEnergyPerLayerDecorations( acc, kernel ):
    """getClusterEnergyPerLayerDecorationsLegacy( acc, kernel ) -> return a list of names of the
    decorations added to the egamma object, given the ClusterEnergyPerLayerDecorations
    object (e.g. Photons.E7x11_Lr0, ...)"""
    properties = 'SGKey_photons', 'SGKey_electrons'
    ClusterEnergyPerLayerDecorators = []  
    for toolStr in acc.getEventAlgo(kernel).AugmentationTools:
        toolStr  = f'{toolStr}'
        splitStr = toolStr.split('/')
        tool =  acc.getPublicTool(splitStr[1])
        if splitStr[0] == 'DerivationFramework::ClusterEnergyPerLayerDecorator':
            ClusterEnergyPerLayerDecorators.append( tool )

    decorations = []
    for tool in ClusterEnergyPerLayerDecorators:
        collections = filter(bool, (getPropertyValue(tool, x) for x in properties) )
        for part in collections:
            for layer in getPropertyValue(tool, "layers"):
                decorations.extend(['{part}.E{neta}x{nphi}_Lr{layer}'.format(part=part, 
                    neta=getPropertyValue(tool, 'neta'),
                    nphi=getPropertyValue(tool, 'nphi'),
                    layer=layer)])
    return decorations
