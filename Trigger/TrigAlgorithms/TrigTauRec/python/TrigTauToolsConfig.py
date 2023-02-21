# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def tauAxisCfg(flags, name=''):
    acc = ComponentAccumulator()
    TauAxisSetter = CompFactory.TauAxisSetter(name=name, VertexCorrection=False)
    acc.setPrivateTools(TauAxisSetter)
    return acc

def tauClusterFinderCfg(flags, name=''):
    acc = ComponentAccumulator()
    TauClusterFinder = CompFactory.TauClusterFinder(name=name, UseOriginalCluster = False)
    acc.setPrivateTools(TauClusterFinder)
    return acc

def tauVertexedClusterDecoratorCfg(flags,name=''):
    acc = ComponentAccumulator()
    TauVertexedClusterDecorator = CompFactory.TauVertexedClusterDecorator(name=name,SeedJet = "")
    acc.setPrivateTools(TauVertexedClusterDecorator)
    return acc

def tauMvaTESVariableDecoratorCfg(flags,name=''):
    acc = ComponentAccumulator() 
    MvaTESVariableDecorator = CompFactory.MvaTESVariableDecorator(name=name,Key_vertexInputContainer='',
                                                     EventShapeKey='',
                                                     VertexCorrection = False)
    acc.addPublicTool(MvaTESVariableDecorator, primary=True)
    return acc

def tauMvaTESEvaluatorCfg(flags,name=''):
    acc = ComponentAccumulator()
    MvaTESEvaluator = CompFactory.MvaTESEvaluator(name=name,WeightFileName = flags.Trigger.Offline.Tau.MvaTESConfig)
    acc.addPublicTool(MvaTESEvaluator, primary=True)
    return acc
