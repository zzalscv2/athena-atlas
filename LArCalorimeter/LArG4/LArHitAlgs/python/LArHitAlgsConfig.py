"""Define functions for LAr Digitization with ComponentAccumulator

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
# utilities
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from SGComps.AddressRemappingConfig import InputRenameCfg


def LArHitFilterCfg(flags, **kwargs):
    """ Return ComponentAccumulator with LArHitFilter """
    acc = ComponentAccumulator()
    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg
    acc.merge(DetDescrCnvSvcCfg(flags))  # we do not need the whole geometry, identifiers are enough
    acc.merge(InputRenameCfg("LArHitContainer","LArHitEMB","LArHitEMBOLD"))
    acc.merge(InputRenameCfg("LArHitContainer","LArHitEMEC","LArHitEMECOLD"))
    acc.merge(InputRenameCfg("LArHitContainer","LArHitHEC","LArHitHECOLD"))
    acc.merge(InputRenameCfg("LArHitContainer","LArHitFCAL","LArHitFCALOLD"))
    acc.addEventAlgo(CompFactory.LArHitFilter("LArHitFilter"))
    return acc
