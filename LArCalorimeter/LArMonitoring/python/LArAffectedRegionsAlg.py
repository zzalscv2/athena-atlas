#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def LArAffectedRegionsConfig(flags):
    '''Function to configures some algorithms in the monitoring system.'''

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'LArAffectedRegionsAlgCfg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    LArAffectedRegionsConfigCore(helper, CompFactory.LArAffectedRegionsAlg, flags)

    return helper.result()


def LArAffectedRegionsConfigCore(helper, algoinstance, flags):

    larAffectedRegAlg = helper.addAlgorithm(algoinstance,'larAffectedRegAlg')

    #define the group names here, as you'll use them multiple times
    affectedRegGroupName="LArAffectedRegionsMonGroup"


    # Edit properties of a algorithm
    larAffectedRegAlg.AffectedRegionsGroupName=affectedRegGroupName
    isOnline=False
    from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg
    if isComponentAccumulatorCfg():
       if flags.DQ.Environment == 'online':
          isOnline=True
    else:
       from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
       if athenaCommonFlags.isOnline:
          isOnline=True

    larAffectedRegAlg.IsOnline = isOnline


    from LArMonitoring.GlobalVariables import lArDQGlobals #to define the ranges
    larAffReg_hist_path='AffectedRegions/' #histogram path
    

    #EMBPS
    group_name_ending="EMBPS" 
    larAffectedRegAlg.EMBPSname=group_name_ending
    affectedRegGroupEMBPS = helper.addGroup(
        larAffectedRegAlg,
        affectedRegGroupName+group_name_ending,
        '/LAr/',
        'run'
    )


    affectedRegGroupEMBPS.defineHistogram('etaPOS,phi;RAW_LArAffectedRegionsEMBAPS',
                                          title='HV Affected Regions - EMBA - Presampler;#eta;#phi',
                                          type='TH2F',
                                          path=larAffReg_hist_path,
                                          weight='problem',
                                          xbins=lArDQGlobals.HVeta_EMB["EMBAPS"][0],xmin=lArDQGlobals.HVeta_EMB["EMBAPS"][1],xmax=lArDQGlobals.HVeta_EMB["EMBAPS"][2],
                                          ybins=lArDQGlobals.HVphi_EMB["EMBAPS"][0],ymin=lArDQGlobals.HVphi_EMB["EMBAPS"][1],ymax=lArDQGlobals.HVphi_EMB["EMBAPS"][2],
                                          merge='weightedAverage'
    )
    affectedRegGroupEMBPS.defineHistogram('etaNEG,phi;RAW_LArAffectedRegionsEMBCPS',
                                          title='HV Affected Regions - EMBC - Presampler;#eta;#phi',
                                          type='TH2F',
                                          path=larAffReg_hist_path,
                                          weight='problem',
                                          xbins=lArDQGlobals.HVeta_EMB["EMBCPS"][0],xmin=lArDQGlobals.HVeta_EMB["EMBCPS"][1],xmax=lArDQGlobals.HVeta_EMB["EMBCPS"][2],
                                          ybins=lArDQGlobals.HVphi_EMB["EMBCPS"][0],ymin=lArDQGlobals.HVphi_EMB["EMBCPS"][1],ymax=lArDQGlobals.HVphi_EMB["EMBCPS"][2],
                                          merge='weightedAverage'
    )


    #EMB
    group_name_ending="EMB"
    larAffectedRegAlg.EMBname=group_name_ending
    affectedRegGroupEMB = helper.addGroup(
        larAffectedRegAlg,
        affectedRegGroupName+group_name_ending,
        '/LAr/',
        'run'
    )

    affectedRegGroupEMB.defineHistogram('etaPOS,phi;RAW_LArAffectedRegionsEMBA',
                                        title='HV Affected Regions - EMBA - Samplings 1-3;#eta;#phi',
                                        type='TH2F',
                                        path=larAffReg_hist_path,
                                        weight='problem',
                                        xbins=lArDQGlobals.HVeta_EMB["EMBA"][0],xmin=lArDQGlobals.HVeta_EMB["EMBA"][1],xmax=lArDQGlobals.HVeta_EMB["EMBA"][2],
                                        ybins=lArDQGlobals.HVphi_EMB["EMBA"][0],ymin=lArDQGlobals.HVphi_EMB["EMBA"][1],ymax=lArDQGlobals.HVphi_EMB["EMBA"][2],
                                        merge='weightedAverage'
    )
    affectedRegGroupEMB.defineHistogram('etaNEG,phi;RAW_LArAffectedRegionsEMBC',
                                        title='HV Affected Regions - EMBC - Samplings 1-3;#eta;#phi',
                                        type='TH2F',
                                        path=larAffReg_hist_path,
                                        weight='problem',
                                        xbins=lArDQGlobals.HVeta_EMB["EMBC"][0],xmin=lArDQGlobals.HVeta_EMB["EMBC"][1],xmax=lArDQGlobals.HVeta_EMB["EMBC"][2],
                                        ybins=lArDQGlobals.HVphi_EMB["EMBC"][0],ymin=lArDQGlobals.HVphi_EMB["EMBC"][1],ymax=lArDQGlobals.HVphi_EMB["EMBC"][2],
                                        merge='weightedAverage'
    )
        

    #EMECPS
    group_name_ending="EMECPS"
    larAffectedRegAlg.EMECPSname=group_name_ending
    affectedRegGroupEMECPS = helper.addGroup(
        larAffectedRegAlg,
        affectedRegGroupName+group_name_ending,
        '/LAr/',
        'run'
    )
    
    affectedRegGroupEMECPS.defineHistogram('etaPOS,phi;RAW_LArAffectedRegionsEMECAPS',
                                           title='HV Affected Regions - EMECA - Presampler;#eta;#phi',
                                           type='TH2F',
                                           path=larAffReg_hist_path,
                                           weight='problem',
                                           xbins=lArDQGlobals.HVeta_EMEC["EMECAPS"],
                                           ybins=lArDQGlobals.HVphi_EMEC["EMECAPS"],
                                           merge='weightedAverage'
    )
    affectedRegGroupEMECPS.defineHistogram('etaNEG,phi;RAW_LArAffectedRegionsEMECCPS',
                                           title='HV Affected Regions - EMECC - Presampler;#eta;#phi',
                                           type='TH2F',
                                           path=larAffReg_hist_path,
                                           weight='problem',
                                           xbins=lArDQGlobals.HVeta_EMEC["EMECCPS"],
                                           ybins=lArDQGlobals.HVphi_EMEC["EMECCPS"],
                                           merge='weightedAverage'
    )


    #EMEC
    group_name_ending="EMEC"
    larAffectedRegAlg.EMECname=group_name_ending 
    affectedRegGroupEMEC = helper.addGroup(
        larAffectedRegAlg,
        affectedRegGroupName+group_name_ending,
        '/LAr/',
        'run'
    )
    
    affectedRegGroupEMEC.defineHistogram('etaPOS,phi;RAW_LArAffectedRegionsEMECA',
                                         title='HV Affected Regions - EMECA - Samplings 1-3;#eta;#phi',
                                         type='TH2F',
                                         path=larAffReg_hist_path,
                                         weight='problem',
                                         xbins=lArDQGlobals.HVeta_EMEC["EMECA"],
                                         ybins=lArDQGlobals.HVphi_EMEC["EMECA"],
                                         merge='weightedAverage'
    )
    affectedRegGroupEMEC.defineHistogram('etaNEG,phi;RAW_LArAffectedRegionsEMECC',
                                         title='HV Affected Regions - EMECC - Samplings 1-3;#eta;#phi',
                                         type='TH2F',
                                         path=larAffReg_hist_path,
                                         weight='problem',
                                         xbins=lArDQGlobals.HVeta_EMEC["EMECC"],
                                         ybins=lArDQGlobals.HVphi_EMEC["EMECC"],
                                         merge='weightedAverage'
    )


    #HEC0
    group_name_ending="HEC0"
    larAffectedRegAlg.HEC0name=group_name_ending
    affectedRegGroupHEC0 = helper.addGroup(
        larAffectedRegAlg,
        affectedRegGroupName+group_name_ending,
        '/LAr/',
        'run'
    )
        
    affectedRegGroupHEC0.defineHistogram('etaPOS,phi;RAW_LArAffectedRegionsHECA0',
                                         title='HV Affected Regions - HECA - Layer 1;#eta;#phi',
                                         type='TH2F',
                                         path=larAffReg_hist_path,
                                         weight='problem',
                                         xbins=lArDQGlobals.HVeta_HECFcal["HECA"][0],xmin=lArDQGlobals.HVeta_HECFcal["HECA"][1],xmax=lArDQGlobals.HVeta_HECFcal["HECA"][2],
                                         ybins=lArDQGlobals.HVphi_HECFcal["HECA"][0],ymin=lArDQGlobals.HVphi_HECFcal["HECA"][1],ymax=lArDQGlobals.HVphi_HECFcal["HECA"][2],
                                         merge='weightedAverage'
    )
    affectedRegGroupHEC0.defineHistogram('etaNEG,phi;RAW_LArAffectedRegionsHECC0',
                                         title='HV Affected Regions - HECC - Layer 1;#eta;#phi',
                                         type='TH2F',
                                         path=larAffReg_hist_path,
                                         weight='problem',
                                         xbins=lArDQGlobals.HVeta_HECFcal["HECC"][0],xmin=lArDQGlobals.HVeta_HECFcal["HECC"][1],xmax=lArDQGlobals.HVeta_HECFcal["HECC"][2],
                                         ybins=lArDQGlobals.HVphi_HECFcal["HECC"][0],ymin=lArDQGlobals.HVphi_HECFcal["HECC"][1],ymax=lArDQGlobals.HVphi_HECFcal["HECC"][2],
                                         merge='weightedAverage'
    )

    #HEC1
    group_name_ending="HEC1"
    larAffectedRegAlg.HEC1name=group_name_ending
    affectedRegGroupHEC1 = helper.addGroup(
        larAffectedRegAlg,
        affectedRegGroupName+group_name_ending,
        '/LAr/',
        'run'
    )
    
    affectedRegGroupHEC1.defineHistogram('etaPOS,phi;RAW_LArAffectedRegionsHECA1',
                                         title='HV Affected Regions - HECA - Layer 2;#eta;#phi',
                                         type='TH2F',
                                         path=larAffReg_hist_path,
                                         weight='problem',
                                         xbins=lArDQGlobals.HVeta_HECFcal["HECA"][0],xmin=lArDQGlobals.HVeta_HECFcal["HECA"][1],xmax=lArDQGlobals.HVeta_HECFcal["HECA"][2],
                                         ybins=lArDQGlobals.HVphi_HECFcal["HECA"][0],ymin=lArDQGlobals.HVphi_HECFcal["HECA"][1],ymax=lArDQGlobals.HVphi_HECFcal["HECA"][2],
                                         merge='weightedAverage'
    )
    affectedRegGroupHEC1.defineHistogram('etaNEG,phi;RAW_LArAffectedRegionsHECC1',
                                         title='HV Affected Regions - HECC - Layer 2;#eta;#phi',
                                         type='TH2F',
                                         path=larAffReg_hist_path,
                                         weight='problem',
                                         xbins=lArDQGlobals.HVeta_HECFcal["HECC"][0],xmin=lArDQGlobals.HVeta_HECFcal["HECC"][1],xmax=lArDQGlobals.HVeta_HECFcal["HECC"][2],
                                         ybins=lArDQGlobals.HVphi_HECFcal["HECC"][0],ymin=lArDQGlobals.HVphi_HECFcal["HECC"][1],ymax=lArDQGlobals.HVphi_HECFcal["HECC"][2],
                                         merge='weightedAverage'
    )
    
    #HEC2
    group_name_ending="HEC2"
    larAffectedRegAlg.HEC2name=group_name_ending
    affectedRegGroupHEC2 = helper.addGroup(
        larAffectedRegAlg,
        affectedRegGroupName+group_name_ending,
        '/LAr/',
        'run'
    )
    
    affectedRegGroupHEC2.defineHistogram('etaPOS,phi;RAW_LArAffectedRegionsHECA2',
                                         title='HV Affected Regions - HECA - Layer 3;#eta;#phi',
                                         type='TH2F',
                                         path=larAffReg_hist_path,
                                         weight='problem',
                                         xbins=lArDQGlobals.HVeta_HECFcal["HECA"][0],xmin=lArDQGlobals.HVeta_HECFcal["HECA"][1],xmax=lArDQGlobals.HVeta_HECFcal["HECA"][2],
                                         ybins=lArDQGlobals.HVphi_HECFcal["HECA"][0],ymin=lArDQGlobals.HVphi_HECFcal["HECA"][1],ymax=lArDQGlobals.HVphi_HECFcal["HECA"][2],
                                         merge='weightedAverage'
    )
    affectedRegGroupHEC2.defineHistogram('etaNEG,phi;RAW_LArAffectedRegionsHECC2',
                                         title='HV Affected Regions - HECC - Layer 3;#eta;#phi',
                                         type='TH2F',
                                         path=larAffReg_hist_path,
                                         weight='problem',
                                         xbins=lArDQGlobals.HVeta_HECFcal["HECC"][0],xmin=lArDQGlobals.HVeta_HECFcal["HECC"][1],xmax=lArDQGlobals.HVeta_HECFcal["HECC"][2],
                                         ybins=lArDQGlobals.HVphi_HECFcal["HECC"][0],ymin=lArDQGlobals.HVphi_HECFcal["HECC"][1],ymax=lArDQGlobals.HVphi_HECFcal["HECC"][2],
                                         merge='weightedAverage'
    )
        
    #HEC3
    group_name_ending="HEC3"
    larAffectedRegAlg.HEC3name=group_name_ending
    affectedRegGroupHEC3 = helper.addGroup(
        larAffectedRegAlg,
        affectedRegGroupName+group_name_ending,
        '/LAr/',
        'run'
    )
    
    affectedRegGroupHEC3.defineHistogram('etaPOS,phi;RAW_LArAffectedRegionsHECA3',
                                         title='HV Affected Regions - HECA - Layer 4;#eta;#phi',
                                         type='TH2F',
                                         path=larAffReg_hist_path,
                                         weight='problem',
                                         xbins=lArDQGlobals.HVeta_HECFcal["HECA"][0],xmin=lArDQGlobals.HVeta_HECFcal["HECA"][1],xmax=lArDQGlobals.HVeta_HECFcal["HECA"][2],
                                         ybins=lArDQGlobals.HVphi_HECFcal["HECA"][0],ymin=lArDQGlobals.HVphi_HECFcal["HECA"][1],ymax=lArDQGlobals.HVphi_HECFcal["HECA"][2],
                                         merge='weightedAverage'
    )
    affectedRegGroupHEC3.defineHistogram('etaNEG,phi;RAW_LArAffectedRegionsHECC3',
                                         title='HV Affected Regions - HECC - Layer 4;#eta;#phi',
                                         type='TH2F',
                                         path=larAffReg_hist_path,
                                         weight='problem',
                                         xbins=lArDQGlobals.HVeta_HECFcal["HECC"][0],xmin=lArDQGlobals.HVeta_HECFcal["HECC"][1],xmax=lArDQGlobals.HVeta_HECFcal["HECC"][2],
                                         ybins=lArDQGlobals.HVphi_HECFcal["HECC"][0],ymin=lArDQGlobals.HVphi_HECFcal["HECC"][1],ymax=lArDQGlobals.HVphi_HECFcal["HECC"][2],
                                         merge='weightedAverage'
    )
    
    #FCal0
    group_name_ending="FCal0"
    larAffectedRegAlg.FCal0name=group_name_ending
    affectedRegGroupFCal0 = helper.addGroup(
        larAffectedRegAlg,
        affectedRegGroupName+group_name_ending,
        '/LAr/',
        'run'
    )
    
    affectedRegGroupFCal0.defineHistogram('etaPOS,phi;RAW_LArAffectedRegionsFCalA0',
                                          title='HV Affected Regions - FCalA - Layer 1;#eta;#phi',
                                          type='TH2F',
                                          path=larAffReg_hist_path,
                                          weight='problem',
                                          xbins=lArDQGlobals.HVeta_HECFcal["FCalA"][0],xmin=lArDQGlobals.HVeta_HECFcal["FCalA"][1],xmax=lArDQGlobals.HVeta_HECFcal["FCalA"][2],
                                          ybins=lArDQGlobals.HVphi_HECFcal["FCalA"][0],ymin=lArDQGlobals.HVphi_HECFcal["FCalA"][1],ymax=lArDQGlobals.HVphi_HECFcal["FCalA"][2],
                                          merge='weightedAverage'
    )
    affectedRegGroupFCal0.defineHistogram('etaNEG,phi;RAW_LArAffectedRegionsFCalC0',
                                          title='HV Affected Regions - FCalC - Layer 1;#eta;#phi',
                                          type='TH2F',
                                          path=larAffReg_hist_path,
                                          weight='problem',
                                          xbins=lArDQGlobals.HVeta_HECFcal["FCalC"][0],xmin=lArDQGlobals.HVeta_HECFcal["FCalC"][1],xmax=lArDQGlobals.HVeta_HECFcal["FCalC"][2],
                                          ybins=lArDQGlobals.HVphi_HECFcal["FCalC"][0],ymin=lArDQGlobals.HVphi_HECFcal["FCalC"][1],ymax=lArDQGlobals.HVphi_HECFcal["FCalC"][2],
                                          merge='weightedAverage'
    )

    #FCal1
    group_name_ending="FCal1"
    larAffectedRegAlg.FCal1name=group_name_ending
    affectedRegGroupFCal1 = helper.addGroup(
        larAffectedRegAlg,
        affectedRegGroupName+group_name_ending,
        '/LAr/',
        'run'
    )
    
    affectedRegGroupFCal1.defineHistogram('etaPOS,phi;RAW_LArAffectedRegionsFCalA1',
                                          title='HV Affected Regions - FCalA - Layer 2;#eta;#phi',
                                          type='TH2F',
                                          path=larAffReg_hist_path,
                                          weight='problem',
                                          xbins=lArDQGlobals.HVeta_HECFcal["FCalA"][0],xmin=lArDQGlobals.HVeta_HECFcal["FCalA"][1],xmax=lArDQGlobals.HVeta_HECFcal["FCalA"][2],
                                          ybins=lArDQGlobals.HVphi_HECFcal["FCalA"][0],ymin=lArDQGlobals.HVphi_HECFcal["FCalA"][1],ymax=lArDQGlobals.HVphi_HECFcal["FCalA"][2],
                                          merge='weightedAverage'
    )
    affectedRegGroupFCal1.defineHistogram('etaNEG,phi;RAW_LArAffectedRegionsFCalC1',
                                          title='HV Affected Regions - FCalC - Layer 2;#eta;#phi',
                                          type='TH2F',
                                          path=larAffReg_hist_path,
                                          weight='problem',
                                          xbins=lArDQGlobals.HVeta_HECFcal["FCalC"][0],xmin=lArDQGlobals.HVeta_HECFcal["FCalC"][1],xmax=lArDQGlobals.HVeta_HECFcal["FCalC"][2],
                                          ybins=lArDQGlobals.HVphi_HECFcal["FCalC"][0],ymin=lArDQGlobals.HVphi_HECFcal["FCalC"][1],ymax=lArDQGlobals.HVphi_HECFcal["FCalC"][2],
                                          merge='weightedAverage'
    )

    #FCal2
    group_name_ending="FCal2"
    larAffectedRegAlg.FCal2name=group_name_ending
    affectedRegGroupFCal2 = helper.addGroup(
        larAffectedRegAlg,
        affectedRegGroupName+group_name_ending,
        '/LAr/',
        'run'
    )
        
    affectedRegGroupFCal2.defineHistogram('etaPOS,phi;RAW_LArAffectedRegionsFCalA2',
                                          title='HV Affected Regions - FCalA - Layer 3;#eta;#phi',
                                          type='TH2F',
                                          path=larAffReg_hist_path,
                                          weight='problem',
                                          xbins=lArDQGlobals.HVeta_HECFcal["FCalA"][0],xmin=lArDQGlobals.HVeta_HECFcal["FCalA"][1],xmax=lArDQGlobals.HVeta_HECFcal["FCalA"][2],
                                          ybins=lArDQGlobals.HVphi_HECFcal["FCalA"][0],ymin=lArDQGlobals.HVphi_HECFcal["FCalA"][1],ymax=lArDQGlobals.HVphi_HECFcal["FCalA"][2],
                                          merge='weightedAverage'
    )
    affectedRegGroupFCal2.defineHistogram('etaNEG,phi;RAW_LArAffectedRegionsFCalC2',
                                          title='HV Affected Regions - FCalC - Layer 3;#eta;#phi',
                                          type='TH2F',
                                          path=larAffReg_hist_path,
                                          weight='problem',
                                          xbins=lArDQGlobals.HVeta_HECFcal["FCalC"][0],xmin=lArDQGlobals.HVeta_HECFcal["FCalC"][1],xmax=lArDQGlobals.HVeta_HECFcal["FCalC"][2],
                                          ybins=lArDQGlobals.HVphi_HECFcal["FCalC"][0],ymin=lArDQGlobals.HVphi_HECFcal["FCalC"][1],ymax=lArDQGlobals.HVphi_HECFcal["FCalC"][2],
                                          merge='weightedAverage'
    )


if __name__=='__main__':

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from LArMonitoring.LArMonConfigFlags import addLArMonFlags
    flags.addFlagsCategory("LArMon", addLArMonFlags)

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RAW_RUN2

    flags.Output.HISTFileName = 'LArAffectedRegionsOutput.root'
    flags.DQ.enableLumiAccess = False
    flags.DQ.useTrigger = False
    flags.lock()


    from CaloRec.CaloRecoConfig import CaloRecoCfg
    cfg=CaloRecoCfg(flags)

    #add affected regions
    affregmon = LArAffectedRegionsConfig(flags)
    cfg.merge(affregmon)

    flags.dump()
    f=open("AffectedRegionsMonMaker.pkl","wb")
    cfg.store(f)
    f.close()

