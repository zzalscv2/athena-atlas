# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

def FastCaloSimFactory(name="FastCaloSimFactory", **kwargs):

    from AthenaCommon.Logging import logging
    mlog = logging.getLogger('FastCaloSimFactory::configure:')

    from AthenaCommon.AppMgr import ToolSvc

    mlog.info("now configure the non-interacting propagator...")
    from TrkExSTEP_Propagator.TrkExSTEP_PropagatorConf import Trk__STEP_Propagator
    niPropagator = Trk__STEP_Propagator()
    niPropagator.MaterialEffects = False
    ToolSvc += niPropagator
    mlog.info("configure nono-interacting propagator finished")

    from AthenaCommon.AlgSequence import AthSequencer
    condSeq = AthSequencer("AthCondSeq")

    # use NI navigator method checks for 
    # ISF_Flags.UseTrackingGeometryCond
    from TrkExTools.TimedExtrapolator import getNINavigator
    navigator =  getNINavigator (name="FCSNavigator")
    ToolSvc += navigator

    mlog.info("now configure the TimedExtrapolator...")
    from TrkExTools.TimedExtrapolator import TimedExtrapolator
    timedExtrapolator = TimedExtrapolator()
    timedExtrapolator.STEP_Propagator = niPropagator
    timedExtrapolator.Navigator = navigator
    timedExtrapolator.ApplyMaterialEffects = False
    ToolSvc += timedExtrapolator
    mlog.info("configure TimedExtrapolator finished")

    if not hasattr(condSeq, 'CellInfoContainerCondAlg'):
        from FastCaloSim.FastCaloSimConf import CellInfoContainerCondAlg
        condSeq += CellInfoContainerCondAlg("CellInfoContainerCondAlg")

    from TrkDetDescrSvc.TrkDetDescrJobProperties import TrkDetFlags

    kwargs.setdefault("CaloEntrance", TrkDetFlags.InDetContainerName())
    kwargs.setdefault("Extrapolator", timedExtrapolator)

    from FastCaloSim.FastCaloSimConf import FastShowerCellBuilderTool
    theFastShowerCellBuilderTool = FastShowerCellBuilderTool(name, **kwargs)

    try:
        ParticleParametrizationFileName = theFastShowerCellBuilderTool.ParticleParametrizationFileName
    except Exception:
        ParticleParametrizationFileName = ""

    if ParticleParametrizationFileName == "" and len(theFastShowerCellBuilderTool.AdditionalParticleParametrizationFileNames) == 0:
        ParticleParametrizationFileName = "FastCaloSim/v1/ParticleEnergyParametrization.root"

    theFastShowerCellBuilderTool.ParticleParametrizationFileName = ParticleParametrizationFileName
    mlog.info("ParticleParametrizationFile=%s",
              ParticleParametrizationFileName)

    mlog.info("all values:")
    mlog.info(theFastShowerCellBuilderTool)

    return theFastShowerCellBuilderTool
