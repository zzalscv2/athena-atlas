from TrigInDetConfig.ConfigSettings import getInDetTrigConfig; 
from AthenaCommon.SystemOfUnits import GeV; 
getInDetTrigConfig('tauCore')._pTmin = 1*GeV; 
getInDetTrigConfig('tauIso')._pTmin = 1*GeV; 
getInDetTrigConfig('tauIso')._Xi2max = 9; 
getInDetTrigConfig('bjet')._Xi2max = 9;
