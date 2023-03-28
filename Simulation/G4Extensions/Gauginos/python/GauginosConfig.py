# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import CfgMgr
def getGauginosPhysicsTool(name="GauginosPhysicsTool", **kwargs):
    from AthenaCommon.SystemOfUnits import GeV,ns # noqa: F401
    from G4AtlasApps.SimFlags import simFlags
    # Example specialConfiguration {'GMSBSlepton': '100.0*GeV', 'GMSBGravitino': '1e-07*GeV', 'GMSBSleptonTime': '0.01*ns'}
    NeutralinoMass = 0*GeV
    if simFlags.specialConfiguration.get_Value().has_key("coannihilationNeutralino"):
        NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("coannihilationNeutralino", "0*GeV"))
        # The Neutralino is stable in this scenario, therefore leaving
        # NeutralinoLifetime at the C++ default value of -1
    elif simFlags.specialConfiguration.get_Value().has_key("GMSBNeutralino"):
        NeutralinoMass = eval(simFlags.specialConfiguration.get_Value().get("GMSBNeutralino", "0*GeV"))
        GMSBTime = eval(simFlags.specialConfiguration.get_Value().get("GMSBLifeTime", "0*GeV"))
        kwargs.setdefault("NeutralinoLifetime",    GMSBTime)

    kwargs.setdefault("NeutralinoStable",      simFlags.specialConfiguration.get_Value().has_key("coannihilationNeutralino"))
    kwargs.setdefault("NeutralinoMass",        NeutralinoMass)
    ##kwargs.setdefault("NeutralinoWidth",       0.0*GeV);
    ##kwargs.setdefault("NeutralinoCharge",      0);
    ##kwargs.setdefault("NeutralinoPDGCode",     1000039);
    ##kwargs.setdefault("NeutralinoShortlived",  False);

    if simFlags.specialConfiguration.get_Value().has_key("GMSBGravitino"):
        GMSBGravitino = eval(simFlags.specialConfiguration.get_Value().get("GMSBGravitino", None))
        kwargs.setdefault("GravitinoMass",       GMSBGravitino);
        ##kwargs.setdefault("GravitinoWidth",       0.0*GeV);
        ##kwargs.setdefault("GravitinoCharge",      0);
        ##kwargs.setdefault("GravitinoPDGCode",     1000022);
        ##kwargs.setdefault("GravitinoStable",      True);
        ##kwargs.setdefault("GravitinoLifetime",    -1);
        ##kwargs.setdefault("GravitinoShortlived",  False);
    return CfgMgr.GauginosPhysicsTool(name, **kwargs)
