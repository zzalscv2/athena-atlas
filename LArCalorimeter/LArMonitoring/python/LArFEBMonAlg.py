#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def LArFEBMonConfig(flags, cellDebug=False, dspDebug=False):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'LArFEBMonAlgCfg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    LArFEBMonConfigCore(helper, CompFactory.LArFEBMonAlg,flags,cellDebug, dspDebug)

    rv = ComponentAccumulator()

    # adding LArFebErrorSummary algo
    from LArROD.LArFebErrorSummaryMakerConfig import LArFebErrorSummaryMakerCfg
    rv.merge(LArFebErrorSummaryMakerCfg(flags))
    
    rv.merge(helper.result())

    return rv

def LArFEBMonConfigCore(helper,algoinstance,flags, cellDebug=False, dspDebug=False):

    from LArMonitoring.GlobalVariables import lArDQGlobals

    larFEBMonAlg = helper.addAlgorithm(algoinstance,'larFEBMonAlg')

    GroupName="FEBMon"
    nslots=[]
    for i in range(0,len(lArDQGlobals.FEB_Slot)): 
       nslots.append(lArDQGlobals.FEB_Slot[lArDQGlobals.Partitions[i]][1])

    larFEBMonAlg.MonGroup=GroupName
    larFEBMonAlg.PartitionNames=lArDQGlobals.Partitions
    larFEBMonAlg.SubDetNames=lArDQGlobals.SubDet
    larFEBMonAlg.Streams=lArDQGlobals.defaultStreamNames

    isCOMP200=False
    from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg
    if isComponentAccumulatorCfg():
      if "COMP200" in flags.IOVDb.DatabaseInstance:
         isCOMP200=True
    else:      
      from IOVDbSvc.CondDB import conddb
      if conddb.GetInstance() == 'COMP200':
         isCOMP200=True

    if not isCOMP200:
       dbString="<db>COOLONL_LAR/CONDBR2</db>"
       persClass="AthenaAttributeList"
       fld="/LAR/Configuration/DSPThresholdFlat/Thresholds"
       if isComponentAccumulatorCfg():
          havethem=False
          for c in helper.resobj.getServices(): 
              if c.getName()=="IOVDbSvc":
                 iovDbSvc=c
                 condLoader=helper.resobj.getCondAlgo("CondInputLoader")
                 havethem=True
                 break
              pass
          if not havethem:
             from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
             helper.resobj.merge(IOVDbSvcCfg(flags))
             condLoader=helper.resobj.getCondAlgo("CondInputLoader")
             iovDbSvc=helper.resobj.getService("IOVDbSvc")
       else:   
          from AthenaCommon import CfgGetter
          iovDbSvc=CfgGetter.getService("IOVDbSvc")
          from AthenaCommon.AlgSequence import AthSequencer
          condSeq = AthSequencer("AthCondSeq")
          condLoader=condSeq.CondInputLoader

       iovDbSvc.Folders.append(fld+dbString)
       condLoader.Load.append((persClass,fld))
       larFEBMonAlg.Run2DSPThresholdsKey = fld
    else:
       fld='/LAR/Configuration/DSPThreshold/Thresholds'
       db='LAR_ONL'
       obj='LArDSPThresholdsComplete'
       if isComponentAccumulatorCfg():
           from IOVDbSvc.IOVDbSvcConfig import addFolders
           helper.resobj.merge(addFolders(flags,fld,db,obj))
       else:
           conddb.addFolder (db, fld, className=obj)
       larFEBMonAlg.Run1DSPThresholdsKey = 'LArDSPThresholds'


    Group = helper.addGroup(
        larFEBMonAlg,
        GroupName,
        '/LAr/'+GroupName+'/'
    )


    #Summary histos
    summary_hist_path='Summary/'
    
    #-- TTree for corrupted events timestamp
    Group.defineTree('timestamp,time_ns,febHwId,febErrorType;LArCorrupted', 
                     path=summary_hist_path,
                     title='Timestamps of corrupted LAr events',
                     treedef='timestamp/i:time_ns/i:febHwId/vector<int>:febErrorType/vector<int>')
    
    Group.defineHistogram('nbFEB;NbOfReadoutFEBGlobal', 
                                  title='# of readout FEB/DSP header',
                                  type='TH1I',
                                  path=summary_hist_path,
                                  opt='kAlwaysCreate',
                                  xbins=lArDQGlobals.N_FEB+11, xmin=-0.5, xmax=lArDQGlobals.N_FEB+10+0.5)
    Group.defineHistogram('nbFEBpart,part;NbOfEvts2d', 
                                  title='# of readout FEB/DSP header;Num. FEBs;Partition',
                                  type='TH2I',
                                  path=summary_hist_path,
                                  opt='kAlwaysCreate',
                                  xbins=lArDQGlobals.N_FEB_Parttions_Max, xmin=-0.5, xmax=lArDQGlobals.N_FEB_Parttions_Max-0.5,
                                  ybins=lArDQGlobals.N_Partitions, ymin=-0.5, ymax=lArDQGlobals.N_Partitions-0.5,
                                  ylabels=lArDQGlobals.Partitions
                                  )
    Group.defineHistogram('febError,part;NbOfLArFEBMonErrors_dE', 
                                  title='# of data corruption errors',
                                  type='TH2I',
                                  path=summary_hist_path,
                                  opt='kAlwaysCreate',
                                  xbins=lArDQGlobals.N_FEBErrors, xmin=0.5, xmax=lArDQGlobals.N_FEBErrors+0.5,
                                  ybins=lArDQGlobals.N_Partitions, ymin=-0.5, ymax=lArDQGlobals.N_Partitions-0.5,
                                  xlabels=lArDQGlobals.FEBErrors, ylabels=lArDQGlobals.Partitions)
    Group.defineHistogram('dspThrADC;dspThresholdsADC', 
                          title='DSP thresholds to readout samples;Number of cells;Cell threshold in ADC counts',
                          type='TH1I',
                          path=summary_hist_path,
                          xbins=lArDQGlobals.DSPThr_Bins+1, xmin=-0.5, xmax=lArDQGlobals.DSPThr_Bins+0.5,
                          merge='identical')
    Group.defineHistogram('dspThrQT;dspThresholds_qfactortime', 
                          title='DSP thresholds to readout (qfactor+time);Number of cells;Cell threshold in ADC counts',
                          type='TH1I',
                          path=summary_hist_path,
                          xbins=lArDQGlobals.DSPThr_Bins+1, xmin=-0.5, xmax=lArDQGlobals.DSPThr_Bins+0.5,
                          merge='identical')
    Group.defineHistogram('EvtType;Eventtype', 
                                  title='Event type (1st readout FEB)',
                                  type='TH1I',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.Evt_Bins, xmin=lArDQGlobals.Evt_Min, xmax=lArDQGlobals.Evt_Max,
                                  xlabels=lArDQGlobals.Evt_labels)
    Group.defineHistogram('LVL1Trig;TriggerWord', 
                                  title='Number of Events per L1 trigger word (8 bits);L1 trigger word',
                                  type='TH1I',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.L1Trig_Bins, xmin=lArDQGlobals.L1Trig_Min, xmax=lArDQGlobals.L1Trig_Max)
    Group.defineHistogram('LVL1TrigAllDSP;TriggerWordAllDSP', 
                                  title='Number of L1 trigger word per DSP (8 bits);L1 trigger word',
                                  type='TH1I',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.L1Trig_Bins, xmin=lArDQGlobals.L1Trig_Min, xmax=lArDQGlobals.L1Trig_Max)
    Group.defineHistogram('EvtRej;EventsRejected', 
                                  title='Nb of events rejected (at least one error)',
                                  type='TH1I',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.EvtRej_Bins, xmin=lArDQGlobals.EvtRej_Min, xmax=lArDQGlobals.EvtRej_Max,
                                  xlabels=lArDQGlobals.EvtRej_labels)
    Group.defineHistogram('LB0,EvtRejYield;RAW_YieldOfRejectedEventsVsLB', 
                                  title='Yield of corrupted events (DATACORRUPTED);Luminosity Block;Yield(%)',
                                  type='TProfile',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)
    Group.defineHistogram('LB0,EvtRejYieldOut;RAW_YieldOfRejectedEventsVsLBout', 
                                  title='Yield of corrupted events (DATACORRUPTED) not vetoed by time window;Luminosity Block;Yield(%)',
                                  type='TProfile',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)
    Group.defineHistogram('LB0,EvtOneErrorYield;RAW_YieldOfOneErrorEventsVsLB', 
                                  title='Yield of events with >=1 FEB in error;Luminosity Block;Yield(%)',
                                  type='TProfile',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)
    Group.defineHistogram('rejBits;rejectionBits', 
                                  title='Errors at the origin of event rejection;Bits;Number of (rejected) events',
                                  type='TH1I',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.rejBits_Bins, xmin=-0.5, xmax=lArDQGlobals.rejBits_Bins-0.5)
    Group.defineHistogram('LB0;NbOfEventsVsLB', 
                                  title='Nb of events per LB;Luminosity Block',
                                  type='TH1I',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)
    Group.defineHistogram('NbOfSweet2;NbOfSw2', 
                                  title='# of cells with samples readout;Number of cells;Number of events',
                                  type='TH1I',
                                  path=summary_hist_path,
                                  xbins=int(lArDQGlobals.N_Cells/10), xmin=-1000, xmax=lArDQGlobals.N_Cells-1000)
    Group.defineHistogram('LB0,LArEvSize;eventSizeVsLB', 
                                  title='LAr event size (w/o ROS headers);Luminosity Block;Megabytes',
                                  type='TProfile',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)
    Group.defineHistogram('NbOfSamp;NbOfSamples', 
                                  title='# of samples (1st readout FEB);Samples;Number of events',
                                  type='TH1I',
                                  path=summary_hist_path,
                                  xbins=lArDQGlobals.Samples_Bins, xmin=lArDQGlobals.Samples_Min, xmax=lArDQGlobals.Samples_Max)

    isOnline=False
    if isComponentAccumulatorCfg() :
      if flags.DQ.Environment == 'online':
         isOnline=True
    else:
      from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
      if athenaCommonFlags.isOnline:
         isOnline=True

    if isOnline:     
       Group.defineHistogram('LBf,EvtRejYield;RAW_EventsRejectedLB',
                                title='% of events rejected in current LB (online only)',
                                type='TProfile',
                                path=summary_hist_path,
                                xbins=1, xmin=0, xmax=1, xlabels=['% of events'])
       Group.defineHistogram('LB,streamBin,LArEvSizePart;eventSizeStreamVsLB',
                                title='LAr event size per stream per LB (w/o ROS headers)',
                                type='TProfile2D',
                                path=summary_hist_path,
                                xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max,
                                ybins=len(larFEBMonAlg.Streams),ymin=-0.5, ymax= len(larFEBMonAlg.Streams)-0.5,
                                ylabels=list(larFEBMonAlg.Streams) 
                                )

    # Now per partition histograms
    for subdet in range(0,lArDQGlobals.N_SubDet):
       hist_path='/LAr/'+GroupName+'/'+lArDQGlobals.SubDet[subdet]+'/'
       slot_low = lArDQGlobals.FEB_Slot[lArDQGlobals.Partitions[subdet*2]][0] - 0.5
       slot_up  = lArDQGlobals.FEB_Slot[lArDQGlobals.Partitions[subdet*2]][1] + 0.5
       slot_n = int(slot_up - slot_low)
       ft_low = lArDQGlobals.FEB_Feedthrough[lArDQGlobals.Partitions[subdet*2]][0] - 0.5
       ft_up  = lArDQGlobals.FEB_Feedthrough[lArDQGlobals.Partitions[subdet*2]][1] + 0.5
       ft_n = int(ft_up - ft_low)

       darray = helper.addArray([lArDQGlobals.Partitions[2*subdet:2*subdet+2]],larFEBMonAlg,lArDQGlobals.SubDet[subdet],topPath='/')

       darray.defineHistogram('slotPar,FTPar;RAW_Parity',
                              title='Parity error;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotBcid,FTBcid;RAW_BCID',
                              title='BCID mismatch betw. 2 halves of FEB;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotRadd,FTRadd;RAW_RADD',
                              title='Sample header mismatch betw. 2 halves of FEB;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotEvtid,FTEvtid;RAW_EVTID',
                              title='EVTID mismatch betw. 2 halves of FEB;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotScac,FTScac;RAW_SCACStatus',
                              title='Wrong SCAC status in one half of a FEB;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotscout,FTscout;RAW_scaOutOfRange',
                              title='Sca out of range;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotgain,FTgain;RAW_gainMismatch',
                              title='Gain mismatch within time samples;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slottype,FTtype;RAW_typeMismatch',
                              title='Event type mismatch;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotsmp,FTsmp;RAW_badNbOfSamp',
                              title='Non uniform number of samples;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotzero,FTzero;RAW_zeroSamp',
                              title='Empty FEB data blocks;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotsum,FTsum;RAW_checkSum',
                              title='Checksum / DSP block size;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              opt='kAlwaysCreate',
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotmis,FTmis;RAW_missingHeader',
                              title='Missing header ;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotgain,FTgain;RAW_badGain',
                              title='Bad gain ;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotabs,FTabs;RAW_LArFEBMonErrorsAbsolute',
                              title='Nb of events with at least one error ;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              opt='kAlwaysCreate',
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)
        
       darray.defineHistogram('slotmist,FTmist;RAW_missingTriggerType',
                              title='LVL1 trigger type missing or different from event type ;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              opt='kAlwaysCreate',
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotnb,FTnb;RAW_nbOfEvts',
                              title='Nb of events (DSP header check only) ;Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              opt='kAlwaysCreate',
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotnb,FTnb,weightsweet1;RAW_NbOfSweet1PerFEB',
                              title='Average # of cells with (qfactor+time) readout ;Slot;FT',
                              type='TProfile2D',
                              path=hist_path,
                              opt='kAlwaysCreate',
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('slotnb,FTnb,weightsweet2;RAW_NbOfSweet2PerFEB',
                              title='Average # of cells with samples readout ;Slot;FT',
                              type='TProfile2D',
                              path=hist_path,
                              opt='kAlwaysCreate',
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up)

       darray.defineHistogram('nbFEBpart;nbOfFebBlocks',
                              title='# of readout FEBs (DSP header check only) ;Slot;FT',
                              type='TH1I',
                              path=hist_path,
                              xbins=lArDQGlobals.N_FEB_Parttions_Max, xmin=-0.5, xmax=lArDQGlobals.N_FEB_Parttions_Max-0.5)

       darray.defineHistogram('slotMasked,FTMasked;RAW_knownFaultyFEB',
                              title='FEB with known errors (1:err. ignored 2:FEB masked);Slot;FT',
                              type='TH2I',
                              path=hist_path,
                              xbins=slot_n,xmin=slot_low,xmax=slot_up,
                              ybins=ft_n, ymin=ft_low, ymax=ft_up,
                              merge='identical')

       darray.defineHistogram('LB,LArEvSizePart;eventSizeVsLB',
                              title='LAr event size per LB (w/o ROS headers);Luminosity Block',
                              type='TProfile',
                              path=hist_path,
                              xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max)

       if isOnline:
          darray.defineHistogram('LBf,erronl;RAW_EventsRejectedLB',
                                title='% of events rejected in current LB (online only)',
                                type='TProfile',
                                path=hist_path,
                                opt='kAlwaysCreate',
                                xbins=1, xmin=0, xmax=1, xlabels=['% of events'])
          darray.defineHistogram('LB,streamBin,LArEvSizePart;eventSizeStreamVsLB',
                                title='LAr event size per stream per LB (w/o ROS headers)',
                                type='TProfile2D',
                                path=hist_path,
                                xbins=lArDQGlobals.LB_Bins, xmin=lArDQGlobals.LB_Min, xmax=lArDQGlobals.LB_Max,
                                ybins=len(larFEBMonAlg.Streams),ymin=-0.5, ymax= len(larFEBMonAlg.Streams)-0.5,
                                ylabels=list(larFEBMonAlg.Streams)
                                )
       pass



    

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

   flags.Output.HISTFileName = 'LArFEBMonOutput.root'
   flags.DQ.enableLumiAccess = True
   flags.DQ.useTrigger = False
   flags.lock()


   from CaloRec.CaloRecoConfig import CaloRecoCfg
   cfg=CaloRecoCfg(flags)

   #from CaloD3PDMaker.CaloD3PDConfig import CaloD3PDCfg,CaloD3PDAlg
   #cfg.merge(CaloD3PDCfg(flags, filename=flags.Output.HISTFileName, streamname='CombinedMonitoring'))

   aff_acc = LArFEBMonConfig(flags)
   cfg.merge(aff_acc)

   cfg.printConfig()

   flags.dump()
   f=open("LArFEBMon.pkl","wb")
   cfg.store(f)
   f.close()

