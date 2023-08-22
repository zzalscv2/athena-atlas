# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# TriggerListsHelper: helper class which retrieves the full set of triggers needed for
# trigger matching in the DAODs and can then return them when needed. 

from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType
from PathResolver import PathResolver
import re

def read_trig_list_file(fname):
   """Read a text file containing a list of triggers
   Returns the list of triggers held in the file
   """
   triggers = []
   with open(PathResolver.FindCalibFile(fname)) as fp:
      for line in fp:
         line = line.strip()
         if line == "" or line.startswith("#"):
            continue
         triggers.append(line)
   return triggers


class TriggerListsHelper:
    def __init__(self, flags):
        self.flags = flags
        TriggerAPI.setConfigFlags(flags)
        self.Run2TriggerNamesTau = []
        self.Run2TriggerNamesNoTau = []
        self.Run3TriggerNames = []
        self.GetTriggerLists()

    def GetTriggerLists(self):
        #====================================================================
        # TRIGGER CONTENT
        #====================================================================
        ## See https://twiki.cern.ch/twiki/bin/view/Atlas/TriggerAPI
        ## Get single and multi mu, e, photon triggers
        ## Jet, tau, multi-object triggers not available in the matching code
        allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
        trig_el  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el,  livefraction=0.8)
        trig_mu  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu,  livefraction=0.8)
        trig_g   = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.g,   livefraction=0.8)
        trig_tau = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.tau, livefraction=0.8)
        ## Add cross-triggers for some sets
        trig_em = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.mu,  livefraction=0.8)
        trig_et = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.tau, livefraction=0.8)
        trig_mt = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu, additionalTriggerType=TriggerType.tau, livefraction=0.8)
        # Note that this seems to pick up both isolated and non-isolated triggers already, so no need for extra grabs
        trig_txe = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.tau, additionalTriggerType=TriggerType.xe, livefraction=0.8)

        extra_notau = read_trig_list_file("DerivationFrameworkPhys/run2ExtraMatchingTriggers.txt")
        extra_tau = read_trig_list_file("DerivationFrameworkPhys/run2ExtraMatchingTauTriggers.txt")

        ## Merge and remove duplicates
        trigger_names_full_notau = list(set(trig_el+trig_mu+trig_g+trig_em+trig_et+trig_mt+extra_notau))
        trigger_names_full_tau = list(set(trig_tau+trig_txe+extra_tau))
        #
        ## Now reduce the list...
        trigger_names_notau = []
        trigger_names_tau = []
        from AthenaConfiguration.AutoConfigFlags import GetFileMD

        if self.flags.Reco.EnableTrigger or self.flags.Trigger.triggerConfig == 'INFILE':

            if self.flags.Trigger.EDMVersion == 3:
                # These regular expressions should be replaced with a proper TriggerAPI for Run 3
                r_tau = re.compile("HLT_.*tau.*")
                r_notau = re.compile("HLT_[1-9]*(e|mu|g|j).*")
                for chain_name in GetFileMD(self.flags.Input.Files)['TriggerMenu']['HLTChains']:
                    result_tau = r_tau.match(chain_name)
                    result_notau = r_notau.match(chain_name)
                    if result_tau is not None: trigger_names_tau.append(chain_name)
                    if result_notau is not None: trigger_names_notau.append(chain_name)
                trigger_names_all = set.union(set(trigger_names_notau), set(trigger_names_tau))
                trigger_names_all = list(trigger_names_all)
                trigger_names_notau = set(trigger_names_notau) - set(trigger_names_tau)
                trigger_names_notau = list(trigger_names_notau)
                self.Run3TriggerNames = trigger_names_all
                self.Run3TriggerNamesNoTau = trigger_names_notau
                self.Run3TriggerNamesTau = trigger_names_tau
            else:
            # Note: ['TriggerMenu']['HLTChains'] python access is maintained for compatibility with Run 2 MC
            # POOL inputs (containing xAOD::TriggerMenu).
                for chain_name in GetFileMD(self.flags.Input.Files)['TriggerMenu']['HLTChains']:
                    if chain_name in trigger_names_full_notau: trigger_names_notau.append(chain_name)
                    if chain_name in trigger_names_full_tau:   trigger_names_tau.append(chain_name)
                self.Run2TriggerNamesNoTau = trigger_names_notau
                self.Run2TriggerNamesTau = trigger_names_tau

        return
