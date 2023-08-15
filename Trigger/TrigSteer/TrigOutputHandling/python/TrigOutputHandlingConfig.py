# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool

def HLTResultMTMakerCfg(flags, name="HLTResultMTMaker"):

   m = CompFactory.HLTResultMTMaker(name)

   # ROBs/SubDets which are enabled but not necessarily part of the ROS-ROB map
   from libpyeformat_helper import SourceIdentifier,SubDetector
   subdets = [
      SubDetector.TDAQ_CALO_CLUSTER_PROC_ROI,
      SubDetector.TDAQ_CALO_JET_PROC_ROI,
      SubDetector.TDAQ_HLT,
      SubDetector.TDAQ_FTK,
      SubDetector.TDAQ_CALO_TOPO_PROC,
      SubDetector.TDAQ_CALO_DIGITAL_PROC,
      SubDetector.TDAQ_CALO_FEAT_EXTRACT_ROI,
   ]
   m.ExtraSubDets = []
   for subdetId in subdets:
      m.ExtraSubDets.append( int(subdetId) )

   def addROBs(dest,subdet,modules):
      for moduleId in modules:
         dest.append(SourceIdentifier(subdet,moduleId).code())

   m.ExtraROBs = []
   addROBs(m.ExtraROBs, SubDetector.TDAQ_CALO_CLUSTER_PROC_ROI, [0xa8, 0xa9, 0xaa, 0xab])
   addROBs(m.ExtraROBs, SubDetector.TDAQ_CALO_JET_PROC_ROI,     [0xac, 0xad])
   addROBs(m.ExtraROBs, SubDetector.TDAQ_MUON_CTP_INTERFACE,    [0x01])
   addROBs(m.ExtraROBs, SubDetector.TDAQ_CTP,                   [0x01])
   addROBs(m.ExtraROBs, SubDetector.TDAQ_CALO_TOPO_PROC,        [0x81, 0x91, 0x82, 0x92])

   # Configure HLT result monitoring histograms
   m.MonTool = GenericMonitoringTool(flags, 'MonTool', HistPath='HLTFramework/'+name)
   m.MonTool.defineHistogram('TIME_makeResult', path='EXPERT', type='TH1F', title='makeResult() call time;Time [ms];Events',
                             xbins=200, xmin=0, xmax=50 )
   m.MonTool.defineHistogram('TIME_makeResult_extRange', path='EXPERT', type='TH1F', title='makeResult() call time;Time [ms];Events',
                             xbins=200, xmin=0, xmax=50, opt='kCanRebin' )
   m.MonTool.defineHistogram('PEB_RemovedROBs_SubDet', path='EXPERT', type='TH1F',
                             title='SubDet ID of ROBs removed from PEB streams because disabled in DetMask;;Entries',
                             xbins=80, xmin=0, xmax=80)
   m.MonTool.defineHistogram('PEB_RemovedSubDets', path='EXPERT', type='TH1F',
                             title='SubDetectors removed from PEB streams because disabled in DetMask;;Entries',
                             xbins=80, xmin=0, xmax=80)

   return m

