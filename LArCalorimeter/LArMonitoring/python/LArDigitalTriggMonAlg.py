#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

# Define some handy strings for plot labels
RMScut = "with ADC_max-pedestal > 10*RMS(DB)"
badSCcut = "(excluding bad SC)"
taucut = "with abs(#tau) > 3"

from ROOT import TMath

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


    SCGroup.defineHistogram('Mmaxpos,Mpartition;Partition_maxSamplePosition', 
                            title='Partition vs. position of max sample',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                            ybins=lArDQGlobals.N_Partitions, ymin=-0.5, ymax=lArDQGlobals.N_Partitions-0.5,
                            xlabels = [str(x) for x in range(1,nsamples+1)],      
                            ylabels=lArDQGlobals.Partitions)

    SCGroup.defineHistogram('Msampos,MADC;ADCZoom_samplePosition',
                            title='ADC (zoom) vs sample position',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                            xlabels = [str(x) for x in range(1,nsamples+1)],
                            ybins=750, ymin=0, ymax=1300) #start from 0 otherwise miss endcap pedestals
    
    SCGroup.defineHistogram('Msampos,MADC;ADCFullRange_samplePosition', 
                            title='ADC vs sample position',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                            xlabels = [str(x) for x in range(1,nsamples+1)],
                            ybins=500, ymin=0, ymax=5000) #raw ADC is 12 bit

    SCGroup.defineHistogram('Msampos,Pedestal;PedestalFullRange_samplePosition',
                            title='Pedestal vs sample position',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                            xlabels = [str(x) for x in range(1,nsamples+1)],
                            ybins=500, ymin=0, ymax=5000) #raw ADC is 12 bit 


    SCGroup.defineHistogram('MlatomeSourceIdBIN,MADC;ADCFullRange_LATOME',
                            title='ADC vs LATOME name; ; ADC',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                            ybins=500, ymin=-2, ymax=2500, #raw ADC is 12 bit
                            xlabels=BinLabel)

    SCGroup.defineHistogram('MlatomeSourceIdBIN,Pedestal;Pedestal_LATOME',
                            title='Pedestal vs LATOME name; ; Pedestal',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                            ybins=500, ymin=-2, ymax=2500, #raw ADC is 12 bit
                            xlabels=BinLabel)


    #have to make plots per sampling and per subdetector, as in LArCoverageAlg, also update GlobalVariables.py
    SCGroup.defineHistogram('MSCetaEcomp,MSCphiEcomp;EtaPhiMap_EnergyMismatch', 
                            title='SCs where E_{T} offline != E_{T} online: #phi vs #eta;#eta;#phi',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025
                            ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size
    
    SCGroup.defineHistogram('MSCeta,MSCphi;Coverage_eta_phi', 
                            title='SC coverage: #phi vs #eta;#eta;#phi',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025
                            ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size

    
    SCGroup.defineHistogram('superCellEta_Satur_all,superCellPhi_Satur_all;Coverage_phi_eta_Saturation_all',
                            title='SC saturation coverage: #phi vs #eta;#eta;#phi',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025
                            ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size      

    SCGroup.defineHistogram('superCellEta_Satur,superCellPhi_Satur;Coverage_phi_eta_Saturation',
                            title='SC saturation coverage (Bad SCs excluded): #phi vs #eta;#eta;#phi',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025
                            ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size      

    SCGroup.defineHistogram('OFCb_overflow_eta_all,OFCb_overflow_phi_all;Coverage_OFCb_overflow_all',
                            title='SC OFCb overflow coverage: #phi vs #eta;#eta;#phi',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025 
                            ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size      
    
    SCGroup.defineHistogram('OFCa_overflow_eta_all,OFCa_overflow_phi_all;Coverage_OFCa_overflow_all',
                            title='SC OFCa overflow coverage: #phi vs #eta;#eta;#phi',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025
                            ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size      

    SCGroup.defineHistogram('OFCb_overflow_eta,OFCb_overflow_phi;Coverage_OFCb_overflow',
                            title='SC OFCb overflow coverage '+badSCcut+': #phi vs #eta;#eta;#phi',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025
                            ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size      

    SCGroup.defineHistogram('OFCa_overflow_eta,OFCa_overflow_phi;Coverage_OFCa_overflow',
                            title='SC OFCa overflow coverage '+badSCcut+': #phi vs #eta;#eta;#phi',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025
                            ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size      

    ### END OFC TESTS


    SCGroup.defineHistogram('badQualBit_eta,badQualBit_phi;CoveragePhiEta_BadQualityBit',
                            title='SC coverage: #phi vs #eta for bad quality bits;#eta;#phi',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025 
                            ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size 


    SCGroup.defineHistogram('eta_rms_fromDB_all,phi_rms_fromDB_all;CoveragePhiEta_Cut5RMS',
                            title='SC coverage: #phi vs #eta '+RMScut+';#eta;#phi',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025
                            ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size 

    SCGroup.defineHistogram('eta_rms_fromDB_all,phi_rms_fromDB_all;CoveragePhiEta_Cut5RMS',
                            title='SC coverage: #phi vs #eta '+RMScut+';#eta;#phi',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025
                            ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size                                                                                               
    SCGroup.defineHistogram('MlatomeSourceIdBIN,Mmaxpos;MaxSamplePosition_LATOME',
                            title='Position of max sample vs. LATOME '+RMScut+'',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                            ybins=nsamples,ymin=0.5,ymax=nsamples+0.5,
                            xlabels=BinLabel,
                            ylabels = [str(x) for x in range(1,nsamples+1)])  
        

    SCGroup.defineHistogram('Menergy_onl,Menergy_ofl;OnlOfl_Et_2D',
                            title='LATOME E_{T} vs Offline Computation; E_{T} Onl;E_{T} Offl[MeV]',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=100,xmin=0,xmax=20000,
                            ybins=100,ymin=0,ymax=20000)
    
    SCGroup.defineHistogram('MSCEt_diff;OnlOfl_Etdiff',
                            title='LATOME E_{T} vs Offline Computation; E_{T} Onl - E_{T} Offl[MeV]; Evts;',
                            type='TH1F',
                            path=sc_hist_path,
                            xbins=200,xmin=-100,xmax=100)

    SCGroup.defineHistogram('MSCtime;OfflineLATOMEtime',
                            title='LATOME #tau from Offline Computation;#tau [ns]; Evts;',
                            type='TH1F',
                            path=sc_hist_path,
                            xbins=100,xmin=-25,xmax=25)

    SCGroup.defineHistogram('LB,MSCtime;MeanOfflineLATOMEtime_perLB',
                            title='Average LATOME #tau from Offline computation per LB; LumiBloc; #tau [ns]',
                            type='TProfile',
                            path=sc_hist_path,
                            xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)


    #for part in LArDigitalTriggMonAlg.LayerNames:
    for part in ["EMBPA", "EMBPC", "EMB1A", "EMB1C", "EMB2A", "EMB2C", "EMB3A", "EMB3C", "HEC0A", "HEC0C", "HEC1A", "HEC1C", "HEC2A", "HEC2C", "HEC3A", "HEC3C", "EMECPA", "EMECPC", "EMEC1A", "EMEC1C", "EMEC2A", "EMEC2C", "EMEC3A", "EMEC3C", "FCAL1A", "FCAL1C", "FCAL2A", "FCAL2C", "FCAL3A", "FCAL3C"]:
        Part = part[:-2]
        if Part == "FCAL": 
            Part = "FCal"
        Side = part[-1]
        Sampling = part[-2]
        if Sampling == "P": 
            Sampling = "0"

        
        SCGroup.defineHistogram('MSCtime_'+part+';OfflineLATOMEtime_'+part,
                                title='LATOME #tau from Offline Computation;#tau [ns]; Evts;',
                                type='TH1F',
                                path=sc_hist_path + 'OfflineLATOMETiming',
                                xbins=100,xmin=-25,xmax=25)

        SCGroup.defineHistogram('MSCtimeNoZero_'+part+';OfflineLATOMEtime_Nonzero_'+part,
                                title='LATOME #tau from Offline Computation (exclude zero);#tau [ns]; Evts;',
                                type='TH1F',
                                path=sc_hist_path + 'OfflineLATOMETiming',
                                xbins=100,xmin=-25,xmax=25)

        SCGroup.defineHistogram('tau_above_cut_eta_'+part+',tau_above_cut_phi_'+part+';Tau_above_cut_'+part,
                                title='SC where '+taucut+' '+part+' '+badSCcut+': #phi vs #eta;#eta;#phi',
                                type='TH2F',
                                path=sc_hist_path+'CoveragePerPartition/tauCut',
                                xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
                                ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])    


        SCGroup.defineHistogram('BCID,AvEnergy_'+part+';AvEnergyVsBCID_'+part, 
                                title='Average Energy vs BCID '+part+'; BCID; Energy per SC [MeV]',
                                type='TProfile',
                                path=sc_hist_path + 'BaselineCorrection',
                                xbins=3564,xmin=-0.5,xmax=3563.5,
                                ybins=10, ymin=-20, ymax=20)
        
        SCGroup.defineHistogram('BCID,ADC_'+part+';ADCvsBCID_'+part, 
                                title='ADC value vs BCID '+part+'; BCID; ADC Value',
                                type='TProfile',
                                path=sc_hist_path + 'BaselineCorrection/Raw_ADC',
                                xbins=3564,xmin=-0.5,xmax=3563.5,
                                ybins=500, ymin=0, ymax=5000)

        SCGroup.defineHistogram('superCellEta_'+part+',superCellPhi_'+part+';CoveragePhiEta_'+part,
                                title='SC coverage '+part+': #phi vs #eta;#eta;#phi',
                                type='TH2F',
                                path=sc_hist_path+'CoveragePerPartition/NoCut',
                                xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
                                ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])    
        
        SCGroup.defineHistogram('eta_rms_fromDB_'+part+',phi_rms_fromDB_'+part+';CoveragePhiEta_CutRMSfromDB_'+part,
                                title='SC coverage '+part+': #phi vs #eta '+RMScut+';#eta;#phi',
                                type='TH2F',
                                path=sc_hist_path+'CoveragePerPartition/RMSCut',
                                xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
                                ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])
        
        SCGroup.defineHistogram('badQualBit_eta_'+part+',badQualBit_phi_'+part+';CoveragePhiEta_BadQualityBit_'+part,
                                title='SC coverage '+part+': #phi vs #eta for bad quality bits;#eta;#phi',
                                type='TH2F',
                                path=sc_hist_path+'CoveragePerPartition/BadQualityBit',
                                xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
                                ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])
        
        SCGroup.defineHistogram('Msampos, Diff_ADC_Pedesdal_Norm_'+part+'; Diff_ADC_Pedestal_vs_SamplePos'+part,
                                title='(ADC-ped)/fabs(ADC_max-ped) '+RMScut+' in '+part+'; Sample position; (ADC - pedestal) / fabs(ADC_max - pedestal)',
                                type='TH2F',
                                path=sc_hist_path+'Diff_ADC_Pedestal',
                                ybins=40,ymin=-1,ymax=1,
                                xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                                xlabels = [str(x) for x in range(1,nsamples+1)])

        
        # --> IT SEEMS TH3 IS NOT IMPLEMENTED YET...
        #SCGroup.defineHistogram('superCellEta_Et10_'+part+',superCellPhi_Et10_'+part+',superCellEt_Et10_'+part+';CoveragePhiEta_Et10_'+part,
        #                          title='SC Energy $E_T>10$ coverage'+part+': #phi vs #eta;#eta;#phi;$E_T$',
        #                          type='TH3F',
        #                          path=sc_hist_path,
        #                          xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
        #                          ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])
        
        #SCGroup.defineHistogram('superCellEta_'+part+',superCellPhi_'+part+',superCellEt_'+part+';CoveragePhiEta_'+part,
        #                          title='SC Energy coverage'+part+': #phi vs #eta;#eta;#phi;$E_T$',
        #                          type='TH3F',
        #                          path=sc_hist_path,
        #                          xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
        #                          ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])
        
        SCGroup.defineHistogram('superCellEta_EtCut_'+part+',superCellPhi_EtCut_'+part+';CoveragePhiEta_EtCut_'+part,
                                title='SC coverage E_T>1GeV '+part+' '+badSCcut+': #phi vs #eta;#eta;#phi',
                                type='TH2F',
                                path=sc_hist_path+'CoveragePerPartition/CutET_1GeV',
                                xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
                                ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])
        
        SCGroup.defineHistogram('superCellEta_EtCut_'+part+',superCellPhi_EtCut10_'+part+';CoveragePhiEta_EtCut10_'+part,
                                title='SC coverage E_T>10GeV '+part+' '+badSCcut+': #phi vs #eta;#eta;#phi',
                                type='TH2F',
                                path=sc_hist_path+'CoveragePerPartition/CutET_10GeV',
                                xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
                                ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])

        SCGroup.defineHistogram('LB,MSCtime_'+part+';MeanOfflineLATOMEtime_perLB_'+part,
                                title='Average LATOME #tau from Offline computation per LB in '+part+'; LumiBloc; #tau [ns]',
                                type='TProfile',
                                path=sc_hist_path + 'OfflineLATOMETiming/perLB',
                                xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)



    for layer in ["0","1","2","3"]: 

        SCGroup.defineHistogram('Mmaxpos_'+layer+';MaxSamplePosition_Layer_'+layer,
                                title='Position of max sample in layer '+layer+' '+RMScut, 
                                type='TH1F',
                                path=sc_hist_path,
                                xbins=nsamples,xmin=0.5,xmax=nsamples+0.5)



    SCGroup.defineHistogram('Diff_ADC_Pedesdal;Diff_ADC_Ped',
                            title='LATOME (ADC-ped); (ADC - pedestal)',
                            type='TH1F',
                            path=sc_hist_path,
                            xbins=50,xmin=-25,xmax=25)

    SCGroup.defineHistogram('Msampos,Diff_ADC_Pedesdal_Norm;Diff_ADC_Pedestal_vs_SamplePos',
                            title='(ADC-ped)/fabs(ADC_max-ped) '+RMScut+'; Sample position; (ADC - pedestal) / fabs(ADC_max - pedestal)',
                            type='TH2F',
                            path=sc_hist_path,
                            ybins=40,ymin=-1,ymax=1,
                            xbins=nsamples,xmin=0.5,xmax=nsamples+0.5,
                            xlabels = [str(x) for x in range(1,nsamples+1)])
    
    SCGroup.defineHistogram('Menergy_onl,Menergy_ofl;OnlOfl_Et_2D',
                            title='LATOME E_{T} vs Offline Computation; E_{T} Onl [MeV*12.5];E_{T} Offl[MeV*12.5]',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=100,xmin=-1000,xmax=20000,
                            ybins=100,ymin=-1000,ymax=20000)
    
    SCGroup.defineHistogram('MSCEt_diff;OnlOfl_Etdiff',
                            title='LATOME E_{T} vs Offline Computation; E_{T} Onl - E_{T} Offl[MeV*12.5]; Evts;',
                            type='TH1F',
                            path=sc_hist_path,
                            xbins=200,xmin=-100,xmax=100)
    
    SCGroup.defineHistogram('Monl_energy_tauSelFail;OnlEt_failTauSelOffline',
                            title='LATOME E_{T} when the tau selection is failed offline; E_{T} Onl [MeV*12.5]; Evts;',
                            type='TH1F',
                            path=sc_hist_path,
                            xbins=200,xmin=-100,xmax=100)
    
    SCGroup.defineHistogram('MSCtime;OfflineLATOMEtime',
                            title='LATOME #tau from Offline Computation  '+badSCcut+' ;#tau [ns]; Evts;',
                            type='TH1F',
                            path=sc_hist_path + 'OfflineLATOMETiming',
                            xbins=100,xmin=-25,xmax=25)

    SCGroup.defineHistogram('MSCtimeNoZero;OfflineLATOMEtime_Nonzero',
                            title='LATOME #tau from Offline Computation (exclude zero, excluding bad SC);#tau [ns]; Evts;',
                            type='TH1F',
                            path=sc_hist_path + 'OfflineLATOMETiming',
                            xbins=100,xmin=-25,xmax=25)
    
    SCGroup.defineHistogram('MlatomeSourceIdBIN,MSCeT;SCeT_LATOME',
                            title='SC eT [MeV] vs LATOME name; ; E_{T}^{SC} [MeV]',
                            type='TH2F',
                            path=sc_hist_path,
                            xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                            ybins=200, ymin=-1000, ymax=200000,
                            xlabels=BinLabel)


    SCGroup.defineHistogram('MSCsatur;SCsaturation', 
                            title='SC saturation',
                            type='TH1F',
                            path=sc_hist_path,
                            xbins=3, xmin=0, xmax=3)

    SCGroup.defineHistogram('MSCeT;SCeT', 
                            title='SC eT [MeV]',
                            type='TH1F',
                            path=sc_hist_path,
                            xbins=500, xmin=-100, xmax=400)

    SCGroup.defineHistogram('MSCeT_Nonzero;SCeT_Nonzero', 
                            title='SC eT [MeV]',
                            type='TH1F',
                            path=sc_hist_path,
                            xbins=500, xmin=-100, xmax=400)


    SCGroup.defineHistogram('MNsamples;SCNsamples', 
                            title='Nsamples',
                            type='TH1F',
                            path=sc_hist_path,
                            xbins=40,xmin=1,xmax=40)

    SCGroup.defineHistogram('MSCChannel;SCchannel', 
                            title='SC Channel number',
                            type='TH1F',
                            path=sc_hist_path,                                  
                            xbins=360,xmin=1,xmax=360)

    SCGroup.defineHistogram('MlatomeSourceId;LATOMEsourceID', 
                            title='LATOME sourceID',
                            type='TH1F',
                            path=sc_hist_path,                                  
                            xbins=1000,xmin=4000000,xmax=6000000)

    
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




