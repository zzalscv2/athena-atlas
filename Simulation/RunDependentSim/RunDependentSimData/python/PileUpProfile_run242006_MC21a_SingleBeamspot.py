# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Equivalent of PileUpProfile_run242006_MC21a_SingleBeamspot.py, but
# with Phase-II upgrade run numbers.

def setupProfile(flags, scaleTaskLength=1):

  def _evts(x):
    return int(scaleTaskLength * x)

  return [
    {'run':242006, 'lb':1, 'starttstamp':1412006000, 'evts':_evts(1),  'mu':15.5},
    {'run':242006, 'lb':2, 'starttstamp':1412006010, 'evts':_evts(2),  'mu':16.5},
    {'run':242006, 'lb':3, 'starttstamp':1412006020, 'evts':_evts(5),  'mu':17.5},
    {'run':242006, 'lb':4, 'starttstamp':1412006030, 'evts':_evts(9),  'mu':18.5},
    {'run':242006, 'lb':5, 'starttstamp':1412006040, 'evts':_evts(13), 'mu':19.5},
    {'run':242006, 'lb':6, 'starttstamp':1412006050, 'evts':_evts(18), 'mu':20.5},
    {'run':242006, 'lb':7, 'starttstamp':1412006060, 'evts':_evts(22), 'mu':21.5},
    {'run':242006, 'lb':8, 'starttstamp':1412006070, 'evts':_evts(24), 'mu':22.5},
    {'run':242006, 'lb':9, 'starttstamp':1412006080, 'evts':_evts(28), 'mu':23.5},
    {'run':242006, 'lb':10, 'starttstamp':1412006090, 'evts':_evts(27), 'mu':24.5},
    {'run':242006, 'lb':11, 'starttstamp':1412006100, 'evts':_evts(31), 'mu':25.5},
    {'run':242006, 'lb':12, 'starttstamp':1412006110, 'evts':_evts(32), 'mu':26.5},
    {'run':242006, 'lb':13, 'starttstamp':1412006120, 'evts':_evts(33), 'mu':27.5},
    {'run':242006, 'lb':14, 'starttstamp':1412006130, 'evts':_evts(38), 'mu':28.5},
    {'run':242006, 'lb':15, 'starttstamp':1412006140, 'evts':_evts(36), 'mu':29.5},
    {'run':242006, 'lb':16, 'starttstamp':1412006150, 'evts':_evts(38), 'mu':30.5},
    {'run':242006, 'lb':17, 'starttstamp':1412006160, 'evts':_evts(42), 'mu':31.5},
    {'run':242006, 'lb':18, 'starttstamp':1412006170, 'evts':_evts(38), 'mu':32.5},
    {'run':242006, 'lb':19, 'starttstamp':1412006180, 'evts':_evts(45), 'mu':33.5},
    {'run':242006, 'lb':20, 'starttstamp':1412006190, 'evts':_evts(43), 'mu':34.5},
    {'run':242006, 'lb':21, 'starttstamp':1412006200, 'evts':_evts(39), 'mu':35.5},
    {'run':242006, 'lb':22, 'starttstamp':1412006210, 'evts':_evts(44), 'mu':36.5},
    {'run':242006, 'lb':23, 'starttstamp':1412006220, 'evts':_evts(40), 'mu':37.5},
    {'run':242006, 'lb':24, 'starttstamp':1412006230, 'evts':_evts(37), 'mu':38.5},
    {'run':242006, 'lb':25, 'starttstamp':1412006240, 'evts':_evts(38), 'mu':39.5},
    {'run':242006, 'lb':26, 'starttstamp':1412006250, 'evts':_evts(36), 'mu':40.5},
    {'run':242006, 'lb':27, 'starttstamp':1412006260, 'evts':_evts(36), 'mu':41.5},
    {'run':242006, 'lb':28, 'starttstamp':1412006270, 'evts':_evts(36), 'mu':42.5},
    {'run':242006, 'lb':29, 'starttstamp':1412006280, 'evts':_evts(38), 'mu':43.5},
    {'run':242006, 'lb':30, 'starttstamp':1412006290, 'evts':_evts(41), 'mu':44.5},
    {'run':242006, 'lb':31, 'starttstamp':1412006300, 'evts':_evts(53), 'mu':45.5},
    {'run':242006, 'lb':32, 'starttstamp':1412006310, 'evts':_evts(57), 'mu':46.5},
    {'run':242006, 'lb':33, 'starttstamp':1412006320, 'evts':_evts(64), 'mu':47.5},
    {'run':242006, 'lb':34, 'starttstamp':1412006330, 'evts':_evts(68), 'mu':48.5},
    {'run':242006, 'lb':35, 'starttstamp':1412006340, 'evts':_evts(74), 'mu':49.5},
    {'run':242006, 'lb':36, 'starttstamp':1412006350, 'evts':_evts(80), 'mu':50.5},
    {'run':242006, 'lb':37, 'starttstamp':1412006360, 'evts':_evts(86), 'mu':51.5},
    {'run':242006, 'lb':38, 'starttstamp':1412006370, 'evts':_evts(91), 'mu':52.5},
    {'run':242006, 'lb':39, 'starttstamp':1412006380, 'evts':_evts(81), 'mu':53.5},
    {'run':242006, 'lb':40, 'starttstamp':1412006390, 'evts':_evts(73), 'mu':54.5},
    {'run':242006, 'lb':41, 'starttstamp':1412006400, 'evts':_evts(67), 'mu':55.5},
    {'run':242006, 'lb':42, 'starttstamp':1412006410, 'evts':_evts(55), 'mu':56.5},
    {'run':242006, 'lb':43, 'starttstamp':1412006420, 'evts':_evts(46), 'mu':57.5},
    {'run':242006, 'lb':44, 'starttstamp':1412006430, 'evts':_evts(33), 'mu':58.5},
    {'run':242006, 'lb':45, 'starttstamp':1412006440, 'evts':_evts(31), 'mu':59.5},
    {'run':242006, 'lb':46, 'starttstamp':1412006450, 'evts':_evts(24), 'mu':60.5},
    {'run':242006, 'lb':47, 'starttstamp':1412006460, 'evts':_evts(16), 'mu':61.5},
    {'run':242006, 'lb':48, 'starttstamp':1412006470, 'evts':_evts(16), 'mu':62.5},
    {'run':242006, 'lb':49, 'starttstamp':1412006480, 'evts':_evts(13), 'mu':63.5},
    {'run':242006, 'lb':50, 'starttstamp':1412006490, 'evts':_evts(9),  'mu':64.5},
    {'run':242006, 'lb':51, 'starttstamp':1412006500, 'evts':_evts(9),  'mu':65.5},
    {'run':242006, 'lb':52, 'starttstamp':1412006510, 'evts':_evts(8),  'mu':66.5},
    {'run':242006, 'lb':53, 'starttstamp':1412006520, 'evts':_evts(5),  'mu':67.5},
    {'run':242006, 'lb':54, 'starttstamp':1412006530, 'evts':_evts(4),  'mu':68.5},
    {'run':242006, 'lb':55, 'starttstamp':1412006540, 'evts':_evts(4),  'mu':69.5},
    {'run':242006, 'lb':56, 'starttstamp':1412006550, 'evts':_evts(4),  'mu':70.5},
    {'run':242006, 'lb':57, 'starttstamp':1412006560, 'evts':_evts(3),  'mu':71.5},
    {'run':242006, 'lb':58, 'starttstamp':1412006570, 'evts':_evts(2),  'mu':72.5},
    {'run':242006, 'lb':59, 'starttstamp':1412006580, 'evts':_evts(2),  'mu':73.5},
    {'run':242006, 'lb':60, 'starttstamp':1412006590, 'evts':_evts(2),  'mu':74.5},
    {'run':242006, 'lb':61, 'starttstamp':1412006600, 'evts':_evts(1),  'mu':75.5},
    {'run':242006, 'lb':62, 'starttstamp':1412006610, 'evts':_evts(1),  'mu':76.5},
    {'run':242006, 'lb':63, 'starttstamp':1412006620, 'evts':_evts(1),  'mu':77.5},
    {'run':242006, 'lb':64, 'starttstamp':1412006630, 'evts':_evts(1),  'mu':78.5},
    {'run':242006, 'lb':65, 'starttstamp':1412006640, 'evts':_evts(1),  'mu':79.5},
    {'run':242006, 'lb':66, 'starttstamp':1412006650, 'evts':_evts(1),  'mu':80.5},
    {'run':242006, 'lb':67, 'starttstamp':1412006660, 'evts':_evts(1),  'mu':81.5},
    {'run':242006, 'lb':68, 'starttstamp':1412006670, 'evts':_evts(1),  'mu':82.5},
    {'run':242006, 'lb':69, 'starttstamp':1412006680, 'evts':_evts(1),  'mu':83.5},
    {'run':242006, 'lb':70, 'starttstamp':1412006690, 'evts':_evts(1),  'mu':84.5},
  ]
