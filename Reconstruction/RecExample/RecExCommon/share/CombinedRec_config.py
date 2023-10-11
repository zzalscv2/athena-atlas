from AthenaCommon.Logging import logging


mlog = logging.getLogger('CombinedRec_config')


from AthenaCommon.GlobalFlags import globalflags
from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
from RecExConfig.RecFlags import rec
from RecExConfig.RecAlgsFlags import recAlgs
from RecExConfig.ObjKeyStore import objKeyStore

from AthenaCommon.Resilience import treatException,protectedInclude

# Import the new style config flags, for those domains using them.
# Note they are locked at this stage and one cannot modify them.
# Adjustments should be made before the locking in RecExCommon_topOptions.py
from AthenaConfiguration.AllConfigFlags import ConfigFlags

# use to flag domain
import PerfMonComps.DomainsRegistry as pdr

from AODFix.AODFix import *
AODFix_Init()

from CaloRec.CaloRecFlags import jobproperties
from InDetRecExample.InDetJobProperties import InDetFlags
#
# functionality : electron photon Reconstruction
#
#

pdr.flag_domain('egamma')
if rec.doEgamma() and rec.doESD():
    from egammaConfig.egammaReconstructionConfig import (
        egammaReconstructionCfg)
    CAtoGlobalWrapper(egammaReconstructionCfg, ConfigFlags)
    if InDetFlags.doR3LargeD0() and InDetFlags.storeSeparateLargeD0Container():
        from egammaConfig.egammaLRTReconstructionConfig import (
            egammaLRTReconstructionCfg)
        CAtoGlobalWrapper(egammaLRTReconstructionCfg, ConfigFlags)
AODFix_postEgammaRec()

#
# functionality : CaloExtensionBuilder setup
# to be used  in tau, pflow, e/gamma
#
pdr.flag_domain('CaloExtensionBuilder')
if  (rec.doESD()) and (recAlgs.doEFlow() or rec.doTau() or rec.doMuonCombined()):    
    try:
        from TrackToCalo.CaloExtensionBuilderAlgConfig import CaloExtensionBuilder
        CaloExtensionBuilder(False)
    except Exception:
        treatException("Cannot include CaloExtensionBuilder !")

    # Now setup Large Radius Tracks version (LRT), only if LRT enabled
    from InDetRecExample.InDetJobProperties import InDetFlags
    if InDetFlags.doR3LargeD0() and InDetFlags.storeSeparateLargeD0Container():
        # CaloExtensionBuilder was already imported above,
        # and an exception would have been thrown
        # if that had failed.
        CaloExtensionBuilder(True)


#
# functionality : Muon combined reconstruction
#
pdr.flag_domain('muoncomb')
if rec.doESD() and rec.doMuonCombined() and DetFlags.Muon_on() and DetFlags.ID_on():
    try:
        from MuonCombinedConfig.MuonCombinedReconstructionConfig import MuonCombinedReconstructionCfg
        CAtoGlobalWrapper(MuonCombinedReconstructionCfg, ConfigFlags)
    except Exception:
        treatException("Could not set up combined muon reconstruction. Switched off !")
        rec.doMuonCombined = False

#
#  functionality : add cells crossed by high pt ID tracks
#
if rec.doESD() and recAlgs.doTrackParticleCellAssociation() and DetFlags.ID_on() and DetFlags.Calo_on():
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
if recAlgs.doEFlow() and (rec.readESD() or (DetFlags.haveRIO.ID_on() and DetFlags.haveRIO.Calo_allOn())):
    try:        
        from eflowRec.PFRun3Config import PFCfg
        CAtoGlobalWrapper(PFCfg, ConfigFlags)
        #from eflowRec import ScheduleCHSPFlowMods
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

    from IsolationAlgs.IsolationSteeringConfig import IsolationSteeringCfg
    CAtoGlobalWrapper(IsolationSteeringCfg, ConfigFlags)
    
if jetOK and recAlgs.doMuonSpShower() and DetFlags.detdescr.Muon_on() and DetFlags.haveRIO.Calo_on() :
    try:
        include("MuonSpShowerBuilderAlgs/MuonSpShowerBuilder_jobOptions.py")
    except Exception:
        treatException("Could not set up MuonSpShower. Switched off !")
        recAlgs.doMuonSpShower=False
else:
    recAlgs.doMuonSpShower=False

pdr.flag_domain('btagging')
#By default disable b-tagging from ESD, unless user has set it and locked it to true upstream
if rec.readESD():
    rec.doBTagging=False
if (jetOK or rec.readESD()) and rec.doBTagging() and  DetFlags.ID_on() and DetFlags.Muon_on():
    # Configure BTagging algorithm
    from BTagging.BTagConfig import BTagRecoSplitCfg
    CAtoGlobalWrapper(BTagRecoSplitCfg, ConfigFlags)

#
# functionality : tau reconstruction
#
pdr.flag_domain('tau')
from tauRec.tauRecFlags import tauFlags
if (jetOK or tauFlags.isStandalone) and rec.doTau():
    protectedInclude ("tauRec/tauRec_config.py")    
AODFix_posttauRec()

#
# functionality: Flow element tau links
#
pdr.flag_domain('eflow')
from eflowRec.eflowRecFlags import jobproperties
if recAlgs.doEFlow() and jobproperties.eflowRecFlags.usePFFlowElementAssoc:
    try:
        from eflowRec.PFCfg import PFTauFlowElementLinkingCfg
        CAtoGlobalWrapper(PFTauFlowElementLinkingCfg,ConfigFlags)        
    except Exception:
        treatException("Could not set up tau-FE links")    

#Links to/from global FE containers created during jet finding
if(jetOK):
    try:
        from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
        CAtoGlobalWrapper(PFGlobalFlowElementLinkingCfg,ConfigFlags)
    except Exception:
        treatException("Could not setup PFGlobalFlowElementAssoc Algorithms")
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
if rec.doCaloRinger and rec.doESD():
  from CaloRingerAlgs.CaloRingerAlgsConfig import CaloRingerAlgsCfg
  CAtoGlobalWrapper(CaloRingerAlgsCfg, ConfigFlags)




