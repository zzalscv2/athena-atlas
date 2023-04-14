
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
def MuonIdHelperSvc(name="MuonIdHelperSvc",**kwargs):
    from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags
    from MuonIdHelpers.MuonIdHelpersConf import Muon__MuonIdHelperSvc
    kwargs.setdefault("HasCSC", MuonGeometryFlags.hasCSC())
    kwargs.setdefault("HasSTGC", MuonGeometryFlags.hasSTGC())
    kwargs.setdefault("HasMM", MuonGeometryFlags.hasMM())
    kwargs.setdefault("HasMDT", True)
    kwargs.setdefault("HasRPC", True)
    kwargs.setdefault("HasTGC", True)
    return Muon__MuonIdHelperSvc(name,**kwargs)