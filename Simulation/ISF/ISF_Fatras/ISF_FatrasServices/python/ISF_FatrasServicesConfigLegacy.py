# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
Service and Tool configurations for ISF for ISF_FatrasServicesConfig
KG Tan, 04/12/2012
Updated by J. Chapman
"""

from AthenaCommon import CfgMgr
from AthenaCommon.CfgGetter import getPublicTool

from AthenaCommon.DetFlags import DetFlags
from TrkDetDescrSvc.TrkDetDescrJobProperties import TrkDetFlags

from ISF_Config.ISF_jobProperties import ISF_Flags # IMPORTANT: Flags must be set before tools are retrieved
from ISF_FatrasServices.ISF_FatrasJobProperties import ISF_FatrasFlags
from ISF_FatrasServices.FatrasTuning import FatrasTuningFlags
from ISF_Algorithms.collection_merger_helpers import generate_mergeable_collection_name


#################################################################################
# Material for the Geometry
#
# if only ID is switched on different COOLDB Folders
#    and different Tag is used
#
#################################################################################

def initialiseCoolDataBaseFolder():
    if TrkDetFlags.ConfigurationOutputLevel() < 3 :
      print ('[ Configuration : start ] *** FatrasTrackingGeometry ********************************')
      print ('[ TrackingGeometrySvc ]')

    # check whether the material retrieval is ment to be from COOL
    if TrkDetFlags.MaterialSource() == 'COOL' :
        # the tag names
        CoolDataBaseFolder = TrkDetFlags.MaterialStoreGateKey()
        AtlasMaterialTag = TrkDetFlags.MaterialTagBase()+str(TrkDetFlags.MaterialVersion())+'_'

        if TrkDetFlags.ConfigurationOutputLevel() < 3 :
           print ('[ TrackingGeometrySvc ] Associating DB folder : ',CoolDataBaseFolder)
           print ('[ TrackingGeometrySvc ]     base material tag : ',AtlasMaterialTag)

        # we need the conditions interface
        from IOVDbSvc.CondDB import conddb
        # use a local database
        if TrkDetFlags.MaterialDatabaseLocal():
            # specify the local database
            DataBasePath  = TrkDetFlags.MaterialDatabaseLocalPath()
            DataBaseName  = TrkDetFlags.MaterialDatabaseLocalName()
            MagicTag      = TrkDetFlags.MaterialMagicTag()
            DataBaseConnection = '<dbConnection>sqlite://X;schema='+DataBasePath+DataBaseName+';dbname=OFLP200</dbConnection>'
            conddb.blockFolder('/GLOBAL/TrackingGeo/LayerMaterial')
            conddb.addFolderWithTag('',DataBaseConnection+CoolDataBaseFolder,AtlasMaterialTag+MagicTag,force=True,
                                    className = 'Trk::LayerMaterialMap')
            if TrkDetFlags.ConfigurationOutputLevel() < 3 :
                print ('[ TrackingGeometrySvc ] Using Local Database: '+DataBaseConnection)
            # make sure that the pool files are in the catalog
            #from PoolSvc.PoolSvcConf import PoolSvc
            #PoolSvc.ReadCatalog += [ DataBasePath+'PoolFileCatalog.xml' ]
        else :
            # load the right folders (preparation for calo inclusion)
            cfolder = CoolDataBaseFolder +'<tag>TagInfoMajor/'+AtlasMaterialTag+'/GeoAtlas</tag>'
            conddb.addFolderSplitMC('GLOBAL',cfolder,cfolder,
                                    className = 'Trk::LayerMaterialMap')

    #HACK: CoolDataBaseFolder may not be set at this point! Is this right? -KG
    return CoolDataBaseFolder


######################################################################################
# validation & process sampling
def getFatrasPhysicsValidationTool(name="ISF_FatrasPhysicsValidationTool", **kwargs):
    kwargs.setdefault("ValidationStreamName", "ISFFatras")

    from ISF_FatrasTools.ISF_FatrasToolsConf import iFatras__PhysicsValidationTool
    return iFatras__PhysicsValidationTool(name, **kwargs)


#################################################################################
# The Geometry Builder
#################################################################################

# Not used anywhere - not migrated to CA config
def getInDetTrackingGeometryBuilder(name="ISF_InDetTrackingGeometryBuilder", **kwargs):
    # get hand on the ID Tracking Geometry Builder
    kwargs.setdefault("namePrefix"              , 'Fatras')
    kwargs.setdefault("setLayerAssociation"     , False)
    #kwargs.setdefault("VolumeEnclosureOuterR"   , 1148.) ### HACK: Cannot set via imput arguments. Is this right?? -kg
    kwargs.setdefault("buildTrtStrawLayers" , True)
    from InDetTrackingGeometry.ConfiguredInDetTrackingGeometryBuilder import ConfiguredInDetTrackingGeometryBuilder as IDGeometryBuilder
    t = IDGeometryBuilder(name, **kwargs )
    t.VolumeEnclosureOuterR = 1148.
    #t.EnvelopeDefinitionSvc = 'ISF_EnvelopeDefSvc'
    return t

# Not used anywhere - not migrated to CA config
def getFatrasCaloTrackingGeometryBuilder(name="ISF_FatrasCaloTrackingGeometryBuilder", **kwargs):
    # get hand on the ID Tracking Geometry Builder
    #kwargs.setdefault("RecordLayerIndexCaloSampleMap"   , True) ### HACK: Cannot set via imput arguments. Is this right?? -kg

    from CaloTrackingGeometry.ConfiguredCaloTrackingGeometryBuilder import ConfiguredCaloTrackingGeometryBuilder as ConfiguredCaloGeo
    t = ConfiguredCaloGeo(name, **kwargs)
    t.RecordLayerIndexCaloSampleMap = True
    #t.EnvelopeDefinitionSvc = 'ISF_EnvelopeDefSvc'
    return t

# Not used anywhere - not migrated to CA config
def getFatrasMuonTrackingGeometryBuilder(name="ISF_FatrasMuonTrackingGeometryBuilder", **kwargs):
    # get hand on the MS Tracking Geometry Builder

    from MuonTrackingGeometry.ConfiguredMuonTrackingGeometryBuilder import ConfiguredMuonTrackingGeometryBuilder as ConfiguredMuonGeo
    t = ConfiguredMuonGeo(name, **kwargs)
    return t

# Not used anywhere - not migrated to CA config
def getFatrasGeometryBuilder(name="ISF_FatrasGeometryBuilder", **kwargs):
    # the geometry builder alg tool
    # switch on the Detectors
    #kwargs.setdefault("InDetTrackingGeometry"   , DetFlags.ID_on())
    #kwargs.setdefault("CaloTrackingGeometry"    , DetFlags.Calo_on())
    #kwargs.setdefault("MuonTrackingGeometry"    , DetFlags.Muon_on())
    if DetFlags.ID_on():
        kwargs.setdefault("InDetTrackingGeometryBuilder",getPublicTool('ISF_InDetTrackingGeometryBuilder'))
    if DetFlags.Calo_on():
        kwargs.setdefault("CaloTrackingGeometryBuilder" ,getPublicTool('ISF_FatrasCaloTrackingGeometryBuilder'))
    if DetFlags.Muon_on():
        kwargs.setdefault("MuonTrackingGeometryBuilder" ,getPublicTool('ISF_FatrasMuonTrackingGeometryBuilder'))

    from TrkDetDescrTools.TrkDetDescrToolsConf import Trk__GeometryBuilder
    return Trk__GeometryBuilder(name, **kwargs )

# Not used anywhere - not migrated to CA config
def getFatrasTrackingGeometrySvc(name="ISF_FatrasTrackingGeometrySvc", **kwargs):
    # get hand on the ID Tracking Geometry Builder
    # register the Builder
    kwargs.setdefault("GeometryBuilder"             , getPublicTool('ISF_FatrasGeometryBuilder'))
    # and register the name under which the geometry is registered
    kwargs.setdefault("TrackingGeometryName"        , 'ISF_FatrasTrackingGeometry')
    # global scaling of the material if necessary
    #kwargs.setdefault("GlobalScaleFactor"           , FatrasTuningFlags.MaterialScalor())
    # some flags - only works when ID is on
    #kwargs.setdefault("AssignMaterialFromCOOL"      , True)
    kwargs.setdefault("BuildGeometryFromTagInfo"    , True)
    # switch to the rigth CoolDB folder
    #kwargs.setdefault("InputLayerMaterialSetName"   , initialiseCoolDataBaseFolder()) ## HACK: This is set during "initialiseCoolDataBaseFolder()"

    # then get the tuning parameters
    # if FatrasTuningFlags.MaterialDistortionsX0.statusOn and FatrasTuningFlags.MaterialDistortionsRho.statusOn:
    #    # Material scaling & distortion
    #    kwargs.setdefault("RandomDistortions"           , True)
    #    kwargs.setdefault("DistortionsRadiationLength"  , FatrasTuningFlags.MaterialDistortionsX0())
    #    kwargs.setdefault("DistortionsDensity"          , FatrasTuningFlags.MaterialDistortionsRho())
    #    kwargs.setdefault("RecordDistortions"           , FatrasValidationFlags.MaterialDistortions())
    #    kwargs.setdefault("DistortionOutputFolder"      , '/'+FatrasValidationFlags.ValidationStream()+'/MaterialDistortions')
    #    from TrkDetDescrSvc.TrkDetDescrSvcConf import CalibrationTrackingGeometrySvc as GeometrySvc
    #else:

    from TrkDetDescrSvc.TrkDetDescrSvcConf import Trk__TrackingGeometrySvc as GeometrySvc
    return GeometrySvc(name, **kwargs )

################################################################################
# TRANSPORT SECTION
#   TODO: should be in the future rationalized to one transport tool
################################################################################


def getFatrasNavigator(name="ISF_FatrasNavigator", **kwargs):
    # the Navigator (needed for several instances)

    from AthenaCommon.AlgSequence import AthSequencer
    condSeq = AthSequencer("AthCondSeq")

    # Decide if we want to use the Tracking Geometry 
    # from conditions or not 
    if (ISF_Flags.UseTrackingGeometryCond 
            and 'TrackingGeometryKey' not in kwargs):
        if not hasattr(condSeq, 'AtlasTrackingGeometryCondAlg'):
            from InDetCondFolders import InDetAlignFolders_FATRAS  # noqa: F401
            from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlg import (
                ConfiguredTrackingGeometryCondAlg)
            TrkGeoCondAlg = ConfiguredTrackingGeometryCondAlg(
                'AtlasTrackingGeometryCondAlg')
            condSeq += TrkGeoCondAlg

        kwargs.setdefault('TrackingGeometryKey',
                          condSeq.AtlasTrackingGeometryCondAlg.TrackingGeometryWriteKey)
    elif 'TrackingGeometrySvc' not in kwargs:
        from TrkDetDescrSvc.AtlasTrackingGeometrySvc import AtlasTrackingGeometrySvc
        kwargs.setdefault('TrackingGeometrySvc', AtlasTrackingGeometrySvc)
        kwargs.setdefault('TrackingGeometryKey', '')

    from TrkExTools.TrkExToolsConf import Trk__Navigator
    return Trk__Navigator(name, **kwargs)
    
  
# Not used anywhere - not migrated to CA config
def getFatrasNeutralPropagatorID(name="ISF_FatrasNeutralPropagatorID", **kwargs):
    # the neutral particle propagator
    from TrkExSlPropagator.TrkExSlPropagatorConf import Trk__StraightLinePropagator as NeutralPropagator
    return NeutralPropagator(name, **kwargs )

def getFatrasChargedPropagator(name="ISF_FatrasChargedPropagator", **kwargs):
    # the charged particle propagator
    from TrkExRungeKuttaPropagator.TrkExRungeKuttaPropagatorConf import Trk__RungeKuttaPropagator as ChargedPropagator
    return ChargedPropagator(name, **kwargs )
    #from TrkExSTEP_Propagator.TrkExSTEP_PropagatorConf import Trk__STEP_Propagator as ChargedPropagator
    #return ChargedPropagator(name, **kwargs )


################################################################################
# PARTICLE DECAY SECTION
################################################################################

# Not used anywhere - not migrated to CA config
def getG4RunManagerHelper(name="ISF_G4RunManagerHelper", **kwargs):
    return CfgMgr.iGeant4__G4RunManagerHelper(name, **kwargs)

def getFatrasPdgG4Particle(name="ISF_FatrasPdgG4Particle", **kwargs):
    return CfgMgr.iFatras__PDGToG4Particle(name, **kwargs )

def getFatrasParticleDecayHelper(name="ISF_FatrasParticleDecayHelper", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    if not simFlags.RandomSeedList.checkForExistingSeed( "FatrasG4" ):
      simFlags.RandomSeedList.addSeed( "FatrasG4" , 23491234, 23470291 )
    kwargs.setdefault("RandomNumberService" , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"    , ISF_FatrasFlags.RandomStreamName())
    kwargs.setdefault("G4RandomStreamName"    , "FatrasG4") # TODO: read stream name "FatrasG4" from Fatras jobProperties
    kwargs.setdefault("ParticleBroker"  , ISF_Flags.ParticleBroker())
    kwargs.setdefault("ParticleTruthSvc", simFlags.TruthStrategy.TruthServiceName())
    kwargs.setdefault("PDGToG4ParticleConverter", getPublicTool('ISF_FatrasPdgG4Particle'))
    # the validation output
    kwargs.setdefault("ValidationMode"              , ISF_Flags.ValidationMode())
    kwargs.setdefault("PhysicsValidationTool"       , getPublicTool('ISF_FatrasPhysicsValidationTool'))
    #kwargs.setdefault("G4RunManagerHelper"  , getPublicTool('ISF_G4RunManagerHelper'))

    return CfgMgr.iFatras__G4ParticleDecayHelper(name, **kwargs )

############################################################################
# (1)  Charged Leptons and Hadrons
#
# FATRAS internal physics list
#
# SubProcesses handled
# (i) Hadronic Interactions
#     the G4 MaterialEffectsUpdator

def getFatrasG4HadIntProcessor(name="ISF_FatrasG4HadIntProcessor", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("RandomNumberService" , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"    , ISF_FatrasFlags.RandomStreamName())
    kwargs.setdefault("ParticleBroker"      , ISF_Flags.ParticleBroker())
    kwargs.setdefault("TruthRecordSvc"      , simFlags.TruthStrategy.TruthServiceName())
    kwargs.setdefault("PhysicsValidationTool"       , getPublicTool('ISF_FatrasPhysicsValidationTool'))
    kwargs.setdefault('ValidationMode'      , ISF_Flags.ValidationMode())
    kwargs.setdefault("MomentumCut"        , FatrasTuningFlags.MomCutOffSec())

    return CfgMgr.iFatras__G4HadIntProcessor(name, **kwargs )

def getFatrasParametricHadIntProcessor(name="ISF_FatrasParametricHadIntProcessor", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    #   Fatras Hadronic Interaction Processor
    #   hadronic interaction creator
    kwargs.setdefault("RandomNumberService" , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"    , ISF_FatrasFlags.RandomStreamName())
    kwargs.setdefault("ParticleBroker"      , ISF_Flags.ParticleBroker())
    kwargs.setdefault("TruthRecordSvc"      , simFlags.TruthStrategy.TruthServiceName())
    kwargs.setdefault("HadronicInteractionScaleFactor"  , FatrasTuningFlags.HadronicInteractionProbabilityScalor())
    kwargs.setdefault("MinimumHadronicInitialEnergy"    , FatrasTuningFlags.MomCutOffSec())
    kwargs.setdefault("MinimumHadronicOutEnergy"        , FatrasTuningFlags.MomCutOffSec())
    kwargs.setdefault("HadronicInteractionValidation"   , False)
    kwargs.setdefault("PhysicsProcessCode"              , 121) # TODO: to be taken from central definition
    kwargs.setdefault("PhysicsValidationTool"       , getPublicTool('ISF_FatrasPhysicsValidationTool'))
    kwargs.setdefault('ValidationMode'      , ISF_Flags.ValidationMode())


    from ISF_FatrasTools.ISF_FatrasToolsConf import iFatras__HadIntProcessorParametric
    return iFatras__HadIntProcessorParametric(name, **kwargs )

# (iv) photon conversion
def getFatrasConversionCreator(name="ISF_FatrasConversionCreator", **kwargs):
    #   Fatras Hadronic Interaction Processor
    #   hadronic interaction creator
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("RandomNumberService" , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"    , ISF_FatrasFlags.RandomStreamName())
    kwargs.setdefault("ParticleBroker"  , ISF_Flags.ParticleBroker())
    kwargs.setdefault("TruthRecordSvc"  , simFlags.TruthStrategy.TruthServiceName())
    kwargs.setdefault("PhysicsValidationTool"       , getPublicTool('ISF_FatrasPhysicsValidationTool'))
    kwargs.setdefault("PhysicsProcessCode"              , 14) # TODO: to be taken from central definition
    kwargs.setdefault('ValidationMode'      , ISF_Flags.ValidationMode())

    from ISF_FatrasTools.ISF_FatrasToolsConf import iFatras__PhotonConversionTool
    return iFatras__PhotonConversionTool(name, **kwargs )

def getFatrasProcessSamplingTool(name="ISF_FatrasProcessSamplingTool", **kwargs):

    # random number service
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault( "RandomNumberService", simFlags.RandomSvc())
    # truth record
    kwargs.setdefault("TruthRecordSvc"      , simFlags.TruthStrategy.TruthServiceName())
    # decays
    kwargs.setdefault("ParticleDecayHelper" , getPublicTool('ISF_FatrasParticleDecayHelper'))
    # photon conversion
    kwargs.setdefault("PhotonConversionTool"     , getPublicTool('ISF_FatrasConversionCreator'))
    # hadronic interactions
    kwargs.setdefault("HadronicInteraction"         , True)
    #kwargs.setdefault("HadronicInteractionProcessor", getPublicTool('ISF_FatrasParametricHadIntProcessor'))
    kwargs.setdefault("HadronicInteractionProcessor", getPublicTool('ISF_FatrasG4HadIntProcessor'))
    # the validation output
    kwargs.setdefault("ValidationMode"              , ISF_Flags.ValidationMode())
    kwargs.setdefault("PhysicsValidationTool"       , getPublicTool('ISF_FatrasPhysicsValidationTool'))

    from ISF_FatrasTools.ISF_FatrasToolsConf import iFatras__ProcessSamplingTool
    return iFatras__ProcessSamplingTool(name, **kwargs)
#
# (ii) Energy Loss
#      - Ionization and Bremstrahlung loss
#      - assing the Bethe-Heitler Eloss updator
def getFatrasEnergyLossUpdator(name="ISF_FatrasEnergyLossUpdator", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("RandomNumberService" , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"    , ISF_FatrasFlags.RandomStreamName())
    kwargs.setdefault("UsePDG_EnergyLossFormula", True)
    kwargs.setdefault("EnergyLossDistribution", 2)
    #kwargs.setdefault("EnergyLossUpdator"  , getPublicTool('AtlasEnergyLossUpdator'))

    from ISF_FatrasTools.ISF_FatrasToolsConf import iFatras__McEnergyLossUpdator
    return iFatras__McEnergyLossUpdator(name, **kwargs )

# Not used anywhere - not migrated to CA config
def getFatrasEnergyLossSamplerBetheHeitler(name="ISF_FatrasEnergyLossSamplerBetheHeitler", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("RandomNumberService" , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"    , ISF_FatrasFlags.RandomStreamName())
    kwargs.setdefault("ScaleFactor"  , FatrasTuningFlags.BetheHeitlerScalor())
    
    from ISF_FatrasTools.ISF_FatrasToolsConf import iFatras__EnergyLossSamplerBetheHeitler
    return iFatras__EnergyLossSamplerBetheHeitler(name, **kwargs )

#
# (iii) Multiple scattering
def getFatrasMultipleScatteringUpdator(name="ISF_FatrasMultipleScatteringUpdator", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("RandomNumberService"   , simFlags.RandomSvcMT() )
    kwargs.setdefault("RandomStreamName"      , 'TrkExRnd' ) # TODO: read stream name "TrkExRnd" from Fatras jobProperties
    kwargs.setdefault("GaussianMixtureModel"  , FatrasTuningFlags.GaussianMixtureModel())

    from TrkExTools.TrkExToolsConf import Trk__MultipleScatteringUpdator
    return Trk__MultipleScatteringUpdator(name, **kwargs )

# Not used anywhere - not migrated to CA config
def getFatrasMultipleScatteringSamplerHighland(name="ISF_MultipleScatteringSamplerHighland", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    if not simFlags.RandomSeedList.checkForExistingSeed( "TrkExRnd" ):
      simFlags.RandomSeedList.addSeed( "TrkExRnd" , 12412330, 37849324 )
    kwargs.setdefault("RandomNumberService"   , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"      , 'TrkExRnd' ) # TODO: read stream name "TrkExRnd" from Fatras jobProperties
    from ISF_FatrasTools.ISF_FatrasToolsConf import iFatras__MultipleScatteringSamplerHighland
    return iFatras__MultipleScatteringSamplerHighland(name, **kwargs )

# Not used anywhere - not migrated to CA config
def getFatrasMultipleScatteringSamplerGaussianMixture(name="ISF_MultipleScatteringSamplerGaussianMixture", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    if not simFlags.RandomSeedList.checkForExistingSeed( "TrkExRnd" ):
      simFlags.RandomSeedList.addSeed( "TrkExRnd" , 12412330, 37849324 )
    kwargs.setdefault("RandomNumberService"   , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"      , 'TrkExRnd' ) # TODO: read stream name "TrkExRnd" from Fatras jobProperties
    from ISF_FatrasTools.ISF_FatrasToolsConf import iFatras__MultipleScatteringSamplerGaussianMixture
    return iFatras__MultipleScatteringSamplerGaussianMixture(name, **kwargs )

# Not used anywhere - not migrated to CA config
def getFatrasMultipleScatteringSamplerGeneralMixture(name="ISF_MultipleScatteringSamplerGeneralMixture", **kwargs):
    from G4AtlasApps.SimFlags import simFlags
    if not simFlags.RandomSeedList.checkForExistingSeed( "TrkExRnd" ):
      simFlags.RandomSeedList.addSeed( "TrkExRnd" , 12412330, 37849324 )
    kwargs.setdefault("RandomNumberService"   , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"      , 'TrkExRnd' ) # TODO: read stream name "TrkExRnd" from Fatras jobProperties
    from ISF_FatrasTools.ISF_FatrasToolsConf import iFatras__MultipleScatteringSamplerGeneralMixture
    return iFatras__MultipleScatteringSamplerGeneralMixture(name, **kwargs )

# Combining all in the MaterialEffectsUpdator
def getFatrasMaterialUpdator(name="ISF_FatrasMaterialUpdator", **kwargs):

    from G4AtlasApps.SimFlags import simFlags

    from AthenaCommon.AlgSequence import AthSequencer
    condSeq = AthSequencer("AthCondSeq")

    # Decide if we want to use the Tracking Geometry
    # from conditions or not
    if (ISF_Flags.UseTrackingGeometryCond
            and 'TrackingGeometryReadKey' not in kwargs):
        if not hasattr(condSeq, 'AtlasTrackingGeometryCondAlg'):
            from InDetCondFolders import InDetAlignFolders_FATRAS  # noqa: F401
            from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlg import (
                ConfiguredTrackingGeometryCondAlg)
            TrkGeoCondAlg = ConfiguredTrackingGeometryCondAlg(
                'AtlasTrackingGeometryCondAlg')
            condSeq += TrkGeoCondAlg

        kwargs.setdefault('TrackingGeometryReadKey',
                          condSeq.AtlasTrackingGeometryCondAlg.TrackingGeometryWriteKey)
    elif 'TrackingGeometrySvc' not in kwargs:
        from TrkDetDescrSvc.AtlasTrackingGeometrySvc import AtlasTrackingGeometrySvc
        kwargs.setdefault('TrackingGeometrySvc', AtlasTrackingGeometrySvc)
        kwargs.setdefault('TrackingGeometryReadKey', '')

    kwargs.setdefault("RandomNumberService" , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"    , ISF_FatrasFlags.RandomStreamName())
    kwargs.setdefault("ParticleBroker"              , ISF_Flags.ParticleBroker())
    kwargs.setdefault("TruthRecordSvc"              , simFlags.TruthStrategy.TruthServiceName())
    # hadronic interactions
    kwargs.setdefault("HadronicInteraction"         , True)
    #kwargs.setdefault("HadronicInteractionProcessor", getPublicTool('ISF_FatrasParametricHadIntProcessor'))
    kwargs.setdefault("HadronicInteractionProcessor", getPublicTool('ISF_FatrasG4HadIntProcessor'))
    # enrgy loss
    kwargs.setdefault("EnergyLoss"                  , True)
    kwargs.setdefault("EnergyLossUpdator"           , getPublicTool('ISF_FatrasEnergyLossUpdator'))
    # mutiple scattering
    kwargs.setdefault("MultipleScattering"          , True)
    kwargs.setdefault("MultipleScatteringUpdator"   , getPublicTool('ISF_FatrasMultipleScatteringUpdator'))
    # photon conversion
    kwargs.setdefault("PhotonConversionTool"        , getPublicTool('ISF_FatrasConversionCreator'))
    # the validation output
    kwargs.setdefault("ValidationMode"              , ISF_Flags.ValidationMode())
    kwargs.setdefault("BremPhotonValidation"        , False)
    kwargs.setdefault("EnergyDepositValidation"     , False)
    # the properties given throuth the JobProperties interface
    kwargs.setdefault("MomentumCut"                 , FatrasTuningFlags.MomCutOffSec())
    kwargs.setdefault("MinimumBremPhotonMomentum"   , FatrasTuningFlags.MomCutOffSec())
    #
    kwargs.setdefault("PhysicsValidationTool"       , getPublicTool('ISF_FatrasPhysicsValidationTool'))
    kwargs.setdefault("ProcessSamplingTool"         , getPublicTool('ISF_FatrasProcessSamplingTool'))
    kwargs.setdefault("ParticleDecayHelper"             , getPublicTool('ISF_FatrasParticleDecayHelper'))
    # MCTruth Process Code
    kwargs.setdefault("BremProcessCode"             , 3) # TODO: to be taken from central definition

    from ISF_FatrasTools.ISF_FatrasToolsConf import iFatras__McMaterialEffectsUpdator
    return iFatras__McMaterialEffectsUpdator(name, **kwargs )

def getFatrasSTEP_Propagator(name="ISF_FatrasSTEP_Propagator", **kwargs):
    kwargs.setdefault("MomentumCutOff"                 , FatrasTuningFlags.MomCutOffSec() )
    kwargs.setdefault("SimulationMode"                 , True )
    kwargs.setdefault("SimMatEffUpdator"               , getPublicTool('ISF_FatrasMaterialUpdator') )
    # the step propagaor
    from TrkExSTEP_Propagator.TrkExSTEP_PropagatorConf import Trk__STEP_Propagator as StepPropagator
    return StepPropagator(name, **kwargs )

# the definition of an extrapolator (to be cleaned up)
def getFatrasExtrapolator(name="ISF_FatrasExtrapolator", **kwargs):
    # Charged Transport Tool
    # assign the tools
    kwargs.setdefault("Navigator"               , getPublicTool('ISF_FatrasNavigator'))
    kwargs.setdefault("MaterialEffectsUpdators" , [ getPublicTool('ISF_FatrasMaterialUpdator') ])
    kwargs.setdefault("Propagators"             , [ getPublicTool('ISF_FatrasChargedPropagator') ])
    kwargs.setdefault("STEP_Propagator"         , getPublicTool('ISF_FatrasSTEP_Propagator'))
    # Fatras specific: stop the trajectory
    kwargs.setdefault("StopWithNavigationBreak" , True)
    kwargs.setdefault("StopWithUpdateKill"      , True)
    kwargs.setdefault("RobustSampling"          , True)
    kwargs.setdefault("ResolveMuonStation"      , True)
    kwargs.setdefault("UseMuonMatApproximation" , True)

    from TrkExTools.TrkExToolsConf import Trk__TimedExtrapolator as TimedExtrapolator
    return TimedExtrapolator(name, **kwargs )
    #from TrkExTools.TrkExToolsConf import Trk__Extrapolator as Extrapolator
    #return Extrapolator(name, **kwargs )

################################################################################
# HIT CREATION SECTION
################################################################################

#   Fatras Hadronic Interaction Processor
#   hadronic interaction creator
def getFatrasHitCreatorPixel(name="ISF_FatrasHitCreatorPixel", **kwargs):
    bare_collection_name = "PixelHits"
    mergeable_collection_suffix = "_Fatras"
    merger_input_property = "PixelHits"
    region = 'ID'
    hits_collection_name = generate_mergeable_collection_name(bare_collection_name,
                                                              mergeable_collection_suffix,
                                                              merger_input_property,
                                                              region)

    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("RandomNumberService" , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"    , ISF_FatrasFlags.RandomStreamName())
    kwargs.setdefault("IdHelperName"    , 'PixelID')
    kwargs.setdefault("CollectionName"  , hits_collection_name)

    kwargs.setdefault("ConditionsTool", "")
    kwargs.setdefault("UseConditionsTool", False)

    from ISF_FatrasToolsID.ISF_FatrasToolsIDConf import iFatras__HitCreatorSilicon
    return iFatras__HitCreatorSilicon(name, **kwargs )

def getFatrasHitCreatorSCT(name="ISF_FatrasHitCreatorSCT", **kwargs):
    bare_collection_name = "SCT_Hits"
    mergeable_collection_suffix = "_Fatras"
    merger_input_property = "SCTHits"
    region = 'ID'
    hits_collection_name = generate_mergeable_collection_name(bare_collection_name,
                                                              mergeable_collection_suffix,
                                                              merger_input_property,
                                                              region)
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("RandomNumberService" , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"    , ISF_FatrasFlags.RandomStreamName())
    kwargs.setdefault("IdHelperName"    , 'SCT_ID')
    kwargs.setdefault("CollectionName"  , hits_collection_name)

    kwargs.setdefault("ConditionsTool", "")
    kwargs.setdefault("UseConditionsTool", False)

    from ISF_FatrasToolsID.ISF_FatrasToolsIDConf import iFatras__HitCreatorSilicon
    return iFatras__HitCreatorSilicon(name, **kwargs )

def getFatrasHitCreatorTRT(name="ISF_FatrasHitCreatorTRT", **kwargs):
    bare_collection_name = "TRTUncompressedHits"
    mergeable_collection_suffix = "_Fatras"
    merger_input_property = "TRTUncompressedHits"
    region = 'ID'
    hits_collection_name = generate_mergeable_collection_name(bare_collection_name,
                                                              mergeable_collection_suffix,
                                                              merger_input_property,
                                                              region)
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("RandomNumberService" , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"    , ISF_FatrasFlags.RandomStreamName())
    kwargs.setdefault("CollectionName"  , hits_collection_name)

    kwargs.setdefault("StrawStatusSummaryTool", "")

    from ISF_FatrasToolsID.ISF_FatrasToolsIDConf import iFatras__HitCreatorTRT
    return iFatras__HitCreatorTRT(name, **kwargs )

def getFatrasPileupHitCreatorPixel(name="ISF_FatrasPileupHitCreatorPixel", **kwargs):
    kwargs.setdefault("CollectionName"  , 'PileupPixelHits')
    return getFatrasHitCreatorPixel(name, **kwargs)

def getFatrasPileupHitCreatorSCT(name="ISF_FatrasPileupHitCreatorSCT", **kwargs):
    kwargs.setdefault("CollectionName"  , 'PileupSCT_Hits')
    return getFatrasHitCreatorSCT(name, **kwargs)

def getFatrasPileupHitCreatorTRT(name="ISF_FatrasPileupHitCreatorTRT", **kwargs):
    kwargs.setdefault("CollectionName"  , 'PileupTRTUncompressedHits')
    return getFatrasHitCreatorTRT(name, **kwargs)

################################################################################
# TRACK CREATION SECTION
################################################################################
def getFatrasSimHitCreatorID(name="ISF_FatrasSimHitCreatorID", **kwargs):
    kwargs.setdefault("PixelHitCreator" , getPublicTool('ISF_FatrasHitCreatorPixel'))
    kwargs.setdefault("SctHitCreator"   , getPublicTool('ISF_FatrasHitCreatorSCT'))
    kwargs.setdefault("TrtHitCreator"   , getPublicTool('ISF_FatrasHitCreatorTRT'))
    kwargs.setdefault("OutputLevel"     , ISF_FatrasFlags.OutputLevelGeneral())

    from ISF_FatrasToolsID.ISF_FatrasToolsIDConf import iFatras__SimHitCreatorID
    return iFatras__SimHitCreatorID(name, **kwargs )

def getFatrasPileupSimHitCreatorID(name="ISF_FatrasPileupSimHitCreatorID", **kwargs):
    kwargs.setdefault("PixelHitCreator" , getPublicTool('ISF_FatrasPileupHitCreatorPixel'))
    kwargs.setdefault("SctHitCreator"   , getPublicTool('ISF_FatrasPileupHitCreatorSCT'))
    kwargs.setdefault("TrtHitCreator"   , getPublicTool('ISF_FatrasPileupHitCreatorTRT'))
    return getFatrasSimHitCreatorID(name, **kwargs )

def getFatrasSimHitCreatorMS(name="ISF_FatrasSimHitCreatorMS", **kwargs):
    mergeable_collection_suffix = "_Fatras"
    region = 'MUON'

    mdt_bare_collection_name = "MDT_Hits"
    mdt_merger_input_property = "MDTHits"
    mdt_hits_collection_name = generate_mergeable_collection_name(mdt_bare_collection_name,
                                                                  mergeable_collection_suffix,
                                                                  mdt_merger_input_property,
                                                                  region)
    rpc_bare_collection_name = "RPC_Hits"
    rpc_merger_input_property = "RPCHits"
    rpc_hits_collection_name = generate_mergeable_collection_name(rpc_bare_collection_name,
                                                                  mergeable_collection_suffix,
                                                                  rpc_merger_input_property,
                                                                  region)
    tgc_bare_collection_name = "TGC_Hits"
    tgc_merger_input_property = "TGCHits"
    tgc_hits_collection_name = generate_mergeable_collection_name(tgc_bare_collection_name,
                                                                  mergeable_collection_suffix,
                                                                  tgc_merger_input_property,
                                                                  region)
    from AtlasGeoModel.MuonGMJobProperties import MuonGeometryFlags
    if MuonGeometryFlags.hasCSC() and DetFlags.simulate.CSC_on():
        csc_bare_collection_name = "CSC_Hits"
        csc_merger_input_property = "CSCHits"
        csc_hits_collection_name = generate_mergeable_collection_name(csc_bare_collection_name,
                                                                      mergeable_collection_suffix,
                                                                      csc_merger_input_property,
                                                                      region)
    else:
        csc_hits_collection_name = "CSC_Hits_Fatras"

    if MuonGeometryFlags.hasSTGC() and DetFlags.simulate.sTGC_on():
        stgc_bare_collection_name = "sTGC_Hits"
        stgc_merger_input_property = "sTGCHits"
        stgc_hits_collection_name = generate_mergeable_collection_name(stgc_bare_collection_name,
                                                                  mergeable_collection_suffix,
                                                                  stgc_merger_input_property,
                                                                  region)
    else:
        stgc_hits_collection_name = "sTGC_Hits_Fatras"

    if MuonGeometryFlags.hasMM() and DetFlags.simulate.MM_on():
        mm_bare_collection_name = "MM_Hits"
        mm_merger_input_property = "MMHits"
        mm_hits_collection_name = generate_mergeable_collection_name(mm_bare_collection_name,
                                                                  mergeable_collection_suffix,
                                                                  mm_merger_input_property,
                                                                  region)
    else:
        mm_hits_collection_name = "MM_Hits_Fatras"

    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault("RandomNumberService" , simFlags.RandomSvc() )
    kwargs.setdefault("RandomStreamName"    , ISF_FatrasFlags.RandomStreamName())
    kwargs.setdefault("Extrapolator" , getPublicTool('ISF_FatrasExtrapolator'))
    kwargs.setdefault("MDTCollectionName", mdt_hits_collection_name)
    kwargs.setdefault("RPCCollectionName", rpc_hits_collection_name)
    kwargs.setdefault("TGCCollectionName", tgc_hits_collection_name)
    kwargs.setdefault("CSCCollectionName", csc_hits_collection_name)
    kwargs.setdefault("sTGCCollectionName", stgc_hits_collection_name)
    kwargs.setdefault("MMCollectionName", mm_hits_collection_name)

    from MuonTGRecTools.MuonTGRecToolsConf import Muon__MuonTGMeasurementTool
    MuonTGMeasurementTool = Muon__MuonTGMeasurementTool(  name = 'MuonTGMeasurementTool', 
                                                          UseDSManager = True )
    from AthenaCommon.AppMgr import ToolSvc
    ToolSvc += MuonTGMeasurementTool
    kwargs.setdefault("MeasurementTool", MuonTGMeasurementTool)

    from ISF_FatrasToolsMS.ISF_FatrasToolsMSConf import iFatras__SimHitCreatorMS
    return iFatras__SimHitCreatorMS(name, **kwargs )

################################################################################
# SIMULATION TOOL and SERVICE
################################################################################
def getFatrasKinematicFilter(name="ISF_FatrasKinematicFilter", **kwargs):
    kwargs.setdefault("MaxEtaSymmetric" , 10.)
    kwargs.setdefault("MinMomentum"     , FatrasTuningFlags.MomCutOffSec())

    from ISF_Tools.ISF_ToolsConf import ISF__KinematicParticleFilter
    return ISF__KinematicParticleFilter(name, **kwargs )

def getFatrasSimTool(name="ISF_FatrasSimTool", **kwargs):
    kwargs.setdefault("SimHitCreatorID" , getPublicTool('ISF_FatrasSimHitCreatorID'))
    kwargs.setdefault("SimHitCreatorMS" , getPublicTool('ISF_FatrasSimHitCreatorMS'))
    # TODO: G4 Tools can not be used at the same time as Geant4 inside ISF
    kwargs.setdefault("ParticleDecayHelper" , getPublicTool('ISF_FatrasParticleDecayHelper'))
    kwargs.setdefault("ParticleHelper"      , getPublicTool('ISF_ParticleHelper'))
    # the filter setup
    kwargs.setdefault("TrackFilter"         , getPublicTool('ISF_FatrasKinematicFilter'))
    kwargs.setdefault("NeutralFilter"       , getPublicTool('ISF_FatrasKinematicFilter'))
    kwargs.setdefault("PhotonFilter"        , getPublicTool('ISF_FatrasKinematicFilter'))
    # extrapolator - test setup
    kwargs.setdefault("Extrapolator"           , getPublicTool('ISF_FatrasExtrapolator'))
    #
    kwargs.setdefault("PhysicsValidationTool"       , getPublicTool('ISF_FatrasPhysicsValidationTool'))
    kwargs.setdefault("ProcessSamplingTool"         , getPublicTool('ISF_FatrasProcessSamplingTool'))
    # set the output level
    kwargs.setdefault("OutputLevel"         , ISF_FatrasFlags.OutputLevelGeneral())
    kwargs.setdefault("ValidationOutput"              , ISF_Flags.ValidationMode())
    # random number service
    from G4AtlasApps.SimFlags import simFlags
    kwargs.setdefault( "RandomNumberService", simFlags.RandomSvc())

    from ISF_FatrasTools.ISF_FatrasToolsConf import iFatras__TransportTool
    return iFatras__TransportTool(name, **kwargs )

def getFatrasPileupSimTool(name="ISF_FatrasPileupSimTool", **kwargs):
    kwargs.setdefault("SimHitCreatorID" , getPublicTool('ISF_FatrasPileupSimHitCreatorID'))
    return getFatrasSimTool(name, **kwargs)


## FatrasSimulationSvc Configurations
def getFatrasSimServiceID(name="ISF_FatrasSimSvc", **kwargs):
    kwargs.setdefault("Identifier"      , "Fatras")
    kwargs.setdefault("SimulatorTool"  , "ISF_FatrasSimulatorToolST")
    from AthenaCommon import CfgMgr
    return CfgMgr.ISF__LegacySimSvc(name, **kwargs )


# Not used anywhere - not migrated to CA config
def getFatrasGeoIDFixSimServiceID(name="ISF_FatrasGeoIDFixSimSvc", **kwargs):
    kwargs.setdefault("EnableGeoIDOverride"      , True  )
    kwargs.setdefault("GeoIDOverrideZ"           , 3150. )
    kwargs.setdefault("GeoIDOverride"            , 3     ) # ISF::fAtlasCalo
    return getFatrasSimServiceID(name, **kwargs )


def getFatrasPileupSimServiceID(name="ISF_FatrasPileupSimSvc", **kwargs):
    kwargs.setdefault("SimulatorTool"  , "ISF_FatrasPileupSimulatorToolST")
    return getFatrasSimServiceID(name, **kwargs)


## FatrasSimulatorTool Configurations
def getFatrasSimulatorToolST(name="ISF_FatrasSimulatorToolST", **kwargs):
    kwargs.setdefault("IDSimulationTool"  , getPublicTool('ISF_FatrasSimTool'))
    kwargs.setdefault("SimulationTool"  , getPublicTool('ISF_FatrasSimTool'))
    # set the output level
    kwargs.setdefault("OutputLevel"         , ISF_FatrasFlags.OutputLevelGeneral())
    # register Fatras random number stream if not already registered
    from G4AtlasApps.SimFlags import simFlags
    if not simFlags.RandomSeedList.checkForExistingSeed( "FatrasRnd" ):
      simFlags.RandomSeedList.addSeed( "FatrasRnd", 81234740, 23474923 )
    return CfgMgr.ISF__FatrasSimTool(name, **kwargs)

# Not used anywhere - not migrated to CA config
def getFatrasSimulatorTool(name="ISF_FatrasSimulatorTool", **kwargs):
    kwargs.setdefault("IDSimulationSelectors"       , [ 'ISF_DefaultFatrasSelector' ] )
    kwargs.setdefault("CaloSimulationSelectors"     , [ 'ISF_MuonFatrasSelector' ]    )
    kwargs.setdefault("MSSimulationSelectors"       , [ 'ISF_DefaultFatrasSelector' ] )
    return getFatrasSimulatorToolST(name, **kwargs)


def getFatrasPileupSimulatorToolST(name="ISF_FatrasPileupSimulatorToolST", **kwargs):
    kwargs.setdefault("IDSimulationTool", getPublicTool('ISF_FatrasPileupSimTool'))
    kwargs.setdefault("SimulationTool"  , getPublicTool('ISF_FatrasSimTool'))
    return getFatrasSimulatorToolST(name, **kwargs)


def getFatrasPileupSimulatorTool(name="ISF_FatrasPileupSimulatorTool", **kwargs):
    kwargs.setdefault("IDSimulationSelectors"       , [ 'ISF_DefaultFatrasSelector' ] )
    kwargs.setdefault("CaloSimulationSelectors"     , [ 'ISF_MuonFatrasSelector' ]    )
    kwargs.setdefault("MSSimulationSelectors"       , [ 'ISF_DefaultFatrasSelector' ] )
    return getFatrasPileupSimulatorToolST(name, **kwargs)


#def getFatrasPileupSimulatorTool_noHits(name="ISF_FatrasPileupTool_noHits", **kwargs):
#    kwargs.setdefault("SimService", "ISF_FatrasPileupSimSvc_noHits" )
#    return CfgMgr.ISF__FatrasSimTool(name, **kwargs)

