#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
from ROOT import TMath

def LArDigitalTriggMonConfigOld(inputFlags, topSequence):
    from AthenaMonitoring.AthMonitorCfgHelper import AthMonitorCfgHelperOld
    from LArMonitoring.LArMonitoringConf import LArDigitalTriggMonAlg

    helper = AthMonitorCfgHelperOld(inputFlags, 'LArDigitalTriggMonAlgCfg')
    LArDigitalTriggMonConfigCore(helper, LArDigitalTriggMonAlg,inputFlags)
    return helper.result()

def LArDigitalTriggMonConfig(inputFlags):
    '''Function to configures some algorithms in the monitoring system.'''
    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools
    
    from AthenaMonitoring.AthMonitorCfgHelper import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'LArDigitalTriggMonAlgCfg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    return LArDigitalTriggMonConfigCore(helper, CompFactory.LArDigitalTriggMonAlg,inputFlags)

def LArDigitalTriggMonConfigCore(helper, algoinstance,inputFlags):

    from LArMonitoring.GlobalVariables import lArDQGlobals
    
    from AthenaCommon.AlgSequence import AthSequencer
    from IOVDbSvc.CondDB import conddb
    
    #get SC onl-offl mapping from DB    
    from AthenaConfiguration.ComponentFactory import isRun3Cfg
    folder="/LAR/Identifier/OnOffIdMap_SC"
    persClass="AthenaAttributeList"
    dbString="<db>COOLONL_LAR/CONDBR2</db>"
    
    from AthenaCommon.Logging import logging
    mlog = logging.getLogger( 'LArDigitalTriggMon' )
    if isRun3Cfg():
        from AthenaConfiguration.ComponentFactory import CompFactory
        try:
            condLoader=helper.resobj.getCondAlgo("CondInputLoader")
        except Exception:
            from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
            helper.resobj.merge(IOVDbSvcCfg(inputFlags))
            condLoader=helper.resobj.getCondAlgo("CondInputLoader")
            iovDbSvc=helper.resobj.getService("IOVDbSvc")
            helper.resobj.addCondAlgo(CompFactory.LArOnOffMappingAlg("LArOnOffMappingAlgSC",ReadKey=folder, WriteKey="LArOnOffIdMapSC", isSuperCell=True)) 
    else:
        from LArCabling.LArCablingAccess import  LArLATOMEMappingSC
        LArLATOMEMappingSC()
        from LArRecUtils.LArRecUtilsConf import LArOnOffMappingAlg
        from AthenaCommon import CfgGetter
        iovDbSvc=CfgGetter.getService("IOVDbSvc")
        condSeq = AthSequencer("AthCondSeq")
        if hasattr(condSeq,"LArOnOffMappingAlgSC") :
            return #Already there....

        folderPed="/LAR/ElecCalibFlatSC/Pedestal"
        conddb.addFolder('LAR_ONL', folderPed, className = 'CondAttrListCollection')
        condLoader=condSeq.CondInputLoader
        condLoader.Load.append(("CondAttrListCollection",folderPed))

        condSeq+=LArOnOffMappingAlg("LArOnOffMappingAlgSC",ReadKey=folder, WriteKey="LArOnOffIdMapSC", isSuperCell=True)
        from LArRecUtils.LArRecUtilsConf import LArFlatConditionsAlg_LArPedestalSC_ as LArPedestalSCFlatCondAlg
        condSeq+=LArPedestalSCFlatCondAlg("LArPedestalSCFlatCondAlg",ReadKey=folderPed,WriteKey="LArPedestalSC")
         
    iovDbSvc.Folders.append(folder+dbString)
    condLoader.Load.append((persClass,folder))

    larDigitalTriggMonAlg = helper.addAlgorithm(algoinstance,'larDigitalTriggMonAlg')

    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
    mlog.info("Run number: ",ConfigFlags.Input.RunNumber[0])


    hasEtId = False
    hasEt = False
    hasAdc = False
    hasAdcBas = False

    try:
        runinfo=getLArDTInfoForRun(ConfigFlags.Input.RunNumber[0], connstring="COOLONL_LAR/CONDBR2")
    except Exception:
        mlog.warning("Could not get DT run info, using defaults !")

    else:   
        for i in range(0,len(runinfo.streamTypes())):
            mlog.info("runinfo.streamTypes()[i]: ",runinfo.streamTypes()[i])
            if runinfo.streamTypes()[i] ==  "SelectedEnergy":
                hasEtId = True
                larDigitalTriggMonAlg.EtName = "SC_ET_ID"
            if runinfo.streamTypes()[i] ==  "Energy":
                hasEt = True
                larDigitalTriggMonAlg.EtName = "SC_ET"
            if runinfo.streamTypes()[i] ==  "RawADC":
                hasAdc = True
                larDigitalTriggMonAlg.AdcName = "SC"
            if runinfo.streamTypes()[i] ==  "ADC":
                hasAdcBas = True
                larDigitalTriggMonAlg.AdcName = "SC_ADC_BAS"
                
    if (hasEtId and hasEt): #prefer EtId if both in recipe

        hasEt = False
        larDigitalTriggMonAlg.EtName = "SC_ET_ID"

    if (hasAdc and hasAdcBas): #prefer Raw Adc if both in recipe

        hasAdc = False
        larDigitalTriggMonAlg.AdcName = "SC"

    mlog.info("Mux settings from COOL:")
    mlog.info("has ET Id: ",hasEtId)
    mlog.info("has ET: ",hasEt)
    mlog.info("has ADC: ",hasAdc)
    mlog.info("has ADC Bas: ",hasAdcBas)
    mlog.info("Et name: ",larDigitalTriggMonAlg.EtName)
    mlog.info("ADC name: ",larDigitalTriggMonAlg.AdcName)

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
                                  xbins=32,xmin=0.5,xmax=32.5,
                                  ybins=lArDQGlobals.N_Partitions, ymin=-0.5, ymax=lArDQGlobals.N_Partitions-0.5,
                                  xlabels = [str(x) for x in range(1,33)],      
                                  ylabels=lArDQGlobals.Partitions)

    SCGroup.defineHistogram('Msampos,MADC;ADCZoom_samplePosition',
                                  title='ADC (zoom) vs sample position',
                                  type='TH2F',
                                  path=sc_hist_path,
                                  xbins=32,xmin=0.5,xmax=32.5,
                                  xlabels = [str(x) for x in range(1,33)],
                                  ybins=750, ymin=0, ymax=1300) #start from 0 otherwise miss endcap pedestals

    SCGroup.defineHistogram('Msampos,MADC;ADCFullRange_samplePosition', 
                                  title='ADC vs sample position',
                                  type='TH2F',
                                  path=sc_hist_path,
                                  xbins=32,xmin=0.5,xmax=32.5,
                                  xlabels = [str(x) for x in range(1,33)],
                                  ybins=500, ymin=0, ymax=5000) #raw ADC is 12 bit

    SCGroup.defineHistogram('Msampos,Pedestal;PedestalFullRange_samplePosition',
                                  title='Pedestal vs sample position',
                                  type='TH2F',
                                  path=sc_hist_path,
                                  xbins=32,xmin=0.5,xmax=32.5,
                                  xlabels = [str(x) for x in range(1,33)],
                                  ybins=500, ymin=0, ymax=5000) #raw ADC is 12 bit 

    SCGroup.defineHistogram('MlatomeSourceIdBIN_A,Mmaxpos;MaxSamplePosition_LATOME_A', 
                                  title='Position of max sample vs LATOME name side A',
                                  type='TH2F',
                                  path=sc_hist_path+'sideA',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=32,ymin=0.5,ymax=32.5,
                                  xlabels=BinLabel_A,
                                  ylabels = [str(x) for x in range(1,33)])      
                            
    SCGroup.defineHistogram('MlatomeSourceIdBIN_C,Mmaxpos;MaxSamplePosition_LATOME_C',
                                  title='Position of max sample vs LATOME name side C',
                                  type='TH2F',
                                  path=sc_hist_path+'sideC',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=32,ymin=0.5,ymax=32.5,
                                  xlabels=BinLabel_C,
                                  ylabels = [str(x) for x in range(1,33)])

    SCGroup.defineHistogram('MlatomeSourceIdBIN_A,Msampos;BadQuality_SamplePosition_vs_LATOME_A',
                                  title='Bad quality bit: Sample position vs LATOME name side A',
                                  type='TH2F',
                                  path=sc_hist_path+'sideA',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=32,ymin=0.5,ymax=32.5,
                                  xlabels=BinLabel_A,
                                  ylabels = [str(x) for x in range(1,33)])

    SCGroup.defineHistogram('MlatomeSourceIdBIN_C,Msampos;BadQuality_SamplePosition_vs_LATOME_C',
                                  title='Bad quality bit: Sample position vs LATOME name side C',
                                  type='TH2F',
                                  path=sc_hist_path+'sideC',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=32,ymin=0.5,ymax=32.5,
                                  xlabels=BinLabel_C,
                                  ylabels = [str(x) for x in range(1,33)])

    SCGroup.defineHistogram('MlatomeSourceIdBIN_A,MADC;ADCZoom_LATOME_A', 
                                  title='ADC (zoom) vs LATOME name side A',
                                  type='TH2F',
                                  path=sc_hist_path+'sideA',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=750, ymin=0, ymax=1300,
                                  xlabels=BinLabel_A)

    SCGroup.defineHistogram('MlatomeSourceIdBIN_C,MADC;ADCZoom_LATOME_C',
                                  title='ADC (zoom) vs LATOME name side C',
                                  type='TH2F',
                                  path=sc_hist_path+'sideC',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=750, ymin=0, ymax=1300,
                                  xlabels=BinLabel_C)


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

    SCGroup.defineHistogram('MlatomeSourceIdBIN,MADC_A;ADCFullRange_LATOME_A', 
                                  title='ADC vs LATOME name side A; ;ADC',
                                  type='TH2F',
                                  path=sc_hist_path+'sideA',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=500, ymin=-2, ymax=2500, #raw ADC is 12 bit
                                  xlabels=BinLabel_A)

    SCGroup.defineHistogram('MlatomeSourceIdBIN,MADC_C;ADCFullRange_LATOME_C',
                                  title='ADC vs LATOME name side C; ;ADC',
                                  type='TH2F',
                                  path=sc_hist_path+'sideC',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=500, ymin=-2, ymax=2500, #raw ADC is 12 bit                                                                                                                         
                                  xlabels=BinLabel_C)
    

    #have to make plots per sampling and per subdetector, as in LArCoverageAlg, also update GlobalVariables.py
    SCGroup.defineHistogram('MSCeta,MSCphi;Coverage_eta_phi', 
                                  title='SC coverage: #phi vs #eta;#eta;#phi',
                                  type='TH2F',
                                  path=sc_hist_path,
                                  xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025
                                  ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size


    SCGroup.defineHistogram('superCellEta_Satur,superCellPhi_Satur;Coverage_phi_eta_Saturation',
                                  title='SC saturation coverage: #phi vs #eta;#eta;#phi',
                                  type='TH2F',
                                  path=sc_hist_path,
                                  xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025                                                                                                                                                                                    
                                  ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size      

    SCGroup.defineHistogram('badQualBit_eta,badQualBit_phi;CoveragePhiEta_BadQualityBit',
                                  title='SC coverage: #phi vs #eta for bad quality bits;#eta;#phi',
                                  type='TH2F',
                                  path=sc_hist_path,
                                  xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025 
                                  ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size 


    SCGroup.defineHistogram('eta_rms_all,phi_rms_all;CoveragePhiEta_CutRMS',
                                  title='SC coverage: #phi vs #eta with rms>5;#eta;#phi',
                                  type='TH2F',
                                  path=sc_hist_path,
                                  xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025                                                                                                             
                                  ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size 

    SCGroup.defineHistogram('eta_rms_fromDB_all,phi_rms_fromDB_all;CoveragePhiEta_Cut5RMS',
                                  title='SC coverage: #phi vs #eta with ADC_max-pedestal > 5*RMS(DB);#eta;#phi',
                                  type='TH2F',
                                  path=sc_hist_path,
                                  xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025                                                                                                                   
                                  ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size 

    SCGroup.defineHistogram('eta_rms_fromDB_all,phi_rms_fromDB_all;CoveragePhiEta_Cut5RMS',
                                  title='SC coverage: #phi vs #eta with ADC_max-pedestal > 5*RMS(DB);#eta;#phi',
                                  type='TH2F',
                                  path=sc_hist_path,
                                  xbins=400, xmin=-5, xmax=5, #smallest eta bin size 0.025                                                                                                                    
                                  ybins=64, ymin=-TMath.Pi(), ymax=TMath.Pi()) #at most 64 phi bins of 0.1 size                                                                                               
 



    #NEW    
    #for part in LArDigitalTriggMonAlg.LayerNames:
    for part in ["EMBPA", "EMBPC", "EMB1A", "EMB1C", "EMB2A", "EMB2C", "EMB3A", "EMB3C", "HEC0A", "HEC0C", "HEC1A", "HEC1C", "HEC2A", "HEC2C", "HEC3A", "HEC3C", "EMECPA", "EMECPC", "EMEC1A", "EMEC1C", "EMEC2A", "EMEC2C", "EMEC3A", "EMEC3C", "FCAL1A", "FCAL1C", "FCAL2A", "FCAL2C", "FCAL3A", "FCAL3C"]:
        Part = part[:-2]
        if Part == "FCAL": 
            Part = "FCal"
        Side = part[-1]
        Sampling = part[-2]
        if Sampling == "P": 
            Sampling = "0"
        

        SCGroup.defineHistogram('BCID,AvEnergy_'+part+';AvEnergyVsBCID_'+part, 
                                  title='Average Energy vs BCID '+part+'; BCID; Energy per SC [MeV]',
                                  type='TH2F',
                                  path=sc_hist_path + 'BaselineCorrection',
                                  xbins=3564,xmin=-0.5,xmax=3563.5,
                                  ybins=24, ymin=-20, ymax=100)
        
        SCGroup.defineHistogram('superCellEta_'+part+',superCellPhi_'+part+';CoveragePhiEta_'+part,
                                  title='SC coverage '+part+': #phi vs #eta;#eta;#phi',
                                  type='TH2F',
                                  path=sc_hist_path+'CoveragePerPartition/NoCut',
                                  xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
                                  ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])    
        
        SCGroup.defineHistogram('eta_rms_'+part+',phi_rms_'+part+';CoveragePhiEta_CutRMS_'+part,
                                  title='SC coverage '+part+': #phi vs #eta with RMS>5;#eta;#phi',
                                  type='TH2F',
                                  path=sc_hist_path+'CoveragePerPartition/CutRMS',
                                  xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
                                  ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])

        SCGroup.defineHistogram('eta_rms_fromDB_'+part+',phi_rms_fromDB_'+part+';CoveragePhiEta_CutRMSfromDB_'+part,
                                  title='SC coverage '+part+': #phi vs #eta with ADC_max - pestal > 5*RMS(DB);#eta;#phi',
                                  type='TH2F',
                                  path=sc_hist_path+'CoveragePerPartition/CutFiveRMS',
                                  xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
                                  ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])

        SCGroup.defineHistogram('badQualBit_eta_'+part+',badQualBit_phi_'+part+';CoveragePhiEta_BadQualityBit_'+part,
                                  title='SC coverage '+part+': #phi vs #eta for bad quality bits;#eta;#phi',
                                  type='TH2F',
                                  path=sc_hist_path+'CoveragePerPartition/BadQualityBit',
                                  xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
                                  ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])

        SCGroup.defineHistogram('Msampos, Diff_ADC_Pedesdal_Norm_'+part+'; Diff_ADC_Pedestal_vs_SamplePos'+part,
                                  title='(ADC-ped)/fabs(ADC_max-ped) with ADC_max-ped > 5xRMS in '+part+'; Sample position; (ADC - pedestal) / fabs(ADC_max - pedestal)',
                                  type='TH2F',
                                  path=sc_hist_path+'Diff_ADC_Pedestal',
                                  ybins=40,ymin=-1,ymax=1,
                                  xbins=32,xmin=0.5,xmax=32.5,

                                  xlabels = [str(x) for x in range(1,33)])

                    
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
                                  title='SC coverage E_T>1GeV '+part+': #phi vs #eta;#eta;#phi',
                                  type='TH2F',
                                  path=sc_hist_path+'CoveragePerPartition/CutET_1GeV',
                                  xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
                                  ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])

        SCGroup.defineHistogram('superCellEta_EtCut_'+part+',superCellPhi_EtCut10_'+part+';CoveragePhiEta_EtCut10_'+part,
                                  title='SC coverage E_T>10GeV '+part+': #phi vs #eta;#eta;#phi',
                                  type='TH2F',
                                  path=sc_hist_path+'CoveragePerPartition/CutET_10GeV',
                                  xbins=lArDQGlobals.Cell_Variables["etaRange"][Part][Side][Sampling],
                                  ybins=lArDQGlobals.Cell_Variables["phiRange"][Part][Side][Sampling])




    for layer in ["0","1","2","3"]: 
        SCGroup.defineHistogram('MlatomeSourceIdBIN_A,Mmaxpos_'+layer+';MaxSamplePosition_LATOME_A_Layer'+layer,
                                  title='Position of max sample vs. LATOME side A in layer '+layer,
                                  type='TH2F',
                                  path=sc_hist_path+'sideA',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=32,ymin=0.5,ymax=32.5,
                                  xlabels=BinLabel_A,
                                  ylabels = [str(x) for x in range(1,33)])  
                                
        SCGroup.defineHistogram('MlatomeSourceIdBIN_C,Mmaxpos_'+layer+';MaxSamplePosition_LATOME_C_Layer'+layer,
                                  title='Position of max sample vs. LATOME side C in layer '+layer,
                                  type='TH2F',
                                  path=sc_hist_path+'sideC',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=32,ymin=0.5,ymax=32.5,
                                  xlabels=BinLabel_C,
                                  ylabels = [str(x) for x in range(1,33)])

        SCGroup.defineHistogram('MlatomeSourceIdBIN_A_'+layer+'_cutRMS,Mmaxpos_'+layer+'_cutRMS'+';MaxSamplePosition_LATOME_A_Layer'+layer+'_cutRMS',                                                                                                                   
                                  title='Position of max sample vs. LATOME side A in layer '+layer+' with RMS > 5',                                                                                                                                                        
                                  type='TH2F',                                                                                                                                                                                                                                
                                  path=sc_hist_path+'sideA/MaxSample_vs_LATOME/CutRMS',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,                                                                                                                                                                                      
                                  ybins=32,ymin=0.5,ymax=32.5,                                                                                                                                                                                                                
                                  xlabels=BinLabel_A,                                                                                                                                                                                                                         
                                  ylabels = [str(x) for x in range(1,33)])

        SCGroup.defineHistogram('MlatomeSourceIdBIN_C_'+layer+'_cutRMS,Mmaxpos_'+layer+'_cutRMS'+';MaxSamplePosition_LATOME_C_Layer'+layer+'_cutRMS',
                                  title='Position of max sample vs. LATOME side C in layer '+layer+' with RMS > 5',
                                  type='TH2F',
                                  path=sc_hist_path+'sideC/MaxSample_vs_LATOME/CutRMS',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=32,ymin=0.5,ymax=32.5,
                                  xlabels=BinLabel_C,
                                  ylabels = [str(x) for x in range(1,33)])

        SCGroup.defineHistogram('MlatomeSourceIdBIN_A_'+layer+'_cutRMSfromDB,Mmaxpos_'+layer+'_cutRMSfromDB'+';MaxSamplePosition_LATOME_A_Layer'+layer+'_cutRMSfromDB',
                                  title='Position of max sample vs. LATOME side A in layer '+layer+' with RMS > 3*RMS(DB)',
                                  type='TH2F',
                                  path=sc_hist_path+'sideA/MaxSample_vs_LATOME/CutRMSfromDB',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=32,ymin=0.5,ymax=32.5,
                                  xlabels=BinLabel_A,
                                  ylabels = [str(x) for x in range(1,33)])

        SCGroup.defineHistogram('MlatomeSourceIdBIN_C_'+layer+'_cutRMSfromDB,Mmaxpos_'+layer+'_cutRMSfromDB'+';MaxSamplePosition_LATOME_C_Layer'+layer+'_cutRMSfromDB',
                                  title='Position of max sample vs. LATOME side C in layer '+layer+' with RMS > 3*RMS(DB)',
                                  type='TH2F',
                                  path=sc_hist_path+'sideC/MaxSample_vs_LATOME/CutRMSfromDB',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=32,ymin=0.5,ymax=32.5,
                                  xlabels=BinLabel_C,
                                  ylabels = [str(x) for x in range(1,33)])

        SCGroup.defineHistogram('Mmaxpos_'+layer+';MaxSamplePosition_Layer_'+layer,
                                title='Position of max sample in layer '+layer+' when ADC_max - ped > 5xRMS(DB)', 
                                type='TH1F',
                                path=sc_hist_path,
                                xbins=32,xmin=0.5,xmax=32.5)



    SCGroup.defineHistogram('MlatomeSourceIdBIN_A,MSCChannel;Coverage_SCchan_LATOME_A', 
                                  title='SC coverage: channel vs LATOME name side A',
                                  type='TH2F',
                                  path=sc_hist_path+'sideA',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,      
                                  ybins=360,ymin=1,ymax=360,
                                  xlabels=BinLabel_A)

    SCGroup.defineHistogram('MlatomeSourceIdBIN_C,MSCChannel;Coverage_SCchan_LATOME_C',
                                  title='SC coverage: channel vs LATOME name side C',
                                  type='TH2F',
                                  path=sc_hist_path+'sideC',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=360,ymin=1,ymax=360,
                                  xlabels=BinLabel_C)

    SCGroup.defineHistogram('Msampos, Diff_ADC_Pedesdal_Norm; Diff_ADC_Pedestal_vs_SamplePos',
                                  title='(ADC-ped)/fabs(ADC_max-ped) with ADC_max-ped > 5xRMS; Sample position; (ADC - pedestal) / fabs(ADC_max - pedestal)',
                                  type='TH2F',
                                  path=sc_hist_path,
                                  ybins=40,ymin=-1,ymax=1,
                                  xbins=32,xmin=0.5,xmax=32.5,

                                  xlabels = [str(x) for x in range(1,33)])

    SCGroup.defineHistogram('MlatomeSourceIdBIN,MSCeT;SCeT_LATOME',
                                  title='SC eT [MeV] vs LATOME name; ; E_{T}^{SC} [MeV]',
                                  type='TH2F',
                                  path=sc_hist_path,
                                  xbins=NLatomeBins,xmin=1,xmax=NLatomeBins+1,
                                  ybins=200, ymin=0, ymax=200000,
                                  xlabels=BinLabel)

    SCGroup.defineHistogram('MlatomeSourceIdBIN_A,MSCeT_A;SCeT_LATOME_A', 
                                  title='SC eT [MeV] vs LATOME name side A; ; E_{T}^{SC} [MeV]',
                                  type='TH2F',
                                  path=sc_hist_path+'sideA',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,      
                                  ybins=200, ymin=0, ymax=200000,
                                  xlabels=BinLabel_A)

    SCGroup.defineHistogram('MlatomeSourceIdBIN_C,MSCeT_C;SCeT_LATOME_C',
                                  title='SC eT [MeV] vs LATOME name side C; ; E_{T}^{SC} [MeV]',
                                  type='TH2F',
                                  path=sc_hist_path+'sideC',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=200, ymin=0, ymax=200000,
                                  xlabels=BinLabel_C)

    SCGroup.defineHistogram('MlatomeSourceIdBIN_A,MSCsatur;SCsaturation_LATOME_A', 
                                  title='SC saturation vs LATOME name side A',
                                  type='TH2F',
                                  path=sc_hist_path+'sideA',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,      
                                  ybins=3, ymin=-0.5, ymax=2.5,
                                  xlabels=BinLabel_A,
                                  ylabels=["0","1","2"])

    SCGroup.defineHistogram('MlatomeSourceIdBIN_C,MSCsatur;SCsaturation_LATOME_C',
                                  title='SC saturation vs LATOME name side C',
                                  type='TH2F',
                                  path=sc_hist_path+'sideC',
                                  xbins=NLatomeBins_side,xmin=1,xmax=NLatomeBins_side+1,
                                  ybins=3, ymin=-0.5, ymax=2.5,
                                  xlabels=BinLabel_C,
                                  ylabels=["0","1","2"])

    SCGroup.defineHistogram('MSCsatur;SCsaturation', 
                                  title='SC saturation',
                                  type='TH1F',
                                  path=sc_hist_path,
                                  xbins=3, xmin=0, xmax=3)

    SCGroup.defineHistogram('MSCeT;SCeT', 
                                  title='SC eT [MeV]',
                                  type='TH1F',
                                  path=sc_hist_path,
                                  xbins=400, xmin=0, xmax=400)


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

   from AthenaConfiguration.AllConfigFlags import ConfigFlags
   from AthenaCommon.Logging import log
   from AthenaCommon.Constants import DEBUG, ERROR
   log.setLevel(DEBUG)
   
   #from LArMonitoring.LArMonitoringConf import  LArSuperCellMonAlg
   from LArMonitoring.LArMonConfigFlags import createLArMonConfigFlags
   createLArMonConfigFlags()

   ConfigFlags.Input.Files = ["/eos/atlas/atlastier0/rucio/data21_idcomm/physics_CosmicCalo/00401410/data21_idcomm.00401410.physics_CosmicCalo.merge.RAW/data21_idcomm.00401410.physics_CosmicCalo.merge.RAW._lb0722._SFO-ALL._0001.1"]
   
   ConfigFlags.Output.HISTFileName = 'LArDigitalTriggMonOutput.root'
   ConfigFlags.DQ.enableLumiAccess = False
   ConfigFlags.DQ.useTrigger = False
   from AthenaConfiguration.Enums import BeamType
   ConfigFlags.Beam.Type = BeamType.Collisions
   ConfigFlags.lock()

   # in case of tier0 workflow:
   from CaloRec.CaloRecoConfig import CaloRecoCfg
   cfg=CaloRecoCfg(ConfigFlags)

   #from AthenaCommon.AppMgr import (ServiceMgr as svcMgr,ToolSvc)
   from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
   SCData_acc =  LArRawSCDataReadingCfg(ConfigFlags)
   SCData_acc.OutputLevel=DEBUG
 
   cfg.merge(SCData_acc)


   aff_acc = LArDigitalTriggMonConfig(ConfigFlags)
   cfg.merge(aff_acc)
   
   cfg.getCondAlgo("LArHVCondAlg").OutputLevel=ERROR
   #cfg.getEventAlgo("LArHVCondAlg").OutputLevel=ERROR

   ConfigFlags.dump()
   f=open("LArDigitalTriggMon.pkl","wb")
   cfg.store(f)
   f.close()




