# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoring.DQConfigFlags import DQDataType

import math

class TrigTauMonAlgBuilder:

  _configured = False
  _get_monitoring_mode_success = False

  data_type = ''
  pp_mode = False
  cosmic_mode = False
  mc_mode = False
  activate_tau = False
  tauList = []

  basePath = 'HLT/TauMon'

  def __init__(self, helper):
 
    from AthenaCommon.Logging import logging
    self.__logger = logging.getLogger( 'TrigTauMonAlgBuilder' )
    self.helper = helper
    if not self._configured:
      self.configureMode()
  
  def configureMode(self):

    self.__logger.info("TrigTauMonToolBuilder.configureMode()")
    self._get_monitoring_mode_success = self.get_monitoring_mode()
    if self._get_monitoring_mode_success is False:
      self.__logger.warning("Error getting monitoring mode, default monitoring lists will be used.")
    else:
      self.__logger.info("Configuring for %s", self.data_type)

    self.activate_tau=True 

    
  def configure(self):
    self.setProperties()
    self.configureMonitor()
    self.configureHistograms()


  def getTrigInfo( self, trigger ):

    class TrigTauInfo(object):

      def __init__(self, trigger):
        self.__chain = trigger

      def chain(self):
        return self.__chain

      def isL1Item(self):
        return True if self.chain().startswith('L1') else False

      def isComboChain(self):
        isComboChain = False
        splits = self.chain().split("_")
        Npart = 0
        for split in splits:
            if split.startswith('tau'):
                Npart+=1
        if Npart > 1:
            isComboChain = True
        return isComboChain        

      def L1seed(self):
        l1seed = ''
        splits = self.chain().split("_")
        for split in splits:
            if split.startswith('L1TAU') or split.startswith('L1eTAU') or split.startswith('L1jTAU') or split.startswith('L1cTAU'):
                l1seed = split
        return l1seed

      def isDiTau(self):
        return True if ( self.chain().count('tau') == 2 and '03dRAB' in self.chain()) else False

      def isTAndP(self):
        return True if ( ("ivarloose" in self.chain() or "ivarmedium" in self.chain()) and '03dRAB' in self.chain()) else False

    return TrigTauInfo(trigger)


  def get_monitoring_mode(self):

    self.__logger.info("TrigTauMonToolBuilder.get_monitoring_mode()")
    self.data_type = self.helper.flags.DQ.DataType
    if self.data_type is DQDataType.MC:
      self.mc_mode = True
      return True
    elif self.data_type is DQDataType.Collisions:
      self.pp_mode = True
      return True
    elif self.data_type is DQDataType.Cosmics:
      self.cosmic_mode = True
      return True
    else:
      return False


  def setProperties(self):

    self.__logger.info("TrigTauMonToolBuilder.setProperties()")
    self.basePath = 'HLT/TauMon'
   
    if self.pp_mode is True:
      self.setDefaultProperties()
    elif self.cosmic_mode is True:
      # This should be change in future
      self.setDefaultProperties()
    elif self.mc_mode is True:
      # This should be change in future
      self.setDefaultProperties()
    else:
      self.__logger.info('No monitoring mode configured, use default')
      self.setDefaultProperties()


    self.__logger.info('Configuring tau chains %s',self.tauList)


  def setDefaultProperties(self):

    ### monitorig groups
    from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
    moniAccess=getHLTMonitoringAccess(self.helper.flags)
    monitoring_tau=moniAccess.monitoredChains(signatures="tauMon",monLevels=["shifter","t0","val"])
  
    # if mon groups not found fall back to hard-coded trigger monitoring list
    if len(monitoring_tau) == 0:
      monitoring_tau = [
       # tau0
       'HLT_tau0_ptonly_L1TAU8',
       'HLT_tau0_ptonly_L1TAU60',
       # tau25
       'HLT_tau25_idperf_tracktwoMVA_L1TAU12IM',
       'HLT_tau25_perf_tracktwoMVA_L1TAU12IM',
       'HLT_tau25_looseRNN_tracktwoMVA_L1TAU12IM',
       'HLT_tau25_mediumRNN_tracktwoMVA_L1TAU12IM',
       'HLT_tau25_tightRNN_tracktwoMVA_L1TAU12IM',
       # tau35
       'HLT_tau35_idperf_tracktwoMVA_L1TAU20IM',
       'HLT_tau35_perf_tracktwoMVA_L1TAU20IM',
       'HLT_tau35_looseRNN_tracktwoMVA_L1TAU20IM',
       'HLT_tau35_mediumRNN_tracktwoMVA_L1TAU20IM',
       'HLT_tau35_tightRNN_tracktwoMVA_L1TAU20IM',
       # tau60
       'HLT_tau60_mediumRNN_tracktwoMVA_L1TAU40',
       # tau80
       'HLT_tau80_mediumRNN_tracktwoMVA_L1TAU60', 
       # tau160
       'HLT_tau160_ptonly_L1TAU100',
       'HLT_tau160_idperf_tracktwoMVA_L1TAU100',
       'HLT_tau160_perf_tracktwoMVA_L1TAU100',
       'HLT_tau160_mediumRNN_tracktwoMVA_L1TAU100',
       # tau180
       'HLT_tau180_mediumRNN_tracktwoLLP_L1TAU100',
       'HLT_tau180_tightRNN_tracktwoLLP_L1TAU100',
       # tau200
       'HLT_tau200_mediumRNN_tracktwoMVA_L1TAU100',
       'HLT_tau200_mediumRNN_tracktwoLLP_L1TAU100',
       'HLT_tau200_tightRNN_tracktwoLLP_L1TAU100',
       # ditau
       'HLT_tau80_mediumRNN_tracktwoMVA_tau60_mediumRNN_tracktwoMVA_03dRAB_L1TAU60_2TAU40',
       'HLT_tau80_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB30_L1TAU60_DR-TAU20ITAU12I',
       'HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB30_L1DR-TAU20ITAU12I-J25',
       'HLT_tau80_mediumRNN_tracktwoLLP_tau60_mediumRNN_tracktwoLLP_03dRAB_L1TAU60_2TAU40',
       'HLT_tau80_mediumRNN_tracktwoLLP_tau60_tightRNN_tracktwoLLP_03dRAB_L1TAU60_2TAU40',
       'HLT_tau80_tightRNN_tracktwoLLP_tau60_tightRNN_tracktwoLLP_03dRAB_L1TAU60_2TAU40',
       'HLT_tau100_mediumRNN_tracktwoLLP_tau80_mediumRNN_tracktwoLLP_03dRAB_L1TAU60_2TAU40',
       'HLT_tau35_mediumRNN_tracktwoMVA_tau25_mediumRNN_tracktwoMVA_03dRAB_L1TAU20IM_2TAU12IM_4J12p0ETA25',
       'HLT_tau40_mediumRNN_tracktwoMVA_tau35_mediumRNN_tracktwoMVA_03dRAB_L1TAU25IM_2TAU20IM_2J25_3J20',
       # Tag and probe
       'HLT_e26_lhtight_ivarloose_tau20_mediumRNN_tracktwoMVA_03dRAB_L1EM22VHI',
       'HLT_mu26_ivarmedium_tau20_mediumRNN_tracktwoMVA_03dRAB_L1MU14FCH',
       # LRT 
       'HLT_tau25_idperf_trackLRT_L1TAU12IM',
       'HLT_tau80_idperf_trackLRT_L1TAU60',
       'HLT_tau160_idperf_trackLRT_L1TAU100',
       'HLT_tau25_mediumRNN_trackLRT_L1TAU12IM', 
       'HLT_tau80_mediumRNN_trackLRT_L1TAU60',
       'HLT_tau160_mediumRNN_trackLRT_L1TAU100',
       # Phase-I
       'HLT_tau25_idperf_tracktwoMVA_L1eTAU20',
       'HLT_tau25_perf_tracktwoMVA_L1eTAU20', 
       'HLT_tau25_mediumRNN_tracktwoMVA_L1eTAU20',
       'HLT_tau25_idperf_tracktwoMVA_L1eTAU20M',
       'HLT_tau25_perf_tracktwoMVA_L1eTAU20M',
       'HLT_tau25_mediumRNN_tracktwoMVA_L1eTAU20M',     
       'HLT_tau35_mediumRNN_tracktwoMVA_L1eTAU30', 
       'HLT_tau160_mediumRNN_tracktwoMVA_L1eTAU140',
    ]

    self.tauList = monitoring_tau

  #
  # Create all minitor algorithms
  #
  def configureMonitor( self ):

    if self.activate_tau:

      self.__logger.info( "Creating the Tau monitor algorithm...")
      self.tauMonAlg = self.helper.addAlgorithm( CompFactory.TrigTauMonitorAlgorithm, "TrigTauMonAlg" )
      self.tauMonAlg.TriggerList=self.tauList    
      isMC = False
      self.data_type = self.helper.flags.DQ.DataType
      if self.data_type == DQDataType.MC:
         isMC = True
      self.tauMonAlg.isMC = isMC


  def configureHistograms(self):

    if self.activate_tau and self.tauMonAlg:
      self.bookExpertHistograms( self.tauMonAlg, self.tauMonAlg.TriggerList )

  #
  # Booking all histograms
  #
  def bookExpertHistograms( self, monAlg, triggers ):

    self.__logger.info( "Booking all histograms for alg: %s", monAlg.name )

    l1seeds = []

    for trigger in triggers:
      info = self.getTrigInfo(trigger)
  
      l1seeds.append(info.L1seed())

      self.bookTruth( monAlg, trigger, nProng='1P')
      self.bookTruth( monAlg, trigger, nProng='3P')
      self.bookTruthEfficiency( monAlg, trigger, nProng='1P')
      self.bookTruthEfficiency( monAlg, trigger, nProng='3P')

      if(info.isDiTau()):
        self.bookDiTauVars(monAlg, trigger)   
        self.bookDiTauHLTEffHistograms(monAlg, trigger)
      elif(info.isTAndP()):
        self.bookTAndPVars(monAlg, trigger)
        self.bookTAndPHLTEffHistograms(monAlg, trigger)
      else:
        self.bookbasicVars( monAlg, trigger, '0P', online=True )
        self.bookbasicVars( monAlg, trigger, '1P', online=True )
        self.bookbasicVars( monAlg, trigger, 'MP', online=True )
        self.bookbasicVars( monAlg, trigger, '1P', online=False )
        self.bookbasicVars( monAlg, trigger, '3P', online=False )

        self.bookHLTEffHistograms( monAlg, trigger,nProng='1P')
        self.bookHLTEffHistograms( monAlg, trigger,nProng='3P')

        self.bookRNNInputVars( monAlg, trigger,nProng='0P', online=True )
        self.bookRNNInputVars( monAlg, trigger,nProng='1P', online=True )
        self.bookRNNInputVars( monAlg, trigger,nProng='MP', online=True )
        self.bookRNNInputVars( monAlg, trigger,nProng='1P', online=False )
        self.bookRNNInputVars( monAlg, trigger,nProng='3P', online=False )
        self.bookRNNTrack( monAlg, trigger, online=True )
        self.bookRNNCluster( monAlg, trigger, online=True )
        self.bookRNNTrack( monAlg, trigger, online=False )
        self.bookRNNCluster( monAlg, trigger, online=False )


    #remove duplicated from L1 seed list
    l1seeds = list(dict.fromkeys(l1seeds))
    for l1seed in l1seeds:
        if not l1seed : 
            continue
        self.bookL1( monAlg, l1seed)
        self.bookL1EffHistograms( monAlg, l1seed, nProng='1P')
        self.bookL1EffHistograms( monAlg, l1seed, nProng='3P') 
           

  #
  # Booking HLT efficiencies
  #

  def bookHLTEffHistograms( self, monAlg, trigger, nProng ):

    monGroupName = trigger+'_HLT_Efficiency_'+nProng
    monGroupPath = 'HLT_Efficiency/'+trigger+'/HLT_Efficiency_'+ nProng

    monGroup = self.helper.addGroup( monAlg, monGroupName, 
                              self.basePath+'/'+monGroupPath )

    def defineEachStepHistograms(xvariable, xlabel, xbins, xmin, xmax):

       monGroup.defineHistogram(monGroupName+'_HLTpass,'+monGroupName+'_'+xvariable+';EffHLT_'+xvariable+'_wrt_Offline',
                                title='HLT Efficiency ' +trigger+' '+nProng+';'+xlabel+';Efficiency',
                                type='TEfficiency',xbins=xbins,xmin=xmin,xmax=xmax,opt='kAlwaysCreate')

    defineEachStepHistograms('tauPt', 'p_{T} [GeV]', 60, 0.0, 300.)
    defineEachStepHistograms('tauEta','#eta', 13, -2.6, 2.6)
    defineEachStepHistograms('tauPhi','#phi', 16, -3.2, 3.2) 
    defineEachStepHistograms('averageMu', 'average pileup', 10, 0., 80.)

 
    def defineEachStepHistogramsCoarse(xvariable, xlabel, binning):
   
        monGroup.defineHistogram(monGroupName+'_HLTpass,'+monGroupName+'_'+xvariable+';EffHLT_'+xvariable+'_wrt_Offline',
                                title='HLT Efficiency ' +trigger+' '+nProng+';'+xlabel+';Efficiency',
                                type='TEfficiency',xbins=binning, xmin=binning[0], xmax=binning[-1], opt='kAlwaysCreate')

    #define coarse binning
    binning = [0,500]
    if 'HLT_tau0' in trigger: binning   = [ 0, 5, 10, 15, 20, 50, 100, 150, 250, 500 ] 
    elif 'HLT_tau25' in trigger: binning = [15, 20, 25, 30, 35, 40, 50, 60, 80, 150, 250, 500 ]
    elif 'HLT_tau35' in trigger: binning = [25, 30, 35, 40, 50, 60, 80, 150,  250, 500 ]
    elif 'HLT_tau60' in trigger: binning = [50, 55, 60, 65, 70, 80, 110, 150,  250, 500 ]
    elif 'HLT_tau80' in trigger: binning = [70, 75, 80, 85, 90,  110, 150,  250, 500 ]
    elif 'HLT_tau160' in trigger: binning = [150, 155, 160, 165, 170, 180, 200, 240, 300, 500 ]
    elif 'HLT_tau200' in trigger: binning = [190, 195, 200, 205, 210, 240, 300, 500 ]
    defineEachStepHistogramsCoarse('tauPt_coarse', 'p_{T} [GeV]', binning)

    # save quantities in TTree for offline analysis
    monGroup.defineTree(monGroupName+'_tauPt,'+monGroupName+'_tauEta,'+monGroupName+'_tauPhi,'+monGroupName+'_averageMu,'+monGroupName+'_HLTpass;HLTEffTree', treedef=monGroupName+'_tauPt/F:'+monGroupName+'_tauEta/F:'+monGroupName+'_tauPhi/F:'+monGroupName+'_averageMu/F:'+monGroupName+'_HLTpass/I')

  #
  # Booking DiTau efficiencies
  #

  def bookDiTauHLTEffHistograms(self, monAlg, trigger):
  
    monGroupName = trigger+'_DiTauHLT_Efficiency'
    monGroupPath = 'DiTauHLT_Efficiency/'+trigger+'/DiTauHLT_Efficiency'

    monGroup = self.helper.addGroup( monAlg, monGroupName,
                              self.basePath+'/'+monGroupPath )

    def defineEachStepHistograms(xvariable, xlabel, xbins, xmin, xmax):

       monGroup.defineHistogram(monGroupName+'_DiTauHLTpass,'+monGroupName+'_'+xvariable+';EffDiTauHLT_'+xvariable+'_wrt_Offline',
                                title='DiTau HLT Efficiency ' +trigger+';'+xlabel+';Efficiency',
                                type='TEfficiency',xbins=xbins,xmin=xmin,xmax=xmax,opt='kAlwaysCreate')

    defineEachStepHistograms('dR',' dR(#tau,#tau)',20,0,4)
    defineEachStepHistograms('dEta',' dEta(#tau,#tau)',20,0,4)
    defineEachStepHistograms('dPhi',' dPhi(#tau,#tau)',8, -3.2, 3.2)
    defineEachStepHistograms('averageMu', 'average pileup', 10, 0., 80.)


    # save quantities in TTree for offline analysis
    monGroup.defineTree(monGroupName+'_dR,'+monGroupName+'_dEta,'+monGroupName+'_dPhi,'+monGroupName+'_averageMu,'+monGroupName+'_DiTauHLTpass;DiTauHLTEffTree', treedef=monGroupName+'_dR/F:'+monGroupName+'_dEta/F:'+monGroupName+'_dPhi/F:'+monGroupName+'_averageMu/F:'+monGroupName+'_DiTauHLTpass/I')

  #
  # Booking TAndP efficiencies
  #

  def bookTAndPHLTEffHistograms(self, monAlg, trigger):
  
    monGroupName = trigger+'_TAndPHLT_Efficiency'
    monGroupPath = 'TAndPHLT_Efficiency/'+trigger+'/TAndPHLT_Efficiency'

    monGroup = self.helper.addGroup( monAlg, monGroupName,
                              self.basePath+'/'+monGroupPath )

    def defineEachStepHistograms(xvariable, xlabel, xbins, xmin, xmax):

       monGroup.defineHistogram(monGroupName+'_TAndPHLTpass,'+monGroupName+'_'+xvariable+';EffTAndPHLT_'+xvariable+'_wrt_Offline',
                                title='TAndP HLT Efficiency ' +trigger+';'+xlabel+';Efficiency',
                                type='TEfficiency',xbins=xbins,xmin=xmin,xmax=xmax,opt='kAlwaysCreate')

    defineEachStepHistograms('tauPt', 'p_{T} [GeV]', 60, 0.0, 300.)
    defineEachStepHistograms('tauEta','#eta', 13, -2.6, 2.6)
    defineEachStepHistograms('tauPhi','#phi', 16, -3.2, 3.2)
    defineEachStepHistograms('dR',' dR(#tau,lep)',20,0,4)
    defineEachStepHistograms('dEta',' dEta(#tau,lep)',20,0,4)
    defineEachStepHistograms('dPhi',' dPhi(#tau,lep)',8, -3.2, 3.2)
    defineEachStepHistograms('averageMu', 'average pileup', 10, 0., 80.)

     
    def defineEachStepHistogramsCoarse(xvariable, xlabel, binning):
 
       monGroup.defineHistogram(monGroupName+'_TAndPHLTpass,'+monGroupName+'_'+xvariable+';EffTAndPHLT_'+xvariable+'_wrt_Offline',
                                title='TAndP HLT Efficiency ' +trigger+';'+xlabel+';Efficiency',
                                type='TEfficiency',xbins=binning, xmin=binning[0], xmax=binning[-1],opt='kAlwaysCreate')

    binning = [0,500]
    if 'tau20' in trigger : binning = [10, 15, 20, 25, 30, 35, 40, 50, 60, 80, 150, 250, 500 ]
    elif 'tau25' in trigger: binning = [15, 20, 25, 30, 35, 40, 50, 60, 80, 150, 250, 500 ]
    elif 'tau35' in trigger: binning = [25, 30, 35, 40, 50, 60, 80, 150,  250, 500 ]
    elif 'tau60' in trigger: binning = [50, 55, 60, 65, 70, 80, 110, 150,  250, 500 ]
    elif 'tau80' in trigger: binning = [70, 75, 80, 85, 90,  110, 150,  250, 500 ]
    elif 'tau160' in trigger: binning = [150, 155, 160, 165, 170, 180, 200, 240, 300, 500 ]
    elif 'tau200' in trigger: binning = [190, 195, 200, 205, 210, 240, 300, 500 ]

    defineEachStepHistogramsCoarse('tauPt_coarse', 'p_{T} [GeV]', binning)

    # save quantities in TTree for offline analysis
    monGroup.defineTree(monGroupName+'_tauPt,'+monGroupName+'_tauEta,'+monGroupName+'_tauPhi,'+monGroupName+'_dR,'+monGroupName+'_dEta,'+monGroupName+'_dPhi,'+monGroupName+'_averageMu,'+monGroupName+'_TAndPHLTpass;TAndPHLTEffTree', treedef=monGroupName+'_tauPt/F:'+monGroupName+'_tauEta/F:'+monGroupName+'_tauPhi/F:'+monGroupName+'_dR/F:'+monGroupName+'_dEta/F:'+monGroupName+'_dPhi/F:'+monGroupName+'_averageMu/F:'+monGroupName+'_TAndPHLTpass/I')

  #
  # Booking L1 efficiencies
  #
 
  def bookL1EffHistograms( self, monAlg, L1seed, nProng ):

    monGroupName = L1seed+'_L1_Efficiency_'+nProng
    monGroupPath = 'L1_Efficiency/'+L1seed+'/L1_Efficiency_'+ nProng

    monGroup = self.helper.addGroup( monAlg, monGroupName,
                              self.basePath+'/'+monGroupPath )

    def defineEachStepHistograms(xvariable, xlabel, xbins, xmin, xmax):

       monGroup.defineHistogram(monGroupName+'_L1pass,'+monGroupName+'_'+xvariable+';EffL1_'+xvariable+'_wrt_Offline',
                                title='L1 Efficiency ' +L1seed+' '+nProng+';'+xlabel+';Efficiency',
                                type='TEfficiency',xbins=xbins,xmin=xmin,xmax=xmax, opt='kAlwaysCreate')

    defineEachStepHistograms('tauPt', 'p_{T} [GeV]', 60, 0.0, 300.)
    defineEachStepHistograms('tauEta','#eta', 13, -2.6, 2.6)
    defineEachStepHistograms('tauPhi','#phi', 16, -3.2, 3.2)
    defineEachStepHistograms('averageMu', 'average pileup', 10, 0., 80.)

  #
  # Booking L1 Variables
  #

  def bookL1( self, monAlg, trigL1Item):

    monGroupName = trigL1Item+'_L1'
    monGroupPath = 'L1/'+trigL1Item

    monGroup = self.helper.addGroup( monAlg, monGroupName,
                              self.basePath+'/'+monGroupPath )

    monGroup.defineHistogram('L1RoIEt,L1RoIEta', type='TH2F', title='L1 RoI Et vs Eta; E_{T}[GeV]; #eta',
                            xbins=100,xmin=0,xmax=100,
                             ybins=60,ymin=-2.5,ymax=2.5, opt='kAlwaysCreate')
    monGroup.defineHistogram('L1RoIEt,L1RoIPhi', type='TH2F', title='L1 RoI Et vs Phi; E_{T}[GeV]; #phi',
                            xbins=100,xmin=0,xmax=100,
                             ybins=100,ymin=-math.pi,ymax=math.pi, opt='kAlwaysCreate')
    monGroup.defineHistogram('L1RoIEta,L1RoIPhi', type='TH2F', title='L1 RoI Eta vs Phi; #eta; #phi',
                            xbins=60,xmin=-2.5,xmax=2.5,
                             ybins=100,ymin=-math.pi,ymax=math.pi, opt='kAlwaysCreate')
    monGroup.defineHistogram('L1RoIEMIsol', title='L1 RoI EM Isol; E_{T}^{EM Isol}[GeV]; N RoI',xbins=16,xmin=-2,xmax=30,opt='kAlwaysCreate')
    monGroup.defineHistogram('L1RoIEta', title='L1 RoI Eta; #eta; N RoI',xbins=60,xmin=-2.5,xmax=2.5,opt='kAlwaysCreate')
    monGroup.defineHistogram('L1RoIHadCore', title='L1 RoI HAD Core; E_{T}^{HAD}[GeV]; N RoI',xbins=16,xmin=-2,xmax=30,opt='kAlwaysCreate')
    monGroup.defineHistogram('L1RoIHadIsol', title='L1 RoI HAD Isol; E_{T}^{HAD Isol}[GeV]; N RoI',xbins=16,xmin=-2,xmax=30,opt='kAlwaysCreate')
    monGroup.defineHistogram('L1RoIPhi', title='L1 RoI Phi; #phi; N RoI',xbins=100,xmin=-math.pi,xmax=math.pi,opt='kAlwaysCreate')
    monGroup.defineHistogram('L1RoITauClus', title='L1 RoI Tau Clust Energy; E_{T}[GeV]; N RoI',xbins=260,xmin=0,xmax=130,opt='kAlwaysCreate')
    monGroup.defineHistogram('L1RoITauClus,L1RoIEMIsol', type='TH2F', title='L1 RoI TauClus vs EMiso; E_{T}[GeV]; E_{T}^{EM Isol}[GeV]',
                            xbins=140,xmin=10,xmax=80,
                             ybins=42,ymin=-1,ymax=20,opt='kAlwaysCreate')
    monGroup.defineHistogram('L1RoIEt', title='L1 RoI Tau Clust Energy; E_{T}[GeV]; N RoI',xbins=30,xmin=0,xmax=150,opt='kAlwaysCreate')
    monGroup.defineHistogram('L1RoIRCore', title='L1 RoI RCore isolation; rCore isolation; N RoI',xbins=250,xmin=0,xmax=1,opt='kAlwaysCreate')
    monGroup.defineHistogram('L1RoIRHad' , title='L1 RoI RHAD isolation; rHad isolation; N RoI'  ,xbins=250,xmin=0,xmax=1,opt='kAlwaysCreate')
    monGroup.defineHistogram('L1RoIIso', title='L1 RoI isolation; isolation [GeV]; N RoI'  ,xbins=15,xmin=0,xmax=30,opt='kAlwaysCreate')
                             
  #
  # Book RNN Variables
  #
  def bookRNNInputVars( self, monAlg, trigger,nProng, online ):

    monGroupName = trigger+'_RNN_'+('HLT' if online else 'Offline')+'_InputScalar_'+nProng
    monGroupPath = 'RNN/InputScalar_'+nProng+'/'+trigger+('/HLT' if online else '/Offline')

    monGroup = self.helper.addGroup( monAlg, monGroupName, 
                              self.basePath+'/'+monGroupPath )

    monGroup.defineHistogram('centFrac', title='Centrality Fraction ('+nProng+'); centFrac; Events',xbins=50,xmin=-0.05,xmax=1.2,opt='kAlwaysCreate')
    monGroup.defineHistogram('etOverPtLeadTrk', title='etOverPtLeadTrk log ('+nProng+'); etOverPtLeadTrk_log; Events',xbins=60,xmin=-3.,xmax=3.,opt='kAlwaysCreate')
    monGroup.defineHistogram('dRmax', title='max dR of associated tracks ('+nProng+'); dRmax; Events',xbins=50,xmin=-0.1,xmax=0.3,opt='kAlwaysCreate')
    monGroup.defineHistogram('absipSigLeadTrk', title='AbsIpSigLeadTrk ('+nProng+'); absipSigLeadTrk; Events',xbins=25,xmin=0.0,xmax=20.0,opt='kAlwaysCreate')
    monGroup.defineHistogram('sumPtTrkFrac', title='SumPtTrkFrac ('+nProng+'); SumPtTrkFrac; Events',xbins=50,xmin=-0.5,xmax=1.1,opt='kAlwaysCreate')
    monGroup.defineHistogram('emPOverTrkSysP', title='EMPOverTrkSysP log ('+nProng+'); EMPOverTrkSysP_log; Events',xbins=50,xmin=-5.,xmax=3.,opt='kAlwaysCreate')
    monGroup.defineHistogram('ptRatioEflowApprox', title='ptRatioEflowApprox ('+nProng+'); ptRatioEflowApprox; Events',xbins=50,xmin=0.0,xmax=2.0,opt='kAlwaysCreate')
    monGroup.defineHistogram('mEflowApprox', title='mEflowApprox log ('+nProng+'); mEflowApprox_log; Events',xbins=50,xmin=0.,xmax=5.,opt='kAlwaysCreate')
    monGroup.defineHistogram('ptDetectorAxis', title='ptDetectorAxis log ('+nProng+'); ptDetectorAxis_log; Events',xbins=50,xmin=0.,xmax=5.,opt='kAlwaysCreate')
    if nProng=='MP' or nProng=='3P':  
      monGroup.defineHistogram('massTrkSys', title='massTrkSys log ('+nProng+'); massTrkSys_log; Events',xbins=50,xmin=0.,xmax=3.,opt='kAlwaysCreate')
      monGroup.defineHistogram('trFlightPathSig', title='trFlightPathSig ('+nProng+'); trFlightPathSig; Events', xbins=100, xmin=-20., xmax=40,opt='kAlwaysCreate')

  def bookRNNTrack( self, monAlg, trigger, online ):

    monGroupName = trigger+'_RNN_'+('HLT' if online else 'Offline')+'_InputTrack'
    monGroupPath = 'RNN/InputTrack/'+trigger+('/HLT' if online else '/Offline')

    monGroup = self.helper.addGroup( monAlg, monGroupName,
                              self.basePath+'/'+monGroupPath )

    monGroup.defineHistogram('n_track', title='number of tracks; N tracks; Events', xbins=15, xmin=0, xmax=15, opt='kAlwaysCreate')
    monGroup.defineHistogram('track_pt_log',title='track_pt_log;track_pt_log;Events',xbins=20,xmin=2,xmax=7,opt='kAlwaysCreate')
    monGroup.defineHistogram('track_pt_jetseed_log',title='track_pt_jetseed_log;track_pt_jetseed_log;Events',xbins=50,xmin=2,xmax=7,opt='kAlwaysCreate')
    monGroup.defineHistogram('track_eta', title='track_eta; track_eta; Events', xbins=26,xmin=-2.6,xmax=2.6,opt='kAlwaysCreate')
    monGroup.defineHistogram('track_phi', title='track_phi; track_phi; Events', xbins=16,xmin=-3.2,xmax=3.2,opt='kAlwaysCreate')
    monGroup.defineHistogram('track_dEta',title='track_dEta;track_dEta;Events',xbins=100,xmin=-0.5,xmax=0.5,opt='kAlwaysCreate')
    monGroup.defineHistogram('track_dPhi',title='track_dPhi;track_dPhi;Events',xbins=100,xmin=-0.5,xmax=0.5,opt='kAlwaysCreate')
    monGroup.defineHistogram('track_d0_abs_log',title='track_d0_abs_log;track_d0_abs_log;Events',xbins=50,xmin=-7,xmax=2,opt='kAlwaysCreate')
    monGroup.defineHistogram('track_z0sinThetaTJVA_abs_log',title='track_z0sinThetaTJVA_abs_log;track_z0sinThetaTJVA_abs_log;Events',xbins=50,xmin=-10,xmax=4,opt='kAlwaysCreate')
    monGroup.defineHistogram('track_nIBLHitsAndExp',title='track_nIBLHitsAndExp; track_nIBLHitsAndExp;Events',xbins=3,xmin=0,xmax=3,opt='kAlwaysCreate')
    monGroup.defineHistogram('track_nPixelHitsPlusDeadSensors',title='track_nPixelHitsPlusDeadSensors;track_nPixelHitsPlusDeadSensors;Events',xbins=11,xmin=0,xmax=11,opt='kAlwaysCreate')
    monGroup.defineHistogram('track_nSCTHitsPlusDeadSensors',title='track_nSCTHitsPlusDeadSensors;track_nSCTHitsPlusDeadSensors;Events',xbins=20,xmin=0,xmax=20,opt='kAlwaysCreate')
    monGroup.defineHistogram('track_eta,track_phi', type='TH2F', title='Track Eta vs Phi; track eta; track phi', xbins=26,xmin=-2.6,xmax=2.6, ybins=16,ymin=-3.2,ymax=3.2,opt='kAlwaysCreate')
    monGroup.defineHistogram('track_dEta,track_dPhi', type='TH2F', title='Track dEta vs dPhi; track_dEta ; track_dPhi', xbins=100,xmin=-0.5,xmax=0.5,ybins=100,ymin=-0.5,ymax=0.5,opt='kAlwaysCreate')        

  def bookRNNCluster( self, monAlg, trigger, online ):

    monGroupName = trigger+'_RNN_'+('HLT' if online else 'Offline')+'_InputCluster'
    monGroupPath = 'RNN/InputCluster/'+trigger+('/HLT' if online else '/Offline')

    monGroup = self.helper.addGroup( monAlg, monGroupName,
                              self.basePath+'/'+monGroupPath )

    monGroup.defineHistogram('n_cluster', title='number of cluster; N cluster; Events', xbins=30, xmin=0, xmax=30, opt='kAlwaysCreate')
    monGroup.defineHistogram('cluster_et_log',title='cluster_et_log; cluster_et_log;Events',xbins=30,xmin=0,xmax=6,opt='kAlwaysCreate')
    monGroup.defineHistogram('cluster_pt_jetseed_log',title='cluster_pt_jetseed_log; cluster_pt_jetseed_log;Events',xbins=50,xmin=2,xmax=7,opt='kAlwaysCreate')
    monGroup.defineHistogram('cluster_eta', title='cluster_eta; cluster_eta; Events', xbins=26,xmin=-2.6,xmax=2.6,opt='kAlwaysCreate')
    monGroup.defineHistogram('cluster_phi', title='cluster_phi; cluster_phi; Events', xbins=16,xmin=-3.2,xmax=3.2,opt='kAlwaysCreate')
    monGroup.defineHistogram('cluster_dEta',title='cluster_dEta; cluster_dEta;Events',xbins=100,xmin=-0.5,xmax=0.5,opt='kAlwaysCreate')
    monGroup.defineHistogram('cluster_dPhi',title='cluster_dPhi; cluster_dPhi;Events',xbins=100,xmin=-0.5,xmax=0.5,opt='kAlwaysCreate')
    monGroup.defineHistogram('cluster_SECOND_R_log10',title='cluster_SECOND_R_log10; cluster_SECOND_R_log10;Events',xbins=50,xmin=-3,xmax=7,opt='kAlwaysCreate')
    monGroup.defineHistogram('cluster_SECOND_LAMBDA_log10',title='cluster_SECOND_LAMBDA_log10; cluster_SECOND_LAMBDA_log10;Events',xbins=50,xmin=-3,xmax=7,opt='kAlwaysCreate')
    monGroup.defineHistogram('cluster_CENTER_LAMBDA_log10',title='cluster_CENTER_LAMBDA_log10; cluster_CENTER_LAMBDA_log10;Events',xbins=50,xmin=-2,xmax=5,opt='kAlwaysCreate')
    monGroup.defineHistogram('cluster_eta,cluster_phi', type='TH2F', title='Cluster Eta vs Phi; cluster eta; cluster phi', xbins=26,xmin=-2.6,xmax=2.6, ybins=16,ymin=-3.2,ymax=3.2,opt='kAlwaysCreate')
    monGroup.defineHistogram('cluster_dEta,cluster_dPhi', type='TH2F', title='Cluster dEta vs dPhi; cluster_dEta ; cluster_dPhi', xbins=100,xmin=-0.5,xmax=0.5,ybins=100,ymin=-0.5,ymax=0.5,opt='kAlwaysCreate')

  def bookbasicVars( self, monAlg, trigger, nProng, online ):
  
    monGroupName = trigger+('HLT' if online else 'Offline')+'_basicVars_'+nProng
    monGroupPath = 'basicVars/'+trigger+('/HLT' if online else '/Offline')+'_'+nProng

    monGroup = self.helper.addGroup( monAlg, monGroupName,
                              self.basePath+'/'+monGroupPath )
 
    binning = [0,500]
    if 'tau0' in trigger: binning  = list(range(0,80,5)) + list(range(80,120,10))+list(range(120,160,20))+list(range(160,240,40))+list(range(240,420,60)) + [500]
    elif 'tau20' in trigger: binning  = list(range(15,80,5)) + list(range(80,120,10))+list(range(120,160,20))+list(range(160,240,40))+list(range(240,420,60)) + [500]
    elif 'tau25' in trigger: binning  = list(range(20,80,5)) + list(range(80,120,10))+list(range(120,160,20))+list(range(160,240,40))+list(range(240,420,60)) + [500]
    elif 'tau35' in trigger: binning  = list(range(30,80,5)) + list(range(80,120,10))+list(range(120,160,20))+list(range(160,240,40))+list(range(240,420,60)) + [500]
    elif 'tau60' in trigger: binning  = list(range(55,80,5)) + list(range(80,120,10))+list(range(120,160,20))+list(range(160,240,40))+list(range(240,420,60)) + [500]
    elif 'tau80' in trigger: binning  = [75,80] + list(range(80,120,10))+list(range(120,160,20))+list(range(160,240,40))+list(range(240,420,60)) + [500]
    elif 'tau160' in trigger: binning = [155,160] + list(range(160,240,40))+list(range(240,420,60)) + [500]
    elif 'tau200' in trigger: binning = [195,200,240] + list(range(240,420,60)) + [500]

    monGroup.defineHistogram('hEFEt', title='EF Et;E_{T}[GeV];Nevents',xbins=binning, opt='kAlwaysCreate')
    monGroup.defineHistogram('hEFEta', title='EF TrigCaloCluster Eta; #eta ; Nevents',xbins=26,xmin=-2.6,xmax=2.6,opt='kAlwaysCreate')
    monGroup.defineHistogram('hEFPhi', title='EF TrigCaloCluster Phi; #phi ; Nevents',xbins=16,xmin=-3.2,xmax=3.2,opt='kAlwaysCreate')
    monGroup.defineHistogram('hEFnTrack', title='EF number of tracks;number of tracks;Nevents',xbins=10,xmin=0,xmax=10,opt='kAlwaysCreate')

    monGroup.defineHistogram('hEFEta,hEFPhi', type='TH2F', title='Eta vs Phi; #eta ; #phi',
                               xbins=26,xmin=-2.6,xmax=2.6,ybins=16,ymin=-3.2,ymax=3.2,opt='kAlwaysCreate')
    monGroup.defineHistogram('hEFEt,hEFPhi', type='TH2F',  title='Et vs Phi; E_{T} [GeV]; #phi',
                               xbins=binning, ybins=16,ymin=-3.2,ymax=3.2,opt='kAlwaysCreate') 
    monGroup.defineHistogram('hEFEt,hEFEta', type='TH2F',  title='Et vs Eta; E_{T} [GeV]; #eta',
                               xbins=binning, ybins=26,ymin=-2.6,ymax=2.6,opt='kAlwaysCreate')
   
    monGroup.defineHistogram('hEFnWideTrack', title='EF number of wide tracks;number of tracks;Nevents',xbins=10,xmin=0,xmax=10,opt='kAlwaysCreate')

    monGroup.defineHistogram('hRNNScore', title='EF RNN score; RNN score;Nevents',xbins=20,xmin=0,xmax=1,opt='kAlwaysCreate')
    monGroup.defineHistogram('hRNNScoreSigTrans', title='EF RNN trans score; RNN Trans score;Nevents',xbins=20,xmin=0,xmax=1,opt='kAlwaysCreate')
    monGroup.defineHistogram('haverageMu', title='EF averageMu; averageMu; Nevents',xbins=20,xmin=0,xmax=80,opt='kAlwaysCreate')
    monGroup.defineHistogram('hTauVertexX', title='EF Tau Vertex X; Tau Vertex X; Nevents', xbins=100, xmin=-1, xmax=1, opt='kAlwaysCreate')
    monGroup.defineHistogram('hTauVertexY', title='EF Tau Vertex Y; Tau Vertex Y; Nevents', xbins=100, xmin=-2, xmax=0, opt='kAlwaysCreate')
    monGroup.defineHistogram('hTauVertexZ', title='EF Tau Vertex Z; Tau Vertex Z; Nevents', xbins=120, xmin=-120, xmax=120, opt='kAlwaysCreate')

  def bookDiTauVars(self, monAlg, trigger):
    
    monGroupName = trigger+"_DiTauVars"
    monGroupPath = 'DiTauVars/'+trigger

    monGroup = self.helper.addGroup( monAlg, monGroupName,
                              self.basePath+'/'+monGroupPath )    

    monGroup.defineHistogram('hleadEFEt,hsubleadEFEt', type='TH2F', title='lead Et vs sublead Et; lead E_{T} [GeV] ; sublead E_{T} [GeV]',
                               xbins=50,xmin=0,xmax=250,ybins=50,ymin=0,ymax=250,opt='kAlwaysCreate')
    monGroup.defineHistogram('hleadEFEta,hsubleadEFEta', type='TH2F', title='lead Eta vs sublead Eta; lead #eta ; sublead #eta',
                               xbins=26,xmin=-2.6,xmax=2.6,ybins=26,ymin=-2.6,ymax=2.6,opt='kAlwaysCreate')
    monGroup.defineHistogram('hleadEFPhi,hsubleadEFPhi', type='TH2F', title='lead Phi vs sublead Phi; lead #phi ; sublead #phi',
                               xbins=16,xmin=-3.2,xmax=3.2,ybins=16,ymin=-3.2,ymax=3.2,opt='kAlwaysCreate') 
    monGroup.defineHistogram('hdR', title='EF dR(#tau,#tau);dR(#tau,#tau);Nevents',xbins=40,xmin=0,xmax=4,opt='kAlwaysCreate')
    monGroup.defineHistogram('hdEta', title='EF dEta(#tau,#tau);dEta(#tau,#tau);Nevents',xbins=40,xmin=0,xmax=4,opt='kAlwaysCreate')
    monGroup.defineHistogram('hdPhi', title='EF dPhi(#tau,#tau);dPhi(#tau,#tau);Nevents',xbins=16,xmin=-3.2,xmax=3.2,opt='kAlwaysCreate')

    monGroup.defineHistogram('Pt', title='Pt DiTau; P_{t}; Nevents', xbins=50,xmin=0,xmax=250,opt='kAlwaysCreate')
    monGroup.defineHistogram('Eta', title='Eta(#tau,#tau);Eta(#tau,#tau);Nevents',xbins=26,xmin=-2.6,xmax=2.6,opt='kAlwaysCreate')
    monGroup.defineHistogram('Phi', title='Phi(#tau,#tau);Phi(#tau,#tau);Nevents',xbins=16,xmin=-3.2,xmax=3.2,opt='kAlwaysCreate')
    monGroup.defineHistogram('M', title='M(#tau,#tau);M_{#tau,#tau};Nevents',xbins=50,xmin=0,xmax=250,opt='kAlwaysCreate')
    monGroup.defineHistogram('dPt', title='dPt |leading-subleading|; P_{t}; Nevents', xbins=20,xmin=0,xmax=200,opt='kAlwaysCreate')  

  def bookTAndPVars(self, monAlg, trigger):

    monGroupName = trigger+"_TAndPVars"
    monGroupPath = 'TAndPVars/'+trigger

    monGroup = self.helper.addGroup( monAlg, monGroupName,
                              self.basePath+'/'+monGroupPath )

    monGroup.defineHistogram('hdR', title='EF dR(#tau,lep);dR(#tau,lep);Nevents',xbins=40,xmin=0,xmax=4,opt='kAlwaysCreate')
    monGroup.defineHistogram('hdEta', title='EF dEta(#tau,lep);dEta(#tau,lep);Nevents',xbins=40,xmin=0,xmax=4,opt='kAlwaysCreate')
    monGroup.defineHistogram('hdPhi', title='EF dPhi(#tau,lep);dPhi(#tau,lep);Nevents',xbins=16,xmin=-3.2,xmax=3.2,opt='kAlwaysCreate')

    monGroup.defineHistogram('Pt', title='Pt DiTau; P_{t}; Nevents', xbins=50,xmin=0,xmax=250,opt='kAlwaysCreate')
    monGroup.defineHistogram('Eta', title='Eta DiTau;Eta;Nevents',xbins=26,xmin=-2.6,xmax=2.6,opt='kAlwaysCreate')
    monGroup.defineHistogram('Phi', title='Phi DiTau;Phi;Nevents',xbins=16,xmin=-3.2,xmax=3.2,opt='kAlwaysCreate')
    monGroup.defineHistogram('M', title='M(#tau,lep) DiTau;M_{#tau,lep};Nevents',xbins=50,xmin=0,xmax=250,opt='kAlwaysCreate')
    monGroup.defineHistogram('dPt', title='dPt |lep-#tau|; P_{t}; Nevents', xbins=20,xmin=0,xmax=200,opt='kAlwaysCreate')

  def bookTruth( self, monAlg, trigger, nProng):

    monGroupName = trigger+'_EFVsTruth_'+nProng
    monGroupPath = 'EFVsTruth/'+trigger+'/EFVsTruth_'+nProng 
    monGroup = self.helper.addGroup( monAlg, monGroupName,
                              self.basePath+'/'+monGroupPath )

    monGroup.defineHistogram('pt_vis,Etratio',title='Etratiovis vs Pt_{vis}; Pt_{vis};(reco pt - truth vis pt)/truth vis pt ',type='TProfile',xbins=21,xmin=20,xmax=250)
    monGroup.defineHistogram('eta_vis,Etratio',title='Etratiovis vs #eta_{vis}; #eta_{vis}; (reco pt - truth vis pt)/truth vis pt ', type='TProfile', xbins=21,xmin=-3,xmax=3)
    monGroup.defineHistogram('phi_vis,Etratio',title='Etratiovis vs #phi_{vis}; #phi_{vis}; (reco pt - truth vis pt)/truth vis pt ', type='TProfile', xbins=21,xmin=-3,xmax=3)

    monGroup.defineHistogram('pt_vis', title='Pt_vis Value; P_{tvis}; Nevents', xbins=50,xmin=0,xmax=250)
    monGroup.defineHistogram('eta_vis', title='Eta_vis Value; #eta_{vis};Nevents', xbins=26,xmin=-2.6,xmax=2.6)
    monGroup.defineHistogram('phi_vis', title='Phi_vis Value; #phi_{vis}; Nevents', xbins=16,xmin=-3.2,xmax=3.2)

  def bookTruthEfficiency( self, monAlg, trigger, nProng):
  
    monGroupName = trigger+'_Truth_Efficiency_'+nProng
    monGroupPath = 'Truth_Efficiency/'+trigger+'/Truth_Efficiency_'+nProng
    monGroup = self.helper.addGroup( monAlg, monGroupName,
                              self.basePath+'/'+monGroupPath )
  
    def defineEachStepHistograms(xvariable, xlabel, xbins, xmin, xmax):

       monGroup.defineHistogram(monGroupName+'_HLTpass,'+monGroupName+'_'+xvariable+';EffHLT_'+xvariable+'_wrt_Truth',
                                title='HLT Efficiency ' +trigger+' ' +nProng+ ';'+xlabel+';Efficiency',
                                type='TEfficiency',xbins=xbins,xmin=xmin,xmax=xmax)

    defineEachStepHistograms('pt_vis', 'Pt_{vis} [GeV]', 60, 0.0, 300.)
    defineEachStepHistograms('eta_vis','#eta_{vis}', 13, -2.6, 2.6)
    defineEachStepHistograms('phi_vis','#phi_{vis}', 16, -3.2, 3.2)

