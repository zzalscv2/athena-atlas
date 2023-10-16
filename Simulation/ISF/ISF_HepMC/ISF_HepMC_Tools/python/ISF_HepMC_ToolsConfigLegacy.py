# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
Tools configurations for ISF
KG Tan, 17/06/2012
"""

from AthenaCommon import CfgMgr
from AthenaCommon.SystemOfUnits import MeV, mm

#--------------------------------------------------------------------------------------------------
## GenParticleFilters

def getParticleFinalStateFilter(name="ISF_ParticleFinalStateFilter", **kwargs):
    from ISF_Config.ISF_jobProperties import ISF_Flags
    G4NotInUse = not ISF_Flags.UsingGeant4.get_Value()
    from G4AtlasApps.SimFlags import simFlags
    G4NotInUse = G4NotInUse and simFlags.ISFRun.get_Value()
    # use CheckGenInteracting==False to allow GenEvent neutrinos to propagate into the simulation
    kwargs.setdefault("CheckGenSimStable", G4NotInUse)
    kwargs.setdefault("CheckGenInteracting", G4NotInUse)
    return CfgMgr.ISF__GenParticleFinalStateFilter(name, **kwargs)

def getParticleSimAcceptList(name="ISF_ParticleSimAcceptList", **kwargs):
    # GenParticleSimAcceptList
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("UseShadowEvent", simFlags.UseShadowEvent())
    kwargs.setdefault("MinimumDecayRadiusQS", simFlags.QuasiStableParticleRadius.get_Value())
    return CfgMgr.ISF__GenParticleSimAcceptList(name, **kwargs)

def getGenParticleSimQuasiStableFilter(name="ISF_GenParticleSimQuasiStableFilter", **kwargs):
    # GenParticleSimQuasiStableFilter
    from G4AtlasApps.SimFlags import simFlags
    QSP_radius =  simFlags.QuasiStableParticleRadius.get_Value()
    kwargs.setdefault("MinProdRadius", [QSP_radius, 0.] )
    kwargs.setdefault("MinDecayRadius", [QSP_radius, QSP_radius] )
    return CfgMgr.ISF__GenParticleSimQuasiStableFilter(name, **kwargs)

def getParticleSimAcceptList_ExtraParticles(name="ISF_ParticleSimAcceptList_ExtraParticles", **kwargs):
    # GenParticleSimAcceptList_LongLived
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("UseShadowEvent", simFlags.UseShadowEvent())
    kwargs.setdefault("MinimumDecayRadiusQS", simFlags.QuasiStableParticleRadius.get_Value())
    kwargs.setdefault('AcceptLists' , ['G4particle_acceptlist.txt', 'G4particle_acceptlist_ExtraParticles.txt'] )
    return CfgMgr.ISF__GenParticleSimAcceptList(name, **kwargs)

def getParticlePositionFilter(name="ISF_ParticlePositionFilter", **kwargs):
    # ParticlePositionFilter
    kwargs.setdefault('GeoIDService' , 'ISF_GeoIDSvc'    )
    return CfgMgr.ISF__GenParticlePositionFilter(name, **kwargs)

def getParticlePositionFilterID(name="ISF_ParticlePositionFilterID", **kwargs):
    # importing Reflex dictionary to access AtlasDetDescr::AtlasRegion enum
    import ROOT, cppyy
    cppyy.load_library('libAtlasDetDescrDict')
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion

    kwargs.setdefault('CheckRegion'  , [ AtlasRegion.fAtlasID ] )
    return getParticlePositionFilter(name, **kwargs)

def getParticlePositionFilterCalo(name="ISF_ParticlePositionFilterCalo", **kwargs):
    # importing Reflex dictionary to access AtlasDetDescr::AtlasRegion enum
    import ROOT, cppyy
    cppyy.load_library('libAtlasDetDescrDict')
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion

    kwargs.setdefault('CheckRegion'  , [ AtlasRegion.fAtlasID,
                                            AtlasRegion.fAtlasForward,
                                            AtlasRegion.fAtlasCalo ] )
    return getParticlePositionFilter(name, **kwargs)

def getParticlePositionFilterMS(name="ISF_ParticlePositionFilterMS", **kwargs):
    # importing Reflex dictionary to access AtlasDetDescr::AtlasRegion enum
    import ROOT, cppyy
    cppyy.load_library('libAtlasDetDescrDict')
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion

    kwargs.setdefault('CheckRegion'  , [ AtlasRegion.fAtlasID,
                                            AtlasRegion.fAtlasForward,
                                            AtlasRegion.fAtlasCalo,
                                            AtlasRegion.fAtlasMS ] )
    return getParticlePositionFilter(name, **kwargs)

def getParticlePositionFilterWorld(name="ISF_ParticlePositionFilterWorld", **kwargs):
    # importing Reflex dictionary to access AtlasDetDescr::AtlasRegion enum
    import ROOT, cppyy
    cppyy.load_library('libAtlasDetDescrDict')
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion
    kwargs.setdefault('CheckRegion'  , [ AtlasRegion.fAtlasID,
                                            AtlasRegion.fAtlasForward,
                                            AtlasRegion.fAtlasCalo,
                                            AtlasRegion.fAtlasMS,
                                            AtlasRegion.fAtlasCavern ] )
    return getParticlePositionFilter(name, **kwargs)

def getParticlePositionFilterDynamic(name="ISF_ParticlePositionFilterDynamic", **kwargs):
    # automatically choose the best fitting filter region
    from AthenaCommon.DetFlags import DetFlags
    if DetFlags.Muon_on():
      return getParticlePositionFilterWorld(name, **kwargs)
    elif DetFlags.Calo_on():
      return getParticlePositionFilterCalo(name, **kwargs)
    elif DetFlags.ID_on():
      return getParticlePositionFilterID(name, **kwargs)
    else:
      return getParticlePositionFilterWorld(name, **kwargs)

def getGenParticleInteractingFilter(name="ISF_GenParticleInteractingFilter", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    simdict = simFlags.specialConfiguration.get_Value()
    if simdict is not None and "InteractingPDGCodes" in simdict:
        kwargs.setdefault('AdditionalInteractingParticleTypes', eval(simdict["InteractingPDGCodes"]))
    if simdict is not None and "NonInteractingPDGCodes" in simdict:
        kwargs.setdefault('AdditionalNonInteractingParticleTypes', eval(simdict["NonInteractingPDGCodes"]))
    return CfgMgr.ISF__GenParticleInteractingFilter(name, **kwargs)

def getEtaPhiFilter(name="ISF_EtaPhiFilter", **kwargs):
    # EtaPhiFilter
    from AthenaCommon.DetFlags import DetFlags
    EtaRange = 7.0 if DetFlags.geometry.Lucid_on() else 6.0
    kwargs.setdefault('MinEta' , -EtaRange)
    kwargs.setdefault('MaxEta' , EtaRange)
    kwargs.setdefault('MaxApplicableRadius', 30*mm)
    return CfgMgr.ISF__GenParticleGenericFilter(name, **kwargs)


def getGenParticleFilters(isQS=False):
    genParticleFilterList = []
    from G4AtlasApps.SimFlags import simFlags
    if isQS:
        genParticleFilterList = [simFlags.ParticleSimAcceptList.get_Value()]
    else:
        genParticleFilterList = ['ISF_ParticleFinalStateFilter'] # not used for Quasi-stable particle simulation
    from G4AtlasApps.SimFlags import simFlags
    if "ATLAS" in simFlags.SimLayout():
        from AthenaCommon.BeamFlags import jobproperties
        if jobproperties.Beam.beamType() != "cosmics":
            genParticleFilterList += ['ISF_ParticlePositionFilterDynamic']
            from AthenaCommon.DetFlags import DetFlags
            if not (DetFlags.geometry.AFP_on() or DetFlags.geometry.ALFA_on() or DetFlags.geometry.FwdRegion_on()) and\
               (not simFlags.CavernBG.statusOn or simFlags.CavernBG.get_Value() == 'Signal') and\
               not (simFlags.SimulateCavern.statusOn and simFlags.SimulateCavern.get_Value()):
                genParticleFilterList += ['ISF_EtaPhiFilter']
    genParticleFilterList += ['ISF_GenParticleInteractingFilter']
    return genParticleFilterList


def getTruthPreselectionTool(name="TruthPreselectionTool", **kwargs):
    from ISF_Config.ISF_jobProperties import ISF_Flags
    kwargs.setdefault( "GenParticleFilters", getGenParticleFilters(ISF_Flags.Simulator.isQuasiStable()) )
    kwargs.setdefault( "QuasiStableParticleFilter", "ISF_GenParticleSimQuasiStableFilter" )
    return CfgMgr.ISF__TruthPreselectionTool(name, **kwargs)


#--------------------------------------------------------------------------------------------------
## Truth Strategies

# Brems: fBremsstrahlung (3)
# Conversion: fGammaConversion (14), fGammaConversionToMuMu (15), fPairProdByCharged (4)
# Decay: 201-298, fAnnihilation(5), fAnnihilationToMuMu (6), fAnnihilationToHadrons (7)
# Ionization: fIonisation (2), fPhotoElectricEffect (12)
# Hadronic: (111,121,131,141,151,161,210)
# Compton: fComptonScattering (13)
# G4 process types:
#  http://www-geant4.kek.jp/lxr/source//processes/management/include/G4ProcessType.hh
# G4 EM sub types:
#  http://www-geant4.kek.jp/lxr/source//processes/electromagnetic/utils/include/G4EmProcessSubType.hh
# G4 HadInt sub types:
#  http://www-geant4.kek.jp/lxr/source//processes/hadronic/management/include/G4HadronicProcessType.hh#L46
def getTruthStrategyGroupID_MC15(name="ISF_MCTruthStrategyGroupID_MC15", **kwargs):
    kwargs.setdefault('ParentMinPt'         , 100.*MeV)
    kwargs.setdefault('ChildMinPt'          , 300.*MeV)
    kwargs.setdefault('VertexTypes'         , [ 3, 14, 15, 4, 5, 6, 7, 2, 12, 13 ])
    kwargs.setdefault('VertexTypeRangeLow'  , 201)  # All kinds of decay processes
    kwargs.setdefault('VertexTypeRangeHigh' , 298)  # ...
    kwargs.setdefault('Regions', [1,2]) # Could import AtlasDetDescr::AtlasRegion enum as in TruthService CfgGetter methods here
    return CfgMgr.ISF__GenericTruthStrategy(name, **kwargs)


def getTruthStrategyGroupID(name="ISF_MCTruthStrategyGroupID", **kwargs):
    kwargs.setdefault('ParentMinPt'         , 100.*MeV)
    kwargs.setdefault('ChildMinPt'          , 100.*MeV)
    kwargs.setdefault('VertexTypes'         , [ 3, 14, 15, 4, 5, 6, 7, 2, 12, 13 ])
    kwargs.setdefault('VertexTypeRangeLow'  , 201)  # All kinds of decay processes
    kwargs.setdefault('VertexTypeRangeHigh' , 298)  # ...
    kwargs.setdefault('Regions', [1,2])
    return CfgMgr.ISF__GenericTruthStrategy(name, **kwargs)


def getTruthStrategyGroupIDHadInt_MC15(name="ISF_MCTruthStrategyGroupIDHadInt_MC15", **kwargs):
    kwargs.setdefault('ParentMinPt'                       , 100.*MeV)
    kwargs.setdefault('ChildMinPt'                        , 300.*MeV)
    kwargs.setdefault('VertexTypes'                       , [ 111, 121, 131, 141, 151, 161, 210 ])
    kwargs.setdefault('AllowChildrenOrParentPassKineticCuts' , True)
    kwargs.setdefault('Regions', [1])
    return CfgMgr.ISF__GenericTruthStrategy(name, **kwargs)


def getTruthStrategyGroupIDHadInt(name="ISF_MCTruthStrategyGroupIDHadInt", **kwargs):
    kwargs.setdefault('ParentMinPt'                       , 100.*MeV)
    kwargs.setdefault('ChildMinPt'                        , 100.*MeV)
    kwargs.setdefault('VertexTypes'                       , [ 111, 121, 131, 141, 151, 161, 210 ])
    kwargs.setdefault('AllowChildrenOrParentPassKineticCuts' , True)
    kwargs.setdefault('Regions', [1])
    return CfgMgr.ISF__GenericTruthStrategy(name, **kwargs)


def getTruthStrategyGroupCaloMuBrem_MC15(name="ISF_MCTruthStrategyGroupCaloMuBrem_MC15", **kwargs):
    kwargs.setdefault('ParentMinEkin'       , 500.*MeV)
    kwargs.setdefault('ChildMinEkin'        , 300.*MeV)
    kwargs.setdefault('VertexTypes'         , [ 3 ])
    kwargs.setdefault('ParentPDGCodes'      , [ 13, -13 ])
    kwargs.setdefault('Regions', [3])
    return CfgMgr.ISF__GenericTruthStrategy(name, **kwargs)

def getTruthStrategyGroupCaloMuBrem(name="ISF_MCTruthStrategyGroupCaloMuBrem", **kwargs):
    kwargs.setdefault('ParentMinEkin'       , 500.*MeV)
    kwargs.setdefault('ChildMinEkin'        , 100.*MeV)
    kwargs.setdefault('VertexTypes'         , [ 3 ])
    kwargs.setdefault('ParentPDGCodes'      , [ 13, -13 ])
    kwargs.setdefault('Regions', [3])
    return CfgMgr.ISF__GenericTruthStrategy(name, **kwargs)

def getTruthStrategyGroupCaloDecay_MC15(name="ISF_MCTruthStrategyGroupCaloDecay_MC15", **kwargs):
    kwargs.setdefault('ParentMinEkin'       , 1000.*MeV)
    kwargs.setdefault('ChildMinEkin'        , 500.*MeV)
    kwargs.setdefault('VertexTypes'         , [ 5, 6, 7 ])
    kwargs.setdefault('VertexTypeRangeLow'  , 201)  # All kinds of decay processes
    kwargs.setdefault('VertexTypeRangeHigh' , 298)  # ...
    kwargs.setdefault('Regions', [3])
    return CfgMgr.ISF__GenericTruthStrategy(name, **kwargs)

def getTruthStrategyGroupCaloDecay(name="ISF_MCTruthStrategyGroupCaloDecay", **kwargs):
    kwargs.setdefault('ParentMinPt'         , 1000.*MeV)
    kwargs.setdefault('ChildMinPt'          , 500.*MeV)
    kwargs.setdefault('VertexTypes'         , [ 5, 6, 7 ])
    kwargs.setdefault('VertexTypeRangeLow'  , 201)  # All kinds of decay processes
    kwargs.setdefault('VertexTypeRangeHigh' , 298)  # ...
    kwargs.setdefault('Regions', [3])
    return CfgMgr.ISF__GenericTruthStrategy(name, **kwargs)

def getValidationTruthStrategy(name="ISF_ValidationTruthStrategy", **kwargs):
    kwargs.setdefault('ParentMinP'          , 50.*MeV)
    kwargs.setdefault('Regions', [1,3])
    return CfgMgr.ISF__ValidationTruthStrategy(name, **kwargs)

def getLLPTruthStrategy(name="ISF_LLPTruthStrategy", **kwargs):
    kwargs.setdefault('PassProcessCodeRangeLow',  200 )
    kwargs.setdefault('PassProcessCodeRangeHigh', 299 )
    # ProcessCategory==9 corresponds to the 'fUserDefined' G4ProcessType:
    #   http://www-geant4.kek.jp/lxr/source//processes/management/include/G4ProcessType.hh
    kwargs.setdefault('PassProcessCategory',      9   ) # ==
    kwargs.setdefault('Regions', [1,2,3,4])
    return CfgMgr.ISF__LLPTruthStrategy(name, **kwargs)

def getKeepLLPDecayChildrenStrategy(name="ISF_KeepLLPDecayChildrenStrategy", **kwargs):
    # ProcessCategory==9 corresponds to the 'fUserDefined' G4ProcessType:
    #   http://www-geant4.kek.jp/lxr/source//processes/management/include/G4ProcessType.hh
    kwargs.setdefault('PassProcessCategory' , 9  ) # ==
    kwargs.setdefault('VertexTypeRangeLow'  , 200) # All kinds of decay processes
    kwargs.setdefault('VertexTypeRangeHigh' , 299) # ...
    kwargs.setdefault('BSMParent'           , True)
    return CfgMgr.ISF__KeepChildrenTruthStrategy(name, **kwargs)

def getKeepLLPHadronicInteractionChildrenStrategy(name="ISF_KeepLLPHadronicInteractionChildrenStrategy", **kwargs):
    kwargs.setdefault('VertexTypes'          , [ 111, 121, 131, 141, 151, 161, 210 ])
    kwargs.setdefault('BSMParent'            , True)
    return CfgMgr.ISF__KeepChildrenTruthStrategy(name, **kwargs)

def getKeepAllDecayChildrenStrategy(name="ISF_KeepAllDecayChildrenStrategy", **kwargs):
    # ProcessCategory==9 corresponds to the 'fUserDefined' G4ProcessType:
    #   http://www-geant4.kek.jp/lxr/source//processes/management/include/G4ProcessType.hh
    kwargs.setdefault('PassProcessCategory' , 9  ) # ==
    kwargs.setdefault('VertexTypeRangeLow'  , 200) # All kinds of decay processes
    kwargs.setdefault('VertexTypeRangeHigh' , 299) # ...
    return CfgMgr.ISF__KeepChildrenTruthStrategy(name, **kwargs)

def getKeepHadronicInteractionChildrenStrategy(name="ISF_KeepHadronicInteractionChildrenStrategy", **kwargs):
    kwargs.setdefault('VertexTypes'          , [ 111, 121, 131, 141, 151, 161, 210 ])
    return CfgMgr.ISF__KeepChildrenTruthStrategy(name, **kwargs)
