# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

def setupProfile(ScaleTaskLength=1):

  def _evts(x):
    return int(ScaleTaskLength * x)

  return [
    {'run':310000, 'lb':1, 'starttstamp':1550000000, 'dt':0.000, 'evts':_evts(1), 'mu':0.500, 'force_new':False},
    {'run':310000, 'lb':2, 'starttstamp':1550000060, 'dt':0.000, 'evts':_evts(1), 'mu':1.500, 'force_new':False},
    {'run':310000, 'lb':3, 'starttstamp':1550000120, 'dt':0.000, 'evts':_evts(1), 'mu':2.500, 'force_new':False},
    {'run':310000, 'lb':4, 'starttstamp':1550000180, 'dt':0.000, 'evts':_evts(1), 'mu':3.500, 'force_new':False},
    {'run':310000, 'lb':5, 'starttstamp':1550000240, 'dt':0.000, 'evts':_evts(1), 'mu':4.500, 'force_new':False},
    {'run':310000, 'lb':6, 'starttstamp':1550000300, 'dt':0.000, 'evts':_evts(1), 'mu':5.500, 'force_new':False},
    {'run':310000, 'lb':7, 'starttstamp':1550000360, 'dt':0.000, 'evts':_evts(1), 'mu':6.500, 'force_new':False},
    {'run':310000, 'lb':8, 'starttstamp':1550000420, 'dt':0.000, 'evts':_evts(1), 'mu':7.500, 'force_new':False},
    {'run':310000, 'lb':9, 'starttstamp':1550000480, 'dt':0.000, 'evts':_evts(1), 'mu':8.500, 'force_new':False},
    {'run':310000, 'lb':10, 'starttstamp':1550000540, 'dt':0.000, 'evts':_evts(1), 'mu':9.500, 'force_new':False},
    {'run':310000, 'lb':11, 'starttstamp':1550000600, 'dt':0.000, 'evts':_evts(2), 'mu':10.500, 'force_new':False},
    {'run':310000, 'lb':12, 'starttstamp':1550000660, 'dt':0.000, 'evts':_evts(3), 'mu':11.500, 'force_new':False},
    {'run':310000, 'lb':13, 'starttstamp':1550000720, 'dt':0.000, 'evts':_evts(5), 'mu':12.500, 'force_new':False},
    {'run':310000, 'lb':14, 'starttstamp':1550000780, 'dt':0.000, 'evts':_evts(7), 'mu':13.500, 'force_new':False},
    {'run':310000, 'lb':15, 'starttstamp':1550000840, 'dt':0.000, 'evts':_evts(9), 'mu':14.500, 'force_new':False},
    {'run':310000, 'lb':16, 'starttstamp':1550000900, 'dt':0.000, 'evts':_evts(12), 'mu':15.500, 'force_new':False},
    {'run':310000, 'lb':17, 'starttstamp':1550000960, 'dt':0.000, 'evts':_evts(15), 'mu':16.500, 'force_new':False},
    {'run':310000, 'lb':18, 'starttstamp':1550001020, 'dt':0.000, 'evts':_evts(18), 'mu':17.500, 'force_new':False},
    {'run':310000, 'lb':19, 'starttstamp':1550001080, 'dt':0.000, 'evts':_evts(23), 'mu':18.500, 'force_new':False},
    {'run':310000, 'lb':20, 'starttstamp':1550001140, 'dt':0.000, 'evts':_evts(28), 'mu':19.500, 'force_new':False},
    {'run':310000, 'lb':21, 'starttstamp':1550001200, 'dt':0.000, 'evts':_evts(33), 'mu':20.500, 'force_new':False},
    {'run':310000, 'lb':22, 'starttstamp':1550001260, 'dt':0.000, 'evts':_evts(39), 'mu':21.500, 'force_new':False},
    {'run':310000, 'lb':23, 'starttstamp':1550001320, 'dt':0.000, 'evts':_evts(43), 'mu':22.500, 'force_new':False},
    {'run':310000, 'lb':24, 'starttstamp':1550001380, 'dt':0.000, 'evts':_evts(48), 'mu':23.500, 'force_new':False},
    {'run':310000, 'lb':25, 'starttstamp':1550001440, 'dt':0.000, 'evts':_evts(51), 'mu':24.500, 'force_new':False},
    {'run':310000, 'lb':26, 'starttstamp':1550001500, 'dt':0.000, 'evts':_evts(54), 'mu':25.500, 'force_new':False},
    {'run':310000, 'lb':27, 'starttstamp':1550001560, 'dt':0.000, 'evts':_evts(57), 'mu':26.500, 'force_new':False},
    {'run':310000, 'lb':28, 'starttstamp':1550001620, 'dt':0.000, 'evts':_evts(59), 'mu':27.500, 'force_new':False},
    {'run':310000, 'lb':29, 'starttstamp':1550001680, 'dt':0.000, 'evts':_evts(60), 'mu':28.500, 'force_new':False},
    {'run':310000, 'lb':30, 'starttstamp':1550001740, 'dt':0.000, 'evts':_evts(62), 'mu':29.500, 'force_new':False},
    {'run':310000, 'lb':31, 'starttstamp':1550001800, 'dt':0.000, 'evts':_evts(63), 'mu':30.500, 'force_new':False},
    {'run':310000, 'lb':32, 'starttstamp':1550001860, 'dt':0.000, 'evts':_evts(64), 'mu':31.500, 'force_new':False},
    {'run':310000, 'lb':33, 'starttstamp':1550001920, 'dt':0.000, 'evts':_evts(64), 'mu':32.500, 'force_new':False},
    {'run':310000, 'lb':34, 'starttstamp':1550001980, 'dt':0.000, 'evts':_evts(65), 'mu':33.500, 'force_new':False},
    {'run':310000, 'lb':35, 'starttstamp':1550002040, 'dt':0.000, 'evts':_evts(65), 'mu':34.500, 'force_new':False},
    {'run':310000, 'lb':36, 'starttstamp':1550002100, 'dt':0.000, 'evts':_evts(66), 'mu':35.500, 'force_new':False},
    {'run':310000, 'lb':37, 'starttstamp':1550002160, 'dt':0.000, 'evts':_evts(66), 'mu':36.500, 'force_new':False},
    {'run':310000, 'lb':38, 'starttstamp':1550002220, 'dt':0.000, 'evts':_evts(66), 'mu':37.500, 'force_new':False},
    {'run':310000, 'lb':39, 'starttstamp':1550002280, 'dt':0.000, 'evts':_evts(66), 'mu':38.500, 'force_new':False},
    {'run':310000, 'lb':40, 'starttstamp':1550002340, 'dt':0.000, 'evts':_evts(66), 'mu':39.500, 'force_new':False},
    {'run':310000, 'lb':41, 'starttstamp':1550002400, 'dt':0.000, 'evts':_evts(65), 'mu':40.500, 'force_new':False},
    {'run':310000, 'lb':42, 'starttstamp':1550002460, 'dt':0.000, 'evts':_evts(63), 'mu':41.500, 'force_new':False},
    {'run':310000, 'lb':43, 'starttstamp':1550002520, 'dt':0.000, 'evts':_evts(61), 'mu':42.500, 'force_new':False},
    {'run':310000, 'lb':44, 'starttstamp':1550002580, 'dt':0.000, 'evts':_evts(59), 'mu':43.500, 'force_new':False},
    {'run':310000, 'lb':45, 'starttstamp':1550002640, 'dt':0.000, 'evts':_evts(56), 'mu':44.500, 'force_new':False},
    {'run':310000, 'lb':46, 'starttstamp':1550002700, 'dt':0.000, 'evts':_evts(53), 'mu':45.500, 'force_new':False},
    {'run':310000, 'lb':47, 'starttstamp':1550002760, 'dt':0.000, 'evts':_evts(48), 'mu':46.500, 'force_new':False},
    {'run':310000, 'lb':48, 'starttstamp':1550002820, 'dt':0.000, 'evts':_evts(44), 'mu':47.500, 'force_new':False},
    {'run':310000, 'lb':49, 'starttstamp':1550002880, 'dt':0.000, 'evts':_evts(40), 'mu':48.500, 'force_new':False},
    {'run':310000, 'lb':50, 'starttstamp':1550002940, 'dt':0.000, 'evts':_evts(35), 'mu':49.500, 'force_new':False},
    {'run':310000, 'lb':51, 'starttstamp':1550003000, 'dt':0.000, 'evts':_evts(31), 'mu':50.500, 'force_new':False},
    {'run':310000, 'lb':52, 'starttstamp':1550003060, 'dt':0.000, 'evts':_evts(26), 'mu':51.500, 'force_new':False},
    {'run':310000, 'lb':53, 'starttstamp':1550003120, 'dt':0.000, 'evts':_evts(22), 'mu':52.500, 'force_new':False},
    {'run':310000, 'lb':54, 'starttstamp':1550003180, 'dt':0.000, 'evts':_evts(18), 'mu':53.500, 'force_new':False},
    {'run':310000, 'lb':55, 'starttstamp':1550003240, 'dt':0.000, 'evts':_evts(15), 'mu':54.500, 'force_new':False},
    {'run':310000, 'lb':56, 'starttstamp':1550003300, 'dt':0.000, 'evts':_evts(12), 'mu':55.500, 'force_new':False},
    {'run':310000, 'lb':57, 'starttstamp':1550003360, 'dt':0.000, 'evts':_evts(10), 'mu':56.500, 'force_new':False},
    {'run':310000, 'lb':58, 'starttstamp':1550003420, 'dt':0.000, 'evts':_evts(8), 'mu':57.500, 'force_new':False},
    {'run':310000, 'lb':59, 'starttstamp':1550003480, 'dt':0.000, 'evts':_evts(6), 'mu':58.500, 'force_new':False},
    {'run':310000, 'lb':60, 'starttstamp':1550003540, 'dt':0.000, 'evts':_evts(4), 'mu':59.500, 'force_new':False},
    {'run':310000, 'lb':61, 'starttstamp':1550003600, 'dt':0.000, 'evts':_evts(3), 'mu':60.500, 'force_new':False},
    {'run':310000, 'lb':62, 'starttstamp':1550003660, 'dt':0.000, 'evts':_evts(2), 'mu':61.500, 'force_new':False},
    {'run':310000, 'lb':63, 'starttstamp':1550003720, 'dt':0.000, 'evts':_evts(2), 'mu':62.500, 'force_new':False},
    {'run':310000, 'lb':64, 'starttstamp':1550003780, 'dt':0.000, 'evts':_evts(1), 'mu':63.500, 'force_new':False},
    {'run':310000, 'lb':65, 'starttstamp':1550003840, 'dt':0.000, 'evts':_evts(1), 'mu':64.500, 'force_new':False},
    {'run':310000, 'lb':66, 'starttstamp':1550003900, 'dt':0.000, 'evts':_evts(1), 'mu':65.500, 'force_new':False},
    {'run':310000, 'lb':67, 'starttstamp':1550003960, 'dt':0.000, 'evts':_evts(1), 'mu':66.500, 'force_new':False},
    {'run':310000, 'lb':68, 'starttstamp':1550004020, 'dt':0.000, 'evts':_evts(1), 'mu':67.500, 'force_new':False},
    {'run':310000, 'lb':69, 'starttstamp':1550004080, 'dt':0.000, 'evts':_evts(1), 'mu':68.500, 'force_new':False},
    {'run':310000, 'lb':70, 'starttstamp':1550004140, 'dt':0.000, 'evts':_evts(1), 'mu':69.500, 'force_new':False},
    {'run':310000, 'lb':71, 'starttstamp':1550004200, 'dt':0.000, 'evts':_evts(1), 'mu':70.500, 'force_new':False},
    {'run':310000, 'lb':72, 'starttstamp':1550004260, 'dt':0.000, 'evts':_evts(1), 'mu':71.500, 'force_new':False},
    {'run':310000, 'lb':73, 'starttstamp':1550004320, 'dt':0.000, 'evts':_evts(1), 'mu':72.500, 'force_new':False},
    {'run':310000, 'lb':74, 'starttstamp':1550004380, 'dt':0.000, 'evts':_evts(1), 'mu':73.500, 'force_new':False},
    {'run':310000, 'lb':75, 'starttstamp':1550004440, 'dt':0.000, 'evts':_evts(1), 'mu':74.500, 'force_new':False},
    {'run':310000, 'lb':76, 'starttstamp':1550004500, 'dt':0.000, 'evts':_evts(1), 'mu':75.500, 'force_new':False},
    {'run':310000, 'lb':77, 'starttstamp':1550004560, 'dt':0.000, 'evts':_evts(1), 'mu':76.500, 'force_new':False},
    {'run':310000, 'lb':78, 'starttstamp':1550004620, 'dt':0.000, 'evts':_evts(1), 'mu':77.500, 'force_new':False},
    {'run':310000, 'lb':79, 'starttstamp':1550004680, 'dt':0.000, 'evts':_evts(1), 'mu':78.500, 'force_new':False},
    {'run':310000, 'lb':80, 'starttstamp':1550004740, 'dt':0.000, 'evts':_evts(1), 'mu':79.500, 'force_new':False},
    {'run':310000, 'lb':81, 'starttstamp':1550004800, 'dt':0.000, 'evts':_evts(1), 'mu':80.500, 'force_new':False},
    {'run':310000, 'lb':82, 'starttstamp':1550004860, 'dt':0.000, 'evts':_evts(1), 'mu':81.500, 'force_new':False},
    {'run':310000, 'lb':83, 'starttstamp':1550004920, 'dt':0.000, 'evts':_evts(1), 'mu':82.500, 'force_new':False},
    {'run':310000, 'lb':84, 'starttstamp':1550004980, 'dt':0.000, 'evts':_evts(1), 'mu':83.500, 'force_new':False},
    {'run':310000, 'lb':85, 'starttstamp':1550005040, 'dt':0.000, 'evts':_evts(1), 'mu':84.500, 'force_new':False},
    {'run':310000, 'lb':86, 'starttstamp':1550005100, 'dt':0.000, 'evts':_evts(1), 'mu':85.500, 'force_new':False},
    {'run':310000, 'lb':87, 'starttstamp':1550005160, 'dt':0.000, 'evts':_evts(1), 'mu':86.500, 'force_new':False},
    {'run':310000, 'lb':88, 'starttstamp':1550005220, 'dt':0.000, 'evts':_evts(1), 'mu':87.500, 'force_new':False},
    {'run':310000, 'lb':89, 'starttstamp':1550005280, 'dt':0.000, 'evts':_evts(1), 'mu':88.500, 'force_new':False},
    {'run':310000, 'lb':90, 'starttstamp':1550005340, 'dt':0.000, 'evts':_evts(1), 'mu':89.500, 'force_new':False},
    {'run':310000, 'lb':91, 'starttstamp':1550005400, 'dt':0.000, 'evts':_evts(1), 'mu':90.500, 'force_new':False},
]