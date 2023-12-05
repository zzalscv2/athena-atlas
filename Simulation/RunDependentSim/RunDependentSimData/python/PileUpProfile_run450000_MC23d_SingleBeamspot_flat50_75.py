# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Equivalent of PileUpProfile_run450000_MC23d_MultiBeamspot.py_flat50_75, but
# without the division into different beam spots.

def setupProfile(flags, scaleTaskLength=1):

  def _evts(x):
    return int(scaleTaskLength * x)

  return [
    {'run':450000, 'lb':1, 'starttstamp':1625000060, 'evts':_evts(80), 'mu':50.5},
    {'run':450000, 'lb':2, 'starttstamp':1625000120, 'evts':_evts(80), 'mu':51.5},
    {'run':450000, 'lb':3, 'starttstamp':1625000180, 'evts':_evts(80), 'mu':52.5},
    {'run':450000, 'lb':4, 'starttstamp':1625000240, 'evts':_evts(80), 'mu':53.5},
    {'run':450000, 'lb':5, 'starttstamp':1625000300, 'evts':_evts(80), 'mu':54.5},
    {'run':450000, 'lb':6, 'starttstamp':1625000360, 'evts':_evts(80), 'mu':55.5},
    {'run':450000, 'lb':7, 'starttstamp':1625000420, 'evts':_evts(80), 'mu':56.5},
    {'run':450000, 'lb':8, 'starttstamp':1625000480, 'evts':_evts(80), 'mu':57.5},
    {'run':450000, 'lb':9, 'starttstamp':1625000540, 'evts':_evts(80), 'mu':58.5},
    {'run':450000, 'lb':10, 'starttstamp':1625000600, 'evts':_evts(80), 'mu':59.5},
    {'run':450000, 'lb':11, 'starttstamp':1625000660, 'evts':_evts(80), 'mu':60.5},
    {'run':450000, 'lb':12, 'starttstamp':1625000720, 'evts':_evts(80), 'mu':61.5},
    {'run':450000, 'lb':13, 'starttstamp':1625000780, 'evts':_evts(80), 'mu':62.5},
    {'run':450000, 'lb':14, 'starttstamp':1625000840, 'evts':_evts(80), 'mu':63.5},
    {'run':450000, 'lb':15, 'starttstamp':1625000900, 'evts':_evts(80), 'mu':64.5},
    {'run':450000, 'lb':16, 'starttstamp':1625000960, 'evts':_evts(80), 'mu':65.5},
    {'run':450000, 'lb':17, 'starttstamp':1625001020, 'evts':_evts(80), 'mu':66.5},
    {'run':450000, 'lb':18, 'starttstamp':1625001080, 'evts':_evts(80), 'mu':67.5},
    {'run':450000, 'lb':19, 'starttstamp':1625001140, 'evts':_evts(80), 'mu':68.5},
    {'run':450000, 'lb':20, 'starttstamp':1625001200, 'evts':_evts(80), 'mu':69.5},
    {'run':450000, 'lb':21, 'starttstamp':1625001260, 'evts':_evts(80), 'mu':70.5},
    {'run':450000, 'lb':22, 'starttstamp':1625001320, 'evts':_evts(80), 'mu':71.5},
    {'run':450000, 'lb':23, 'starttstamp':1625001380, 'evts':_evts(80), 'mu':72.5},
    {'run':450000, 'lb':24, 'starttstamp':1625001440, 'evts':_evts(80), 'mu':73.5},
    {'run':450000, 'lb':25, 'starttstamp':1625001500, 'evts':_evts(80), 'mu':74.5},
  ]
