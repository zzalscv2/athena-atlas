#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
#
# File: D3PDMakerTest/test/D3PDMakerTest1.py
# Author: snyder@bnl.gov
# Date: Dec, 2023, from old config
# Purpose: Test D3PD making.

from D3PDMakerConfig.D3PDMakerFlags import configFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


configFlags.Exec.MaxEvents = 20
configFlags.fillFromArgs()
configFlags.lock()


###########################################################################



from D3PDMakerCoreComps.MakerAlgConfig import MakerAlgConfig
from D3PDMakerCoreComps.D3PDObject import \
     make_SGDataVector_D3PDObject, make_SG_D3PDObject, make_Void_D3PDObject, \
     D3PDObject
from D3PDMakerCoreComps.SimpleAssociation import SimpleAssociation
from D3PDMakerCoreComps.ContainedVectorMultiAssociation \
     import ContainedVectorMultiAssociation
from D3PDMakerCoreComps.ContainedMultiAssociation \
     import ContainedMultiAssociation
from EventCommonD3PDMaker.DRIndexMultiAssociation import DRIndexMultiAssociation
from EventCommonD3PDMaker.DRIndexAssociation      import DRIndexAssociation


D3PD = CompFactory.D3PD
D3PDTest = CompFactory.D3PDTest


Obj1D3PDObject = \
           make_SGDataVector_D3PDObject ('D3PDTest::Obj1Container',
                                         'obj1container',
                                         'o1_')
Obj1D3PDObject.defineBlock (0, 'Obj1',  D3PDTest.Obj1FillerTool)
Obj1D3PDObject.defineBlock (0, 'Obj1a', D3PDTest.Obj12FillerTool)
Obj1D3PDObject.defineBlock (0, 'Def2',  D3PDTest.DefaultFillerTool2)


#####################


Obj2aAssoc = SimpleAssociation (Obj1D3PDObject,
                                D3PDTest.Obj1Obj2AssociationTool,
                                prefix = 'o2a_')
Obj2aAssoc.defineBlock (0, 'Obj2Assoc', D3PDTest.Obj2FillerTool)

Obj2bAssoc = ContainedVectorMultiAssociation \
             (Obj1D3PDObject,
              D3PDTest.Obj1Obj2MultiAssociationTool,
              prefix = 'o2b_')
Obj2bAssoc.defineBlock (0, 'Obj2', D3PDTest.Obj2FillerTool)
Obj2bAssoc.defineBlock (0, 'Obja2', D3PDTest.Obj12FillerTool)

Obj2cAssoc = SimpleAssociation (Obj1D3PDObject,
                                D3PDTest.ToObj2AssociationTool,
                                prefix = 'o2c_')
Obj2cAssoc.defineBlock (0, 'Obj2Assoc_c', D3PDTest.Obj2FillerTool)

Obj2dAssoc = SimpleAssociation (Obj2cAssoc,
                                D3PDTest.ToObj2AssociationTool,
                                prefix = 'o2d_',
                                blockname = 'Assoc_d')
Obj2dAssoc.defineBlock (0, 'Obj2Assoc_d', D3PDTest.Obj2FillerTool)

Obj2eAssoc = ContainedMultiAssociation (Obj2cAssoc,
                                D3PDTest.ToObj2MultiAssociationTool,
                                prefix = 'o2e_',
                                blockname = 'Assoc_e')
Obj2eAssoc.defineBlock (0, 'Obj2Assoc_e', D3PDTest.Obj2FillerTool)


Obj1D3PDObject2 = \
           make_SGDataVector_D3PDObject ('D3PDTest::Obj1Container',
                                         'obj1',
                                         'o12_')
Obj1D3PDObject2Assoc1 = ContainedMultiAssociation \
                        (Obj1D3PDObject2,
                         D3PDTest.Obj1Obj2MultiAssociationTool,
                         blockname = 'Obj1D3PDObject2Assoc1',
                         prefix = 'o2a_')
Obj1D3PDObject2Assoc1.defineBlock (0, 'Obj2a', D3PDTest.Obj2FillerTool)
Obj1D3PDObject2Assoc2 = ContainedMultiAssociation \
                        (Obj1D3PDObject2,
                         D3PDTest.Obj1Obj2MultiAssociationTool,
                         Which = 1,
                         blockname = 'Obj1D3PDObject2Assoc2',
                         prefix = 'o2b_')
Obj1D3PDObject2Assoc2.defineBlock (0, 'Obj2b', D3PDTest.Obj2FillerTool)

