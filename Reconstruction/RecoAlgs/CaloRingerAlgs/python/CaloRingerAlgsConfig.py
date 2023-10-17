# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import GeV
from ROOT import CaloCell_ID


def CaloRingsAsymElectronBuilderCfg(flags, name="CaloRingsAsymElectronBuilder", **kwargs):
    NRings = [8, 64, 8, 8, 4, 4, 4]
    kwargs.setdefault(
        'EtaWidth', [0.025, 0.003125, 0.025, 0.05, 0.1, 0.1, 0.2])
    kwargs.setdefault('PhiWidth', [0.098174770424681, 0.098174770424681,
                                   0.024543692606170, 0.024543692606170,
                                   0.098174770424681, 0.098174770424681,
                                   0.098174770424681])
    kwargs.setdefault('NRings', [(rings-1)*4+1 for rings in NRings])
    kwargs.setdefault('CellMaxDEtaDist', .2)
    kwargs.setdefault('CellMaxDPhiDist', .2)
    kwargs.setdefault('Layers', [CaloCell_ID.PreSamplerB, CaloCell_ID.PreSamplerE,
                                 CaloCell_ID.EMB1,        CaloCell_ID.EME1,
                                 CaloCell_ID.EMB2,        CaloCell_ID.EME2,
                                 CaloCell_ID.EMB3,        CaloCell_ID.EME3,
                                 CaloCell_ID.HEC0,        CaloCell_ID.TileBar0,
                                 CaloCell_ID.TileGap3, CaloCell_ID.TileExt0,
                                 CaloCell_ID.HEC1,        CaloCell_ID.HEC2,
                                 CaloCell_ID.TileBar1, CaloCell_ID.TileGap1,
                                 CaloCell_ID.TileExt1,
                                 CaloCell_ID.HEC3,        CaloCell_ID.TileBar2,
                                 CaloCell_ID.TileGap2, CaloCell_ID.TileExt2])
    kwargs.setdefault('RingSetNLayers', [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault('useShowerShapeBarycenter', flags.CaloRinger.useShowerShapeBarycenter)
    kwargs.setdefault('CellsContainerName', flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault('CaloRingsContainerName', 'ElectronCaloAsymRings')
    kwargs.setdefault('RingSetContainerName', 'ElectronAsymRingSets')
    kwargs.setdefault('MinPartEnergy',flags.CaloRinger.minElectronEnergy*GeV)
    kwargs.setdefault('doTransverseEnergy',flags.CaloRinger.doTransverseEnergy)
    
    tool = CompFactory.Ringer.CaloAsymRingsBuilder(name, **kwargs)
    return tool


def CaloRingsAsymPhotonBuilderCfg(flags, name="CaloRingsAsymPhotonBuilder", **kwargs):
    NRings = [8, 64, 8, 8, 4, 4, 4]
    kwargs.setdefault(
        'EtaWidth', [0.025, 0.003125, 0.025, 0.05, 0.1, 0.1, 0.2])
    kwargs.setdefault('PhiWidth', [0.098174770424681, 0.098174770424681,
                                   0.024543692606170, 0.024543692606170,
                                   0.098174770424681, 0.098174770424681,
                                   0.098174770424681])
    kwargs.setdefault('NRings', [(rings-1)*4+1 for rings in NRings])
    kwargs.setdefault('CellMaxDEtaDist', .2)
    kwargs.setdefault('CellMaxDPhiDist', .2)
    kwargs.setdefault('Layers', [CaloCell_ID.PreSamplerB, CaloCell_ID.PreSamplerE,
                                 CaloCell_ID.EMB1,        CaloCell_ID.EME1,
                                 CaloCell_ID.EMB2,        CaloCell_ID.EME2,
                                 CaloCell_ID.EMB3,        CaloCell_ID.EME3,
                                 CaloCell_ID.HEC0,        CaloCell_ID.TileBar0,
                                 CaloCell_ID.TileGap3, CaloCell_ID.TileExt0,
                                 CaloCell_ID.HEC1,        CaloCell_ID.HEC2,
                                 CaloCell_ID.TileBar1, CaloCell_ID.TileGap1,
                                 CaloCell_ID.TileExt1,
                                 CaloCell_ID.HEC3,        CaloCell_ID.TileBar2,
                                 CaloCell_ID.TileGap2, CaloCell_ID.TileExt2])
    kwargs.setdefault('RingSetNLayers', [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault('useShowerShapeBarycenter', flags.CaloRinger.useShowerShapeBarycenter)
    kwargs.setdefault('CellsContainerName', flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault('CaloRingsContainerName', 'PhotonCaloAsymRings')
    kwargs.setdefault('RingSetContainerName', 'PhotonAsymRingSets')
    kwargs.setdefault('MinPartEnergy', flags.CaloRinger.minPhotonEnergy*GeV)
    kwargs.setdefault('doTransverseEnergy',flags.CaloRinger.doTransverseEnergy)
    
    tool = CompFactory.Ringer.CaloAsymRingsBuilder(name, **kwargs)
    return tool


def CaloRingsElectronBuilderCfg(flags, name="CaloRingsElectronBuilder", **kwargs):
    kwargs.setdefault(
        'EtaWidth', [0.025, 0.003125, 0.025, 0.05, 0.1, 0.1, 0.2])
    kwargs.setdefault('PhiWidth', [0.098174770424681, 0.098174770424681, 0.024543692606170,
                      0.024543692606170, 0.098174770424681, 0.098174770424681, 0.098174770424681])
    kwargs.setdefault('NRings', [8, 64, 8, 8, 4, 4, 4])
    kwargs.setdefault('CellMaxDEtaDist', .2)
    kwargs.setdefault('CellMaxDPhiDist', .2)
    kwargs.setdefault('Layers', [CaloCell_ID.PreSamplerB, CaloCell_ID.PreSamplerE,
                                 CaloCell_ID.EMB1,        CaloCell_ID.EME1,
                                 CaloCell_ID.EMB2,        CaloCell_ID.EME2,
                                 CaloCell_ID.EMB3,        CaloCell_ID.EME3,
                                 CaloCell_ID.HEC0,        CaloCell_ID.TileBar0,
                                 CaloCell_ID.TileGap3, CaloCell_ID.TileExt0,
                                 CaloCell_ID.HEC1,        CaloCell_ID.HEC2,
                                 CaloCell_ID.TileBar1, CaloCell_ID.TileGap1,
                                 CaloCell_ID.TileExt1,
                                 CaloCell_ID.HEC3,        CaloCell_ID.TileBar2,
                                 CaloCell_ID.TileGap2, CaloCell_ID.TileExt2])
    kwargs.setdefault('RingSetNLayers', [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault('useShowerShapeBarycenter', flags.CaloRinger.useShowerShapeBarycenter)
    kwargs.setdefault('CellsContainerName', flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault('CaloRingsContainerName', 'ElectronCaloRings')
    kwargs.setdefault('RingSetContainerName', 'ElectronRingSets')
    kwargs.setdefault('MinPartEnergy', flags.CaloRinger.minElectronEnergy*GeV)
    kwargs.setdefault('doTransverseEnergy',flags.CaloRinger.doTransverseEnergy)
    
    tool = CompFactory.Ringer.CaloRingsBuilder(name, **kwargs)
    return tool


def CaloRingsPhotonBuilderCfg(flags, name="CaloRingsPhotonBuilder", **kwargs):
    kwargs.setdefault(
        'EtaWidth', [0.025, 0.003125, 0.025, 0.05, 0.1, 0.1, 0.2])
    kwargs.setdefault('PhiWidth', [0.098174770424681, 0.098174770424681, 0.024543692606170,
                      0.024543692606170, 0.098174770424681, 0.098174770424681, 0.098174770424681])
    kwargs.setdefault('NRings', [8, 64, 8, 8, 4, 4, 4])
    kwargs.setdefault('CellMaxDEtaDist', .2)
    kwargs.setdefault('CellMaxDPhiDist', .2)
    kwargs.setdefault('Layers', [CaloCell_ID.PreSamplerB, CaloCell_ID.PreSamplerE,
                                 CaloCell_ID.EMB1,        CaloCell_ID.EME1,
                                 CaloCell_ID.EMB2,        CaloCell_ID.EME2,
                                 CaloCell_ID.EMB3,        CaloCell_ID.EME3,
                                 CaloCell_ID.HEC0,        CaloCell_ID.TileBar0,
                                 CaloCell_ID.TileGap3, CaloCell_ID.TileExt0,
                                 CaloCell_ID.HEC1,        CaloCell_ID.HEC2,
                                 CaloCell_ID.TileBar1, CaloCell_ID.TileGap1,
                                 CaloCell_ID.TileExt1,
                                 CaloCell_ID.HEC3,        CaloCell_ID.TileBar2,
                                 CaloCell_ID.TileGap2, CaloCell_ID.TileExt2])
    kwargs.setdefault('RingSetNLayers', [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault('useShowerShapeBarycenter', flags.CaloRinger.useShowerShapeBarycenter)
    kwargs.setdefault('CellsContainerName', flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault('CaloRingsContainerName', 'PhotonCaloRings')
    kwargs.setdefault('RingSetContainerName', 'PhotonRingSets')
    kwargs.setdefault('MinPartEnergy', flags.CaloRinger.minPhotonEnergy*GeV)
    kwargs.setdefault('doTransverseEnergy',flags.CaloRinger.doTransverseEnergy)
    
    tool = CompFactory.Ringer.CaloRingsBuilder(name, **kwargs)
    return tool


def CaloRingerElectronsInputReaderCfg(flags, name="CaloRingerElectronsReader", **kwargs):

    if 'Asym' in name:
        builderTool = CaloRingsAsymElectronBuilderCfg(flags)
    else:
        builderTool = CaloRingsElectronBuilderCfg(flags)

    kwargs.setdefault('crBuilder', builderTool)
    kwargs.setdefault('inputKey', flags.Egamma.Keys.Output.Electrons)
    kwargs.setdefault('builderAvailable', True)
    inputReaderTool = CompFactory.Ringer.CaloRingerElectronsReader(
        name, **kwargs)

    return inputReaderTool, builderTool


def CaloRingerPhotonsInputReaderCfg(flags,
                                    name="CaloRingerPhotonsReader",
                                    **kwargs):
    if 'Asym' in name:
        builderTool = CaloRingsAsymPhotonBuilderCfg(flags)
    else:
        builderTool = CaloRingsPhotonBuilderCfg(flags)

    kwargs.setdefault('crBuilder', builderTool)
    kwargs.setdefault('inputKey', flags.Egamma.Keys.Output.Photons)
    kwargs.setdefault('builderAvailable', True)
    inputReaderTool = CompFactory.Ringer.CaloRingerPhotonsReader(
        name, **kwargs)

    return inputReaderTool, builderTool


def caloRingerOutputList(flags,name="CaloRingerOutputList"):

    toOuput = []
    if flags.CaloRinger.buildElectronRings:
        toOuput +=[ 'xAOD::RingSetContainer#ElectronRingSets',
                    'xAOD::RingSetAuxContainer#ElectronRingSetsAux.',
                    'xAOD::CaloRingsContainer#ElectronCaloRings',
                    'xAOD::CaloRingsAuxContainer#ElectronCaloRingsAux.',
                    ]
    if flags.CaloRinger.buildElectronAsymRings:
        toOuput +=[ 'xAOD::RingSetContainer#ElectronAsymRingSets',
                    'xAOD::RingSetAuxContainer#ElectronAsymRingSetsAux.',
                    'xAOD::CaloRingsContainer#ElectronCaloAsymRings',
                    'xAOD::CaloRingsAuxContainer#ElectronCaloAsymRingsAux.',
                    ]

    if flags.CaloRinger.buildPhotonRings:
        toOuput += [
                    'xAOD::RingSetContainer#PhotonRingSets',
                    'xAOD::RingSetAuxContainer#PhotonRingSetsAux.',
                    'xAOD::CaloRingsContainer#PhotonCaloRings',
                    'xAOD::CaloRingsAuxContainer#PhotonCaloRingsAux.'
                    ]   
    if flags.CaloRinger.buildPhotonAsymRings:
        toOuput +=[ 
                    'xAOD::RingSetContainer#PhotonAsymRingSets',
                    'xAOD::RingSetAuxContainer#PhotonAsymRingSetsAux.',
                    'xAOD::CaloRingsContainer#PhotonCaloAsymRings',
                    'xAOD::CaloRingsAuxContainer#PhotonCaloAsymRingsAux.',
                    ]
    if not toOuput: 
        toOuput +=[ 
                    'xAOD::RingSetContainer#PhotonRingSets',
                    'xAOD::RingSetAuxContainer#PhotonRingSetsAux.',
                    'xAOD::CaloRingsContainer#PhotonCaloRings',
                    'xAOD::CaloRingsAuxContainer#PhotonCaloRingsAux.'
                    ]

    return toOuput

def CaloRingerOutputCfg(flags, name="CaloRingerOutput"):
    """Configure the CaloRingerOutput."""
    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD

    toOuput = caloRingerOutputList(flags)
    acc = ComponentAccumulator()

    if flags.Output.doWriteAOD:
        acc.merge(addToAOD(flags, toOuput))
    if flags.Output.doWriteESD:
        acc.merge(addToESD(flags, toOuput))
        
    return acc


def CaloRingerAlgsCfg(flags, name="CaloRingerAlgorithm", **kwargs):
    """Configure the CaloRingerAlgs."""
    acc = ComponentAccumulator()
    readerTools = []

    if flags.CaloRinger.buildElectronRings:
        electronInputReaderTool, electronBuilderTool = CaloRingerElectronsInputReaderCfg(flags)
        readerTools.append(electronInputReaderTool)
        acc.addPublicTool(electronBuilderTool)
        acc.addPublicTool(electronInputReaderTool)
    
    if flags.CaloRinger.buildElectronAsymRings:
        electronInputReaderTool, electronBuilderTool = CaloRingerElectronsInputReaderCfg(flags, name='CaloAsymRingerElectronsReader')
        readerTools.append(electronInputReaderTool)
        acc.addPublicTool(electronBuilderTool)
        acc.addPublicTool(electronInputReaderTool)

    if flags.CaloRinger.buildPhotonRings:
        photonInputReaderTool, photonBuilderTool = CaloRingerPhotonsInputReaderCfg(flags)
        readerTools.append(photonInputReaderTool)
        acc.addPublicTool(photonBuilderTool)
        acc.addPublicTool(photonInputReaderTool)
    
    if flags.CaloRinger.buildPhotonAsymRings:
        photonInputReaderTool, photonBuilderTool = CaloRingerPhotonsInputReaderCfg(flags, name='CaloAsymRingerPhotonReader')
        readerTools.append(photonInputReaderTool)
        acc.addPublicTool(photonBuilderTool)
        acc.addPublicTool(photonInputReaderTool)

    kwargs.setdefault('inputReaderTools', readerTools)

    CaloRingerAlgorithm = CompFactory.Ringer.CaloRingerAlgorithm(name, **kwargs)
    acc.addEventAlgo(CaloRingerAlgorithm)

    return acc


def CaloRingerSteeringCfg(flags,
                          name="CaloRingerSteering"):
    """Configure the full calo Ringer"""

    acc = ComponentAccumulator()
    acc.merge(CaloRingerAlgsCfg(flags))
    acc.merge(CaloRingerOutputCfg(flags))
    return acc


if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    ConfigFlags.Input.Files = defaultTestFiles.RDO_RUN2
    ConfigFlags.Output.doWriteAOD = True
    ConfigFlags.Output.ESDFileName = 'testing.ESD.root'
    ConfigFlags.lock()
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(ConfigFlags)
    cfg.merge(CaloRingerAlgsCfg(ConfigFlags))
