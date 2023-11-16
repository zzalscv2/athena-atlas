#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def AlignmentErrorToolCfg(flags, name="AlignmentErrorTool", **kwargs):
    acc = ComponentAccumulator()

    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc.merge(MuonGeoModelCfg(flags))

    if not (flags.IOVDb.DatabaseInstance == 'COMP200' \
        or 'HLT'  in flags.IOVDb.GlobalTag \
        or flags.Common.isOnline or flags.Muon.MuonTrigger) :
        from MuonConfig.MuonGeometryConfig import MuonAlignmentErrorDbAlgCfg
        acc.merge(MuonAlignmentErrorDbAlgCfg(flags))

    #### Id to FixedLongId tool TODO?
    # from MuonTrackAlignAlgs.IdToFixedLongIdToolConfig import IdToFixedIdToolCfg
    # kwargs.setdefault("idTool", acc.popToolsAndMerge(IdToFixedIdToolCfg(flags)))

    #### Id helper svc
    from MuonConfig.MuonGeometryConfig import MuonIdHelperSvcCfg
    kwargs.setdefault("MuonIdHelperSvc", acc.getPrimaryAndMerge(MuonIdHelperSvcCfg(flags)))

    acc.setPrivateTools(
        CompFactory.MuonAlign.AlignmentErrorTool(name, **kwargs))

    return acc

