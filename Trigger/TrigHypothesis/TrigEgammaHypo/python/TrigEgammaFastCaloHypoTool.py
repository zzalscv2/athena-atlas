# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.SystemOfUnits import GeV
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def same( val , tool):
  return [val]*( len( tool.EtaBins ) - 1 )


#
# For electrons
#
def electronRingerFastCaloHypoConfig(flags, name, sequenceOut):
  # make the Hypo
  from AthenaConfiguration.ComponentFactory import CompFactory
  theFastCaloHypo = CompFactory.TrigEgammaFastCaloHypoAlg(name)
  theFastCaloHypo.CaloClusters = sequenceOut
  theFastCaloHypo.PidNames = ["tight", "medium", "loose", "vloose"]
  theFastCaloHypo.RingerNNSelectorTools = createTrigEgammaFastCaloElectronSelectors(flags)

  monTool = GenericMonitoringTool(flags, "MonTool_"+name, HistPath = 'FastCaloL2EgammaHypo/'+name)
  monTool.defineHistogram('TIME_exec', type='TH1F', path='EXPERT', title="Fast Calo Hypo Algtime; time [ us ] ; Nruns", xbins=80, xmin=0.0, xmax=8000.0)
  monTool.defineHistogram('TIME_NN_exec', type='TH1F', path='EXPERT', title="Fast Calo Hypo NN Algtime; time [ us ] ; Nruns", xbins=50, xmin=0.0, xmax=50)

  theFastCaloHypo.MonTool=monTool
  return theFastCaloHypo

#
# For photons
#
def photonRingerFastCaloHypoConfig(flags, name, sequenceOut):
    # make the Hypo
  from AthenaConfiguration.ComponentFactory import CompFactory
  theFastCaloHypo = CompFactory.TrigEgammaFastCaloHypoAlg(name)
  theFastCaloHypo.CaloClusters = sequenceOut
  theFastCaloHypo.PidNames = ["tight", "medium", "loose"]
  theFastCaloHypo.RingerNNSelectorTools = createTrigEgammaFastCaloPhotonSelectors(flags)

  monTool = GenericMonitoringTool(flags, "MonTool_"+name, HistPath = 'FastCaloL2EgammaHypo/'+name)
  monTool.defineHistogram('TIME_exec', type='TH1F', path='EXPERT', title="Fast Calo Hypo Algtime; time [ us ] ; Nruns", xbins=80, xmin=0.0, xmax=8000.0)
  monTool.defineHistogram('TIME_NN_exec', type='TH1F', path='EXPERT', title="Fast Calo Hypo NN Algtime; time [ us ] ; Nruns", xbins=50, xmin=0.0, xmax=50)

  theFastCaloHypo.MonTool=monTool
  return theFastCaloHypo



def createTrigEgammaFastCaloHypoAlg(flags, name, sequenceOut):
  if 'Electron' in name:
    return electronRingerFastCaloHypoConfig(flags, name, sequenceOut)
  elif 'Photon' in name:
    return photonRingerFastCaloHypoConfig(flags, name, sequenceOut)


def TrigEgammaFastCaloHypoAlgCfg(flags, name, CaloClusters):
  from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
  acc = ComponentAccumulator()
  acc.addEventAlgo(createTrigEgammaFastCaloHypoAlg(flags, name=name, sequenceOut=CaloClusters))
  return acc

#
# For photons only
# NOTE: For future, ringer will be applied at the fast photon step
#
def createTrigEgammaFastCaloHypoAlg_noringer(flags, name, sequenceOut):
  
  # make the Hypo
  from AthenaConfiguration.ComponentFactory import CompFactory
  theFastCaloHypo = CompFactory.TrigEgammaFastCaloHypoAlg(name)
  theFastCaloHypo.CaloClusters = sequenceOut

  # Just for electrons
  theFastCaloHypo.PidNames = []
  theFastCaloHypo.RingerNNSelectorTools = []


  monTool = GenericMonitoringTool(flags, "MonTool_"+name,
                                  HistPath = 'FastCaloL2EgammaHypo/'+name)
  monTool.defineHistogram('TIME_exec', type='TH1F', path='EXPERT', title="Fast Calo Hypo Algtime; time [ us ] ; Nruns", xbins=80, xmin=0.0, xmax=8000.0)
  monTool.defineHistogram('TIME_NN_exec', type='TH1F', path='EXPERT', title="Fast Calo Hypo NN Algtime; time [ us ] ; Nruns", xbins=20, xmin=0.0, xmax=1000.0)

  theFastCaloHypo.MonTool=monTool
  return theFastCaloHypo



