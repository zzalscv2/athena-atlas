#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

'''@file MissingEtDAQConfig.py
@author T. Strebler
@date 2022-06-16
@brief Main CA-based python configuration for MissingEtDQA
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PhysValMETCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("EnableLumi", False)
    kwargs.setdefault("DetailLevel", 10)
    kwargs.setdefault("DoTruth", flags.Input.isMC)

    kwargs.setdefault("JVTToolEMTopo", CompFactory.JetVertexTaggerTool(name="JVTToolEMTopo",
                                                                       JetContainer="AntiKt4EMTopoJets") )
    kwargs.setdefault("JVTToolEMPFlow", CompFactory.JetVertexTaggerTool(name="JVTToolPFlow",
                                                                        JetContainer="AntiKt4EMPFlowJets") )

    from METUtilities.METMakerConfig import getMETMaker
    # for EMTopo jets no NNJvt is calculated so we need to fall back to Jvt (re-calculated in MissingEtDQA::PhysValMET as "NewJvt")
    kwargs.setdefault("METMakerTopo", getMETMaker(name="METMaker_AntiKt4Topo",
                                                  JetSelection="Loose",
                                                  UseR21JvtFallback=True,
                                                  JetJvtMomentName="NewJvt",
                                                  DoPFlow=False) )
    kwargs.setdefault("METMakerPFlow", getMETMaker(name="METMaker_AntiKt4PFlow",
                                                   JetSelection="Loose",
                                                   DoPFlow=True) )

    from METUtilities.METMakerConfig import getMuonSelectionTool, getEleSelLikelihood, getPhotonSelIsEM, getTauSelectionTool
    kwargs.setdefault("MuonSelectionTool", getMuonSelectionTool())
    kwargs.setdefault("ElectronLHSelectionTool", getEleSelLikelihood())
    kwargs.setdefault("PhotonIsEMSelectionTool", getPhotonSelIsEM())
    kwargs.setdefault("TauSelectionTool", getTauSelectionTool())

    from AthenaConfiguration.AutoConfigFlags import GetFileMD
    metadata = GetFileMD(flags.Input.Files)
    isDAOD_PHYSVAL=False
    for class_name, name in metadata['metadata_items'].items():
        if name == 'EventStreamInfo':
            if "DAOD_PHYSVAL" in class_name :
                print ("Running on DAOD_PHYSVAL - will not add TTVA decorations.")
                isDAOD_PHYSVAL=True
            break
    kwargs.setdefault("InputIsDAOD", isDAOD_PHYSVAL)

    kwargs.setdefault("DoMETRefPlots", "xAOD::MissingETContainer#MET_Reference_AntiKt4EMTopo" in flags.Input.TypedCollections)

    tool = CompFactory.MissingEtDQA.PhysValMET(**kwargs)
    acc.setPrivateTools(tool)
    return acc
