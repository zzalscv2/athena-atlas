# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Equivalent of configLumi_run410000_mc23a_MultiBeamspot.py, but
# without the division into different beam spots.

# We need to be able to adjust for different dataset sizes.
if not 'ScaleTaskLength' in dir():   ScaleTaskLength = 1
_evts = lambda x: int(ScaleTaskLength * x)

if not 'logging' in dir(): import logging
digilog = logging.getLogger('Digi_trf')
digilog.info('doing RunLumiOverride configuration from file.')
JobMaker=[
    {'run':410000, 'lb':1, 'starttstamp':1625000060, 'evts':_evts(1),  'mu':15.5},
    {'run':410000, 'lb':2, 'starttstamp':1625000120, 'evts':_evts(1),  'mu':16.5},
    {'run':410000, 'lb':3, 'starttstamp':1625000180, 'evts':_evts(2),  'mu':17.5},
    {'run':410000, 'lb':4, 'starttstamp':1625000240, 'evts':_evts(3),  'mu':18.5},
    {'run':410000, 'lb':5, 'starttstamp':1625000300, 'evts':_evts(5), 'mu':19.5},
    {'run':410000, 'lb':6, 'starttstamp':1625000360, 'evts':_evts(8), 'mu':20.5},
    {'run':410000, 'lb':7, 'starttstamp':1625000420, 'evts':_evts(11), 'mu':21.5},
    {'run':410000, 'lb':8, 'starttstamp':1625000480, 'evts':_evts(14), 'mu':22.5},
    {'run':410000, 'lb':9, 'starttstamp':1625000540, 'evts':_evts(18), 'mu':23.5},
    {'run':410000, 'lb':10, 'starttstamp':1625000600, 'evts':_evts(23), 'mu':24.5},
    {'run':410000, 'lb':11, 'starttstamp':1625000660, 'evts':_evts(29), 'mu':25.5},
    {'run':410000, 'lb':12, 'starttstamp':1625000720, 'evts':_evts(35), 'mu':26.5},
    {'run':410000, 'lb':13, 'starttstamp':1625000780, 'evts':_evts(41), 'mu':27.5},
    {'run':410000, 'lb':14, 'starttstamp':1625000840, 'evts':_evts(45), 'mu':28.5},
    {'run':410000, 'lb':15, 'starttstamp':1625000900, 'evts':_evts(49), 'mu':29.5},
    {'run':410000, 'lb':16, 'starttstamp':1625000960, 'evts':_evts(52), 'mu':30.5},
    {'run':410000, 'lb':17, 'starttstamp':1625001020, 'evts':_evts(54), 'mu':31.5},
    {'run':410000, 'lb':18, 'starttstamp':1625001080, 'evts':_evts(56), 'mu':32.5},
    {'run':410000, 'lb':19, 'starttstamp':1625001140, 'evts':_evts(58), 'mu':33.5},
    {'run':410000, 'lb':20, 'starttstamp':1625001200, 'evts':_evts(59), 'mu':34.5},
    {'run':410000, 'lb':21, 'starttstamp':1625001260, 'evts':_evts(59), 'mu':35.5},
    {'run':410000, 'lb':22, 'starttstamp':1625001320, 'evts':_evts(60), 'mu':36.5},
    {'run':410000, 'lb':23, 'starttstamp':1625001380, 'evts':_evts(59), 'mu':37.5},
    {'run':410000, 'lb':24, 'starttstamp':1625001440, 'evts':_evts(58), 'mu':38.5},
    {'run':410000, 'lb':25, 'starttstamp':1625001500, 'evts':_evts(59), 'mu':39.5},
    {'run':410000, 'lb':26, 'starttstamp':1625001560, 'evts':_evts(60), 'mu':40.5},
    {'run':410000, 'lb':27, 'starttstamp':1625001620, 'evts':_evts(62), 'mu':41.5},
    {'run':410000, 'lb':28, 'starttstamp':1625001680, 'evts':_evts(65), 'mu':42.5},
    {'run':410000, 'lb':29, 'starttstamp':1625001740, 'evts':_evts(70), 'mu':43.5},
    {'run':410000, 'lb':30, 'starttstamp':1625001800, 'evts':_evts(75), 'mu':44.5},
    {'run':410000, 'lb':31, 'starttstamp':1625001860, 'evts':_evts(83), 'mu':45.5},
    {'run':410000, 'lb':32, 'starttstamp':1625001920, 'evts':_evts(84), 'mu':46.5},
    {'run':410000, 'lb':33, 'starttstamp':1625001980, 'evts':_evts(86), 'mu':47.5},
    {'run':410000, 'lb':34, 'starttstamp':1625002040, 'evts':_evts(85), 'mu':48.5},
    {'run':410000, 'lb':35, 'starttstamp':1625002100, 'evts':_evts(81), 'mu':49.5},
    {'run':410000, 'lb':36, 'starttstamp':1625002160, 'evts':_evts(75), 'mu':50.5},
    {'run':410000, 'lb':37, 'starttstamp':1625002220, 'evts':_evts(68), 'mu':51.5},
    {'run':410000, 'lb':38, 'starttstamp':1625002280, 'evts':_evts(59), 'mu':52.5},
    {'run':410000, 'lb':39, 'starttstamp':1625002340, 'evts':_evts(50), 'mu':53.5},
    {'run':410000, 'lb':40, 'starttstamp':1625002400, 'evts':_evts(40), 'mu':54.5},
    {'run':410000, 'lb':41, 'starttstamp':1625002460, 'evts':_evts(31), 'mu':55.5},
    {'run':410000, 'lb':42, 'starttstamp':1625002520, 'evts':_evts(23), 'mu':56.5},
    {'run':410000, 'lb':43, 'starttstamp':1625002580, 'evts':_evts(16), 'mu':57.5},
    {'run':410000, 'lb':44, 'starttstamp':1625002640, 'evts':_evts(11), 'mu':58.5},
    {'run':410000, 'lb':45, 'starttstamp':1625002700, 'evts':_evts(7), 'mu':59.5},
    {'run':410000, 'lb':46, 'starttstamp':1625002760, 'evts':_evts(5), 'mu':60.5},
    {'run':410000, 'lb':47, 'starttstamp':1625002820, 'evts':_evts(3), 'mu':61.5},
    {'run':410000, 'lb':48, 'starttstamp':1625002880, 'evts':_evts(2), 'mu':62.5}
    #--> end hiding
]

include('RunDependentSimData/configCommon.py')

#cleanup python memory
if not "RunDMC_testing_configuration" in dir():
    del JobMaker
