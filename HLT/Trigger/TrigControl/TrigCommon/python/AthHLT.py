# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# Utilities used in athenaHLT.py
#
from AthenaCommon.Logging import logging
log = logging.getLogger('athenaHLT')

from functools import cache

class CondDB:
   _run2 = 236108
   def __init__(self, run):
      self.run = run
   def db_instance(self):
      if self.run>=self._run2:
         return 'CONDBR2'
      else:
         return 'COMP200'
   def sor_folder(self):
      if self.run>=self._run2:
         return '/TDAQ/RunCtrl/SOR'
      else:
         return '/TDAQ/RunCtrl/SOR_Params'

@cache
def get_sor_params(run_number):
   from CoolConvUtilities import AtlCoolLib

   log.info('Reading SOR record for run %s from COOL', run_number)

   cdb = CondDB(run_number)
   dbcon = AtlCoolLib.readOpen('COOLONL_TDAQ/%s' % cdb.db_instance())
   folder = dbcon.getFolder(cdb.sor_folder())

   # need to keep sor variable while using payload (cannot do the following in
   # one single line nor overwrite sor). Otherwise: 1) GC comes into play;
   # 2) the object is deleted; 3) since it's a shared_ptr, the internal
   # cool::IObject also gets deleted; 4) payload is not valid any longer
   try:
      sor = folder.findObject(run_number << 32, 0)
   except Exception:
      return None        # This can happen for unknown run numbers

   payload = sor.payload()
   d = {k: payload[k] for k in payload}
   return d


@cache
def get_trigconf_keys(run_number, lb_number):
   """Read Trigger keys from COOL"""

   from TrigConfStorage.TriggerCoolUtil import TriggerCoolUtil

   cdb = CondDB(run_number)
   db = TriggerCoolUtil.GetConnection(cdb.db_instance())
   run_range = [[run_number,run_number]]
   d = {}
   d['SMK'] = TriggerCoolUtil.getHLTConfigKeys(db, run_range)[run_number]['SMK']

   def findKey(keys):
      for (key, firstLB, lastLB) in keys:
         if lb_number>=firstLB and lb_number<=lastLB:
            return key
      return None

   # Find L1/HLT prescale key
   d['LVL1PSK'] = findKey(TriggerCoolUtil.getL1ConfigKeys(db, run_range)[run_number]['LVL1PSK'])
   d['HLTPSK'] = findKey(TriggerCoolUtil.getHLTPrescaleKeys(db, run_range)[run_number]['HLTPSK2'])

   return d

#
# Testing (used as ctest)
#
if __name__=='__main__':
   # Can be used as script, e.g.: python -m TrigCommon.AthHLT 327265
   import sys
   if len(sys.argv)>1:
      log.info('SOR parameters: %s', get_sor_params(int(sys.argv[1])))
      sys.exit(0)

   # Unit testing case:
   d = get_sor_params(327265)  # Run-2
   print(d)
   assert(d['DetectorMask']=='0000000000000000c10069fffffffff7')

   d = get_sor_params(216416)  # Run-1
   print(d)
   assert(d['DetectorMask']==281474976710647)

   # Config keys
   d = get_trigconf_keys(360026, 1)
   print(d)
   assert(d['SMK']==2749)
   assert(d['LVL1PSK']==15186)
   assert(d['HLTPSK']==17719)

   d = get_trigconf_keys(360026, 100)
   print(d)
   assert(d['SMK']==2749)
   assert(d['LVL1PSK']==23504)
   assert(d['HLTPSK']==17792)
