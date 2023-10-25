from AthenaCommon.Logging import logging


mlog = logging.getLogger( 'CombinedRec_config' )


from AthenaCommon.GlobalFlags  import globalflags
from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
from RecExConfig.RecFlags import rec
from RecExConfig.RecAlgsFlags import recAlgs
from RecExConfig.ObjKeyStore import objKeyStore

from AthenaCommon.Resilience import treatException,protectedInclude


# use to flag domain
import PerfMonComps.DomainsRegistry as pdr

from AODFix.AODFix import *
AODFix_Init()

from CaloRec.CaloRecFlags import jobproperties

#
# functionality : CaloExtensionBuilder setup
# to be used  in tau, pflow, e/gamma
#
pdr.flag_domain('CaloExtensionBuilder')
if (rec.doESD()) and (recAlgs.doEFlow() or rec.doTau() or rec.doEgamma()) : #   or rec.readESD()
    try:
        from TrackToCalo.CaloExtensionBuilderAlgConfig import CaloExtensionBuilder
        CaloExtensionBuilder(False)
    except Exception:
        treatException("Cannot include CaloExtensionBuilder !")

    #Now setup Large Radius Tracks version (LRT), only if LRT enabled    
    from InDetRecExample.InDetJobProperties import InDetFlags
    if InDetFlags.doR3LargeD0() and InDetFlags.storeSeparateLargeD0Container():
        #CaloExtensionBuilder was already imported above, and an exception would have been thrown
        #if that had failed.
        CaloExtensionBuilder(True)

#
# functionality : electron photon identification
#
#
from InDetRecExample.InDetJobProperties import InDetFlags
 
pdr.flag_domain('egamma')
if rec.doEgamma():
    protectedInclude( "egammaRec/egammaRec_jobOptions.py" )
    if InDetFlags.doR3LargeD0() and InDetFlags.storeSeparateLargeD0Container():
        protectedInclude( "egammaRec/egammaLRTRec_jobOptions.py" )
AODFix_postEgammaRec()


#
# functionality : Muon combined reconstruction
#
pdr.flag_domain('muoncomb')
if rec.doMuonCombined() and DetFlags.Muon_on() and DetFlags.ID_on():
    try:
        include ("MuonCombinedRecExample/MuonCombinedRec_config.py")
    except Exception:
        treatException("Could not set up combined muon reconstruction. Switched off !")
        rec.doMuonCombined = False

#
#  functionality : add cells crossed by high pt ID tracks
#
if rec.doESD() and recAlgs.doTrackParticleCellAssociation() and DetFlags.ID_on() and DetFlags.Muon_on() and DetFlags.Calo_on():
    from AthenaCommon.CfgGetter import getPublicTool
    getPublicTool("MuonCombinedInDetDetailedTrackSelectorTool")
    from TrkExTools.AtlasExtrapolator import AtlasExtrapolator
    from TrackToCalo.TrackToCaloConf import Trk__ParticleCaloExtensionTool, Rec__ParticleCaloCellAssociationTool
    pcExtensionTool = Trk__ParticleCaloExtensionTool(Extrapolator = AtlasExtrapolator())
    caloCellAssociationTool = Rec__ParticleCaloCellAssociationTool(ParticleCaloExtensionTool = pcExtensionTool)

    topSequence += CfgMgr.TrackParticleCellAssociationAlg("TrackParticleCellAssociationAlg",
                                                          ParticleCaloCellAssociationTool=caloCellAssociationTool)

    from InDetRecExample.InDetJobProperties import InDetFlags
    if InDetFlags.doR3LargeD0() and InDetFlags.storeSeparateLargeD0Container():
        topSequence += CfgMgr.TrackParticleCellAssociationAlg("LargeD0TrackParticleCellAssociationAlg",
                                                              TrackParticleContainerName="InDetLargeD0TrackParticles",
                                                              ClusterContainerName="InDetLargeD0TrackParticlesAssociatedClusters",
                                                              CaloClusterCellLinkName="InDetLargeD0TrackParticlesAssociatedClusters_links",
                                                              AssociationContainerName="InDetLargeD0TrackParticlesClusterAssociations",
                                                              ParticleCaloCellAssociationTool=caloCellAssociationTool)
    if DetFlags.Muon_on():
        from AthenaCommon.CfgGetter import getPublicTool
        getPublicTool("MuonCombinedInDetDetailedTrackSelectorTool")

#
# functionality : energy flow
#
pdr.flag_domain('eflow')
if recAlgs.doEFlow() and (rec.readESD() or (DetFlags.haveRIO.ID_on() and DetFlags.haveRIO.Calo_allOn() and rec.doMuonCombined())):
    try:
        include( "eflowRec/eflowRec_jobOptions.py" )
    except Exception:
        treatException("Could not set up EFlow. Switched off !")
        recAlgs.doEFlow=False
