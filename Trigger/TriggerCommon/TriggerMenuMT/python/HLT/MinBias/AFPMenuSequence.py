#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable
from AthenaConfiguration.ComponentFactory import CompFactory
from TriggerMenuMT.HLT.Config.MenuComponents import InEventRecoCA, SelectionCA, MenuSequenceCA


@AccumulatorCache
def AFPTrkRecoBaseSequenceCA(flags):
    recoAcc = InEventRecoCA(name='AFPTrackingFS')

    from AthenaCommon.GlobalFlags import globalflags
    if globalflags.InputFormat.is_bytestream():
        acc = ComponentAccumulator()

        # Bytestream converter
        AFP_Raw = CompFactory.AFP_RawDataProvider("AFP_RawDataProvider")
        acc.addEventAlgo(AFP_Raw)

        # Digitalization
        AFP_R2D = CompFactory.AFP_Raw2Digi("AFP_Raw2Digi")
        acc.addEventAlgo(AFP_R2D)

        recoAcc.mergeReco(acc)

    # Cluster reconstruction
    from AFP_SiClusterTools.AFP_SiClusterTools import AFP_SiClusterTools_HLT
    AFP_SiCl = AFP_SiClusterTools_HLT(flags)
    recoAcc.mergeReco(AFP_SiCl)

    # Tracks reconstruction
    from AFP_LocReco.AFP_LocReco import AFP_LocReco_SiD_HLT
    AFP_SID = AFP_LocReco_SiD_HLT(flags)
    recoAcc.mergeReco(AFP_SID)

    return recoAcc


@AccumulatorCache
def AFPTrkSequence(flags):
    def trigStreamerAFPHypoTool(chainDict):
        return conf2toConfigurable(CompFactory.TrigStreamerHypoTool(chainDict['chainName']))

    recoAcc = AFPTrkRecoBaseSequenceCA(flags)

    hypo = CompFactory.TrigStreamerHypoAlg('AFPPassThroughHypo')

    selAcc = SelectionCA('AFPPassThroughSequence')
    selAcc.mergeReco(recoAcc)
    selAcc.addHypoAlgo(hypo)

    return MenuSequenceCA(flags, selAcc, HypoToolGen=trigStreamerAFPHypoTool)


def TrigAFPDijetComboHypoToolCfg(flags, chainDict):
    from TrigAFPHypo.TrigAFPHypoConf import TrigAFPDijetComboHypoTool
    name = chainDict['chainName']
    tool = TrigAFPDijetComboHypoTool(name)

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool_' + name, HistPath='AFPComboHypo/' + tool.getName())
    monTool.defineHistogram('DijetMass', type='TH1F', path='EXPERT', title="Dijet mass", xbins=100, xmin=0, xmax=2000)
    monTool.defineHistogram('DijetRapidity', type='TH1F', path='EXPERT', title="Dijet rapidity", xbins=100, xmin=-5, xmax=5)

    monTool.defineHistogram('XiJet1', type='TH1F', path='EXPERT', title="Jet 1 xi", xbins=100, xmin=0, xmax=1)
    monTool.defineHistogram('XiJet2', type='TH1F', path='EXPERT', title="Jet 2 x1", xbins=100, xmin=0, xmax=1)

    monTool.defineHistogram('PredictProtonAEnergy', type='TH1F', path='EXPERT', title="Predicted proton energy A", xbins=100, xmin=0, xmax=10000)
    monTool.defineHistogram('PredictProtonCEnergy', type='TH1F', path='EXPERT', title="Predicted proton energy C", xbins=100, xmin=0, xmax=10000)

    monTool.defineHistogram('SideA_predictX', type='TH1F', path='EXPERT', title="Predicted X side A", xbins=100, xmin=-100, xmax=100)
    monTool.defineHistogram('SideA_predictY', type='TH1F', path='EXPERT', title="Predicted Y side A", xbins=100, xmin=-100, xmax=100)
    monTool.defineHistogram('SideC_predictX', type='TH1F', path='EXPERT', title="Predicted X side C", xbins=100, xmin=-100, xmax=100)
    monTool.defineHistogram('SideC_predictY', type='TH1F', path='EXPERT', title="Predicted Y side C", xbins=100, xmin=-100, xmax=100)

    monTool.defineHistogram('XDiff', type='TH1F', path='EXPERT', title="X difference", xbins=100, xmin=-100, xmax=100)
    monTool.defineHistogram('YDiff', type='TH1F', path='EXPERT', title="Y difference", xbins=100, xmin=-100, xmax=100)
    monTool.defineHistogram('distance', type='TH1F', path='EXPERT', title="distance", xbins=100, xmin=0, xmax=50)

    monTool.defineHistogram('SideA_trackX', type='TH1F', path='EXPERT', title="Track X side A", xbins=100, xmin=-100, xmax=100)
    monTool.defineHistogram('SideA_trackY', type='TH1F', path='EXPERT', title="Track Y side A", xbins=100, xmin=-100, xmax=100)
    monTool.defineHistogram('SideA_diffX', type='TH1F', path='EXPERT', title="Track X diff side A", xbins=100, xmin=-50, xmax=50)
    monTool.defineHistogram('SideA_diffY', type='TH1F', path='EXPERT', title="Track Y diff side A", xbins=100, xmin=-50, xmax=50)

    monTool.defineHistogram('SideC_trackX', type='TH1F', path='EXPERT', title="Track X side C", xbins=100, xmin=-100, xmax=100)
    monTool.defineHistogram('SideC_trackY', type='TH1F', path='EXPERT', title="Track Y side C", xbins=100, xmin=-100, xmax=100)
    monTool.defineHistogram('SideC_diffX', type='TH1F', path='EXPERT', title="Track X diff side C", xbins=100, xmin=-50, xmax=50)
    monTool.defineHistogram('SideC_diffY', type='TH1F', path='EXPERT', title="Track Y diff side C", xbins=100, xmin=-50, xmax=50)

    tool.MonTool = monTool

    return tool