def treatPidName(pidname):
  if 'tight' in pidname:
    return 'tight'
  elif 'medium' in pidname:
    return 'medium'
  # this should be before loose to works
  elif 'vloose' in pidname:
    return 'vloose'
  else:
    return 'loose'

#
# For electron and photons
#
class TrigEgammaFastCaloHypoToolConfig:

  __operation_points  = [  'tight'    , 
                           'medium'   , 
                           'loose'    , 
                           'vloose'   , 
                           'lhtight'  , 
                           'lhmedium' , 
                           'lhloose'  , 
                           'lhvloose' ,
                           'dnntight' ,
                           'dnnmedium',
                           'dnnloose' ,
                           'dnnvloose',
                           ]


  def __init__(self, name, monGroups, cpart, tool=None):

    from AthenaCommon.Logging import logging
    self.__log = logging.getLogger('TrigEgammaFastCaloHypoTool')
    self.__name = name
    self.__cand = cpart['trigType']
    self.__threshold  = float(cpart['threshold'])
    self.__sel        = 'ion' if 'ion' in cpart['extra'] else (cpart['addInfo'][0] if cpart['addInfo'] else cpart['IDinfo'])
    self.__gsfinfo  = cpart['gsfInfo'] if cpart['trigType']=='e' and cpart['gsfInfo'] else ''
    self.__idperfinfo  = cpart['idperfInfo'] if cpart['trigType']=='e' and cpart['idperfInfo'] else ''
    # self.__noringerinfo = cpart['L2IDAlg'] if cpart['trigType']=='e' else ''
    self.__noringerinfo = cpart['L2IDAlg']
    self.__monGroups = monGroups

    if not tool:
      from AthenaConfiguration.ComponentFactory import CompFactory
      tool = CompFactory.TrigEgammaFastCaloHypoTool( name )
    
    tool.AcceptAll      = False
    tool.UseRinger      = False
    tool.EtaBins        = [0.0, 0.6, 0.8, 1.15, 1.37, 1.52, 1.81, 2.01, 2.37, 2.47]
    tool.ETthr          = same( self.__threshold*GeV, tool )
    tool.dETACLUSTERthr = 0.1
    tool.dPHICLUSTERthr = 0.1
    tool.F1thr          = same( 0.005 , tool)
    tool.ET2thr         = same( 90.0*GeV, tool )
    tool.HADET2thr      = same( 999.0  , tool)
    tool.HADETthr       = same( 0.058  , tool)
    tool.WETA2thr       = same( 99999. , tool)
    tool.WSTOTthr       = same( 99999. , tool)
    tool.F3thr          = same( 99999. , tool)
    tool.CARCOREthr     = same( -9999. , tool)
    tool.CAERATIOthr    = same( -9999. , tool)
    tool.PidName        = ""

    self.__tool = tool

    self.__log.debug( 'Chain     :%s'   , self.__name )
    self.__log.debug( 'Signature :%s'   , self.__cand )
    self.__log.debug( 'Threshold :%s'   , self.__threshold )
    self.__log.debug( 'Pidname   :%s'   , self.__sel )
    self.__log.debug( 'noringerinfo :%s', self.__noringerinfo )


  def chain(self):
    return self.__name
  
  def pidname( self ):
    return self.__sel

  def etthr(self):
    return self.__threshold

  def isElectron(self):
    return 'e' in self.__cand

  def isPhoton(self):
    return 'g' in self.__cand

  def noringerinfo(self):
    return self.__noringerinfo

  def gsfinfo(self):
    return self.__gsfinfo

  def idperfinfo(self):
    return self.__idperfinfo

  def tool(self):
    return self.__tool
  

  def nocut(self):
    
    self.__log.debug( 'Configure nocut' )
    self.tool().AcceptAll      = True
    self.tool().UseRinger      = False
    self.tool().ETthr          = same( self.etthr()*GeV , self.tool())
    self.tool().dETACLUSTERthr = 9999.
    self.tool().dPHICLUSTERthr = 9999.
    self.tool().F1thr          = same( 0.0    , self.tool())
    self.tool().HADETthr       = same( 9999.  , self.tool())
    self.tool().CARCOREthr     = same( -9999. , self.tool())
    self.tool().CAERATIOthr    = same( -9999. , self.tool())


  def etcut(self):

    self.__log.debug( 'Configure etcut or nopid' )
    self.tool().UseRinger      = False
    self.tool().ETthr          = same( ( self.etthr()  -  3 )*GeV, self.tool() )
    self.tool().dETACLUSTERthr = 9999.
    self.tool().dPHICLUSTERthr = 9999.
    self.tool().F1thr          = same( 0.0    ,self.tool())
    self.tool().HADETthr       = same( 9999.  ,self.tool())
    self.tool().CARCOREthr     = same( -9999. ,self.tool())
    self.tool().CAERATIOthr    = same( -9999. ,self.tool())


  def noringer(self):

    self.__log.debug( 'Configure noringer' )
    from TrigEgammaHypo.TrigEgammaFastCutDefs import TrigFastCaloElectronCutMaps
    self.tool().UseRinger   = False
    self.tool().ETthr       = same( ( self.etthr()  - 3 )*GeV , self.tool())
    self.tool().HADETthr    = TrigFastCaloElectronCutMaps( self.etthr() ).MapsHADETthr[self.pidname()]
    self.tool().CARCOREthr  = TrigFastCaloElectronCutMaps( self.etthr() ).MapsCARCOREthr[self.pidname()]
    self.tool().CAERATIOthr = TrigFastCaloElectronCutMaps( self.etthr() ).MapsCAERATIOthr[self.pidname()]


  def nominal(self):

    self.__log.debug( 'Configure ringer' )
    self.tool().UseRinger = True
    self.tool().EtCut     = (self.etthr()-3.)*GeV  
    if not self.pidname() in self.__operation_points:
      self.__log.fatal("Bad selection name: %s" % self.pidname())
    self.tool().PidName = treatPidName(self.pidname())


  #
  # compile the chain
  #
  def compile(self, flags):

    if self.pidname() in ('etcut', 'ion', 'nopid'):
      self.etcut()

    elif self.pidname() in self.__operation_points and 'noringer' in self.noringerinfo() and self.isElectron():
      self.noringer()

    elif self.pidname() in self.__operation_points and 'noringer' not in self.noringerinfo() and self.isElectron():
      self.nominal()

    elif self.pidname() in self.__operation_points and self.isPhoton() and  'ringer'!=self.noringerinfo():
      self.etcut()
    elif self.pidname() in self.__operation_points and self.isPhoton() and  'ringer'==self.noringerinfo():
      self.nominal()
   
    elif self.etthr()==0:
      self.nocut()

    if hasattr(self.tool(), "MonTool"):
      
      doValidationMonitoring = flags.Trigger.doValidationMonitoring # True to monitor all chains for validation purposes
      monGroups = self.__monGroups

      if (any('egammaMon:online' in group for group in monGroups) or doValidationMonitoring):
        self.addMonitoring(flags)


  #
  # Add monitoring tool
  #
  def addMonitoring(self, flags):

    if self.tool().UseRinger:
      monTool = GenericMonitoringTool(flags, 'MonTool'+self.__name)
      monTool.defineHistogram('Eta', type='TH1F', path='EXPERT',title="#eta of Clusters; #eta; number of RoIs", xbins=50,xmin=-2.5,xmax=2.5)
      monTool.defineHistogram('Phi',type='TH1F', path='EXPERT',title="#phi of Clusters; #phi; number of RoIs", xbins=64,xmin=-3.2,xmax=3.2)
      monTool.defineHistogram('Et',type='TH1F', path='EXPERT',title="E_{T} of Clusters; E_{T} [MeV]; number of RoIs", xbins=60,xmin=0,xmax=5e4)
      monTool.defineHistogram('NNOutput',type='TH1F', path='EXPERT',title="NN Output; NN; Count", xbins=17,xmin=-8,xmax=+8)

      monTool.HistPath= 'FastCaloL2EgammaHypo/'+self.__name
      self.tool().MonTool=monTool

    else:

      monTool = GenericMonitoringTool(flags, "MonTool_"+self.__name,
                                      HistPath = 'FastCaloL2EgammaHypo/'+self.__name)
      monTool.defineHistogram('dEta', type='TH1F', path='EXPERT', title="L2Calo Hypo #Delta#eta_{L2 L1}; #Delta#eta_{L2 L1}",
                              xbins=80, xmin=-0.01, xmax=0.01)
      monTool.defineHistogram('dPhi', type='TH1F', path='EXPERT', title="L2Calo Hypo #Delta#phi_{L2 L1}; #Delta#phi_{L2 L1}",
                              xbins=80, xmin=-0.01, xmax=0.01)
      monTool.defineHistogram('Et_em', type='TH1F', path='EXPERT', title="L2Calo Hypo cluster E_{T}^{EM};E_{T}^{EM} [MeV]",
                              xbins=50, xmin=-2000, xmax=100000)
      monTool.defineHistogram('Eta', type='TH1F', path='EXPERT', title="L2Calo Hypo entries per Eta;Eta", xbins=100, xmin=-2.5, xmax=2.5)
      monTool.defineHistogram('Phi', type='TH1F', path='EXPERT', title="L2Calo Hypo entries per Phi;Phi", xbins=128, xmin=-3.2, xmax=3.2)

      cuts=['Input','has one TrigEMCluster', '#Delta #eta L2-L1', '#Delta #phi L2-L1','eta','rCore',
            'eRatio','E_{T}^{EM}', 'E_{T}^{Had}','f_{1}','Weta2','Wstot','F3']

      monTool.defineHistogram('CutCounter', type='TH1I', path='EXPERT', title="L2Calo Hypo Passed Cuts;Cut",
                              xbins=13, xmin=-1.5, xmax=12.5,  opt="kCumulative", xlabels=cuts)

      if flags.Trigger.doValidationMonitoring:
          monTool.defineHistogram('Et_had', type='TH1F', path='EXPERT', title="L2Calo Hypo E_{T}^{had} in first layer;E_{T}^{had} [MeV]",
              xbins=50, xmin=-2000, xmax=100000)
          monTool.defineHistogram('RCore', type='TH1F', path='EXPERT', title="L2Calo Hypo R_{core};E^{3x7}/E^{7x7} in sampling 2",
              xbins=48, xmin=-0.1, xmax=1.1)
          monTool.defineHistogram('Eratio', type='TH1F', path='EXPERT',
              title="L2Calo Hypo E_{ratio};E^{max1}-E^{max2}/E^{max1}+E^{max2} in sampling 1 (excl.crack)",
              xbins=64, xmin=-0.1, xmax=1.5)
          monTool.defineHistogram('EtaBin', type='TH1I', path='EXPERT', title="L2Calo Hypo entries per Eta bin;Eta bin no.",
              xbins=11, xmin=-0.5, xmax=10.5)
          monTool.defineHistogram('F1', type='TH1F', path='EXPERT', title="L2Calo Hypo f_{1};f_{1}", xbins=34, xmin=-0.5, xmax=1.2)
          monTool.defineHistogram('Weta2', type='TH1F', path='EXPERT', title="L2Calo Hypo Weta2; E Width in sampling 2",
              xbins=96, xmin=-0.1, xmax=0.61)
          monTool.defineHistogram('Wstot', type='TH1F', path='EXPERT', title="L2Calo Hypo Wstot; E Width in sampling 1",
              xbins=48, xmin=-0.1, xmax=11.)
          monTool.defineHistogram('F3', type='TH1F', path='EXPERT', title="L2Calo Hypo F3; E3/(E0+E1+E2+E3)",
              xbins=96, xmin=-0.1, xmax=1.1)


      self.tool().MonTool = monTool



