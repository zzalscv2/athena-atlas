# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

def setupProfile(flags, scaleTaskLength=1):

  def _evts(x):
    return int(scaleTaskLength * x)

  return [
    {'run':310000, 'lb':1, 'starttstamp':1550000000, 'dt':0.000, 'evts':_evts(1), 'mu':20.500, 'force_new':False, 'step': 0},
    {'run':310000, 'lb':2, 'starttstamp':1550000060, 'dt':0.000, 'evts':_evts(1), 'mu':21.500, 'force_new':False, 'step': 0},
    {'run':310000, 'lb':3, 'starttstamp':1550000120, 'dt':0.000, 'evts':_evts(1), 'mu':22.500, 'force_new':False, 'step': 0},
    {'run':310000, 'lb':4, 'starttstamp':1550000180, 'dt':0.000, 'evts':_evts(1), 'mu':10.500, 'force_new':False, 'step': 1},
    {'run':310000, 'lb':5, 'starttstamp':1550000240, 'dt':0.000, 'evts':_evts(1), 'mu':11.500, 'force_new':False, 'step': 1},
    {'run':310000, 'lb':6, 'starttstamp':1550000300, 'dt':0.000, 'evts':_evts(1), 'mu':1.500, 'force_new':False, 'step': 2},
    {'run':310000, 'lb':7, 'starttstamp':1550000360, 'dt':0.000, 'evts':_evts(1), 'mu':2.500, 'force_new':False, 'step': 2},
    {'run':310000, 'lb':8, 'starttstamp':1550000420, 'dt':0.000, 'evts':_evts(1), 'mu':3.500, 'force_new':False, 'step': 2},
    {'run':310000, 'lb':9, 'starttstamp':1550000480, 'dt':0.000, 'evts':_evts(1), 'mu':4.500, 'force_new':False, 'step': 2},
    {'run':310000, 'lb':10, 'starttstamp':1550000540, 'dt':0.000, 'evts':_evts(1), 'mu':5.500, 'force_new':False, 'step': 2},
  ]
