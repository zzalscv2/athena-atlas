# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
from TrigInDetConfig.InDetTrigCollectionKeys import TrigPixelKeys, TrigSCTKeys

# Default name of HitDV output
hitDVName = "HLT_HitDV"

def createTrigHitDVHypoAlg(flags, name):
    # make the Hypo
    from TrigLongLivedParticlesHypo.TrigLongLivedParticlesHypoConf import (TrigHitDVHypoAlg)

    # Setup the hypothesis algorithm
    theHitDVHypo = TrigHitDVHypoAlg(name)

    from TrigEDMConfig.TriggerEDMRun3 import recordable
    theHitDVHypo.HitDV = recordable(hitDVName)

    if flags.Input.isMC:
        theHitDVHypo.isMC = True
    else:
        theHitDVHypo.isMC = False

    # monioring
    monTool = GenericMonitoringTool(flags, "IM_MonTool"+name)
    monTool.defineHistogram('jet_pt',        type='TH1F', path='EXPERT', title="p_{T}^{jet} [GeV];p_{T}^{jet} [GeV];Nevents", xbins=50, xmin=0, xmax=200)
    monTool.defineHistogram('jet_eta',       type='TH1F', path='EXPERT', title="#eta^{jet};#eta^{jet};Nevents", xbins=50, xmin=-5.0, xmax=5.0)
    #
    monTool.defineHistogram('n_dvtrks',      type='TH1F', path='EXPERT', title="Nr of HitDVTrks;N HitDVTrks size;Nevents", xbins=50, xmin=0, xmax=1000)
    monTool.defineHistogram('n_dvsps',       type='TH1F', path='EXPERT', title="Nr of HitDVSPs;N HitDVSPs size;Nevents", xbins=50, xmin=0, xmax=100000)
    monTool.defineHistogram('n_jetseeds',    type='TH1F', path='EXPERT', title="Nr of Jet Seeds;N jet seeds;Nevents", xbins=25, xmin=0, xmax=25)
    monTool.defineHistogram('n_jetseedsdel', type='TH1F', path='EXPERT', title="Nr of deleted jet seeds;N jet seeds;Nevents", xbins=25, xmin=0, xmax=25)
    monTool.defineHistogram('n_spseeds',     type='TH1F', path='EXPERT', title="Nr of Ly6/Ly7 SP-doublet Seeds;N SP seeds;Nevents", xbins=25, xmin=0, xmax=25)
    monTool.defineHistogram('n_spseedsdel',  type='TH1F', path='EXPERT', title="Nr of deleted Ly6/Ly7 SP-doublet seeds;N SP seeds;Nevents", xbins=25, xmin=0, xmax=25)
    monTool.defineHistogram('average_mu',    type='TH1F', path='EXPERT', title="Average mu;Average mu;Nevents", xbins=50, xmin=0, xmax=100)
    #
    monTool.defineHistogram('eta1_ly0_spfr', type='TH1F', path='EXPERT', title="Layer#0 hit fraction (|#eta|<1);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('eta1_ly1_spfr', type='TH1F', path='EXPERT', title="Layer#1 hit fraction (|#eta|<1);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('eta1_ly2_spfr', type='TH1F', path='EXPERT', title="Layer#2 hit fraction (|#eta|<1);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('eta1_ly3_spfr', type='TH1F', path='EXPERT', title="Layer#3 hit fraction (|#eta|<1);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('eta1_ly4_spfr', type='TH1F', path='EXPERT', title="Layer#4 hit fraction (|#eta|<1);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('eta1_ly5_spfr', type='TH1F', path='EXPERT', title="Layer#5 hit fraction (|#eta|<1);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('eta1_ly6_spfr', type='TH1F', path='EXPERT', title="Layer#6 hit fraction (|#eta|<1);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('eta1_ly7_spfr', type='TH1F', path='EXPERT', title="Layer#7 hit fraction (|#eta|<1);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('eta1_n_qtrk',   type='TH1F', path='EXPERT', title="Nr of quality tracks (|#eta|<1);Nr of quality tracks;Nevents", xbins=20, xmin=0, xmax=20)
    monTool.defineHistogram('eta1_bdtscore', type='TH1F', path='EXPERT', title="BDT score (|#eta|<1);BDT score;Nevents", xbins=50, xmin=-1.0, xmax=1.0)
    monTool.defineHistogram('1eta2_ly0_spfr', type='TH1F', path='EXPERT', title="Layer#0 hit fraction (1<|#eta|<2);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('1eta2_ly1_spfr', type='TH1F', path='EXPERT', title="Layer#1 hit fraction (1<|#eta|<2);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('1eta2_ly2_spfr', type='TH1F', path='EXPERT', title="Layer#2 hit fraction (1<|#eta|<2);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('1eta2_ly3_spfr', type='TH1F', path='EXPERT', title="Layer#3 hit fraction (1<|#eta|<2);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('1eta2_ly4_spfr', type='TH1F', path='EXPERT', title="Layer#4 hit fraction (1<|#eta|<2);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('1eta2_ly5_spfr', type='TH1F', path='EXPERT', title="Layer#5 hit fraction (1<|#eta|<2);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('1eta2_ly6_spfr', type='TH1F', path='EXPERT', title="Layer#6 hit fraction (1<|#eta|<2);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('1eta2_ly7_spfr', type='TH1F', path='EXPERT', title="Layer#7 hit fraction (1<|#eta|<2);Hit fraction;Nevents", xbins=50, xmin=0.0, xmax=1.0)
    monTool.defineHistogram('1eta2_n_qtrk',   type='TH1F', path='EXPERT', title="Nr of quality tracks (1<|#eta|<2);Nr of quality tracks;Nevents", xbins=20, xmin=0, xmax=20)
    monTool.defineHistogram('1eta2_bdtscore', type='TH1F', path='EXPERT', title="BDT score (1<|#eta|<2);BDT score;Nevents", xbins=50, xmin=-1.0, xmax=1.0)

    monTool.HistPath = 'HitDVHypoAlg'
    theHitDVHypo.MonTool = monTool
    theHitDVHypo.RecJetRoI = "HLT_RecJETRoIs"

    from TrigOnlineSpacePointTool.TrigOnlineSpacePointToolConf import TrigL2LayerNumberTool
    numberingTool = TrigL2LayerNumberTool(name = "TrigL2LayerNumberTool_HitDV")
    numberingTool.UseNewLayerScheme = False
    from AthenaCommon.AppMgr import ToolSvc
    ToolSvc += numberingTool

    # Spacepoint conversion
    from TrigOnlineSpacePointTool.TrigOnlineSpacePointToolConf import TrigSpacePointConversionTool
    spTool = TrigSpacePointConversionTool().clone('TrigSpacePointConversionTool_HitDV')
    spTool.DoPhiFiltering        = False
    spTool.UseNewLayerScheme     = False
    spTool.UseBeamTilt           = False
    spTool.PixelSP_ContainerName = TrigPixelKeys.SpacePoints
    spTool.SCT_SP_ContainerName  = TrigSCTKeys.SpacePoints
    spTool.layerNumberTool       = numberingTool

    from RegionSelector.RegSelToolConfig import makeRegSelTool_Pixel
    from RegionSelector.RegSelToolConfig import makeRegSelTool_SCT

    spTool.RegSelTool_Pixel = makeRegSelTool_Pixel()
    spTool.RegSelTool_SCT   = makeRegSelTool_SCT()

    ToolSvc += spTool

    theHitDVHypo.SpacePointProviderTool = spTool

    return theHitDVHypo


def TrigHitDVHypoToolFromDict( chainDict ):

    log = logging.getLogger('TrigHitDVHypoTool')

    """ Use menu decoded chain dictionary to configure the tool """
    cparts = [i for i in chainDict['chainParts'] if i['signature']=='UnconventionalTracking']
    thresholds = sum([ [cpart['threshold']]*int(cpart['multiplicity']) for cpart in cparts], [])

    name = chainDict['chainName']
    from AthenaConfiguration.ComponentFactory import CompFactory
    tool = CompFactory.TrigHitDVHypoTool(name)

    # set thresholds

    strThr = ""

    thresholds = [ float(THR) for THR in thresholds]

    for THR in thresholds:
        strThr += str(THR)+", "

    log.info("Threshold Values are: %s",strThr)

    tool.cutJetPtGeV = thresholds

    jetEta=[]
    doSPseed=[]
    effBDT=[]

    for cpart in cparts:
        if cpart['IDinfo'] =="loose":
            log.info("Loose ID working point is set")
            jetEta.append(2.0)
            doSPseed.append(True)
            effBDT.append(0.9)
        elif cpart['IDinfo'] =="tight":
            log.info("Tight ID working point is set")
            jetEta.append(1.0)
            doSPseed.append(False)
            effBDT.append(0.75)
        else:
            if cpart['IDinfo'] =="medium":
                log.info("Medium ID working point is set")
            else:
                log.info("no working point specificed. setting medium working point")
            jetEta.append(2.0)
            doSPseed.append(False)
            effBDT.append(0.75)

    tool.cutJetEta = jetEta
    tool.doSPseed  = doSPseed
    tool.effBDT    = effBDT

    return tool