Obj1D3PDObject2Assoc1_c = ContainedMultiAssociation \
                          (Obj1D3PDObject2,
                           D3PDTest.ToObj2MultiAssociationTool,
                           blockname = 'Obj1D3PDObject2Assoc1_c',
                           prefix = 'o2c_')
Obj1D3PDObject2Assoc1_c.defineBlock (0, 'Obj2c', D3PDTest.Obj2FillerTool)


#####################


Obj3D3PDObject = \
       make_SGDataVector_D3PDObject ('DataVector<INavigable4Momentum>',
                                     'obj3container',
                                     'o3_')
Obj3D3PDObject.defineBlock (0, 'Obj3Kin',
                            D3PD.FourMomFillerTool)
Obj3HLV = SimpleAssociation (Obj3D3PDObject,
                             D3PDTest.IN4MHLVAssociationTool,
                             prefix = 'hlv_',
                             blockname = 'Assoc_hlv')
Obj3HLV.defineBlock (0, 'Obj3Kin2',
                     D3PD.FourMomFillerTool)

DRIndexMultiAssociation (Obj3D3PDObject,
                         'DataVector<INavigable4Momentum>',
                         'obj3container',
                         1.0,
                         target = 'o3_')
DRIndexAssociation (Obj3D3PDObject,
                    'DataVector<INavigable4Momentum>',
                    'obj3container',
                    1.0,
                    prefix = 'a1_',
                    target = 'o3_')



#####################


def make_obj4 (name, prefix, object_name):
    getter = D3PDTest.Obj4GetterTool (name + '_Getter',
                                      SGKey = 'obj4container')
    return D3PD.VectorFillerTool (name,
                                  Prefix = prefix,
                                  Getter = getter,
                                  ObjectName = object_name)

Obj4D3PDObject = D3PDObject (make_obj4, 'o4_')
Obj4D3PDObject.defineBlock (0, 'Obj4', D3PDTest.Obj4FillerTool)


#####################


Obj5D3PDObject = \
       make_SGDataVector_D3PDObject ('DataVector<D3PDTest::Obj5>',
                                     'obj5container',
                                     'o5_')

Obj5D3PDObject.defineBlock (0, 'Obj5', D3PDTest.Obj5FillerTool)
Obj5D3PDObject.defineBlock (0, 'Obj5aux', D3PD.AuxDataFillerTool,
                            Vars = ['anInt',
                                    'aFloat',
                                    's = aString #This is a string.',
                                    'aFourvec',
                                    'dummy < int:-999'])


###########################################################################


def D3PDTest1Cfg (flags):
    acc = ComponentAccumulator()

    acc.addEventAlgo (CompFactory.D3PDTest.FillerAlg())
    acc.addEventAlgo (CompFactory.D3PDTest.HitsFillerAlg())
    acc.addService (CompFactory.D3PD.RootD3PDSvc (MasterTree = '',
                                                  IndexMajor ='',
                                                  IndexMinor = ''))
    alg = MakerAlgConfig (flags, acc, 'test1', 'test1.root',
                          ExistDataHeader = False)

    alg += Obj1D3PDObject(99)
    alg += Obj1D3PDObject2(99)
    alg += Obj1D3PDObject(99,
                          prefix = 'o1filt_',
                          getterFilter = 'obj1sel')
    alg += Obj3D3PDObject (99)
    alg += Obj4D3PDObject(99)
    alg += Obj5D3PDObject(99)

    DefD3PDObject = make_Void_D3PDObject ('def_')
    DefD3PDObject.defineBlock (0, 'Def', D3PDTest.DefaultFillerTool)
    alg += DefD3PDObject(99)

    # Testing SimHitsCollection.
    from MuonD3PDMaker.MDTSimHitD3PDObject import MDTSimHitD3PDObject
    alg += MDTSimHitD3PDObject (99, sgkey = 'MDTSimHits')

    from MuonD3PDMaker.TrackRecordD3PDObject import TrackRecordD3PDObject
    alg += TrackRecordD3PDObject (99, sgkey = 'TrackRecord',
                                  exclude = ['TruthHits'])

    acc.addEventAlgo (alg.alg)

    return acc


cfg = MainServicesCfg (configFlags)
cfg.merge (D3PDTest1Cfg (configFlags))

sc = cfg.run (configFlags.Exec.MaxEvents)

if not sc.isFailure():
    import os
    os.system ('python -m D3PDMakerTest.dumptuple1 test1.root')

import sys
sys.exit (sc.isFailure())
