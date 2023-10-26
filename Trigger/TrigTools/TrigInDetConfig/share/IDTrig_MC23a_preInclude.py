from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
from Campaigns.Utils import Campaign
# do not let use this preInclude with campaigns other than MC23a
if flags.Input.MCCampaign is not Campaign.MC23a:
    raise Exception( "MC23a ID Trigger configuration is called when processing {}. Please remove this preInclude from the config/AMI tag".format(flags.Input.MCCampaign))

from TrigInDetConfig.ConfigSettings import getInDetTrigConfig; 
from AthenaCommon.SystemOfUnits import GeV; 
getInDetTrigConfig('tauCore')._pTmin = 1*GeV; 
getInDetTrigConfig('tauIso')._pTmin = 1*GeV; 
getInDetTrigConfig('jetSuper')._zedHalfWidth = 150.;
getInDetTrigConfig('bmumux')._zedHalfWidth = 225.;
getInDetTrigConfig('bmumux')._SuperRoI = False;

flags.Trigger.InDetTracking.RoiZedWidthDefault=0.0
flags.Trigger.InDetTracking.bjet.Xi2max=9.
