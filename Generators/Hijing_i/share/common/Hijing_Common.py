from Hijing_i.Hijing_iConf import Hijing
genSeq += Hijing()

## evgenConfig setup -- use reduced number of events for HI
evgenConfig.generators += ["Hijing"]

## Extra stream persistency
evgenConfig.doNotSaveItems += ["McEventCollection#*"]
evgenConfig.extraSaveItems += ["McEventCollection#GEN_EVENT"] 
evgenConfig.extraSaveItems += ["HijingEventParams#*"] 