@AccumulatorCache
def AFPGlobalRecoSequenceCA(flags):
    recoAcc = InEventRecoCA(name='AFPGlobalFS')

    # ToF Tracks reconstruction
    from AFP_LocReco.AFP_LocReco import AFP_LocReco_TD_HLT
    AFP_TD = AFP_LocReco_TD_HLT(flags)
    recoAcc.mergeReco(AFP_TD)

    # Protons reconstruction
    from AFP_GlobReco.AFP_GlobReco import AFP_GlobReco_HLT
    AFP_Pr = AFP_GlobReco_HLT(flags)
    recoAcc.mergeReco(AFP_Pr)

    # Vertex reconstruction
    from AFP_VertexReco.AFP_VertexReco import AFP_VertexReco_HLT
    AFP_Vtx = AFP_VertexReco_HLT(flags)
    recoAcc.mergeReco(AFP_Vtx)

    return recoAcc


@AccumulatorCache
def AFPGlobalSequence(flags):
    def AFPTOFHypoToolGen(chain_dict):
        from TrigAFPHypo.TrigAFPHypoConf import TrigAFPToFHypoTool
        return TrigAFPToFHypoTool(chain_dict['chainName'])

    recoAcc = AFPGlobalRecoSequenceCA(flags)

    hypo = CompFactory.TrigAFPToFHypoAlg('TrigAFPToFHypoAlg', AFPVertexContainer='HLT_AFPVertexContainer', VertexContainer='HLT_IDVertex_FS')

    selAcc = SelectionCA('AFPGlobalSequence')
    selAcc.mergeReco(recoAcc)
    selAcc.addHypoAlgo(hypo)

    return MenuSequenceCA(flags, selAcc, HypoToolGen=AFPTOFHypoToolGen)


if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.AOD_RUN3_MC
    flags.lock()

    from TriggerMenuMT.HLT.Config.MenuComponents import menuSequenceCAToGlobalWrapper

    afp_trk = AFPTrkSequence(flags)
    afp_trk.ca.printConfig(withDetails=True)
    afp_trk_gw = menuSequenceCAToGlobalWrapper(AFPTrkSequence, flags)

    afp_glob = AFPGlobalSequence(flags)
    afp_glob.ca.printConfig(withDetails=True)
    afp_glob_gw = menuSequenceCAToGlobalWrapper(AFPGlobalSequence, flags)
