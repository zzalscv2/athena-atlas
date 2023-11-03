#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

# Define some handy strings for plot labels


# TO DO  - add defined cut names for selections, add them to plot names

# selections from the digi (ADC) loop
selStr = {}
selStr["passDigiNom"] ="for unmasked SCs with ADC_max-pedestal > 10*RMS(DB) & good quality bits"
selStr["badNotMasked"] = "for unmasked SCs which have bad quality bits"
# selections from the sc (ET) loop
selStr["passSCNom"] = "for unmasked SCs which pass #tau selection with non-zero ET"
selStr["passSCNom1"] = "for unmasked SCs which pass #tau selection with ET > 1 GeV"
selStr["passSCNom10"] = "for unmasked SCs which pass tau selection with ET > 10 GeV"
selStr["passSCNom10tauGt3"] = "for unmasked SCs which pass tau selection with ET > 10 GeV and #tau > 3"
selStr["saturNotMasked"] = "for unmasked SCs which are saturated"
selStr["OFCbOFNotMasked"] = "for unmasked SCs with OFCb in overflow"
selStr["onlofflEmismatch"] = "for unmasked SCs which pass #tau selection where online & offline energies are different"



def LArDigitalTriggMonConfig(inputFlags,larLATOMEBuilderAlg, nsamples=32, streamTypes=[]):
    '''Function to configures some algorithms in the monitoring system.'''
    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools

    from AthenaMonitoring.AthMonitorCfgHelper import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'LArDigitalTriggMonAlgCfg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    from LArMonitoring.GlobalVariables import lArDQGlobals
    
    from AthenaCommon.Logging import logging
    mlog = logging.getLogger( 'LArDigitalTriggMon' )

    if not inputFlags.DQ.enableLumiAccess:
       from LumiBlockComps.LuminosityCondAlgConfig import  LuminosityCondAlgCfg
       helper.resobj.merge(LuminosityCondAlgCfg(inputFlags))

    #get SC onl-offl mapping from DB    
    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    helper.resobj.merge(LArOnOffIdMappingSCCfg(inputFlags))
    
    # and elec. calib. coeffs
    from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBSCCfg
    helper.resobj.merge(LArElecCalibDBSCCfg(inputFlags, condObjs=["Ramp","DAC2uA", "Pedestal", "uA2MeV", "MphysOverMcal", "OFC", "Shape", "HVScaleCorr"]))

    larDigitalTriggMonAlg = helper.addAlgorithm(CompFactory.LArDigitalTriggMonAlg('larDigitalTriggMonAlg'))
    larDigitalTriggMonAlg.ProblemsToMask=["maskedOSUM"] #highNoiseHG","highNoiseMG","highNoiseLG","deadReadout","deadPhys"]
         
    hasEtId = False
    hasEt = False
    hasAdc = False
    hasAdcBas = False
    for i in range(0,len(streamTypes)):
        mlog.info("runinfo.streamTypes()[i]: "+str(streamTypes[i]))
        if streamTypes[i] ==  "SelectedEnergy":
            hasEtId = True
            larDigitalTriggMonAlg.EtName = "SC_ET_ID"
            larDigitalTriggMonAlg.LArRawSCContainerKey = "SC_ET_ID"
        if streamTypes[i] ==  "Energy":
            hasEt = True
            larDigitalTriggMonAlg.EtName = "SC_ET"
            larDigitalTriggMonAlg.LArRawSCContainerKey = "SC_ET"
        if streamTypes[i] ==  "RawADC":
            hasAdc = True
            larDigitalTriggMonAlg.AdcName = "SC"
            larLATOMEBuilderAlg.LArDigitKey = "SC"
            larLATOMEBuilderAlg.isADCBas = False
            larDigitalTriggMonAlg.LArDigitContainerKey = "SC"
        if streamTypes[i] ==  "ADC":
            hasAdcBas = True
            larDigitalTriggMonAlg.AdcName = "SC_ADC_BAS"
            larDigitalTriggMonAlg.LArDigitContainerKey = "SC_ADC_BAS"
            larLATOMEBuilderAlg.isADCBas = True
            larLATOMEBuilderAlg.LArDigitKey = "SC_ADC_BAS"

    if (hasEtId and hasEt): #prefer EtId if both in recipe
        hasEt = False
        larDigitalTriggMonAlg.EtName = "SC_ET_ID"

    if (hasAdc and hasAdcBas): #prefer Raw Adc if both in recipe
        hasAdc = False
        larDigitalTriggMonAlg.AdcName = "SC"

    mlog.info("Mux settings from COOL:")
    mlog.info("has ET Id: "+str(hasEtId))
    mlog.info("has ET: "+str(hasEt))
    mlog.info("has ADC: "+str(hasAdc))
    mlog.info("has ADC Bas: "+str(hasAdcBas))
    mlog.info("Et name: "+str(larDigitalTriggMonAlg.EtName))
    mlog.info("ADC name: "+str(larDigitalTriggMonAlg.AdcName))

    SCGroupName="SC"
    larDigitalTriggMonAlg.SCMonGroup=SCGroupName
    # uncomment if needed:
    #larDigitalTriggMonAlg.FileKey="CombinedMonitoring"

    SCGroup = helper.addGroup(
        larDigitalTriggMonAlg,
        SCGroupName,
        '/LArDigitalTrigger/',
        'run'
    )

    sc_hist_path='/'

    LatomeDetBinMapping = dict([
        ("0x48",{"Subdet":"FCALC","Bin":1}),
        ("0x4c",{"Subdet":"EMEC/HECC","Bin":3}),
        ("0x44",{"Subdet":"EMECC","Bin":11}),
        ("0x4a",{"Subdet":"EMB/EMECC","Bin":27}),
        ("0x42",{"Subdet":"EMBC","Bin":43}),
        ("0x41",{"Subdet":"EMBA","Bin":59}),
        ("0x49",{"Subdet":"EMB/EMECA","Bin":75}),
        ("0x43",{"Subdet":"EMECA","Bin":91}),
        ("0x4b",{"Subdet":"EMEC/HECA","Bin":107}),
        ("0x47",{"Subdet":"FCALA","Bin":115})
    ])    
    NLatomeBins=117
    NLatomeBins_side=59

    BinLabel=[]
    BinLabel_A=[]
    BinLabel_C=[]
    phi=0
    for bb in range (0,NLatomeBins):
        Label=""
        for detID in LatomeDetBinMapping:
            if bb==(LatomeDetBinMapping[detID]["Bin"]-1):
                Label=LatomeDetBinMapping[detID]["Subdet"]
                phi=1
                break
        if bb < NLatomeBins_side:
            BinLabel_C+=[Label+str(phi)]
        else:
            BinLabel_A+=[Label+str(phi)]

        BinLabel+=[Label+str(phi)]
        phi+=1
        
    
    #### Plots from Digi (ADC) loop
    SCGroup.defineHistogram('Digi_maxpos,Digi_partition;Partition_maxSamplePosition', 
                            title='Partition vs. position of max sample '+selStr["passDigiNom"],
                            cutmask='passDigiNom',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                            ybins=lArDQGlobals.N_Partitions, ymin=-0.5, ymax=lArDQGlobals.N_Partitions-0.5,
                            xlabels = [str(x) for x in range(1,nsamples+1)],      
                            ylabels=lArDQGlobals.Partitions)

    SCGroup.defineHistogram('Digi_sampos,Digi_ADC;ADCZoom_samplePosition',
                            title='ADC (zoom) vs sample position '+selStr["passDigiNom"],
                            cutmask='passDigiNom',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                            xlabels = [str(x) for x in range(1,nsamples+1)],
                            ybins=750, ymin=0, ymax=1300) #start from 0 otherwise miss endcap pedestals
    
    SCGroup.defineHistogram('Digi_sampos,Digi_ADC;ADCFullRange_samplePosition', 
                            title='ADC vs sample position '+selStr["passDigiNom"],
                            cutmask='passDigiNom',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                            xlabels = [str(x) for x in range(1,nsamples+1)],
                            ybins=500, ymin=0, ymax=5000) #raw ADC is 12 bit

    SCGroup.defineHistogram('Digi_sampos,Pedestal;PedestalFullRange_samplePosition',
                            title='Pedestal vs sample position '+selStr["passDigiNom"],
                            cutmask='passDigiNom',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                            xlabels = [str(x) for x in range(1,nsamples+1)],
                            ybins=500, ymin=0, ymax=5000) #raw ADC is 12 bit 


    SCGroup.defineHistogram('Digi_latomeSourceIdBIN,Digi_ADC;ADCFullRange_LATOME',
                            title='ADC vs LATOME name '+selStr["passDigiNom"]+'; ; ADC',
                            cutmask='passDigiNom',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                            ybins=500, ymin=-2, ymax=2500, #raw ADC is 12 bit
                            xlabels=BinLabel)

    SCGroup.defineHistogram('Digi_latomeSourceIdBIN,Pedestal;Pedestal_LATOME',
                            title='Pedestal vs LATOME name '+selStr["passDigiNom"]+'; ; Pedestal',
                            cutmask='passDigiNom',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                            ybins=500, ymin=-2, ymax=2500, #raw ADC is 12 bit
                            xlabels=BinLabel)

    SCGroup.defineHistogram('Digi_latomeSourceIdBIN,Digi_maxpos;MaxSamplePosition_LATOME',
                            title='Position of max sample vs. LATOME '+selStr["passDigiNom"],
                            type='TH2F',
                            cutmask='passDigiNom',
                            path=sc_hist_path,
                            xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                            ybins=nsamples,ymin=0.5,ymax=nsamples+0.5,
                            xlabels=BinLabel,
                            ylabels = [str(x) for x in range(1,nsamples+1)])  

    SCGroup.defineHistogram('Digi_latomeSourceIdBIN,Digi_Diff_ADC_Ped;Diff_ADC_Ped_LATOME',
                            title='ADC - Pedestal vs LATOME name '+selStr["passDigiNom"]+'; ; ADC - Pedestal',
                            type='TH2F',
                            cutmask='passDigiNom',
                            path=sc_hist_path,
                            xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                            ybins=64, ymin=-32, ymax=32,
                            xlabels=BinLabel)

    SCGroup.defineHistogram('Digi_Diff_ADC_Ped;Diff_ADC_Ped',
                            title='LATOME (ADC-ped) '+selStr["passDigiNom"]+'; (ADC - pedestal)',
                            type='TH1F',
                            cutmask='passDigiNom',
                            path=sc_hist_path,
                            xbins=50,xmin=-25,xmax=25)

    SCGroup.defineHistogram('Digi_sampos,Digi_Diff_ADC_Ped_Norm;Diff_ADC_Ped_Norm_SamplePos',
                            title='(ADC-ped)/fabs(ADC_max-ped) '+selStr["passDigiNom"]+'; Sample position; (ADC - pedestal) / fabs(ADC_max - pedestal)',
                            type='TH2F',
                            cutmask='passDigiNom',
                            path=sc_hist_path,
                            ybins=40,ymin=-1,ymax=1,
                            xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                            xlabels = [str(x) for x in range(1,nsamples+1)])

    SCGroup.defineHistogram('Digi_latomeSourceIdBIN,Digi_Diff_ADC_Ped_Norm;Diff_ADC_Ped_Norm_LATOME',
                            title='(ADC-ped)/fabs(ADC_max-ped) '+selStr["passDigiNom"]+'; LATOME Name; (ADC - pedestal) / fabs(ADC_max - pedestal)',
                            type='TH2F',
                            cutmask='passDigiNom',
                            path=sc_hist_path,
                            xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                            ybins=64, ymin=-32, ymax=32,
                            xlabels=BinLabel)


    # Plotting for SCs which are NOT masked but have bad quality bits
    SCGroup.defineHistogram('Digi_eta,Digi_phi;Coverage_eta_phi_BadQualityBit',
                            title='SC coverage '+selStr["badNotMasked"]+': #phi vs #eta;#eta;#phi',
                            type='TH2F',
                            cutmask='badNotMasked',
                            path=sc_hist_path,
                            xbins=lArDQGlobals.SuperCell_Variables["etaRange"]["All"]["All"],
                            ybins=lArDQGlobals.SuperCell_Variables["phiRange"]["All"]["All"])    





    #### Plots from the SC ET loop
    for thisSel in [ "passSCNom", "passSCNom1", "passSCNom10", "passSCNom10tauGt3", "onlofflEmismatch", "saturNotMasked", "OFCbOFNotMasked" ]:        
        SCGroup.defineHistogram('SC_eta,SC_phi;Coverage_eta_phi_'+thisSel, 
                                title='SC coverage '+selStr[thisSel]+': #phi vs #eta;#eta;#phi',
                                type='TH2F',
                                cutmask=thisSel,
                                path=sc_hist_path,
                                xbins=lArDQGlobals.SuperCell_Variables["etaRange"]["All"]["All"],
                                ybins=lArDQGlobals.SuperCell_Variables["phiRange"]["All"]["All"])    

    SCGroup.defineHistogram('SC_ET_onl,SC_ET_ofl;OnlOfl_ET_2D',
                            title='LATOME E_{T} vs Offline Computation '+selStr["passSCNom"]+'; E_{T} Onl;E_{T} Offl [GeV]',
                            type='TH2F',
                            cutmask='passSCNom',
                            path=sc_hist_path,
                            xbins=0,xmin=0,xmax=20,
                            ybins=0,ymin=0,ymax=20)
    
    SCGroup.defineHistogram('SC_ET_diff;OnlOfl_Etdiff',
                            title='LATOME E_{T} vs Offline Computation '+selStr["passSCNom"]+'; E_{T} Onl - E_{T} Offl [GeV]; Evts;',
                            type='TH1F',
                            cutmask='passSCNom',
                            path=sc_hist_path,
                            xbins=200,xmin=-10,xmax=10)


    for thisSel in [ "passSCNom", "passSCNom1", "passSCNom10", "passSCNom10tauGt3" ]:
        SCGroup.defineHistogram('SC_time;OfflineLATOMEtime_'+thisSel,
                                title='LATOME #tau from Offline Computation '+selStr[thisSel]+';#tau [ns]; Evts;',
                                type='TH1F',
                                cutmask=thisSel,
                                path=sc_hist_path,
                                xbins=100,xmin=-25,xmax=25)

        SCGroup.defineHistogram('SC_eta,SC_phi,SC_ET_onl;Coverage_Et_onl_'+thisSel,
                                title='SC Energy '+selStr[thisSel]+': #phi vs #eta;#eta;#phi',
                                type='TProfile2D',
                                cutmask=thisSel,
                                path=sc_hist_path,
                                xbins=lArDQGlobals.SuperCell_Variables["etaRange"]["All"]["All"],
                                ybins=lArDQGlobals.SuperCell_Variables["phiRange"]["All"]["All"])


    SCGroup.defineHistogram('lumi_block,SC_time;MeanOfflineLATOMEtime_perLB',
                            title='Average LATOME #tau from Offline computation per LB '+selStr["passSCNom"]+'; LumiBloc; #tau [ns]',
                            type='TProfile',
                            cutmask='passSCNom',
                            path=sc_hist_path,
                            xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)

    SCGroup.defineHistogram('SC_ET_onl;SCeT', 
                            title='SC eT [GeV] '+selStr["passSCNom"],
                            type='TH1F',
                            cutmask='passSCNom',
                            path=sc_hist_path,
                            xbins=500, xmin=-100, xmax=400)

    SCGroup.defineHistogram('SC_latomeSourceIdBIN,SC_ET_onl;SCeT_LATOME',
                            title='SC ET [GeV] vs LATOME name '+selStr["passSCNom"]+'; ; E_{T}^{SC} [GeV]',
                            type='TH2F',
                            cutmask='passSCNom',
                            path=sc_hist_path,
                            xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                            ybins=200, ymin=-10, ymax=200,
                            xlabels=BinLabel)


    SCGroup.defineHistogram('SC_latomeSourceIdBIN,SC_time;MeanOfflineLATOMEtime_perLATOME', 
                            title='Average LATOME #tau from Offline computation per LATOME'+selStr["passSCNom"]+'; LATOME ; #tau [ns]',
                            type='TH2F',
                            cutmask='passSCNom',
                            path=sc_hist_path,
                            xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                            ybins=200, ymin=-50, ymax=50,
                            xlabels=BinLabel)
    SCGroup.defineHistogram('lumi_block,SC_latomeSourceIdBIN,SC_time;MeanOfflineLATOMEtime_perLB_perLATOME',
                            title='SC #tau '+selStr["passSCNom"]+': LATOME vs LB;LB;LATOME',
                            type='TProfile2D',
                            cutmask='passSCNom',
                            path=sc_hist_path,                            
                            xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max,
                            ybins=NLatomeBins,ymin=1,ymax=NLatomeBins+1,
                            ylabels=BinLabel)


    SCGroup.defineHistogram('SC_eta,SC_phi,SC_time;Coverage_offlineLATOMEtime_passSCNom',
                            title='LATOME #tau from Offline Computation '+selStr["passSCNom"]+': #phi vs #eta;#eta;#phi',
                            type='TProfile2D',
                            cutmask='passSCNom',
                            path=sc_hist_path,
                            xbins=lArDQGlobals.SuperCell_Variables["etaRange"]["All"]["All"],
                            ybins=lArDQGlobals.SuperCell_Variables["phiRange"]["All"]["All"])


    #### Plots from LATOME header loop
    SCGroup.defineHistogram('lumi_block,event_size;EventSizeLB',
                            title='Digital trigger event size per LB; LumiBlock; Event size [MB]',
                            type='TProfile',
                            path=sc_hist_path,
                            xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)



    #### Per-subdetector/layer plots
    layerList = ["EMBPA", "EMBPC", "EMB1A", "EMB1C", "EMB2A", "EMB2C", "EMB3A", "EMB3C", "HEC0A", "HEC0C", "HEC1A", "HEC1C", "HEC2A", "HEC2C", "HEC3A", "HEC3C", "EMECPA", "EMECPC", "EMEC1A", "EMEC1C", "EMEC2A", "EMEC2C", "EMEC3A", "EMEC3C", "FCAL1A", "FCAL1C", "FCAL2A", "FCAL2C", "FCAL3A", "FCAL3C"]

    partGroup_digi = helper.addArray([layerList], larDigitalTriggMonAlg, 'LArDigitalTriggerMon_digi', topPath='/LArDigitalTrigger/PerPartition/')
    partGroup_sc = helper.addArray([layerList], larDigitalTriggMonAlg, 'LArDigitalTriggerMon_sc', topPath='/LArDigitalTrigger/PerPartition/')

    
    for part in larDigitalTriggMonAlg.LayerNames:
        selStrPart = {}
        for sel in selStr.keys():
            selStrPart[sel] = "in "+part+" "+selStr[sel]

        Part = part[:-2]
        if Part == "FCAL": 
            Part = "FCal"
        Side = part[-1]
        Sampling = part[-2]
        if Sampling == "P": 
            Sampling = "0"


        #### Plots from Digi (ADC) loop
        partGroup_digi.defineHistogram('Digi_part_eta,Digi_part_phi;Coverage_eta_phi_digi',
                                       title='SC coverage '+selStrPart["passDigiNom"]+': #phi vs #eta;#eta;#phi',
                                       type='TH2F', 
                                       path='Coverage/passDigiNom',
                                       cutmask='Digi_part_passDigiNom',
                                       xbins=lArDQGlobals.SuperCell_Variables["etaRange"][Part][Side][Sampling],
                                       ybins=lArDQGlobals.SuperCell_Variables["phiRange"][Part][Side][Sampling],
                                       pattern=[(part)])

        partGroup_digi.defineHistogram('Digi_part_eta,Digi_part_phi;Coverage_eta_phi_bad',
                                       title='SC coverage '+selStrPart["badNotMasked"]+': #phi vs #eta;#eta;#phi',
                                       type='TH2F', 
                                       path='Coverage/BadNotMasked',
                                       cutmask='Digi_part_badNotMasked',
                                       xbins=lArDQGlobals.SuperCell_Variables["etaRange"][Part][Side][Sampling],
                                       ybins=lArDQGlobals.SuperCell_Variables["phiRange"][Part][Side][Sampling],
                                       pattern=[(part)])


        partGroup_digi.defineHistogram('Digi_part_eta,Digi_part_phi,Digi_part_diff_adc_ped;Coverage_diff_adc_ped',
                                       title='ADC - Pedestal'+selStrPart["passDigiNom"]+': #phi vs #eta;#eta;#phi',
                                       type='TProfile2D',
                                       cutmask='Digi_part_passDigiNom',
                                       path='Diff_ADC_Ped',
                                       xbins=lArDQGlobals.SuperCell_Variables["etaRange"][Part][Side][Sampling],
                                       ybins=lArDQGlobals.SuperCell_Variables["phiRange"][Part][Side][Sampling],
                                       pattern=[(part)])

        partGroup_digi.defineHistogram('Digi_part_BCID, Digi_part_adc;ADCvsBCID', 
                                       title='ADC value vs BCID '+selStrPart["passDigiNom"]+'; BCID; ADC Value',
                                       type='TProfile',
                                       cutmask='Digi_part_passDigiNom',
                                       path='BaselineCorrection/Raw_ADC',
                                       xbins=3564,xmin=-0.5,xmax=3563.5,
                                       ybins=500, ymin=0, ymax=5000,
                                       pattern=[(part)])

        partGroup_digi.defineHistogram('Digi_part_BCID, Digi_part_diff_adc_ped;Diff_ADC_Ped_vs_BCID', 
                                       title='ADC - Ped value vs BCID '+selStrPart["passDigiNom"]+'; BCID; ADC Value',
                                       type='TProfile',
                                       cutmask='Digi_part_passDigiNom',
                                       path='BaselineCorrection/Diff_ADC_Ped',
                                       xbins=3564,xmin=-0.5,xmax=3563.5,
                                       ybins=500, ymin=-5, ymax=5,
                                       pattern=[(part)])


        #### Plots from SC ET loop 

        for thisSel in [ "passSCNom", "passSCNom1", "passSCNom10", "passSCNom10tauGt3", "saturNotMasked", "OFCbOFNotMasked" ]:
            partGroup_sc.defineHistogram('SC_part_eta,SC_part_phi;Coverage_eta_phi_'+thisSel,
                                         title='SC coverage '+selStrPart[thisSel]+': #phi vs #eta;#eta;#phi',
                                         type='TH2F', 
                                         path='Coverage/'+thisSel,
                                         cutmask='SC_part_'+thisSel,
                                         xbins=lArDQGlobals.SuperCell_Variables["etaRange"][Part][Side][Sampling],
                                         ybins=lArDQGlobals.SuperCell_Variables["phiRange"][Part][Side][Sampling],
                                         pattern=[(part)])

        for thisSel in [ "passSCNom", "passSCNom1", "passSCNom10" ]:
            partGroup_sc.defineHistogram('SC_part_time;OfflineLATOMEtime_'+thisSel,
                                         title='LATOME #tau from Offline Computation '+selStrPart[thisSel]+':#tau [ns]; Evts',
                                         type='TH1F', 
                                         path='OfflineLATOMETime/'+thisSel,
                                         cutmask='SC_part_'+thisSel,
                                         xbins=100,xmin=-25,xmax=25,
                                         pattern=[(part)])

            partGroup_sc.defineHistogram('SC_part_BCID,SC_part_time;MeanOfflineLATOMEtime_perBCID_'+thisSel,
                                         title='Average LATOME #tau from Offline computation per BCID '+selStrPart[thisSel]+'; BCID; #tau [ns]',
                                         type='TProfile',
                                         cutmask='SC_part_'+thisSel,
                                         path='OfflineLATOMETime_perBCID/'+thisSel,
                                         xbins=3564,xmin=-0.5,xmax=3563.5, 
                                         pattern=[(part)])

            partGroup_sc.defineHistogram('SC_part_LB,SC_part_time;MeanOfflineLATOMEtime_perLB_'+thisSel,
                                         title='Average LATOME #tau from Offline computation per LB '+selStrPart[thisSel]+'; LumiBlock; #tau [ns]',
                                         type='TProfile',
                                         cutmask='SC_part_'+thisSel,
                                         path='OfflineLATOMETime_perLB/'+thisSel,
                                         xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max,
                                         pattern=[(part)])

            partGroup_sc.defineHistogram('SC_part_eta,SC_part_phi,SC_part_time;Coverage_offlineLATOMEtime_'+thisSel,
                                         title='LATOME #tau from Offline Computation '+selStrPart[thisSel]+': #phi vs #eta;#eta;#phi',
                                         type='TProfile2D',
                                         cutmask='SC_part_'+thisSel,
                                         path='OfflineLATOMETimeProfile/'+thisSel,
                                         xbins=lArDQGlobals.SuperCell_Variables["etaRange"][Part][Side][Sampling],
                                         ybins=lArDQGlobals.SuperCell_Variables["phiRange"][Part][Side][Sampling],
                                         pattern=[(part)])


            partGroup_sc.defineHistogram('SC_part_ET_onl;SCeT_'+thisSel,
                                         title='SC Energy '+selStrPart[thisSel]+';',
                                         type='TH1F',
                                         path='ET_TH1/'+thisSel,
                                         cutmask='SC_part_'+thisSel,
                                         xbins=500, xmin=-100, xmax=400,
                                         pattern=[(part)])

            partGroup_sc.defineHistogram('SC_part_eta,SC_part_phi,SC_part_ET_onl;Coverage_Et_onl_'+thisSel,
                                         title='SC Energy '+selStrPart[thisSel]+': #phi vs #eta;#eta;#phi',
                                         type='TProfile2D',
                                         cutmask='SC_part_'+thisSel,
                                         path='ETProfile/'+thisSel,
                                         xbins=lArDQGlobals.SuperCell_Variables["etaRange"][Part][Side][Sampling],
                                         ybins=lArDQGlobals.SuperCell_Variables["phiRange"][Part][Side][Sampling],
                                         pattern=[(part)])


        partGroup_sc.defineHistogram('SC_part_BCID,SC_part_ET_onl_muscaled;AvEnergyVsBCID',
                                     title='Average Energy vs BCID '+selStrPart["passSCNom"]+'; BCID; Energy per SC [MeV]',
                                     type='TProfile',
                                     cutmask='SC_part_passSCNom',
                                     path='BaselineCorrection/AvEnergyVsBCID',
                                     xbins=3564,xmin=-0.5,xmax=3563.5,
                                     ybins=10, ymin=-20, ymax=20,
                                     pattern=[(part)])


    return helper.result()


