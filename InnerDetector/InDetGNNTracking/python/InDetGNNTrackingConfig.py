#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def DumpObjectsCfg(
        flags, name="DumpObjects", outfile="Dump_GNN4Itk.root", **kwargs):
    '''
    create algorithm which dumps GNN training information to ROOT file
    '''
    acc = ComponentAccumulator()

    acc.addService(
        CompFactory.THistSvc(
            Output=[f"{name} DATAFILE='{outfile}', OPT='RECREATE'"]
        )
    )

    kwargs.setdefault("NtupleFileName", "/DumpObjects/")
    #kwargs.setdefault("NtupleDirectoryName", "3-0-0")
    kwargs.setdefault("NtupleTreeName", "GNN4ITk")
    kwargs.setdefault("rootFile", True)

    acc.addEventAlgo(CompFactory.InDet.DumpObjects(name, **kwargs))


    return acc
