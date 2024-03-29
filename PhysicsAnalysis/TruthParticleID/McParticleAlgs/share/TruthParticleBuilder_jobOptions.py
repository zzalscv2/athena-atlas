####################################
#  TruthParticleBuilder Job Options
#  author: Sebastien Binet
####################################

import EventKernel.ParticleDataType
from ParticleBuilderOptions.AODFlags import AODFlags
from RecExConfig.ObjKeyStore import objKeyStore

# needed runtime dependancy for TruthParticleCnvTool
if not "PartPropSvc" in theApp.Dlls:
    include( "PartPropSvc/PartPropSvc.py" )
    pass

if not "ToolSvc"         in theApp.ExtSvc and \
   not "ToolSvc/ToolSvc" in theApp.ExtSvc:
    theApp.ExtSvc += [ "ToolSvc/ToolSvc"]
    pass

# make sure we have Configurable-aware jobO
# ie: hack that we'll remove someday... FIXME
from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

from McParticleAlgs.JobOptCfg import McAodBuilder,createMcAodBuilder,PileUpClassification

# If we're reprocessing, we may have TruthEvents but not xAODTruthLinks.
# Remake the links here if needed.
#   JM: In the future, this would not work. If TrutheEvents exists, it should
#       be renamed out of the way. Force rerun is effectivly always true.
# This needs to come before the McAodBuilder below; otherwise,
# xAODTruthCnvAlg will also try to rebuild TruthEvents.
if (objKeyStore.isInInput( "xAOD::TruthEventContainer", "TruthEvents" ) and
      not objKeyStore.isInInput( "xAODTruthParticleLinkVector", "xAODTruthLinks" ) ):
    from xAODTruthCnv.xAODTruthCnvConf import xAODMaker__RedoTruthLinksAlg
    job += xAODMaker__RedoTruthLinksAlg("GEN_AOD2xAOD_links")

# Create filtered GEN_AOD (EtaPt Filter) and write SpclMC TruthParticles
# TODO: disabled with HepMC3 migration
# if (objKeyStore.isInInput("McEventCollection", "TruthEvent") and
#         not objKeyStore.isInInput("McEventCollection", "GEN_AOD")):
#     job += McAodBuilder()
#     pass


# TODO: disabled with HepMC3 migration
# def getMcAODBuilder(putype):
#     """ putype is expected to be a string ! """
#     builder =  createMcAodBuilder(
#         name = "McAodBuilder"+putype,
#         outMcEvtCollection  = "GEN_AOD", # this is the input to the CnvTool
#         outTruthParticles   = "SpclMC"+putype,
#         )
#     builder.CnvTool.SelectSignalType =  PileUpClassification.fromString(putype) # min bias only

#     objKeyStore.addStreamAOD("TruthParticleContainer","SpclMC"+putype)
#     return builder


# below, build a new TruthParticleContainer for each type of pile-up.
# we get the list of existing ESD TruthParticleContainer, and guess its pile-up type
# according to its suffix. 
# TODO: disabled with HepMC3 migration
# inputTPContainer = objKeyStore['inputFile'].list("TruthParticleContainer")
# prefix = "TruthParticleContainer#INav4MomTruthEvent"
# for cont in inputTPContainer:
#     suffix = cont[len(prefix):]
#     if suffix == "":
#         # the truth SIGNAL has already been scheduled above
#         continue
#     builder = getMcAODBuilder(suffix)
#     builder.DoFiltering = False    
#     job += builder


from PyUtils.MetaReaderPeeker import metadata
buildTruthMetadata = False
if 'metadata_items' in metadata:
    metadata_items = metadata['metadata_items']
    if 'xAOD::TruthMetaDataAuxContainer_v1' in set(metadata_items.values()):
        buildTruthMetadata = True

if ((objKeyStore.isInInput("McEventCollection", "GEN_AOD") or
     objKeyStore.isInInput("McEventCollection", "TruthEvent"))):
    if not objKeyStore.isInInput("xAOD::TruthEventContainer", "TruthEvents"):
        from xAODTruthCnv.xAODTruthCnvConf import xAODMaker__xAODTruthCnvAlg
        # Pass the TruthEvent same as CA config
        # (Default GEN_AOD)
        job += xAODMaker__xAODTruthCnvAlg("GEN_AOD2xAOD",
                                          AODContainerName='TruthEvent')
    else:
        buildTruthMetadata = True

if buildTruthMetadata:
    from xAODTruthCnv.xAODTruthCnvConf import xAODMaker__TruthMetaDataTool
    ToolSvc += xAODMaker__TruthMetaDataTool( "TruthMetaDataTool" )
    svcMgr.MetaDataSvc.MetaDataTools += [ ToolSvc.TruthMetaDataTool ]
