
from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAvtx_preinclude.py" ) 

from TrigInDetConfig.ConfigSettings import getInDetTrigConfig

getInDetTrigConfig("fullScan")._addSingleTrackVertices = True
getInDetTrigConfig("fullScan")._minNSiHits_vtx = 8
getInDetTrigConfig("fullScan")._TracksMaxZinterval = 3

log.info( "ID Trigger addSingleVertices:  "+str(getInDetTrigConfig("fullScan").addSingleTrackVertices) )
log.info( "ID Trigger minNSiHits:         "+str(getInDetTrigConfig("fullScan").minNSiHits_vtx) )
log.info( "ID Trigger TracksMaxZinterval: "+str(getInDetTrigConfig("fullScan").TracksMaxZinterval) )


