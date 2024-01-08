#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def LArDigitMonConfigOld(flags):
    from AthenaMonitoring.AthMonitorCfgHelper import AthMonitorCfgHelperOld
    from LArMonitoring.LArMonitoringConf import LArDigitMonAlg

    helper = AthMonitorCfgHelperOld(flags, 'LArDigitMonAlgCfg')
    LArDigitMonConfigCore(helper, LArDigitMonAlg,flags)
    return helper.result()

def LArDigitMonConfig(flags):
    '''Function to configures some algorithms in the monitoring system.'''

    # The following class will make a sequence, configure algorithms, and link                                                                   
    # them to GenericMonitoringTools                                                                                                                                 
    
    from AthenaMonitoring.AthMonitorCfgHelper import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'LArDigitMonAlgCfg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    return LArDigitMonConfigCore(helper, CompFactory.LArDigitMonAlg,flags)

def LArDigitMonConfigCore(helper, algoinstance,flags):


    from LArMonitoring.GlobalVariables import lArDQGlobals

    larDigitMonAlg = helper.addAlgorithm(algoinstance,'larDigitMonAlg')

    summaryGroupName="Summary"
    nslots=[]
    for i in range(0,len(lArDQGlobals.FEB_Slot)): 
       nslots.append(lArDQGlobals.FEB_Slot[lArDQGlobals.Partitions[i]][1])

    larDigitMonAlg.SummaryMonGroup=summaryGroupName
    larDigitMonAlg.LArDigitsSubDetNames=lArDQGlobals.SubDet
    larDigitMonAlg.LArDigitsPartitionNames=lArDQGlobals.Partitions
    larDigitMonAlg.LArDigitsNslots=nslots
    larDigitMonAlg.ProblemsToMask=["highNoiseHG","highNoiseMG","highNoiseLG","deadReadout","deadPhys"]
    larDigitMonAlg.Streams=lArDQGlobals.defaultStreamNames 

    summaryGroup = helper.addGroup(
        larDigitMonAlg,
        summaryGroupName,
        '/LAr/Digits'
    )


    summary_hist_path=summaryGroupName+'/'
    
    summaryGroup.defineHistogram('gain,partition;SummaryGain', 
                                  title='Gain;Gain;Partition',
                                  type='TH2F',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.N_Gains,xmin=-0.5,xmax=lArDQGlobals.N_Gains-0.5,
                                  ybins=lArDQGlobals.N_Partitions, ymin=-0.5, ymax=lArDQGlobals.N_Partitions-0.5,
                                  xlabels=lArDQGlobals.Gains,ylabels=lArDQGlobals.Partitions)

    summaryGroup.defineHistogram('LBN;LBN',type='TH1I',
                                 title='Event counter per LB', 
                                 path='',
                                 xbins=lArDQGlobals.LB_Bins,xmin=lArDQGlobals.LB_Min,xmax=lArDQGlobals.LB_Max)
    
    # now individual partitions, because we need a different directories, will have only 2dim arrays (side)
    for subdet in range(0,lArDQGlobals.N_SubDet):
       array = helper.addArray([lArDQGlobals.Partitions[2*subdet:2*subdet+2]],larDigitMonAlg,lArDQGlobals.SubDet[subdet],topPath='/')
       hist_path='/LAr/Digits/'+lArDQGlobals.SubDet[subdet]+'/'
       slot_low = lArDQGlobals.FEB_Slot[lArDQGlobals.Partitions[subdet*2]][0] - 0.5
       slot_up  = lArDQGlobals.FEB_Slot[lArDQGlobals.Partitions[subdet*2]][1] + 0.5
       slot_n = int(slot_up - slot_low)
       ft_low = lArDQGlobals.FEB_Feedthrough[lArDQGlobals.Partitions[subdet*2]][0] - 0.5
       ft_up  = lArDQGlobals.FEB_Feedthrough[lArDQGlobals.Partitions[subdet*2]][1] + 0.5
       ft_n = int(ft_up - ft_low) 
       chan_n = lArDQGlobals.FEB_N_channels
       chan_low = lArDQGlobals.FEB_channels_Min
       chan_up = lArDQGlobals.FEB_channels_Max
       crates_n = lArDQGlobals.FEB_Crates[lArDQGlobals.Partitions[subdet*2]][1]
       crates_low = 0.5
       crates_up = lArDQGlobals.FEB_Crates[lArDQGlobals.Partitions[subdet*2]][1] + 0.5
       array.defineHistogram('Outslot,OutFT;RAW_OutOfRange', 
                                  title='% chan/FEB/events with max out of ',
                                  type='TH2F',
                                  path=hist_path,
                                  weight='Outweight',
                                  xbins=int(slot_n),xmin=slot_low,xmax=slot_up,
                                  ybins=int(ft_n), ymin=ft_low, ymax=ft_up)
       array.defineHistogram('Outcrate,Outchan;RAW_OutOfRangeChan', 
                                  title='% chan/events with max out of  ',
                                  type='TH2F',
                                  path=hist_path,
                                  weight='weight',
                                  xbins=crates_n,xmin=crates_low,xmax=crates_up,
                                  ybins=chan_n, ymin=chan_low, ymax=chan_up)

       array.defineHistogram('Saturslot,SaturFT;RAW_Saturation', 
                                  title='% chan/FEB/events with max=4095 ADC ',
                                  type='TH2F',
                                  path=hist_path,
                                  weight='Saturweight',
                                  xbins=int(slot_n),xmin=slot_low,xmax=slot_up,
                                  ybins=int(ft_n), ymin=ft_low, ymax=ft_up)
       array.defineHistogram('Saturcrate,Saturchan;RAW_SaturationChan', 
                                  title='% chan/events with max=4095 ADC - Med/High Gain - All Stream',
                                  type='TH2F',
                                  path=hist_path,
                                  weight='weight',
                                  xbins=crates_n,xmin=crates_low,xmax=crates_up,
                                  ybins=chan_n, ymin=chan_low, ymax=chan_up)

       array.defineHistogram('SaturLowslot,SaturLowFT;RAW_SaturationLow', 
                                  title='% chan/FEB/events with max=4095 ADC ',
                                  type='TH2F',
                                  path=hist_path,
                                  weight='SaturLowweight',
                                  xbins=int(slot_n),xmin=slot_low,xmax=slot_up,
                                  ybins=int(ft_n), ymin=ft_low, ymax=ft_up)
       array.defineHistogram('SaturLowcrate,SaturLowchan;RAW_SaturationChanLow', 
                                  title='% chan/events with max=4095 ADC - Low Gain - All Stream',
                                  type='TH2F',
                                  path=hist_path,
                                  weight='weight',
                                  xbins=crates_n,xmin=crates_low,xmax=crates_up,
                                  ybins=chan_n, ymin=chan_low, ymax=chan_up)

       array.defineHistogram('Nullslot,NullFT;tNullDigit', 
                                  title='% chan/FEB/events with min=0 ADC ',
                                  type='TH2F',
                                  path=hist_path,
                                  weight='Nullweight',
                                  xbins=int(slot_n),xmin=slot_low,xmax=slot_up,
                                  ybins=int(ft_n), ymin=ft_low, ymax=ft_up)
       array.defineHistogram('Nullcrate,Nullchan;RAW_NullDigitChan', 
                                  title='% chan/events with min=0 ADC - All Gain - All Stream;Crate;Channel',
                                  type='TH2F',
                                  path=hist_path,
                                  xbins=crates_n,xmin=crates_low,xmax=crates_up,
                                  ybins=chan_n, ymin=chan_low, ymax=chan_up)

       array.defineHistogram('slot,FT,MaxPos;RAW_AvePosMaxDig', 
                                  title='Average position of Max Digit;Slot;FT',
                                  type='TProfile2D',
                                  path=hist_path,
                                  xbins=int(slot_n),xmin=slot_low,xmax=slot_up,
                                  ybins=int(ft_n), ymin=ft_low, ymax=ft_up)

       array.defineHistogram('LBN,MaxPos;MaxVsTime', 
                                  title='Average Max Sample vs LumiBlock;Luminosity Block;Average Max Sample',
                                  type='TProfile',
                                  path=hist_path,
                                  xbins=lArDQGlobals.LB_Bins,xmin=lArDQGlobals.LB_Min,xmax=lArDQGlobals.LB_Max)
       array.defineHistogram('MaxPos,Energy;EnVsMaxSample', 
                                  title='Energy vs max sample;Sample Number;Energy [ADC] ',
                                  type='TH2F',
                                  path=hist_path,
                                  xbins=lArDQGlobals.Samples_Bins,xmin=lArDQGlobals.Samples_Min,xmax=lArDQGlobals.Samples_Max,
                                  ybins=lArDQGlobals.Energy_Bins, ymin=lArDQGlobals.Energy_Min, ymax=lArDQGlobals.Energy_Max)

       array.defineHistogram('MaxPos,streamBin;MaxSample_PerStream',
                             title="Position of the Max Sample;Average Max Sample",
                             type='TH2F',
                             path=hist_path,
                             xbins=lArDQGlobals.Samples_Bins,xmin=lArDQGlobals.Samples_Min,xmax=lArDQGlobals.Samples_Max,
                             ybins=len(lArDQGlobals.defaultStreamNames),ymin=-0.5, ymax=len(lArDQGlobals.defaultStreamNames)-0.5,
                             ylabels=lArDQGlobals.defaultStreamNames)
                             

       array.defineHistogram('l1trig,MaxPos;TriggerWord', 
                                  title='Position of max sample per L1 trigger word (8 bits);L1 trigger word;Position of max Sample ',
                                  type='TProfile',
                                  path=hist_path,
                                  xbins=lArDQGlobals.L1Trig_Bins,xmin=lArDQGlobals.L1Trig_Min,xmax=lArDQGlobals.L1Trig_Max)

       array.defineHistogram('Sample,SignalNorm;SignShape', 
                                  title='Normalized Signal Shape;Sample Number;Normalized Signal Shape ',
                                  type='TProfile',
                                  weight='weight',
                                  path=hist_path,
                                  xbins=lArDQGlobals.Samples_Bins,xmin=lArDQGlobals.Samples_Min,xmax=lArDQGlobals.Samples_Max)
    

    from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    if isComponentAccumulatorCfg():
        cfg=ComponentAccumulator()
        cfg.merge(helper.result())
        return cfg
    else:    
        return helper.result()
    

if __name__=='__main__':

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   from AthenaCommon.Logging import log
   from AthenaCommon.Constants import DEBUG
   log.setLevel(DEBUG)

   from AthenaConfiguration.TestDefaults import defaultTestFiles
   flags = initConfigFlags()
   flags.Input.Files = defaultTestFiles.RAW_RUN2

   flags.Output.HISTFileName = 'LArDigitsMonOutput.root'
   flags.DQ.enableLumiAccess = False
   flags.DQ.useTrigger = False
   flags.lock()

   from CaloRec.CaloRecoConfig import CaloRecoCfg
   cfg=CaloRecoCfg(flags)

   from LArCellRec.LArNoisyROSummaryConfig import LArNoisyROSummaryCfg
   cfg.merge(LArNoisyROSummaryCfg(flags))

  # from LArMonitoring.LArDigitMonAlg import LArDigitMonConfig
   aff_acc = LArDigitMonConfig(flags)
   cfg.merge(aff_acc)

   flags.dump()
   f=open("LArDigitMon.pkl","wb")
   cfg.store(f)
   f.close()

   #cfg.run(100)