if __name__=='__main__':

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   from AthenaCommon.Logging import log
   from AthenaCommon.Constants import WARNING,ERROR
   log.setLevel(WARNING)
   flags = initConfigFlags()
   
   def __monflags():
      from LArMonitoring.LArMonConfigFlags import createLArMonConfigFlags
      return createLArMonConfigFlags()

   flags.addFlagsCategory("LArMon", __monflags)

   flags.Input.Files = ["/eos/atlas/atlastier0/rucio/data21_idcomm/physics_CosmicCalo/00401410/data21_idcomm.00401410.physics_CosmicCalo.merge.RAW/data21_idcomm.00401410.physics_CosmicCalo.merge.RAW._lb0722._SFO-ALL._0001.1"]
   
   flags.Output.HISTFileName = 'LArDigitalTriggMonOutput.root'

   flags.DQ.useTrigger = False
   from AthenaConfiguration.Enums import BeamType
   flags.Beam.Type = BeamType.Collisions
   flags.lock()

   # in case of tier0 workflow:
   from CaloRec.CaloRecoConfig import CaloRecoCfg
   cfg=CaloRecoCfg(flags)

   #from AthenaCommon.AppMgr import (ServiceMgr as svcMgr,ToolSvc)
   from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
   SCData_acc =  LArRawSCDataReadingCfg(flags)
   SCData_acc.OutputLevel=WARNING
 
   cfg.merge(SCData_acc)

   aff_acc = LArDigitalTriggMonConfig(flags)
   cfg.merge(aff_acc)
   
   cfg.getCondAlgo("LArHVCondAlg").OutputLevel=ERROR

   flags.dump()
   f=open("LArDigitalTriggMon.pkl","wb")
   cfg.store(f)
   f.close()




