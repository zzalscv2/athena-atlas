#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

def JetEfficiencyMonitoringConfig(inputFlags):
    '''Function to configure LVL1 JetEfficiency algorithm in the monitoring system.'''

    #import math
    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.Enums import Format
    import re

    result = ComponentAccumulator()

    ###########################################################################
    # Jet and particle flow config required for data POOL files
    if inputFlags.Input.Format is Format.POOL and not inputFlags.Input.isMC:
        from JetRecConfig.JetRecConfig import JetRecCfg
        from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow
        from JetRecConfig.JetConfigFlags import jetInternalFlags
        jetInternalFlags.isRecoJob = True
        result.merge( JetRecCfg(inputFlags,AntiKt4EMPFlow) )
        
        from eflowRec.PFCfg import PFGlobalFlowElementLinkingCfg
        if inputFlags.DQ.Environment == "AOD":
          result.merge(PFGlobalFlowElementLinkingCfg(inputFlags, useMuonTopoClusters=True))
        else:
          result.merge(PFGlobalFlowElementLinkingCfg(inputFlags))

        from METReconstruction.METAssociatorCfg import METAssociatorCfg
        result.merge(METAssociatorCfg(inputFlags, 'AntiKt4EMPFlow'))

        from METUtilities.METMakerConfig import getMETMakerAlg
        metCA=ComponentAccumulator()
        metCA.addEventAlgo(getMETMakerAlg('AntiKt4EMPFlow'))
        result.merge(metCA)

    ###########################################################################


    # make the athena monitoring helper
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'JetEfficiencyMonitoringCfg')
    # get any algorithms
    JetEfficiencyMonAlg = helper.addAlgorithm(CompFactory.JetEfficiencyMonitorAlgorithm,'JetEfficiencyMonAlg')

    # add any steering
    groupName = 'JetEfficiencyMonitor' # the monitoring group name is also used for the package name
    JetEfficiencyMonAlg.PackageName = groupName


    #################################################################
    #################################################################
    #################################################################
    #################################################################

    if inputFlags.Input.isMC: emulated = False
    else: emulated = True
    JetEfficiencyMonAlg.Emulated = emulated
    
    #################################################################
    #################################################################
    #################################################################
    #################################################################

    orthogonal_trigger = "L1_RD0_FILLED" #trigger that does not depend on depend on the jet's


    mainDir = 'L1Calo'
    trigPath = 'JetEfficiency/'
    distributionPath = 'Distributions/'
    refEffPath = 'EfficiencyReferences/'
    noRefPath = refEffPath+'noneRef/'
    unbiasedRefPath = refEffPath+'unbiasedRef/'
    orthogonalRefPath = refEffPath+'orthoRef/'
    GeV = 1000

    nbin = 50
    binmin = -50
    binmax = 500*GeV

    etabins=32
    etamin=-3.3
    etamax=3.3

    # add monitoring algorithm to group, with group name and main directory
    myGroup = helper.addGroup(JetEfficiencyMonAlg, groupName , mainDir)
    single_triggers = ['L1_J15', 'L1_J20', 'L1_J25', 'L1_J30', 'L1_J40', 'L1_J50', 'L1_J75',
                'L1_J85', 'L1_J100', 'L1_J120', 'L1_J200', 'L1_J300', 'L1_J400', 'L1_J40_XE50', 'L1_J40_XE60',
                'L1_eEM15', 'L1_eEM22M', 'L1_eTAU20L', 'L1_jJ30', 'L1_jEM20']
    multijet_triggers = ['L1_J85_3J30', 'L1_3J50', 'L1_4J15', 'L1_4J20', 'L1_2J15_XE55', 'L1_2J50_XE40']

    triggers = single_triggers + multijet_triggers
    JetEfficiencyMonAlg.L1TriggerList = triggers

    gfex_triggers = ['L1_gJ20','L1_gJ30','L1_gJ40','L1_gJ50', 'L1_gJ60', 'L1_gJ100', 'L1_gJ160']
    JetEfficiencyMonAlg.SRgfexTriggerList = gfex_triggers

    gfex_LR_triggers = ['L1_gLJ80', 'L1_gLJ100', 'L1_gLJ140', 'L1_gLJ160']
    JetEfficiencyMonAlg.LRgfexTriggerList = gfex_LR_triggers

    
    myGroup.defineHistogram('run',title='Run Number;run;Events',
                            path=trigPath,xbins=1000000,xmin=-0.5,xmax=999999.5)
    myGroup.defineHistogram('raw_pt',title='pT for all leading offline jet with pT > 100 GeV (with no trigger requirments);PT [MeV];Events',
                            path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)
    myGroup.defineHistogram('orthogonal_pt',title='pT for all offline jets that pass the orthogonal trigger;PT [MeV];Events',
                            path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)
    myGroup.defineHistogram('eta',  title='Eta Distribution of offline jets for orthogonal trigger ' + orthogonal_trigger +';#eta; Count',
                                path=trigPath + distributionPath,xbins=etabins,xmin=etamin, xmax=etamax)
    
    ########## Iterate through the offline l1 triggers to make histograms to fill!
    ########## Histograms include efficiency plots using bootstrap, orthogonal refernce and no refernce
    ########## Also includes pt distributions of the various
    #  if inputFlags.Input.isMC: JetEfficiencyMonAlg.BootstrapTrigger='L1_J15'
    # bootstrap_trigger = JetEfficiencyMonAlg.BootstrapTrigger
    for t in single_triggers:
        trigger = t
        values = [int(s) for s in re.findall(r'\d+', t)]
        ptval = max(values)
        binmax = (int(ptval)*GeV) + (200*GeV)

        myGroup.defineHistogram('pt_unbiased_'+t+',pt_unbiased', type='TEfficiency',  title='PT Efficiency of offline jets for trigger ' + trigger + ' wrt unbiased triggers;Offline Jet PT [MeV]; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath+ unbiasedRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:unbiased_'+t,  title='PT distribution of offline jets for trigger ' + trigger + ' wrt unbiased triggers; PT [MeV];Count',
                                path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('eta_unbiased_'+t+',eta_unbiased', type='TEfficiency',  title='Eta Efficiency of offline jets for trigger ' + trigger + ' wrt unbiased triggers;#eta; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath+ unbiasedRefPath,xbins=etabins,xmin=etamin, xmax=etamax)
        myGroup.defineHistogram('eta:unbiased_'+t,  title='Eta Distribution of offline jets for trigger ' + trigger + ' wrt unbiased triggers;#eta; Count',
                                path=trigPath + distributionPath,xbins=etabins,xmin=etamin, xmax=etamax)
                                

        myGroup.defineHistogram('pt_ortho_'+t+',pt_ortho', type='TEfficiency',  title='PT Efficiency of offline jets for trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';Offline Jet PT [MeV]; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath + orthogonalRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:ortho_'+t,  title='PT distribution of offline jets for trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';PT [MeV];Count',
                                path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('eta_ortho_'+t+',eta', type='TEfficiency',  title='Eta Efficiency of offline jets for trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';#eta; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath + orthogonalRefPath,xbins=etabins,xmin=etamin, xmax=etamax)
        myGroup.defineHistogram('eta:ortho_'+t,  title='Eta Distribution of offline jets for trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';#eta; Count',
                                path=trigPath + distributionPath,xbins=etabins,xmin=etamin, xmax=etamax)


        myGroup.defineHistogram('pt_none_'+t+',pt_none', type='TEfficiency',  title='PT Efficiency of offline jets for trigger ' + trigger  +';Offline Jet PT [MeV]; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath + noRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:none_'+t,  title='PT distribution of offline jets for trigger ' + trigger  + ';PT [MeV];Count',
                                path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('eta:none_'+t,  title='Eta Distribution of offline jets for trigger ' + trigger + ';#eta; Count',
                                path=trigPath + distributionPath,xbins=etabins,xmin=etamin, xmax=etamax)

    for t in multijet_triggers:
        trigger = t
        values = [int(s) for s in re.findall(r'\d+', t)]
        ptval = max(values)
        binmax = (int(ptval)*GeV) + (200*GeV)

        myGroup.defineHistogram('pt_unbiased_'+t+',pt_unbiased', type='TEfficiency',  title='PT Efficiency of last offline jet for multijet trigger ' + trigger + ' wrt unbiased triggers;Offline Jet PT [MeV]; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath + unbiasedRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:unbiased_'+t,  title='PT distribution of last offline jet for multijet trigger ' + trigger + ' wrt unbiased triggers; PT [MeV];Count',
                                path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('eta_unbiased_'+t+',eta_unbiased', type='TEfficiency',  title='Eta Efficiency of last offline jet for multijet trigger ' + trigger + ' wrt unbiased triggers;#eta; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath + unbiasedRefPath,xbins=etabins,xmin=etamin, xmax=etamax)
        myGroup.defineHistogram('eta:unbiased_'+t,  title='Eta Distribution of last offline jet for multijet ' + trigger + ' wrt unbiased triggers;#eta; Count',
                                path=trigPath + distributionPath,xbins=etabins,xmin=etamin, xmax=etamax)
                                

        myGroup.defineHistogram('pt_ortho_'+t+',pt_ortho', type='TEfficiency',  title='PT Efficiency of last offline jet for multijet trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';Offline Jet PT [MeV]; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath + orthogonalRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:ortho_'+t,  title='PT distribution of last offline jet for multijet trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';PT [MeV];Count',
                                path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('eta_ortho_'+t+',eta', type='TEfficiency',  title='Eta Efficiency of last offline jet for multijet trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';#eta; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath + orthogonalRefPath,xbins=etabins,xmin=etamin, xmax=etamax)
        myGroup.defineHistogram('eta:ortho_'+t,  title='Eta Distribution of last offline jet for multijet trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';#eta; Count',
                                path=trigPath + distributionPath,xbins=etabins,xmin=etamin, xmax=etamax)


        myGroup.defineHistogram('pt_none_'+t+',pt_none', type='TEfficiency',  title='PT Efficiency of last offline jet for multijet trigger ' + trigger  +';Offline Jet PT [MeV]; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath + noRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:none_'+t,  title='PT distribution of last offline jet for multijet trigger ' + trigger  + ';PT [MeV];Count',
                                path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('eta:none_'+t,  title='Eta Distribution of last offline jet for multijet trigger ' + trigger + ';#eta; Count',
                                path=trigPath + distributionPath,xbins=etabins,xmin=etamin, xmax=etamax)
     

    for t in gfex_triggers:
        trigger = t
        values = [int(s) for s in re.findall(r'\d+', t)]
        ptval = max(values)
        binmax = (int(ptval)*GeV) + (200*GeV)

        if emulated: title_add = " EMULATED "
        else: title_add = " "
        ##looking at the leading offline jets with gfex triggers
        myGroup.defineHistogram('pt_ortho'+t+',pt_ortho', type='TEfficiency',  title='PT Efficiency of leading offline jets for' + title_add + 'gfex SR trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';Offline Jet PT [MeV]; Ratio = # of trigger jets / # of total jets  ',
                                path=trigPath + orthogonalRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:ortho_'+t, title='PT Distribution of leading offline jets for' + title_add + 'gfex SR trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';Offline Jet PT [MeV];Count',
                                path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)

        myGroup.defineHistogram('pt_unbiased'+t+',pt_unbiased', type='TEfficiency',  title='PT Efficiency of leading offline jets for' + title_add + 'gfex SR trigger ' + trigger + ' wrt unbiased triggers; Offline Jet PT [MeV]; Ratio = # of trigger jets / # of total jets  ',
                                path=trigPath + unbiasedRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:unbiased_'+t, title='PT Distribution of leading offline jets for' + title_add + 'gfex SR trigger ' + trigger + ' wrt unbiased triggers ;Offline Jet PT [MeV];Count',
                                path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)

        myGroup.defineHistogram('pt_none'+t+',pt_none', type='TEfficiency',  title='PT Efficiency of leading offline jets for' + title_add + 'gfex SR trigger ' + trigger  + ';Offline Jet PT [MeV]; Ratio = # of trigger jets / # of total jets  ',
                                path=trigPath + noRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:none'+t, title='PT Distribution of leading offline jets for' + title_add + 'gfex SR trigger ' + trigger + ';Offline Jet PT [MeV];Count',
                                path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)


        ##looking at SR gfex jets with gfex triggers
        myGroup.defineHistogram('pt_ortho_SR_'+t+',pt_gfex_SR_ortho', type='TEfficiency',  title= 'ET Efficiency of gFEX SR TOBs for' + title_add + 'trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';gFex SR Jet TOB ET [MeV]; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath + orthogonalRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:ortho_SR_'+t, title=  'ET distribution of gFEX SR TOBs for' + title_add + 'gfex trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';gFex SR Jet TOB ET [MeV];Count',
                                path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)

        myGroup.defineHistogram('pt_unbiased_SR_'+t+',pt_gfex_SR_unbiased', type='TEfficiency',  title= 'ET Efficiency of gFEX SR TOBs for' + title_add + 'trigger ' + trigger + ' wrt unbiased triggers ;gFex SR Jet TOB ET [MeV]; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath + unbiasedRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:unbiased_SR_'+t, title=  'ET distribution of gFEX SR TOBs for' + title_add + 'gfex trigger ' + trigger + ' wrt unbiased triggers ;gFex SR Jet TOB ET [MeV];Count',
                                path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)

        myGroup.defineHistogram('pt_none_SR_'+t+',pt_gfex_SR_none', type='TEfficiency',  title= 'ET Efficiency of gFEX SR TOBs for' + title_add + 'trigger ' + trigger  + ';gFex SR Jet TOB ET [MeV]; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath + noRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:none_SR_'+t, title=  'ET distribution of gFEX SR TOBs for' + title_add + 'gfex trigger ' + trigger +  ';gFex SR Jet TOB ET [MeV];Count',
                                path=trigPath + distributionPath,xbins=nbin,xmin=binmin, xmax=binmax)

    
    for t in gfex_LR_triggers:
        trigger = t
        values = [int(s) for s in re.findall(r'\d+', t)]
        ptval = max(values)
        binmax = (int(ptval)*GeV) + (200*GeV)

        if emulated: title_add = " EMULATED "
        else: title_add = " "
        myGroup.defineHistogram('pt_ortho'+t+',pt_ortho', type='TEfficiency',  title='PT Efficiency of leading offline jets for' + title_add + 'gfex LR trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';Offline Jet PT [MeV]; Ratio = # of trigger jets / # of total jets  ',
                                path=trigPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:ortho_'+t, title='PT Distribution of leading offline jets for' + title_add + 'gfex LR trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';Offline Jet PT [MeV];Count',
                                path=trigPath,xbins=nbin,xmin=binmin, xmax=binmax)

        myGroup.defineHistogram('pt_unbiased'+t+',pt_unbiased', type='TEfficiency',  title='PT Efficiency of leading offline jets for' + title_add + 'gfex LR trigger ' + trigger + ' wrt unbiased triggers; Offline Jet PT [MeV]; Ratio = # of trigger jets / # of total jets  ',
                                path=trigPath + unbiasedRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:unbiased_'+t, title='PT Distribution of leading offline jets for' + title_add + 'gfex LR trigger ' + trigger + ' wrt unbiased triggers ;Offline Jet PT [MeV];Count',
                                path=trigPath + unbiasedRefPath,xbins=nbin,xmin=binmin, xmax=binmax)

        myGroup.defineHistogram('pt_none'+t+',pt_none', type='TEfficiency',  title='PT Efficiency of leading offline jets for' + title_add + 'gfex LR trigger ' + trigger  + ';Offline Jet PT [MeV]; Ratio = # of trigger jets / # of total jets  ',
                                path=trigPath + noRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:none'+t, title='PT Distribution of leading offline jets for' + title_add + 'gfex LR trigger ' + trigger + ';Offline Jet PT [MeV];Count',
                                path=trigPath + noRefPath,xbins=nbin,xmin=binmin, xmax=binmax)


        ##looking at LR gfex jets with gfex triggers
        myGroup.defineHistogram('pt_ortho_LR_'+t+',pt_gfex_LR_ortho', type='TEfficiency',  title= 'ET Efficiency of gFEX LR TOBs for' + title_add + 'trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';gFex LR Jet TOB ET [MeV]; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:ortho_LR_'+t, title=  'ET distribution of gFEX LR TOBs for' + title_add + 'gfex trigger ' + trigger + ' wrt orthogonal trigger ' + orthogonal_trigger + ';gFex LR Jet TOB ET [MeV];Count',
                                path=trigPath,xbins=nbin,xmin=binmin, xmax=binmax)

        myGroup.defineHistogram('pt_unbiased_LR_'+t+',pt_gfex_LR_unbiased', type='TEfficiency',  title= 'ET Efficiency of gFEX LR TOBs for' + title_add + 'trigger ' + trigger + ' wrt unbiased triggers ;gFex LR Jet TOB ET [MeV]; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath + unbiasedRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:unbiased_LR_'+t, title=  'ET distribution of gFEX LR TOBs for' + title_add + 'gfex trigger ' + trigger + ' wrt unbiased triggers ;gFex LR Jet TOB ET [MeV];Count',
                                path=trigPath + unbiasedRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
                                
        myGroup.defineHistogram('pt_none_LR_'+t+',pt_gfex_LR_none', type='TEfficiency',  title= 'ET Efficiency of gFEX LR TOBs for' + title_add + 'trigger ' + trigger  + ';gFex LR Jet TOB ET [MeV]; Ratio = # of trigger jets / # of total jets ',
                                path=trigPath + noRefPath,xbins=nbin,xmin=binmin, xmax=binmax)
        myGroup.defineHistogram('pt:none_LR_'+t, title=  'ET distribution of gFEX LR TOBs for' + title_add + 'gfex trigger ' + trigger +  ';gFex LR Jet TOB ET [MeV];Count',
                                path=trigPath + noRefPath,xbins=nbin,xmin=binmin, xmax=binmax)

    ######## add triggers to the list to be included in "jet of more than 100 pt", what else fired?"" histogram
    trigger_list = ["L1_EM22VHI", "L1_EM24VHI", "L1_2EM15VHI", "L1_2EM20VH", "L1_EM20VH_3EM10VH",
                    "L1_TAU100", "L1_TAU60_2TAU40", "L1_TAU20IM_2TAU12IM_4J12p0ETA25", "L1_TAU25IM_2TAU20IM_2J25_3J20",
                    "L1_EM15VHI_2TAU12IM_XE35", "L1_EM15VHI_2TAU12IM_4J12", "L1_TAU40_2TAU12IM_XE40", "L1_J100", "L1_J120",
                    "L1_J45p0ETA21_3J15p0ETA25", "L1_4J15", "L1_4J20", "L1_3J15p0ETA25_XE40", "L1_J85_3J30", "L1_3J35p0ETA23",
                    "L1_4J15p0ETA25", "L1_5J15p0ETA25", "L1_2J15_XE55", "L1_J40_XE50", "L1_2J50_XE40", "L1_J40_XE60", "L1_XE50",
                    "L1_XE55", "L1_XE60", "L1_HT190-J15s5pETA21", "L1_MJJ-500-NFF", "L1_EM18VHI_MJJ-300", "L1_SC111-CJ15",
                    "L1_DR-TAU20ITAU12I-J25", "L1_TAU60_DR-TAU20ITAU12I", "L1_MU14FCH", "L1_MU18VFCH", "L1_EM15VH_MU8F",
                    "L1_MU8F_TAU20IM", "L1_MU8F_TAU12IM_XE35", "L1_3J50", "L1_J40p0ETA25_2J25_J20p31ETA49", "L1_J400",
                    "L1_J400_LAR", "L1_XE300", "L1_J50p31ETA49", "L1_J75p31ETA49", "L1_2MU8F", "L1_MU8VF_2MU5VF", "L1_3MU3VF",
                    "L1_MU5VF_3MU3VF", "L1_4MU3V", "L1_2MU5VF_3MU3V", "L1_2EM8VH_MU8F", "L1_MU8F_TAU12IM_3J12", "L1_MU8F_2J15_J20",
                    "L1_BPH-0DR3-EM7J15_MU5VF", "L1_MU8F_2J15_J20", "L1_DR-TAU20ITAU12I", "L1_2EM15VH", "L1_TAU60",
                    "L1_HT150-J20s5pETA31_MJJ-400-CF", "L1_TAU25IM_2TAU20IM", "L1_BPH-0M9-EM7-EM5_2MU3V", "L1_MU10BO", "L1_RD0_FILLED"]
    JetEfficiencyMonAlg.TriggerList = trigger_list
    myGroup.defineHistogram('otherTriggers;h_otherTriggers',
                        title='Triggers Firing for Jets with leading Pt > 100 GeV; Triggers; Events',
                        xlabels=trigger_list,
                        type='TH1F', path=trigPath,
                        xbins=len(trigger_list),xmin=0,xmax=len(trigger_list))

    

    acc = helper.result()
    result.merge(acc)
    print("inputFlags.DQ.Environment = " +inputFlags.DQ.Environment )
    return result
 


if __name__=='__main__':
    # For direct tests
    from AthenaCommon.Configurable import Configurable
    Configurable.configurableRun3Behavior = 1

    # set debug level for whole job
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO #DEBUG
    log.setLevel(INFO)

    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    import glob

    # from TrigDecisionTool.TrigDecisionToolConfig import getTrigDecisionTool
    # tdtAcc = getTrigDecisionTool(ConfigFlags)
    # tdt = tdtAcc.getPrimary()


    #inputs = glob.glob('/eos/atlas/atlastier0/rucio/data18_13TeV/physics_Main/00357750/data18_13TeV.00357750.physics_Main.recon.ESD.f1072/data18_13TeV.00357750.physics_Main.recon.ESD.f1072._lb0117._SFO-1._0201.1')
    inputs = glob.glob('/eos/atlas/atlastier0/rucio/data18_13TeV/physics_Main/00354311/data18_13TeV.00354311.physics_Main.recon.ESD.f1129/data18_13TeV.00354311.physics_Main.recon.ESD.f1129._lb0013._SFO-8._0001.1')


    ConfigFlags.Input.Files = inputs
    ConfigFlags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1.root'

    ConfigFlags.lock()
    ConfigFlags.dump() # print all the configs

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(ConfigFlags)
    cfg.merge(PoolReadCfg(ConfigFlags))

    JetEfficiencyMonitorCfg = JetEfficiencyMonitoringConfig(ConfigFlags)
    cfg.merge(JetEfficiencyMonitorCfg)

    # cfg.merge(TrigDecisionToolCfg(ConfigFlags))

    # message level for algorithm
    JetEfficiencyMonitorCfg.getEventAlgo('JetEfficiencyMonAlg').OutputLevel = 1 # 1/2 INFO/DEBUG
    # options - print all details of algorithms, very short summary
    cfg.printConfig(withDetails=False, summariseProps = True)

    nevents=-1
    cfg.run(nevents)
