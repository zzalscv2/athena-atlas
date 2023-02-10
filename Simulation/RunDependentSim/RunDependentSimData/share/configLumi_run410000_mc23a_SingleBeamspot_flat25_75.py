# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# flat mu-profile with single beam spot simulation without bunch-to-bunch variations in luminosity.

# We need to be able to adjust for different dataset sizes.
if not 'ScaleTaskLength' in dir():   ScaleTaskLength = 1
_evts = lambda x: int(ScaleTaskLength * x)

if not 'logging' in dir(): import logging
digilog = logging.getLogger('Digi_trf')
digilog.info('doing RunLumiOverride configuration from file.')
JobMaker=[
    {'run':410000, 'lb':1, 'starttstamp':1625000060, 'evts':_evts(40), 'mu':25.5},
    {'run':410000, 'lb':2, 'starttstamp':1625000120, 'evts':_evts(40), 'mu':26.5},
    {'run':410000, 'lb':3, 'starttstamp':1625000180, 'evts':_evts(40), 'mu':27.5},
    {'run':410000, 'lb':4, 'starttstamp':1625000240, 'evts':_evts(40), 'mu':28.5},
    {'run':410000, 'lb':5, 'starttstamp':1625000300, 'evts':_evts(40), 'mu':29.5},
    {'run':410000, 'lb':6, 'starttstamp':1625000360, 'evts':_evts(40), 'mu':30.5},
    {'run':410000, 'lb':7, 'starttstamp':1625000420, 'evts':_evts(40), 'mu':31.5},
    {'run':410000, 'lb':8, 'starttstamp':1625000480, 'evts':_evts(40), 'mu':32.5},
    {'run':410000, 'lb':9, 'starttstamp':1625000540, 'evts':_evts(40), 'mu':33.5},
    {'run':410000, 'lb':10, 'starttstamp':1625000600, 'evts':_evts(40), 'mu':34.5},
    {'run':410000, 'lb':11, 'starttstamp':1625000660, 'evts':_evts(40), 'mu':35.5},
    {'run':410000, 'lb':12, 'starttstamp':1625000720, 'evts':_evts(40), 'mu':36.5},
    {'run':410000, 'lb':13, 'starttstamp':1625000780, 'evts':_evts(40), 'mu':37.5},
    {'run':410000, 'lb':14, 'starttstamp':1625000840, 'evts':_evts(40), 'mu':38.5},
    {'run':410000, 'lb':15, 'starttstamp':1625000900, 'evts':_evts(40), 'mu':39.5},
    {'run':410000, 'lb':16, 'starttstamp':1625000960, 'evts':_evts(40), 'mu':40.5},
    {'run':410000, 'lb':17, 'starttstamp':1625001020, 'evts':_evts(40), 'mu':41.5},
    {'run':410000, 'lb':18, 'starttstamp':1625001080, 'evts':_evts(40), 'mu':42.5},
    {'run':410000, 'lb':19, 'starttstamp':1625001140, 'evts':_evts(40), 'mu':43.5},
    {'run':410000, 'lb':20, 'starttstamp':1625001200, 'evts':_evts(40), 'mu':44.5},
    {'run':410000, 'lb':21, 'starttstamp':1625001260, 'evts':_evts(40), 'mu':45.5},
    {'run':410000, 'lb':22, 'starttstamp':1625001320, 'evts':_evts(40), 'mu':46.5},
    {'run':410000, 'lb':23, 'starttstamp':1625001380, 'evts':_evts(40), 'mu':47.5},
    {'run':410000, 'lb':24, 'starttstamp':1625001440, 'evts':_evts(40), 'mu':48.5},
    {'run':410000, 'lb':25, 'starttstamp':1625001500, 'evts':_evts(40), 'mu':49.5},
    {'run':410000, 'lb':26, 'starttstamp':1625001560, 'evts':_evts(40), 'mu':50.5},
    {'run':410000, 'lb':27, 'starttstamp':1625001620, 'evts':_evts(40), 'mu':51.5},
    {'run':410000, 'lb':28, 'starttstamp':1625001680, 'evts':_evts(40), 'mu':52.5},
    {'run':410000, 'lb':29, 'starttstamp':1625001740, 'evts':_evts(40), 'mu':53.5},
    {'run':410000, 'lb':30, 'starttstamp':1625001800, 'evts':_evts(40), 'mu':54.5},
    {'run':410000, 'lb':31, 'starttstamp':1625001860, 'evts':_evts(40), 'mu':55.5},
    {'run':410000, 'lb':32, 'starttstamp':1625001920, 'evts':_evts(40), 'mu':56.5},
    {'run':410000, 'lb':33, 'starttstamp':1625001980, 'evts':_evts(40), 'mu':57.5},
    {'run':410000, 'lb':34, 'starttstamp':1625002040, 'evts':_evts(40), 'mu':58.5},
    {'run':410000, 'lb':35, 'starttstamp':1625002100, 'evts':_evts(40), 'mu':59.5},
    {'run':410000, 'lb':36, 'starttstamp':1625002160, 'evts':_evts(40), 'mu':60.5},
    {'run':410000, 'lb':37, 'starttstamp':1625002220, 'evts':_evts(40), 'mu':61.5},
    {'run':410000, 'lb':38, 'starttstamp':1625002280, 'evts':_evts(40), 'mu':62.5},
    {'run':410000, 'lb':39, 'starttstamp':1625002340, 'evts':_evts(40), 'mu':63.5},
    {'run':410000, 'lb':40, 'starttstamp':1625002400, 'evts':_evts(40), 'mu':64.5},
    {'run':410000, 'lb':41, 'starttstamp':1625002460, 'evts':_evts(40), 'mu':65.5},
    {'run':410000, 'lb':42, 'starttstamp':1625002520, 'evts':_evts(40), 'mu':66.5},
    {'run':410000, 'lb':43, 'starttstamp':1625002580, 'evts':_evts(40), 'mu':67.5},
    {'run':410000, 'lb':44, 'starttstamp':1625002640, 'evts':_evts(40), 'mu':68.5},
    {'run':410000, 'lb':45, 'starttstamp':1625002700, 'evts':_evts(40), 'mu':69.5},
    {'run':410000, 'lb':46, 'starttstamp':1625002760, 'evts':_evts(40), 'mu':70.5},
    {'run':410000, 'lb':47, 'starttstamp':1625002820, 'evts':_evts(40), 'mu':71.5},
    {'run':410000, 'lb':48, 'starttstamp':1625002880, 'evts':_evts(40), 'mu':72.5},
    {'run':410000, 'lb':49, 'starttstamp':1625002940, 'evts':_evts(40), 'mu':73.5},
    {'run':410000, 'lb':50, 'starttstamp':1625003000, 'evts':_evts(40), 'mu':74.5},
    #--> end hiding
]

include('RunDependentSimData/configCommon.py')

#cleanup python memory
if not "RunDMC_testing_configuration" in dir():
    del JobMaker