def _IncTool(flags, name, monGroups, cpart, tool=None):
  config = TrigEgammaFastCaloHypoToolConfig(name, monGroups, cpart, tool=tool )
  config.compile(flags)
  return config.tool()


def TrigEgammaFastCaloHypoToolFromDict(flags, chainDict , tool=None):
    """ Use menu decoded chain dictionary to configure the tool """
    cparts = [i for i in chainDict['chainParts'] if ((i['signature']=='Electron') or (i['signature']=='Photon'))]
    return _IncTool( flags, chainDict['chainName'], chainDict['monGroups'], cparts[0], tool=tool)



def createTrigEgammaFastCaloElectronSelectors(flags, ConfigFilePath=None):

    import collections.abc

    if not ConfigFilePath:
      ConfigFilePath = flags.Trigger.egamma.ringerVersion

  
    SelectorNames = collections.OrderedDict({
            'tight'    : 'AsgElectronFastCaloRingerTightSelectorTool',
            'medium'   : 'AsgElectronFastCaloRingerMediumSelectorTool',
            'loose'    : 'AsgElectronFastCaloRingerLooseSelectorTool',
            'vloose'   : 'AsgElectronFastCaloRingerVeryLooseSelectorTool',
            })
        

    ToolConfigFile = collections.OrderedDict({
          'tight'   :['ElectronRingerTightTriggerConfig.conf'    ],
          'medium'  :['ElectronRingerMediumTriggerConfig.conf'   ],
          'loose'   :['ElectronRingerLooseTriggerConfig.conf'    ],
          'vloose'  :['ElectronRingerVeryLooseTriggerConfig.conf'],
          })
    
    selectors = []    
    #from RingerSelectorTools.RingerSelectorToolsConf import Ringer__AsgRingerSelectorTool
    from AthenaConfiguration.ComponentFactory import CompFactory

    for pidname , name in SelectorNames.items():
      SelectorTool=CompFactory.Ringer.AsgRingerSelectorTool(name)
      SelectorTool.ConfigFiles = [ (ConfigFilePath+'/'+path) for path in ToolConfigFile[pidname] ]
      selectors.append(SelectorTool)
    return selectors



