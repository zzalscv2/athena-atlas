# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Equivalent of configLumi_run450000_mc23c_MultiBeamspot.py, but
# without the division into different beam spots.

# We need to be able to adjust for different dataset sizes.
if not 'ScaleTaskLength' in dir():   ScaleTaskLength = 1
_evts = lambda x: int(ScaleTaskLength * x)

if not 'logging' in dir(): import logging
digilog = logging.getLogger('Digi_trf')
digilog.info('doing RunLumiOverride configuration from file.')
JobMaker=[
    {'run':450000, 'lb':1, 'starttstamp':1680000060, 'evts':_evts(1), 'mu':9.5},
    {'run':450000, 'lb':2, 'starttstamp':1680000120, 'evts':_evts(1), 'mu':10.5},
    {'run':450000, 'lb':3, 'starttstamp':1680000180, 'evts':_evts(1), 'mu':11.5},
    {'run':450000, 'lb':4, 'starttstamp':1680000240, 'evts':_evts(1), 'mu':12.5},
    {'run':450000, 'lb':5, 'starttstamp':1680000300, 'evts':_evts(1), 'mu':13.5},
    {'run':450000, 'lb':6, 'starttstamp':1680000360, 'evts':_evts(1), 'mu':14.5},
    {'run':450000, 'lb':7, 'starttstamp':1680000420, 'evts':_evts(1), 'mu':15.5},
    {'run':450000, 'lb':8, 'starttstamp':1680000480, 'evts':_evts(1), 'mu':16.5},
    {'run':450000, 'lb':9, 'starttstamp':1680000540, 'evts':_evts(2), 'mu':17.5},
    {'run':450000, 'lb':10, 'starttstamp':1680000600, 'evts':_evts(4), 'mu':18.5},
    {'run':450000, 'lb':11, 'starttstamp':1680000660, 'evts':_evts(7), 'mu':19.5},
    {'run':450000, 'lb':12, 'starttstamp':1680000720, 'evts':_evts(10), 'mu':20.5},
    {'run':450000, 'lb':13, 'starttstamp':1680000780, 'evts':_evts(13), 'mu':21.5},
    {'run':450000, 'lb':14, 'starttstamp':1680000840, 'evts':_evts(14), 'mu':22.5},
    {'run':450000, 'lb':15, 'starttstamp':1680000900, 'evts':_evts(15), 'mu':23.5},
    {'run':450000, 'lb':16, 'starttstamp':1680000960, 'evts':_evts(15), 'mu':24.5},
    {'run':450000, 'lb':17, 'starttstamp':1680001020, 'evts':_evts(15), 'mu':25.5},
    {'run':450000, 'lb':18, 'starttstamp':1680001080, 'evts':_evts(15), 'mu':26.5},
    {'run':450000, 'lb':19, 'starttstamp':1680001140, 'evts':_evts(15), 'mu':27.5},
    {'run':450000, 'lb':20, 'starttstamp':1680001200, 'evts':_evts(15), 'mu':28.5},
    {'run':450000, 'lb':21, 'starttstamp':1680001260, 'evts':_evts(15), 'mu':29.5},
    {'run':450000, 'lb':22, 'starttstamp':1680001320, 'evts':_evts(15), 'mu':30.5},
    {'run':450000, 'lb':23, 'starttstamp':1680001380, 'evts':_evts(15), 'mu':31.5},
    {'run':450000, 'lb':24, 'starttstamp':1680001440, 'evts':_evts(15), 'mu':32.5},
    {'run':450000, 'lb':25, 'starttstamp':1680001500, 'evts':_evts(15), 'mu':33.5},
    {'run':450000, 'lb':26, 'starttstamp':1680001560, 'evts':_evts(15), 'mu':34.5},
    {'run':450000, 'lb':27, 'starttstamp':1680001620, 'evts':_evts(15), 'mu':35.5},
    {'run':450000, 'lb':28, 'starttstamp':1680001680, 'evts':_evts(15), 'mu':36.5},
    {'run':450000, 'lb':29, 'starttstamp':1680001740, 'evts':_evts(15), 'mu':37.5},
    {'run':450000, 'lb':30, 'starttstamp':1680001800, 'evts':_evts(15), 'mu':38.5},
    {'run':450000, 'lb':31, 'starttstamp':1680001860, 'evts':_evts(15), 'mu':39.5},
    {'run':450000, 'lb':32, 'starttstamp':1680001920, 'evts':_evts(16), 'mu':40.5},
    {'run':450000, 'lb':33, 'starttstamp':1680001980, 'evts':_evts(16), 'mu':41.5},
    {'run':450000, 'lb':34, 'starttstamp':1680002040, 'evts':_evts(17), 'mu':42.5},
    {'run':450000, 'lb':35, 'starttstamp':1680002100, 'evts':_evts(18), 'mu':43.5},
    {'run':450000, 'lb':36, 'starttstamp':1680002160, 'evts':_evts(19), 'mu':44.5},
    {'run':450000, 'lb':37, 'starttstamp':1680002220, 'evts':_evts(19), 'mu':45.5},
    {'run':450000, 'lb':38, 'starttstamp':1680002280, 'evts':_evts(22), 'mu':46.5},
    {'run':450000, 'lb':39, 'starttstamp':1680002340, 'evts':_evts(23), 'mu':47.5},
    {'run':450000, 'lb':40, 'starttstamp':1680002400, 'evts':_evts(24), 'mu':48.5},
    {'run':450000, 'lb':41, 'starttstamp':1680002460, 'evts':_evts(25), 'mu':49.5},
    {'run':450000, 'lb':42, 'starttstamp':1680002520, 'evts':_evts(28), 'mu':50.5},
    {'run':450000, 'lb':43, 'starttstamp':1680002580, 'evts':_evts(30), 'mu':51.5},
    {'run':450000, 'lb':44, 'starttstamp':1680002640, 'evts':_evts(35), 'mu':52.5},
    {'run':450000, 'lb':45, 'starttstamp':1680002700, 'evts':_evts(42), 'mu':53.5},
    {'run':450000, 'lb':46, 'starttstamp':1680002760, 'evts':_evts(49), 'mu':54.5},
    {'run':450000, 'lb':47, 'starttstamp':1680002820, 'evts':_evts(58), 'mu':55.5},
    {'run':450000, 'lb':48, 'starttstamp':1680002880, 'evts':_evts(66), 'mu':56.5},
    {'run':450000, 'lb':49, 'starttstamp':1680002940, 'evts':_evts(77), 'mu':57.5},
    {'run':450000, 'lb':50, 'starttstamp':1680003000, 'evts':_evts(86), 'mu':58.5},
    {'run':450000, 'lb':51, 'starttstamp':1680003060, 'evts':_evts(87), 'mu':59.5},
    {'run':450000, 'lb':52, 'starttstamp':1680003120, 'evts':_evts(98), 'mu':60.5},
    {'run':450000, 'lb':53, 'starttstamp':1680003180, 'evts':_evts(97), 'mu':61.5},
    {'run':450000, 'lb':54, 'starttstamp':1680003240, 'evts':_evts(94), 'mu':62.5},
    {'run':450000, 'lb':55, 'starttstamp':1680003300, 'evts':_evts(95), 'mu':63.5},
    {'run':450000, 'lb':56, 'starttstamp':1680003360, 'evts':_evts(85), 'mu':64.5},
    {'run':450000, 'lb':57, 'starttstamp':1680003420, 'evts':_evts(78), 'mu':65.5},
    {'run':450000, 'lb':58, 'starttstamp':1680003480, 'evts':_evts(66), 'mu':66.5},
    {'run':450000, 'lb':59, 'starttstamp':1680003540, 'evts':_evts(56), 'mu':67.5},
    {'run':450000, 'lb':60, 'starttstamp':1680003600, 'evts':_evts(48), 'mu':68.5},
    {'run':450000, 'lb':61, 'starttstamp':1680003660, 'evts':_evts(41), 'mu':69.5},
    {'run':450000, 'lb':62, 'starttstamp':1680003720, 'evts':_evts(31), 'mu':70.5},
    {'run':450000, 'lb':63, 'starttstamp':1680003780, 'evts':_evts(25), 'mu':71.5},
    {'run':450000, 'lb':64, 'starttstamp':1680003840, 'evts':_evts(18), 'mu':72.5},
    {'run':450000, 'lb':65, 'starttstamp':1680003900, 'evts':_evts(13), 'mu':73.5},
    {'run':450000, 'lb':66, 'starttstamp':1680003960, 'evts':_evts(9), 'mu':74.5},
    {'run':450000, 'lb':67, 'starttstamp':1680004020, 'evts':_evts(9), 'mu':75.5},
    {'run':450000, 'lb':68, 'starttstamp':1680004080, 'evts':_evts(9), 'mu':76.5},
    {'run':450000, 'lb':69, 'starttstamp':1680004140, 'evts':_evts(7), 'mu':77.5},
    {'run':450000, 'lb':70, 'starttstamp':1680004200, 'evts':_evts(6), 'mu':78.5},
    {'run':450000, 'lb':71, 'starttstamp':1680004260, 'evts':_evts(7), 'mu':79.5},
    {'run':450000, 'lb':72, 'starttstamp':1680004320, 'evts':_evts(5), 'mu':80.5},
    {'run':450000, 'lb':73, 'starttstamp':1680004380, 'evts':_evts(6), 'mu':81.5},
    {'run':450000, 'lb':74, 'starttstamp':1680004440, 'evts':_evts(6), 'mu':82.5},
    {'run':450000, 'lb':75, 'starttstamp':1680004500, 'evts':_evts(5), 'mu':83.5},
    {'run':450000, 'lb':76, 'starttstamp':1680004560, 'evts':_evts(4), 'mu':84.5},
    {'run':450000, 'lb':77, 'starttstamp':1680004620, 'evts':_evts(4), 'mu':85.5},
    {'run':450000, 'lb':78, 'starttstamp':1680004680, 'evts':_evts(3), 'mu':86.5},
    {'run':450000, 'lb':79, 'starttstamp':1680004740, 'evts':_evts(2), 'mu':87.5},
    {'run':450000, 'lb':80, 'starttstamp':1680004800, 'evts':_evts(1), 'mu':88.5},
    {'run':450000, 'lb':81, 'starttstamp':1680004860, 'evts':_evts(1), 'mu':89.5},
    {'run':450000, 'lb':82, 'starttstamp':1680004920, 'evts':_evts(1), 'mu':90.5},
    #--> end hiding
]

include('RunDependentSimData/configCommon.py')

#cleanup python memory
if not "RunDMC_testing_configuration" in dir():
    del JobMaker
