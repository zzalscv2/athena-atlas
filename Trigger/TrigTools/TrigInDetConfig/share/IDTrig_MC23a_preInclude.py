from TrigInDetConfig.ConfigSettings import getInDetTrigConfig; 
from AthenaCommon.SystemOfUnits import GeV; 
getInDetTrigConfig('tauCore')._pTmin = 1*GeV; 
getInDetTrigConfig('tauIso')._pTmin = 1*GeV; 
getInDetTrigConfig('jetSuper')._zedHalfWidth = 150.;
getInDetTrigConfig('bphysics')._zedHalfWidth = 225.;
getInDetTrigConfig('bphysics')._SuperRoI = False;

from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
flags.Trigger.InDetTracking.RoiZedWidthDefault=0.0
flags.Trigger.InDetTracking.bjet.Xi2max=9.
