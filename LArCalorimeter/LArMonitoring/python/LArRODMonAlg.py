#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def LArRODMonConfig(flags,cellDebug=False, dspDebug=False):
    from AthenaMonitoring.AthMonitorCfgHelper import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'LArRODMonAlgCfg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    
    
    LArRODMonConfigCore(helper, CompFactory.LArRODMonAlg,flags,cellDebug, dspDebug)

    return helper.result()


def LArRODMonConfigCore(helper, algoinstance,flags, cellDebug=False, dspDebug=False):

    larRODMonAlg = helper.addAlgorithm(algoinstance,'larRODMonAlg')

    from LArMonitoring.GlobalVariables import lArDQGlobals

    
    GroupName="RODMon"
    nslots=[]
    for i in range(0,len(lArDQGlobals.FEB_Slot)): 
       nslots.append(lArDQGlobals.FEB_Slot[lArDQGlobals.Partitions[i]][1])

    larRODMonAlg.MonGroup=GroupName
    larRODMonAlg.LArRODSubDetNames=lArDQGlobals.SubDet
    larRODMonAlg.LArRODPartitionNames=lArDQGlobals.Partitions
    larRODMonAlg.LArRODNslots=nslots

    larRODMonAlg.ADCthreshold = 0
    larRODMonAlg.peakTimeCut = 5.
    larRODMonAlg.SkipNullQT=True 

    # for detailed debugging
    if cellDebug:
       larRODMonAlg.DoCellsDump=True

    if dspDebug:
       larRODMonAlg.DoDspTestDump=True

    if flags.Common.isOnline:
       larRODMonAlg.MaxEvDump=100   

    #from AthenaCommon.Constants import VERBOSE
    #larRODMonAlg.OutputLevel=VERBOSE
    larRODMonAlg.ProblemsToMask=["highNoiseHG","highNoiseMG","highNoiseLG","deadReadout","deadPhys","almostDead","short","sporadicBurstNoise"]

    Group = helper.addGroup(
        larRODMonAlg,
        GroupName,
        '/LAr/DSPMonitoring'
    )


    #Summary histos
    summary_hist_path='Summary/'
    
    Group.defineHistogram('partition,gain;Summary_E', 
                                  title='Summary of errors on Energy per partition and per gain',
                                  type='TH2F',
                                  path=summary_hist_path,
                                  weight='weight_e',
                                  xbins=lArDQGlobals.N_Partitions, xmin=-0.5, xmax=lArDQGlobals.N_Partitions-0.5,
                                  ybins=lArDQGlobals.N_Gains,ymin=-0.5,ymax=lArDQGlobals.N_Gains-0.5,
                                  xlabels=lArDQGlobals.Partitions,ylabels=lArDQGlobals.Gains)

    Group.defineHistogram('partition,gain;Summary_Q', 
                                  title='Summary of errors on Quality per partition and per gain',
                                  type='TH2F',
                                  path=summary_hist_path,
                                  weight='weight_q',
                                  xbins=lArDQGlobals.N_Partitions, xmin=-0.5, xmax=lArDQGlobals.N_Partitions-0.5,
                                  ybins=lArDQGlobals.N_Gains,ymin=-0.5,ymax=lArDQGlobals.N_Gains-0.5,
                                  xlabels=lArDQGlobals.Partitions,ylabels=lArDQGlobals.Gains)

    Group.defineHistogram('partition,gain;Summary_T', 
                                  title='Summary of errors on Time per partition and per gain',
                                  type='TH2F',
                                  path=summary_hist_path,
                                  weight='weight_t',
                                  xbins=lArDQGlobals.N_Partitions, xmin=-0.5, xmax=lArDQGlobals.N_Partitions-0.5,
                                  ybins=lArDQGlobals.N_Gains,ymin=-0.5,ymax=lArDQGlobals.N_Gains-0.5,
                                  xlabels=lArDQGlobals.Partitions,ylabels=lArDQGlobals.Gains)

    Group.defineHistogram('Ediff;E_all', 
                                  title='E_offline - E_online for all partitions;E_offline - E_online (MeV)',
                                  type='TH1F',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.DSPEnergy_Bins, xmin=lArDQGlobals.DSPEnergy_Min, xmax=lArDQGlobals.DSPEnergy_Max)
    Group.defineHistogram('Tdiff;T_all', 
                                  title='T_offline - T_online for all partitions;T_offline - T_online (ps)',
                                  type='TH1F',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.DSPTime_Bins, xmin=lArDQGlobals.DSPTime_Min, xmax=lArDQGlobals.DSPTime_Max)
    Group.defineHistogram('Qdiff;Q_all', 
                                  title='Q_offline - Q_online / sqrt(Q_offline) for all partitions;Q_offline - Q_online / sqrt(Q_offline) (ps)',
                                  type='TH1F',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.DSPQuality_Bins, xmin=lArDQGlobals.DSPQuality_Min, xmax=lArDQGlobals.DSPQuality_Max)

    #Infos histos (vs. LB)
    info_hist_path='Infos/'
    cut = "#delta ADC>"+str(larRODMonAlg.ADCthreshold)+" and |T_offline| < "+str(larRODMonAlg.peakTimeCut)+" ns"
    Group.defineHistogram('LBN,partitionI;EErrorsPerLB',
                                  title='Nb of errors in E per LB -' +cut+';Luminosity Block;Partition',
                                  type='TH2I',
                                  weight='numE',
                                  path=info_hist_path,
                                  xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max,
                                  ybins=lArDQGlobals.N_Partitions, ymin=-0.5, ymax=lArDQGlobals.N_Partitions-0.5,
                                  ylabels = lArDQGlobals.Partitions
          )
    Group.defineHistogram('LBN,partitionI;TErrorsPerLB',
                                  title='Nb of errors in T per LB - ' +cut+';Luminosity Block;Partition',
                                  type='TH2I',
                                  weight='numT',
                                  path=info_hist_path,
                                  xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max,
                                  ybins=lArDQGlobals.N_Partitions, ymin=-0.5, ymax=lArDQGlobals.N_Partitions-0.5,
                                  ylabels = lArDQGlobals.Partitions
          )
    Group.defineHistogram('LBN,partitionI;QErrorsPerLB',
                                  title='Nb of errors in Q per LB - ' +cut+';Luminosity Block;Partition',
                                  type='TH2I',
                                  weight='numQ',
                                  path=info_hist_path,
                                  xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max,
                                  ybins=lArDQGlobals.N_Partitions, ymin=-0.5, ymax=lArDQGlobals.N_Partitions-0.5,
                                  ylabels = lArDQGlobals.Partitions
          )

    #DQMD histos
    dqmd_hist_path='/LAr/DSPMonitoring/DQMD/'
    darray = helper.addArray([lArDQGlobals.Partitions],larRODMonAlg,"RODMon",topPath='/')
    darray.defineHistogram('Ediff,Erange;DE_ranges', title='E_offline - E_online for all ranges ; E_offline - E_online (MeV) ; Energy range',
                           type='TH2F', path=dqmd_hist_path,
                           xbins=lArDQGlobals.DSP1Energy_Bins, xmin=lArDQGlobals.DSP1Energy_Min, xmax=lArDQGlobals.DSP1Energy_Max,
                           ybins=lArDQGlobals.DSPRanges_Bins, ymin=lArDQGlobals.DSPRanges_Min, ymax=lArDQGlobals.DSPRanges_Max,
                           ylabels=lArDQGlobals.DSPRanges 
          )
    Group.defineHistogram('Ediff,Erange;E_ranges_all', title='E_online - E_offline for all ranges ; E_offline - E_online (MeV) ; Energy range',
                           type='TH2F', path='DQMD/',
                           xbins=lArDQGlobals.DSP1Energy_Bins, xmin=lArDQGlobals.DSP1Energy_Min, xmax=lArDQGlobals.DSP1Energy_Max,
                           ybins=lArDQGlobals.DSPRanges_Bins, ymin=lArDQGlobals.DSPRanges_Min, ymax=lArDQGlobals.DSPRanges_Max,
                           ylabels=lArDQGlobals.DSPRanges
          )


    #per partition, currently in one dir only
    part_hist_path='/LAr/DSPMonitoring/perPartition/'
    darray.defineHistogram('Ediff;DE', title='E_offline - E_online;E_offline - E_online (MeV)',
                           type='TH1F', path=part_hist_path,
                           xbins=lArDQGlobals.DSPEnergy_Bins, xmin=lArDQGlobals.DSPEnergy_Min, xmax=lArDQGlobals.DSPEnergy_Max)
    darray.defineHistogram('Tdiff;DT', title='T_offline - T_online;T_offline - T_online  (ps)',
                           type='TH1F', path=part_hist_path,
                           xbins=lArDQGlobals.DSPTime_Bins, xmin=lArDQGlobals.DSPTime_Min, xmax=lArDQGlobals.DSPTime_Max)
    darray.defineHistogram('Qdiff;DQ', title='Q_offline - Q_online / sqrt(Q_offline);Q_offline - Q_online / sqrt(Q_offline)' ,
                           type='TH1F', path=part_hist_path,
                           xbins=lArDQGlobals.DSPTime_Bins, xmin=lArDQGlobals.DSPTime_Min, xmax=lArDQGlobals.DSPTime_Max)

    darray.defineHistogram('slot,FT;RAW_Out_E_FT_vs_SLOT',title='# of cells with E_offline - E_online > numerical precision ; Slot ; Feedthrough',
                           type='TH2I', path=part_hist_path,
                           opt='kAlwaysCreate',
                           weight='weight_e',
                           xbins=lArDQGlobals.FEB_Slot["EMECA"][1], xmin=lArDQGlobals.FEB_Slot["EMECA"][0]-0.5, xmax=lArDQGlobals.FEB_Slot["EMECA"][1]+0.5,
                           ybins=lArDQGlobals.FEB_Feedthrough["EMBA"][1]+1, ymin=lArDQGlobals.FEB_Feedthrough["EMBA"][0]-0.5, ymax=lArDQGlobals.FEB_Feedthrough["EMBA"][1]+0.5)

    darray.defineHistogram('slot,FT;RAW_Out_T_FT_vs_SLOT',title='# of cells with T_offline - T_online > numerical precision ; Slot ; Feedthrough',
                           type='TH2I', path=part_hist_path,
                           opt='kAlwaysCreate',
                           weight='weight_t',
                           xbins=lArDQGlobals.FEB_Slot["EMECA"][1], xmin=lArDQGlobals.FEB_Slot["EMECA"][0]-0.5, xmax=lArDQGlobals.FEB_Slot["EMECA"][1]+0.5,
                           ybins=lArDQGlobals.FEB_Feedthrough["EMBA"][1]+1, ymin=lArDQGlobals.FEB_Feedthrough["EMBA"][0]-0.5, ymax=lArDQGlobals.FEB_Feedthrough["EMBA"][1]+0.5)

    darray.defineHistogram('slot,FT;RAW_Out_Q_FT_vs_SLOT',title='# of cells with Q_offline - Q_online > numerical precision ; Slot ; Feedthrough',
                           type='TH2I', path=part_hist_path,
                           opt='kAlwaysCreate',
                           weight='weight_q',
                           xbins=lArDQGlobals.FEB_Slot["EMECA"][1], xmin=lArDQGlobals.FEB_Slot["EMECA"][0]-0.5, xmax=lArDQGlobals.FEB_Slot["EMECA"][1]+0.5,
                           ybins=lArDQGlobals.FEB_Feedthrough["EMBA"][1]+1, ymin=lArDQGlobals.FEB_Feedthrough["EMBA"][0]-0.5, ymax=lArDQGlobals.FEB_Feedthrough["EMBA"][1]+0.5)

    darray.defineHistogram('Eoff,Eon;Eon_VS_Eoff', title='E_online VS E_offline;E_offline (MeV);E_online (MeV)',
                           type='TH2F', path=part_hist_path,
                           xbins=lArDQGlobals.DSPEonEoff_Bins, xmin=-lArDQGlobals.DSPEonEoff_Max, xmax=lArDQGlobals.DSPEonEoff_Max,
                           ybins=lArDQGlobals.DSPEonEoff_Bins, ymin=-lArDQGlobals.DSPEonEoff_Max, ymax=lArDQGlobals.DSPEonEoff_Max)

    darray.defineHistogram('Toff,Ton;Ton_VS_Toff', title='T_online VS T_offline;T_offline (ps);T_online (ps)',
                           type='TH2F', path=part_hist_path,
                           xbins=lArDQGlobals.DSPTonToff_Bins, xmin=-lArDQGlobals.DSPTonToff_Max, xmax=lArDQGlobals.DSPTonToff_Max,
                           ybins=lArDQGlobals.DSPTonToff_Bins, ymin=-lArDQGlobals.DSPTonToff_Max, ymax=lArDQGlobals.DSPTonToff_Max)

    darray.defineHistogram('Qoff,Qon;Qon_VS_Qoff', title='Q_online VS Q_offline;Q_offline ;Q_online ',
                           type='TH2F', path=part_hist_path,
                           xbins=lArDQGlobals.DSPQonQoff_Bins, xmin=-lArDQGlobals.DSPQonQoff_Max, xmax=lArDQGlobals.DSPQonQoff_Max,
                           ybins=lArDQGlobals.DSPQonQoff_Bins, ymin=-lArDQGlobals.DSPQonQoff_Max, ymax=lArDQGlobals.DSPQonQoff_Max)

    darray.defineHistogram('Sweetc;Sweet_cells', title='Number of sweet Cells in LAr;Sweet cells per feb',
                           type='TH1F', path=part_hist_path,
                           xbins=lArDQGlobals.FEB_N_channels, xmin=lArDQGlobals.FEB_channels_Min, xmax=lArDQGlobals.FEB_channels_Max)

    

if __name__=='__main__':

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()

   from AthenaCommon.Logging import log
   from AthenaCommon.Constants import DEBUG
   log.setLevel(DEBUG)

   from LArMonitoring.LArMonConfigFlags import addLArMonFlags
   flags.addFlagsCategory("LArMon", addLArMonFlags)

   from AthenaConfiguration.TestDefaults import defaultTestFiles
   flags.Input.Files = defaultTestFiles.RAW_RUN2

   flags.Output.HISTFileName = 'LArRODMonOutput.root'
   flags.DQ.enableLumiAccess = False
   flags.DQ.useTrigger = False
   flags.lock()


   from CaloRec.CaloRecoConfig import CaloRecoCfg
   cfg=CaloRecoCfg(flags)

   #from CaloD3PDMaker.CaloD3PDConfig import CaloD3PDCfg,CaloD3PDAlg
   #cfg.merge(CaloD3PDCfg(flags, filename=flags.Output.HISTFileName, streamname='CombinedMonitoring'))

   aff_acc = LArRODMonConfig(flags)
   cfg.merge(aff_acc)

   cfg.printConfig()

   flags.dump()
   f=open("LArRODMon.pkl","wb")
   cfg.store(f)
   f.close()

   #cfg.run(100)