def createTrigEgammaFastCaloPhotonSelectors(flags, ConfigFilePath=None):

    import collections.abc

    if not ConfigFilePath:
      ConfigFilePath = flags.Trigger.egamma.ringerVersion

  
    SelectorNames = collections.OrderedDict({
            'tight'    : 'AsgPhotonFastCaloRingerTightSelectorTool',
            'medium'   : 'AsgPhotonFastCaloRingerMediumSelectorTool',
            'loose'    : 'AsgPhotonFastCaloRingerLooseSelectorTool',
            })
        

    ToolConfigFile = collections.OrderedDict({
          'tight'   :['PhotonRingerTightTriggerConfig.conf'    ],
          'medium'  :['PhotonRingerMediumTriggerConfig.conf'   ],
          'loose'   :['PhotonRingerLooseTriggerConfig.conf'    ],
          })
    
    selectors = []    
    #from RingerSelectorTools.RingerSelectorToolsConf import Ringer__AsgRingerSelectorTool
    from AthenaConfiguration.ComponentFactory import CompFactory

    for pidname , name in SelectorNames.items():
      SelectorTool=CompFactory.Ringer.AsgRingerSelectorTool(name)
      SelectorTool.UseTansigOutput = True # FIXME: Should be removed in the next round
      SelectorTool.ConfigFiles = [ (ConfigFilePath+'/'+path) for path in ToolConfigFile[pidname] ]
      selectors.append(SelectorTool)
    return selectors