else:
    recAlgs.doEFlow=False

#AODFix_postMuonCombinedRec()
#
# functionality : CaloTower protojets + preclustering + KT algorithm + CombinedJetAlg
#
pdr.flag_domain('jet')
jetOK=False
if rec.doJetMissingETTag() and DetFlags.Calo_on():
    try:
        from JetRec.JetRecFlags import jetFlags
        if jetFlags.Enabled():
            include( "JetRec/JetRec_jobOptions.py" )
            jetOK=jetFlags.Enabled()
    except Exception:
        treatException("Could not set up jet reconstruction")
        jetOK=False
    AODFix_postJetRec()

#
# functionality : isolation for egamma and combined muon
#
pdr.flag_domain('egmiso')
if (rec.doESD() and (rec.doMuonCombined() or rec.doEgamma()) and
    (jobproperties.CaloRecFlags.doCaloTopoCluster() or
     objKeyStore.isInInput ('xAOD::ParticleContainer', 'CaloCalTopoClusters'))):
    try:
        from IsolationAlgs.IsoGetter import isoGetter
        isoGetter()
    except Exception:
        treatException("Could not set up isolation. Switched off !")

if jetOK and recAlgs.doMuonSpShower() and DetFlags.detdescr.Muon_on() and DetFlags.haveRIO.Calo_on() :
    try:
        include("MuonSpShowerBuilderAlgs/MuonSpShowerBuilder_jobOptions.py")
    except Exception:
        treatException("Could not set up MuonSpShower. Switched off !")
        recAlgs.doMuonSpShower=False
else:
    recAlgs.doMuonSpShower=False

pdr.flag_domain('btagging')
btaggingOK = False
#By default disable b-tagging from ESD, unless user has set it and locked it to true upstream
if rec.readESD():
    rec.doBTagging=False
if (jetOK or rec.readESD()) and rec.doBTagging() and  DetFlags.ID_on() and DetFlags.Muon_on():
    try:
        from AthenaCommon.Configurable import Configurable
        Configurable.configurableRun3Behavior=1
        from AthenaConfiguration.OldFlags2NewFlags import getNewConfigFlags
        # Translate all needed flags from old jobProperties to a new AthConfigFlag Container
        ConfigFlags = getNewConfigFlags()
        # Additional b-tagging related flags
        ConfigFlags.BTagging.SaveSV1Probabilities = True
        ConfigFlags.BTagging.RunJetFitterNN = True
        # Configure BTagging algorithm
        from BTagging.BTagRun3Config import BTagRecoSplitCfg
        CAtoGlobalWrapper(BTagRecoSplitCfg, ConfigFlags)
    except Exception:
        treatException("Could not set up btagging reconstruction")
        btaggingOK=False
    finally:
        Configurable.configurableRun3Behavior=0
    pass

# Hits associated with high-pt jets for trackless b-tagging
from BTagging.BTaggingFlags import BTaggingFlags
if (jetOK or rec.readESD()) and DetFlags.ID_on() and rec.doWriteAOD() and BTaggingFlags.DoJetHitAssociation:
    try:
        include("JetHitAssociation/jetHitAssociation_config.py")
    except Exception:
        treatException("Could not set up jet hit association")

#
# functionality : tau identification
#
pdr.flag_domain('tau')
if jetOK and rec.doTau():
    protectedInclude ("tauRec/tauRec_config.py")
AODFix_posttauRec()

#
# functionality: Flow element tau links
#
pdr.flag_domain('eflow')
if recAlgs.doEFlow():
    try:
        include( "eflowRec/tauFELinkConfig.py" )
    except Exception:
        treatException("Could not set up tau-FE links")

#
# functionality : Missing Et
#
pdr.flag_domain('jet')
if recAlgs.doMissingET() and DetFlags.Calo_on() and DetFlags.ID_on() and DetFlags.Muon_on():
    try:
        include( "METReconstruction/METReconstruction_jobOptions.py" )
    except Exception:
        treatException("Could not set up MissingET. Switched off !")
        recAlgs.doMissingET=False

else:
    recAlgs.doMissingET=False
AODFix_postMissingETRec()

#
# functionality : Missing Et significance
#

if recAlgs.doMissingETSig() and ( rec.readESD() or DetFlags.haveRIO.Calo_on()) :
  include( "MissingETSig/MissingETSig_jobOptions.py" )
else:
  recAlgs.doMissingETSig=False

#
# Functionality: CaloRinger
#
pdr.flag_domain('caloringer')
if rec.doCaloRinger:
  include('CaloRingerAlgs/CaloRinger_jobOptions.py')



