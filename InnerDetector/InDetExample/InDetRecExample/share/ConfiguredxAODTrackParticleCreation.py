# Blocking the include for after first inclusion
include.block ('InDetRecExample/ConfiguredxAODTrackParticleCreation.py')

# ------------------------------------------------------------
#
# ----------- Setup InDet xAOD::TrackParticle Creation
#
# ------------------------------------------------------------

class ConfiguredxAODTrackParticleCreation:

     def __init__(self, InputTrackCollection = None, InputTrackTruthCollection = None, OutputTrackParticleContainer = None, ClusterSplitProbabilityName = "", AssociationMapName = ""):


         from InDetRecExample.InDetJobProperties import InDetFlags
         from InDetRecExample.InDetKeys import InDetKeys
         #
         # --- get ToolSvc and topSequence
         #
         from AthenaCommon.AppMgr                import ToolSvc
         from AthenaCommon.AlgSequence           import AlgSequence
         from InDetRecExample                    import TrackingCommon
         topSequence = AlgSequence()

         #Always the same (so far) so can in principle go in InDetRecLoadTools
         from InDetRecExample.TrackingCommon import getInDetxAODParticleCreatorTool
         InDetxAODParticleCreatorTool = getInDetxAODParticleCreatorTool(suffix=InputTrackCollection)

         ToolSvc += InDetxAODParticleCreatorTool
         if (InDetFlags.doPrintConfigurables()):
            printfunc (InDetxAODParticleCreatorTool)

         from xAODTrackingCnv.xAODTrackingCnvConf import xAODMaker__TrackCollectionCnvTool
         InDetTrackCollectionCnvTool = xAODMaker__TrackCollectionCnvTool("InDetTrackCollectionCnvTool"+InputTrackCollection,
                                                                         TrackParticleCreator = InDetxAODParticleCreatorTool)


         from xAODTrackingCnv.xAODTrackingCnvConf import xAODMaker__TrackParticleCnvAlg
         xAODTrackParticleCnvAlg = xAODMaker__TrackParticleCnvAlg(name = "InDetxAODParticleCreatorAlg"+InputTrackCollection,
                                                                  ConvertTracks = True,
                                                                  ConvertTrackParticles = False,
                                                                  TrackContainerName = InputTrackCollection,
                                                                  xAODContainerName = OutputTrackParticleContainer,
                                                                  xAODTrackParticlesFromTracksContainerName = OutputTrackParticleContainer,
                                                                  TrackParticleCreator = InDetxAODParticleCreatorTool,
                                                                  TrackCollectionCnvTool = InDetTrackCollectionCnvTool)

         if (InDetFlags.doTruth() and not InputTrackTruthCollection == ''):
             xAODTrackParticleCnvAlg.AddTruthLink = True
             xAODTrackParticleCnvAlg.TrackTruthContainerName = InputTrackTruthCollection

             from MCTruthClassifier.MCTruthClassifierBase import MCTruthClassifier
             xAODTrackParticleCnvAlg.MCTruthClassifier = MCTruthClassifier

         elif (InDetFlags.doTruth() and InputTrackTruthCollection == ''):
             printfunc ("WARNING: ConfiguredxAODTrackParticleCreation - doTruth = True, but no input Truth collection specified!")
         else:
            xAODTrackParticleCnvAlg.AddTruthLink = False

         if InDetFlags.doTIDE_AmbiTrackMonitoring():
             xAODTrackParticleCnvAlg.AugmentObservedTracks = True
             xAODTrackParticleCnvAlg.TracksMapName = InDetKeys.ObservedTracks()+"Map"

         topSequence += xAODTrackParticleCnvAlg
         if (InDetFlags.doPrintConfigurables()):
            printfunc (xAODTrackParticleCnvAlg)