def TriggerEDMSerialiserToolCfg(flags, name="Serialiser"):

   from TrigEDMConfig.DataScoutingInfo import getFullHLTResultID

   TriggerEDMSerialiserTool = CompFactory.TriggerEDMSerialiserTool  
   # Configuration helper methods
   def addCollection(self, typeNameAux, moduleIds, allowTruncation=False):
      defs = [typeNameAux,
              ','.join([f"{id}" for id in moduleIds])]
      if allowTruncation:
         defs.append('allowTruncation')
      self.CollectionsToSerialize.append( ';'.join(defs) )

   def addCollectionToMainResult(self, typeNameAux, allowTruncation=False):
      self.addCollection(typeNameAux, [getFullHLTResultID()], allowTruncation)

   def addCollectionListToResults(self, typeNameAuxList, moduleIds, allowTruncation=False):
      for typeNameAux in typeNameAuxList:
         self.addCollection(typeNameAux, moduleIds, allowTruncation)

   def addCollectionListToMainResult(self, typeNameAuxList, allowTruncation=False):
      self.addCollectionListToResults(typeNameAuxList, [getFullHLTResultID()], allowTruncation)

   # Add the helper methods to the TriggerEDMSerialiserTool python class
   TriggerEDMSerialiserTool.addCollection = addCollection
   TriggerEDMSerialiserTool.addCollectionToMainResult = addCollectionToMainResult
   TriggerEDMSerialiserTool.addCollectionListToResults = addCollectionListToResults
   TriggerEDMSerialiserTool.addCollectionListToMainResult = addCollectionListToMainResult

   # Create and return a serialiser tool object
   serialiser = TriggerEDMSerialiserTool(name)

   from TrigEDMConfig.TriggerEDMRun3 import tpMap
   tpTool = CompFactory.TrigSerTPTool()
   tpTool.TPMap = tpMap()
   serialiser.TPTool = tpTool

   from TrigEDMConfig.DataScoutingInfo import TruncationThresholds as truncThresholds
   serialiser.TruncationThresholds = truncThresholds

   # Configure monitoring histograms
   serialiser.MonTool = GenericMonitoringTool(flags, 'MonTool', HistPath='HLTFramework/'+name)
   for prefix in ['', 'Allowed']:  # two sets of histogram for regular and allowed truncation
      serialiser.MonTool.defineHistogram(f'{prefix}Truncation_ModuleId', path='EXPERT', type='TH1F',
                                         title='Module IDs of truncated HLT results;Module ID;Num of truncated results',
                                         xbins=20, xmin=0, xmax=20)
      serialiser.MonTool.defineHistogram(f'{prefix}Truncation_TotalSize', path='EXPERT', type='TH1F',
                                         title='Total size of truncated HLT result;Size [kB];Num of truncated results',
                                         xbins=200, xmin=0, xmax=5000)
      serialiser.MonTool.defineHistogram(f'{prefix}Truncation_LargestName', path='EXPERT', type='TH1F',
                                         title='Name of the largest collection;;Num of truncated results',
                                         xbins=1, xmin=0, xmax=1)
      serialiser.MonTool.defineHistogram(f'{prefix}Truncation_LargestSize', path='EXPERT', type='TH1F',
                                         title='Size of the largest collection;Size [kB];Num of truncated results',
                                         xbins=200, xmin=0, xmax=5000)

   return serialiser

def StreamTagMakerToolCfg(name="StreamTagMakerTool"):

   stmaker = CompFactory.StreamTagMakerTool(name)
   # Extra configuration may come here

   return stmaker


def TriggerBitsMakerToolCfg(name="TriggerBitsMakerTool"):

   bitsmaker = CompFactory.TriggerBitsMakerTool(name)
   # Extra configuration may come here

   return bitsmaker

def DecisionSummaryMakerAlgCfg(flags, name="DecisionSummaryMakerAlg"):
   alg = CompFactory.DecisionSummaryMakerAlg(name)
   alg.MonTool = GenericMonitoringTool(flags, 'MonTool', HistPath='HLTFramework/'+name)
   alg.MonTool.defineHistogram('RoIsDEta', path='EXPERT', type='TH1F',
                               title='Change of RoI eta position between initial and final RoI;delta eta;N final RoIs',
                               xbins=51, xmin=-1.02, xmax=1.02)
   alg.MonTool.defineHistogram('RoIsDPhi', path='EXPERT', type='TH1F',
                               title='Change of RoI phi position between initial and final RoI;delta phi;N final RoIs',
                               xbins=51, xmin=-1.02, xmax=1.02)
   alg.MonTool.defineHistogram('RoIsDZed', path='EXPERT', type='TH1F',
                               title='Change of RoI z position between initial and final RoI;delta z;N final RoIs',
                               xbins=51, xmin=-204, xmax=204)
   return alg
