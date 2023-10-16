# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
Service configurations for ISF
KG Tan, 17/06/2012
"""

from AthenaCommon import CfgMgr
# importing Reflex dictionary to access AtlasDetDescr::AtlasRegion enum
import ROOT, cppyy
cppyy.load_library("libAtlasDetDescrDict")


def getParticleBrokerSvcNoOrdering(name="ISF_ParticleBrokerSvcNoOrdering", **kwargs):
    kwargs.setdefault('EntryLayerTool', 'ISF_EntryLayerTool')
    kwargs.setdefault('GeoIDSvc', 'ISF_GeoIDSvc')
    kwargs.setdefault('AlwaysUseGeoIDSvc', False)
    from ISF_Config.ISF_jobProperties import ISF_Flags
    kwargs.setdefault('ValidateGeoIDs', ISF_Flags.ValidationMode())
    kwargs.setdefault('ValidationOutput', ISF_Flags.ValidationMode())
    kwargs.setdefault('ValidationStreamName', "ParticleBroker")
    return CfgMgr.ISF__ParticleBrokerDynamicOnReadIn(name, **kwargs)


def getParticleBrokerSvc(name="ISF_ParticleBrokerSvc", **kwargs):
    #kwargs.setdefault('ParticleOrderingTool', 'ISF_InToOutSubDetOrderingTool')
    kwargs.setdefault('ParticleOrderingTool', 'ISF_ParticleOrderingTool')
    return getParticleBrokerSvcNoOrdering(name, **kwargs)


def getAFIIParticleBrokerSvc(name="ISF_AFIIParticleBrokerSvc", **kwargs):
    kwargs.setdefault('EntryLayerTool', 'ISF_AFIIEntryLayerTool')
    return getParticleBrokerSvc(name, **kwargs)


def getAFIIEnergyOrderedParticleBrokerSvc(name="ISF_AFIIEnergyOrderedParticleBrokerSvc", **kwargs):
    kwargs.setdefault('ParticleOrderingTool', 'ISF_EnergyParticleOrderingTool')
    return getAFIIParticleBrokerSvc(name, **kwargs)


def getISFEnvelopeDefSvc(name="ISF_ISFEnvelopeDefSvc", **kwargs):
    # ATLAS common envlope definitions
    kwargs.setdefault("ATLASEnvelopeDefSvc", "AtlasGeometry_EnvelopeDefSvc")
    return CfgMgr.ISF__ISFEnvelopeDefSvc(name, **kwargs)


def getAFIIEnvelopeDefSvc(name="ISF_AFIIEnvelopeDefSvc", **kwargs):
    from AthenaCommon.SystemOfUnits import mm
    # ATLAS common envlope definitions
    kwargs.setdefault("ISFEnvelopeDefSvc", "ISF_ISFEnvelopeDefSvc")
    kwargs.setdefault("InDetMaxExtentZ", 3549.5*mm)
    return CfgMgr.ISF__AFIIEnvelopeDefSvc(name, **kwargs)


def getGeoIDSvc(name="ISF_GeoIDSvc", **kwargs):
    # with ISF volume definitions
    kwargs.setdefault("EnvelopeDefSvc", "ISF_ISFEnvelopeDefSvc")
    return CfgMgr.ISF__GeoIDSvc(name, **kwargs)


def getAFIIGeoIDSvc(name="ISF_AFIIGeoIDSvc", **kwargs):
    kwargs.setdefault("EnvelopeDefSvc", "ISF_AFIIEnvelopeDefSvc")
    return getGeoIDSvc(name, **kwargs)


def getInputConverter(name="ISF_InputConverter", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("UseShadowEvent", simFlags.UseShadowEvent())
    kwargs.setdefault("UseGeneratedParticleMass", False)
    from ISF_HepMC_Tools.ISF_HepMC_ToolsConfigLegacy import getGenParticleFilters
    kwargs.setdefault("GenParticleFilters", getGenParticleFilters())
    return CfgMgr.ISF__InputConverter(name, **kwargs)


def getLongLivedInputConverter(name="ISF_LongLivedInputConverter", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("GenParticleFilters"      , [ simFlags.ParticleSimAcceptList.get_Value(),
                                                    'ISF_ParticlePositionFilterDynamic',
                                                    'ISF_EtaPhiFilter',
                                                    'ISF_GenParticleInteractingFilter', ] )
    kwargs.setdefault('QuasiStableParticlesIncluded', True)
    return getInputConverter(name, **kwargs)


#
# Generic Truth Service Configurations
#

def getGenericTruthService(name="ISF_TruthService", **kwargs):
    kwargs.setdefault('SkipIfNoChildren', True)
    kwargs.setdefault('SkipIfNoParentBarcode', True)
    kwargs.setdefault('ForceEndVtxInRegions', [])
    long_lived_simulators = ['LongLived', 'longLived', 'QS']
    from ISF_Config.ISF_jobProperties import ISF_Flags
    is_long_lived_simulation = any(x in ISF_Flags.Simulator() for x in long_lived_simulators) #FIXME this should be set in a nicer way.
    if is_long_lived_simulation:
        kwargs.setdefault('QuasiStableParticlesIncluded', True)
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("QuasiStableParticleOverwrite", not simFlags.UseShadowEvent())
    return CfgMgr.ISF__TruthSvc(name, **kwargs)


def getValidationTruthService(name="ISF_ValidationTruthService", **kwargs):
    kwargs.setdefault('TruthStrategies', ['ISF_ValidationTruthStrategy'] )
    kwargs.setdefault('IgnoreUndefinedBarcodes', True)
    kwargs.setdefault('PassWholeVertices', True)
    return getGenericTruthService(name, **kwargs)


#
# MC12 Truth Service Configurations
#

def getMC12BeamPipeTruthStrategies():
    return ['ISF_MCTruthStrategyGroupID']


def getMC12IDTruthStrategies():
    return ['ISF_MCTruthStrategyGroupID', 'ISF_MCTruthStrategyGroupIDHadInt']


def getMC12CaloTruthStrategies():
    return ['ISF_MCTruthStrategyGroupCaloMuBrem']


def getMC12MSTruthStrategies():
    return []


def getMC12TruthService(name="ISF_MC12TruthService", **kwargs):
    kwargs.setdefault('TruthStrategies', ['ISF_MCTruthStrategyGroupID',
                                          'ISF_MCTruthStrategyGroupIDHadInt',
                                          'ISF_MCTruthStrategyGroupCaloMuBrem'])
    kwargs.setdefault('IgnoreUndefinedBarcodes', False)
    kwargs.setdefault('PassWholeVertices', True)
    return getGenericTruthService(name, **kwargs)


def getTruthService(name="ISF_TruthService", **kwargs):
    from ISF_Config.ISF_jobProperties import ISF_Flags
    if ISF_Flags.ValidationMode() :
      return getValidationTruthService(name, **kwargs)
    else:
      return getMC12TruthService(name, **kwargs)


def getMC12LLPTruthService(name="ISF_MC12TruthLLPService", **kwargs):
    kwargs.setdefault('TruthStrategies', ['ISF_MCTruthStrategyGroupID',
                                          'ISF_MCTruthStrategyGroupIDHadInt',
                                          'ISF_MCTruthStrategyGroupCaloMuBrem',
                                          'ISF_LLPTruthStrategy'])
    return getMC12TruthService(name, **kwargs)


def getMC12PlusTruthService(name="ISF_MC12PlusTruthService", **kwargs):
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion
    kwargs.setdefault('ForceEndVtxInRegions', [AtlasRegion.fAtlasID] )
    return getMC12TruthService(name, **kwargs)


#
# MC15 Truth Service Configurations
#

def getMC15BeamPipeTruthStrategies():
    return ['ISF_MCTruthStrategyGroupID_MC15']


def getMC15IDTruthStrategies():
    return ['ISF_MCTruthStrategyGroupID_MC15', 'ISF_MCTruthStrategyGroupIDHadInt_MC15']


def getMC15CaloTruthStrategies():
    return ['ISF_MCTruthStrategyGroupCaloMuBrem', 'ISF_MCTruthStrategyGroupCaloDecay_MC15']#FIXME this should be ISF_MCTruthStrategyGroupCaloMuBrem_MC15!!


def getMC15MSTruthStrategies():
    return []


def getMC15TruthService(name="ISF_MC15TruthService", **kwargs):
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion
    kwargs.setdefault('TruthStrategies', ['ISF_MCTruthStrategyGroupID_MC15',
                                          'ISF_MCTruthStrategyGroupIDHadInt_MC15',
                                          'ISF_MCTruthStrategyGroupCaloMuBrem', #FIXME this should be ISF_MCTruthStrategyGroupCaloMuBrem_MC15!!
                                          'ISF_MCTruthStrategyGroupCaloDecay_MC15'])
    kwargs.setdefault('IgnoreUndefinedBarcodes', False)
    kwargs.setdefault('PassWholeVertices', False) # new for MC15 - can write out partial vertices.
    kwargs.setdefault('ForceEndVtxInRegions', [AtlasRegion.fAtlasID])
    return getGenericTruthService(name, **kwargs)


def getMC15aTruthService(name="ISF_MC15aTruthService", **kwargs):
    kwargs.setdefault('ForceEndVtxInRegions', [])
    return getMC15TruthService(name, **kwargs)


def getMC15aPlusTruthService(name="ISF_MC15aPlusTruthService", **kwargs):
    AtlasRegion = ROOT.AtlasDetDescr.AtlasRegion
    kwargs.setdefault('ForceEndVtxInRegions', [AtlasRegion.fAtlasID])
    return getMC15TruthService(name, **kwargs)


def getMC15aPlusLLPTruthService(name="ISF_MC15aPlusLLPTruthService", **kwargs):
    kwargs.setdefault('TruthStrategies', ['ISF_KeepLLPDecayChildrenStrategy',
                                          'ISF_KeepLLPHadronicInteractionChildrenStrategy',
                                          'ISF_MCTruthStrategyGroupID_MC15',
                                          'ISF_MCTruthStrategyGroupIDHadInt_MC15',
                                          'ISF_MCTruthStrategyGroupCaloMuBrem', #FIXME this should be ISF_MCTruthStrategyGroupCaloMuBrem_MC15!!
                                          'ISF_MCTruthStrategyGroupCaloDecay_MC15',
                                          'ISF_LLPTruthStrategy'])
    return getMC15aPlusTruthService(name, **kwargs)


#
# MC16 Truth Service Configurations
#

def getMC16TruthService(name="ISF_MC16TruthService", **kwargs):
    return getMC15aPlusTruthService(name, **kwargs)


def getMC16LLPTruthService(name="ISF_MC16LLPTruthService", **kwargs):
    return getMC15aPlusLLPTruthService(name, **kwargs)


#
# MC18 Truth Service Configurations
#

def getMC18TruthService(name="ISF_MC18TruthService", **kwargs):
    return getMC15aPlusTruthService(name, **kwargs)


def getMC18LLPTruthService(name="ISF_MC18LLPTruthService", **kwargs):
    return getMC15aPlusLLPTruthService(name, **kwargs)
